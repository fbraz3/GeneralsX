/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

/////////ArchiveFile.cpp ///////////////////////
// Bryan Cleveland, August 2002
////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/file.h"
#include "Common/PerfTimer.h"
#include "Utility/compat.h"

// checks to see if str matches searchString.  Search string is done in the
// using * and ? as wildcards. * is used to denote any number of characters,
// and ? is used to denote a single wildcard character.
static Bool SearchStringMatches(AsciiString str, AsciiString searchString)
{
	if (str.getLength() == 0) {
		if (searchString.getLength() == 0) {
			return TRUE;
		}
		return FALSE;
	}
	if (searchString.getLength() == 0) {
		return FALSE;
	}

	const char* c1 = str.str();
	const char* c2 = searchString.str();

	while ((*c1 == *c2) || (*c2 == '?') || (*c2 == '*')) {
		if ((*c1 == *c2) || (*c2 == '?')) {
			++c1;
			++c2;
		}
		else if (*c2 == '*') {
			++c2;
			if (*c2 == 0) {
				return TRUE;
			}
			while (*c1 != 0) {
				if (SearchStringMatches(AsciiString(c1), AsciiString(c2))) {
					return TRUE;
				}
				++c1;
			}
		}
		if (*c1 == 0) {
			if (*c2 == 0) {
				return TRUE;
			}
			return FALSE;
		}
		if (*c2 == 0) {
			return FALSE;
		}
	}
	return FALSE;
}

ArchiveFile::~ArchiveFile()
{
	if (m_file != NULL) {
		m_file->close();
		m_file = NULL;
	}
}

ArchiveFile::ArchiveFile()
	: m_file(NULL)
{
}

void ArchiveFile::addFile(const AsciiString& path, const ArchivedFileInfo* fileInfo)
{
	// fprintf(stderr, "[ArchiveFile::addFile] Adding file: '%s' (filename: '%s', offset: %u, size: %u)\n",
	// 	path.str(), fileInfo->m_filename.str(), fileInfo->m_offset, fileInfo->m_size);
	// fflush(stderr);

	DetailedArchivedDirectoryInfo* dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = path;
	tokenizer.toLower();
	// Use backslash since .big files store Windows-style paths
	tokenizer.nextToken(&token, "\\");

	while (token.getLength() > 0)
	{
		// fprintf(stderr, "[ArchiveFile::addFile] Creating/finding directory: '%s'\n", token.str());
		// fflush(stderr);

		DetailedArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
		if (tempiter == dirInfo->m_directories.end())
		{
			dirInfo = &(dirInfo->m_directories[token]);
			dirInfo->m_directoryName = token;
		}
		else
		{
			dirInfo = &tempiter->second;
		}

		tokenizer.nextToken(&token, "\\");
	}

	// fprintf(stderr, "[ArchiveFile::addFile] Storing file '%s' in final directory\n", fileInfo->m_filename.str());
	// fflush(stderr);

	// Store with lowercase filename for case-insensitive lookup
	AsciiString lowercaseFilename = fileInfo->m_filename;
	lowercaseFilename.toLower();
	dirInfo->m_files[lowercaseFilename] = *fileInfo;
}

void ArchiveFile::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList& filenameList, Bool searchSubdirectories) const
{
	const DetailedArchivedDirectoryInfo* dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = originalDirectory;
	tokenizer.toLower();

	// Normalize forward slashes to backslashes (archive uses Windows-style paths)
	for (char* p = const_cast<char*>(tokenizer.str()); *p; ++p) {
		if (*p == '/') *p = '\\';
	}

	// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] originalDir='%s', search='%s'\n",
	// originalDirectory.str(), searchName.str());
	// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] Tokenizer (normalized): '%s'\n",
	// tokenizer.str());
	// fflush(stderr);

	// Use backslash since .big files store Windows-style paths
	tokenizer.nextToken(&token, "\\");

	while (token.getLength() > 0) {
		// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] Looking for dir token: '%s'\n", token.str());
		// fflush(stderr);

		DetailedArchivedDirectoryInfoMap::const_iterator it = dirInfo->m_directories.find(token);
		if (it != dirInfo->m_directories.end())
		{
			dirInfo = &it->second;
			// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] Found directory: '%s'\n", token.str());
			// fflush(stderr);
		}
		else
		{
			// if the directory doesn't exist, then there aren't any files to be had.
			// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] Directory NOT FOUND: '%s'\n", token.str());
			// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] Available directories at this level (%zu):\n",
			// dirInfo->m_directories.size());
			// fflush(stderr);
			int count = 0;
			for (DetailedArchivedDirectoryInfoMap::const_iterator dit = dirInfo->m_directories.begin();
				dit != dirInfo->m_directories.end() && count < 30; ++dit, ++count)
			{
				// fprintf(stderr, "  [%d] '%s'\n", count, dit->first.str());
			}
			if (dirInfo->m_directories.size() > 30) {
				// fprintf(stderr, "  ... and %zu more\n", dirInfo->m_directories.size() - 30);
			}
			// fflush(stderr);
			return;
		}

		tokenizer.nextToken(&token, "\\");
	}

	// fprintf(stderr, "[ArchiveFile::getFileListInDirectory] Directory found, searching for files matching '%s'\n", searchName.str());
	// fflush(stderr);
	getFileListInDirectory(dirInfo, originalDirectory, searchName, filenameList, searchSubdirectories);
}

void ArchiveFile::getFileListInDirectory(const DetailedArchivedDirectoryInfo* dirInfo, const AsciiString& currentDirectory, const AsciiString& searchName, FilenameList& filenameList, Bool searchSubdirectories) const
{
	DetailedArchivedDirectoryInfoMap::const_iterator diriter = dirInfo->m_directories.begin();
	while (diriter != dirInfo->m_directories.end()) {
		const DetailedArchivedDirectoryInfo* tempDirInfo = &(diriter->second);
		AsciiString tempdirname;
		tempdirname = currentDirectory;
		// Use GET_BIG_FILE_SEPARATOR for archive paths - they always use Windows backslash
		if ((tempdirname.getLength() > 0) && (!tempdirname.endsWith((char*)GET_BIG_FILE_SEPARATOR()))) {
			tempdirname.concat(GET_BIG_FILE_SEPARATOR());
		}
		tempdirname.concat(tempDirInfo->m_directoryName);
		getFileListInDirectory(tempDirInfo, tempdirname, searchName, filenameList, searchSubdirectories);
		diriter++;
	}

	ArchivedFileInfoMap::const_iterator fileiter = dirInfo->m_files.begin();
	while (fileiter != dirInfo->m_files.end()) {
		if (SearchStringMatches(fileiter->second.m_filename, searchName)) {
			AsciiString tempfilename;
			tempfilename = currentDirectory;
			// Use GET_BIG_FILE_SEPARATOR for archive paths - they always use Windows backslash
			if ((tempfilename.getLength() > 0) && (!tempfilename.endsWith((char*)GET_BIG_FILE_SEPARATOR()))) {
				tempfilename.concat(GET_BIG_FILE_SEPARATOR());
			}
			tempfilename.concat(fileiter->second.m_filename);
			if (filenameList.find(tempfilename) == filenameList.end()) {
				// only insert into the list if its not already in there.
				filenameList.insert(tempfilename);
			}
		}
		fileiter++;
	}
}

void ArchiveFile::attachFile(File* file)
{
	if (m_file != NULL) {
		m_file->close();
		m_file = NULL;
	}
	m_file = file;
}

const ArchivedFileInfo* ArchiveFile::getArchivedFileInfo(const AsciiString& filename) const
{
	const DetailedArchivedDirectoryInfo* dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = filename;
	tokenizer.toLower();

	// Normalize forward slashes to backslashes since .big files use Windows-style paths
	// Need to create a new string to properly modify it
	char normalizedPath[512];
	const char* src = tokenizer.str();
	int i;
	for (i = 0; i < 511 && src[i] != '\0'; i++) {
		normalizedPath[i] = (src[i] == '/') ? GET_BIG_FILE_SEPARATOR()[0] : src[i];
	}
	normalizedPath[i] = '\0';  // Null-terminate!
	tokenizer = AsciiString(normalizedPath);

	// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Looking for: '%s', normalized: '%s'\n", filename.str(), tokenizer.str());
	// fflush(stderr);

	// Use backslash separator since .big files store Windows-style paths
	tokenizer.nextToken(&token, GET_BIG_FILE_SEPARATOR());

	while (!token.find('.') || tokenizer.find('.'))
	{
		// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Searching for directory token: '%s'\n", token.str());
		// fflush(stderr);

		DetailedArchivedDirectoryInfoMap::const_iterator it = dirInfo->m_directories.find(token);
		if (it != dirInfo->m_directories.end())
		{
			// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Found directory: '%s'\n", token.str());
			// fflush(stderr);
			dirInfo = &it->second;
		}
		else
		{
			// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Directory NOT FOUND: '%s'\n", token.str());
			// fflush(stderr);
			return NULL;
		}

		// Use backslash separator since .big files store Windows-style paths
		tokenizer.nextToken(&token, GET_BIG_FILE_SEPARATOR());
	}

	// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Final token (filename): '%s'\n", token.str());
	// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Files in directory (%zu total):\n", dirInfo->m_files.size());
	// int debugCount = 0;
	// for (ArchivedFileInfoMap::const_iterator debugIt = dirInfo->m_files.begin();
	// 	debugIt != dirInfo->m_files.end() && debugCount < 10; ++debugIt, ++debugCount) {
	// 	// fprintf(stderr, "  [%d] key='%s' (hex: ", debugCount, debugIt->first.str());
	// 	const char* s = debugIt->first.str();
	// 	for (int i = 0; s[i] && i < 20; i++) {
	// 		// fprintf(stderr, "%02x ", (unsigned char)s[i]);
	// 	}
	// 	fprintf(stderr, ")\n");
	// }
	// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] Searching for token (hex): ");
	// const char* ts = token.str();
	// for (int i = 0; ts[i] && i < 20; i++) {
	// 	fprintf(stderr, "%02x ", (unsigned char)ts[i]);
	// }
	// fprintf(stderr, "\n");
	// fflush(stderr);

	ArchivedFileInfoMap::const_iterator it = dirInfo->m_files.find(token);
	if (it != dirInfo->m_files.end())
	{
		// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] SUCCESS - found file\n");
		// fflush(stderr);
		return &it->second;
	}
	else
	{
		// fprintf(stderr, "[ArchiveFile::getArchivedFileInfo] File NOT FOUND in map\n");
		// fflush(stderr);
		return NULL;
	}

}
