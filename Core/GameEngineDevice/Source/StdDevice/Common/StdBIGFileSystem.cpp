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

//////// StdBIGFileSystem.h ///////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////////////////

#include "Common/AudioAffect.h"
#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/file.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/LocalFileSystem.h"

#if RTS_ZEROHOUR
#include "Common/Registry.h"
#endif

#include "StdDevice/Common/StdBIGFile.h"
#include "StdDevice/Common/StdBIGFileSystem.h"
#include "Utility/endian_compat.h"

static const char *BIGFileIdentifier = "BIGF";

StdBIGFileSystem::StdBIGFileSystem() : ArchiveFileSystem() {
}

StdBIGFileSystem::~StdBIGFileSystem() {
}

void StdBIGFileSystem::init() {
	DEBUG_ASSERTCRASH(TheLocalFileSystem != NULL, ("TheLocalFileSystem must be initialized before TheArchiveFileSystem."));
	if (TheLocalFileSystem == NULL) {
		return;
	}

	// CRASH FIX (Nov 4, 2025): On macOS, Metal/Vulkan initialization may still have background threads
	// running. Add a small delay to allow GPU driver threads to finish before file I/O starts.
	// This prevents AttributeGraph crashes that occur when file I/O races with GPU framework initialization.
	#ifdef __APPLE__
	usleep(100000);  // 100ms delay to allow Metal background threads to settle
	#endif

	// Load .big files from current directory
	// NOTE: This can be called during system initialization and may race with graphics backend initialization
	DEBUG_LOG(("StdBIGFileSystem::init() - Starting .big file loading from current directory"));
	
	loadBigFilesFromDirectory("", "*.big");
	DEBUG_LOG(("StdBIGFileSystem::init() - Completed .big file loading from current directory"));

#if RTS_ZEROHOUR
    // load original Generals assets
    AsciiString installPath;
    GetStringFromGeneralsRegistry("", "InstallPath", installPath );
    //@todo this will need to be ramped up to a crash for release
    DEBUG_ASSERTCRASH(installPath != "", ("Be 1337! Go install Generals!"));
    if (installPath!="") {
      DEBUG_LOG(("StdBIGFileSystem::init() - Loading additional .big files from install path: %s", installPath.str()));
      loadBigFilesFromDirectory(installPath, "*.big");
      DEBUG_LOG(("StdBIGFileSystem::init() - Completed loading .big files from install path"));
    }
#endif

	DEBUG_LOG(("StdBIGFileSystem::init() - COMPLETED"));
}

void StdBIGFileSystem::reset() {
}

void StdBIGFileSystem::update() {
}

void StdBIGFileSystem::postProcessLoad() {
}

ArchiveFile * StdBIGFileSystem::openArchiveFile(const Char *filename) {
	// CRASH FIX (Nov 4, 2025): Add safety checks to prevent AttributeGraph corruption
	// Thread 0 was stuck in fread() while Thread 1 crashed in AttributeGraph memory allocation
	// This suggests race condition during Vulkan+file I/O initialization
	
	if (filename == NULL) {
		DEBUG_CRASH(("StdBIGFileSystem::openArchiveFile - NULL filename pointer"));
		return NULL;
	}

	File *fp = TheLocalFileSystem->openFile(filename, File::READ | File::BINARY);
	AsciiString archiveFileName;
	archiveFileName = filename;
	archiveFileName.toLower();
	Int archiveFileSize = 0;
	Int numLittleFiles = 0;

	ArchiveFile *archiveFile = NEW StdBIGFile(filename, AsciiString::TheEmptyString);

	DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - opening BIG file %s", filename));

	if (fp == NULL) {
		DEBUG_CRASH(("Could not open archive file %s for parsing", filename));
		return NULL;
	}

	AsciiString asciibuf;
	char buffer[_MAX_PATH];
	
	// SAFETY: Catch file read errors that might corrupt memory
	if (!fp->read(buffer, 4)) {
		DEBUG_CRASH(("Could not read BIG identifier from %s", filename));
		fp->close();
		fp = NULL;
		return NULL;
	}
	
	buffer[4] = 0;
	if (strcmp(buffer, BIGFileIdentifier) != 0) {
		DEBUG_CRASH(("Error reading BIG file identifier in file %s", filename));
		fp->close();
		fp = NULL;
		return NULL;
	}

	// read in the file size.
	if (!fp->read(&archiveFileSize, 4)) {
		DEBUG_CRASH(("Could not read file size from %s", filename));
		fp->close();
		fp = NULL;
		return NULL;
	}

	DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - size of archive file is %d bytes", archiveFileSize));
//	char t;

	// read in the number of files contained in this BIG file.
	// change the order of the bytes cause the file size is in reverse byte order for some reason.
	if (!fp->read(&numLittleFiles, 4)) {
		DEBUG_CRASH(("Could not read numLittleFiles from %s", filename));
		fp->close();
		fp = NULL;
		delete archiveFile;
		return NULL;
	}
	
	numLittleFiles = betoh(numLittleFiles);
	
	// SAFETY: Sanity check for corrupted .big files or memory corruption
	if (numLittleFiles < 0 || numLittleFiles > 100000) {
		DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - WARNING: Suspiciously large file count %d in %s", numLittleFiles, filename));
		// Continue anyway, might be legitimate large archive
	}

	DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - %d files are contained in archive", numLittleFiles));
//	for (Int i = 0; i < 2; ++i) {
//		t = buffer[i];
//		buffer[i] = buffer[(4-i)-1];
//		buffer[(4-i)-1] = t;
//	}

	// seek to the beginning of the directory listing.
	fp->seek(0x10, File::START);
	// read in each directory listing.
	ArchivedFileInfo *fileInfo = NEW ArchivedFileInfo;

	for (Int i = 0; i < numLittleFiles; ++i) {
		Int filesize = 0;
		Int fileOffset = 0;
		
		// SAFETY: Read with bounds checking
		if (!fp->read(&fileOffset, 4) || !fp->read(&filesize, 4)) {
			DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - Failed to read file entry %d from %s", i, filename));
			break;  // Exit loop early if file is truncated
		}

		filesize = betoh(filesize);
		fileOffset = betoh(fileOffset);
		
		// SAFETY: Sanity checks for file entries
		if (filesize < 0 || filesize > 1000000000) {  // Warn if > 1GB
			DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - WARNING: Suspiciously large file size %d for entry %d", filesize, i));
		}
		if (fileOffset < 0) {  // Negative offsets don't make sense
			DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - WARNING: Negative offset for entry %d", i));
		}

		fileInfo->m_archiveFilename = archiveFileName;
		fileInfo->m_offset = fileOffset;
		fileInfo->m_size = filesize;

		// read in the path name of the file.
		Int pathIndex = -1;
		do {
			++pathIndex;
			if (pathIndex >= _MAX_PATH - 1) {
				DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - Path name too long in entry %d", i));
				break;
			}
			if (!fp->read(buffer + pathIndex, 1)) {
				DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - Failed to read path character %d", pathIndex));
				break;
			}
		} while (buffer[pathIndex] != 0);

		Int filenameIndex = pathIndex;
		while ((filenameIndex >= 0) && (buffer[filenameIndex] != '\\') && (buffer[filenameIndex] != '/')) {
			--filenameIndex;
		}

		fileInfo->m_filename = (char *)(buffer + filenameIndex + 1);
		fileInfo->m_filename.toLower();
		buffer[filenameIndex + 1] = 0;

		AsciiString path;
		path = buffer;

		AsciiString debugpath;
		debugpath = path;
		debugpath.concat(fileInfo->m_filename);
//		DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - adding file %s to archive file %s, file number %d", debugpath.str(), fileInfo->m_archiveFilename.str(), i));

		archiveFile->addFile(path, fileInfo);
	}

	archiveFile->attachFile(fp);

	delete fileInfo;
	fileInfo = NULL;

	// leave fp open as the archive file will be using it.

	return archiveFile;
}

void StdBIGFileSystem::closeArchiveFile(const Char *filename) {
	// Need to close the specified big file
	ArchiveFileMap::iterator it =  m_archiveFileMap.find(filename);
	if (it == m_archiveFileMap.end()) {
		return;
	}

	if (stricmp(filename, MUSIC_BIG) == 0) {
		// Stop the current audio
		TheAudio->stopAudio(AudioAffect_Music);

		// No need to turn off other audio, as the lookups will just fail.
	}
	DEBUG_ASSERTCRASH(stricmp(filename, MUSIC_BIG) == 0, ("Attempting to close Archive file '%s', need to add code to handle its shutdown correctly.", filename));

	// may need to do some other processing here first.

	delete (it->second);
	m_archiveFileMap.erase(it);
}

void StdBIGFileSystem::closeAllArchiveFiles() {
}

void StdBIGFileSystem::closeAllFiles() {
}

Bool StdBIGFileSystem::loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite) {

	FilenameList filenameList;
	TheLocalFileSystem->getFileListInDirectory(dir, AsciiString(""), fileMask, filenameList, TRUE);

	Bool actuallyAdded = FALSE;
	FilenameListIter it = filenameList.begin();
	while (it != filenameList.end()) {
		ArchiveFile *archiveFile = openArchiveFile((*it).str());

		if (archiveFile != NULL) {
			DEBUG_LOG(("StdBIGFileSystem::loadBigFilesFromDirectory - loading %s into the directory tree.", (*it).str()));
			loadIntoDirectoryTree(archiveFile, overwrite);
			m_archiveFileMap[(*it)] = archiveFile;
			DEBUG_LOG(("StdBIGFileSystem::loadBigFilesFromDirectory - %s inserted into the archive file map.", (*it).str()));
			actuallyAdded = TRUE;
		}

		it++;
	}

	return actuallyAdded;
}
