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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   Generals
//
// Module:    Game Engine Common
//
// File name: ArchiveFileSystem.cpp
//
// Created:   11/26/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"
#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/AsciiString.h"
#include "Common/PerfTimer.h"


//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------

ArchiveFileSystem *TheArchiveFileSystem = nullptr;


//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//------------------------------------------------------
// ArchivedFileInfo
//------------------------------------------------------
ArchiveFileSystem::ArchiveFileSystem()
{
}

ArchiveFileSystem::~ArchiveFileSystem()
{
	ArchiveFileMap::iterator iter = m_archiveFileMap.begin();
	while (iter != m_archiveFileMap.end()) {
		ArchiveFile *file = iter->second;
		delete file;
		iter++;
	}
}

void ArchiveFileSystem::loadIntoDirectoryTree(ArchiveFile *archiveFile, Bool overwrite)
{

	FilenameList filenameList;

	archiveFile->getFileListInDirectory("", "", "*", filenameList, TRUE);

	FilenameListIter it = filenameList.begin();

	while (it != filenameList.end())
	{
		ArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

		AsciiString path;
		AsciiString token;
		AsciiString tokenizer = *it;
		tokenizer.toLower();
		tokenizer.nextToken(&token, "\\/");

		while (!tokenizer.isEmpty())
		{
			path.concat(token);
			path.concat('\\');

			ArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
			if (tempiter == dirInfo->m_directories.end())
			{
				dirInfo = &(dirInfo->m_directories[token]);
				dirInfo->m_path = path;
				dirInfo->m_directoryName = token;
			}
			else
			{
				dirInfo = &tempiter->second;
			}

			tokenizer.nextToken(&token, "\\/");
		}

		// GeneralsX @bugfix felipebraz 16/09/2026 Skip dummy/wildcard archive entries (e.g. Data\* in PatchZH.big)
		if (token.isEmpty() || token.find('*') != nullptr || token.find('?') != nullptr)
		{
			it++;
			continue;
		}

		ArchivedFileLocationMap::iterator fileIt;
		if (overwrite)
		{
			// When overwriting, try place the new value at the beginning of the key list.
			fileIt = dirInfo->m_files.find(token);
		}
		else
		{
			// Append to the end of the key list.
			fileIt = dirInfo->m_files.end();
		}

		dirInfo->m_files.insert(fileIt, std::make_pair(token, archiveFile));

#if defined(DEBUG_LOGGING) && ENABLE_FILESYSTEM_LOGGING
		{
			const stl::const_range<ArchivedFileLocationMap> range = stl::get_range(dirInfo->m_files, token, 0);
			if (range.distance() >= 2)
			{
				ArchivedFileLocationMap::const_iterator rangeIt0;
				ArchivedFileLocationMap::const_iterator rangeIt1;

				if (overwrite)
				{
					rangeIt0 = range.begin;
					rangeIt1 = std::next(rangeIt0);

					DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s, overwriting same file in %s",
						it->str(),
						rangeIt0->second->getName().str(),
						rangeIt1->second->getName().str()
					));
				}
				else
				{
					rangeIt1 = std::prev(range.end);
					rangeIt0 = std::prev(rangeIt1);

					DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s, overwritten by same file in %s",
						it->str(),
						rangeIt1->second->getName().str(),
						rangeIt0->second->getName().str()
					));
				}
			}
			else
			{
				DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s", it->str(), archiveFile->getName().str()));
			}
		}
#endif

		it++;
	}
}

void ArchiveFileSystem::loadMods()
{
	if (TheGlobalData->m_modBIG.isNotEmpty())
	{
		ArchiveFile *archiveFile = openArchiveFile(TheGlobalData->m_modBIG.str());

		if (archiveFile != nullptr) {
			DEBUG_LOG(("ArchiveFileSystem::loadMods - loading %s into the directory tree.", TheGlobalData->m_modBIG.str()));
			loadIntoDirectoryTree(archiveFile, TRUE);
			m_archiveFileMap[TheGlobalData->m_modBIG] = archiveFile;
			DEBUG_LOG(("ArchiveFileSystem::loadMods - %s inserted into the archive file map.", TheGlobalData->m_modBIG.str()));
		}
		else
		{
			DEBUG_LOG(("ArchiveFileSystem::loadMods - could not openArchiveFile(%s)", TheGlobalData->m_modBIG.str()));
		}
	}

	if (TheGlobalData->m_modDir.isNotEmpty())
	{
		MAYBE_UNUSED Bool ret = loadBigFilesFromDirectory(TheGlobalData->m_modDir, "*.big", TRUE);
		(void)ret;
		DEBUG_ASSERTLOG(ret, ("loadBigFilesFromDirectory(%s) returned FALSE!", TheGlobalData->m_modDir.str()));
	}
}

Bool ArchiveFileSystem::doesFileExist(const Char *filename, FileInstance instance) const
{
	return doesFileExist(filename, instance, ARCHIVE_FILTER_ALL);
}

Bool ArchiveFileSystem::doesFileExist(const Char *filename, FileInstance instance, ArchiveFilter filter) const
{
	if (filename == nullptr || filename[0] == '\0')
	{
		return false;
	}

	return getArchiveFile(AsciiString(filename), instance, filter) != nullptr;
}

ArchivedDirectoryInfo* ArchiveFileSystem::friend_getArchivedDirectoryInfo(const Char* directory)
{
	ArchivedDirectoryInfoResult result = getArchivedDirectoryInfo(directory);

	return result.dirInfo;
}

ArchiveFileSystem::ArchivedDirectoryInfoResult ArchiveFileSystem::getArchivedDirectoryInfo(const Char* directory)
{
	ArchivedDirectoryInfoResult result;
	ArchivedDirectoryInfo* dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = directory;
	tokenizer.toLower();
	tokenizer.nextToken(&token, "\\/");

	while (!tokenizer.isEmpty())
	{
		ArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
		if (tempiter != dirInfo->m_directories.end())
		{
			dirInfo = &tempiter->second;
			tokenizer.nextToken(&token, "\\/");
		}
		else
		{
			// the directory doesn't exist
			result.dirInfo = nullptr;
			result.lastToken = AsciiString::TheEmptyString;
			return result;
		}
	}

	result.dirInfo = dirInfo;
	result.lastToken = token;
	return result;
}

File * ArchiveFileSystem::openFile(const Char *filename, Int access, FileInstance instance)
{
	return openFile(filename, access, instance, ARCHIVE_FILTER_ALL);
}

File * ArchiveFileSystem::openFile(const Char *filename, Int access, FileInstance instance, ArchiveFilter filter)
{
	if (filename == nullptr || filename[0] == '\0')
	{
		return nullptr;
	}

	ArchiveFile* archive = getArchiveFile(AsciiString(filename), instance, filter);

	if (archive == nullptr)
		return nullptr;

	return archive->openFile(filename, access);
}

Bool ArchiveFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance) const
{
	return getFileInfo(filename, fileInfo, instance, ARCHIVE_FILTER_ALL);
}

Bool ArchiveFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance, ArchiveFilter filter) const
{
	if (fileInfo == nullptr || filename.isEmpty()) {
		return FALSE;
	}

	ArchiveFile* archive = getArchiveFile(filename, instance, filter);

	if (archive == nullptr)
		return FALSE;

	return archive->getFileInfo(filename, fileInfo);
}

ArchiveFile* ArchiveFileSystem::getArchiveFile(const AsciiString& filename, FileInstance instance, ArchiveFilter filter) const
{
	ArchivedDirectoryInfoResult result = const_cast<ArchiveFileSystem*>(this)->getArchivedDirectoryInfo(filename.str());

	if (!result.valid())
		return nullptr;

	std::pair<ArchivedFileLocationMap::const_iterator, ArchivedFileLocationMap::const_iterator> range =
		result.dirInfo->m_files.equal_range(result.lastToken);

	FileInstance currentInstance = 0;
	for (ArchivedFileLocationMap::const_iterator it = range.first; it != range.second; ++it)
	{
		ArchiveFile* archive = it->second;
		if (filter == ARCHIVE_FILTER_MOD_ONLY && !isModArchive(archive, filename.str()))
		{
			continue;
		}
		if (filter == ARCHIVE_FILTER_NON_MOD_ONLY && isModArchive(archive, filename.str()))
		{
			continue;
		}

		if (currentInstance == instance)
		{
			return archive;
		}
		++currentInstance;
	}

	return nullptr;
}

void ArchiveFileSystem::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	ArchiveFileMap::const_iterator it = m_archiveFileMap.begin();
	while (it != m_archiveFileMap.end()) {
		it->second->getFileListInDirectory(currentDirectory, originalDirectory, searchName, filenameList, searchSubdirectories);
		it++;
	}
}

// GeneralsX @bugfix felipebraz 02/10/2026 Allow mod archives to take precedence over loose stock files.
// Retail Zero Hour shipped stock loose files (e.g. Data/Scripts/SkirmishScripts.scb) that shadow mod archives
// (such as Shockwave's !Shw_scripts.big) unless the launcher explicitly renamed them on disk.
Bool ArchiveFileSystem::isModArchive(ArchiveFile *archive, const Char *filename) const
{
	if (archive == nullptr)
	{
		return FALSE;
	}

	// 1. Script files: SkirmishScripts.scb, MultiplayerScripts.scb, Scripts.ini.
	// Vanilla Zero Hour never packaged these into any .big archive; they only shipped as loose files.
	// If any archive in the archive file system contains them, it is guaranteed to be a mod.
	if (filename != nullptr)
	{
		const char* fnRaw = filename;
		const char* fnLastSlash = strrchr(fnRaw, '/');
		const char* fnLastBackslash = strrchr(fnRaw, '\\');
		const char* fnSplit = fnLastSlash;
		if (fnSplit == nullptr || (fnLastBackslash != nullptr && fnLastBackslash > fnSplit))
		{
			fnSplit = fnLastBackslash;
		}
		const char* fnBase = (fnSplit != nullptr) ? (fnSplit + 1) : fnRaw;
		if (stricmp(fnBase, "SkirmishScripts.scb") == 0 ||
		    stricmp(fnBase, "MultiplayerScripts.scb") == 0 ||
		    stricmp(fnBase, "Scripts.ini") == 0)
		{
			return TRUE;
		}
	}

	// 2. Mod archives starting with '!' (or '!!', '@', etc.) - the universal SAGE mod convention.
	AsciiString archiveName = archive->getName();
	const char* raw = archiveName.str();
	const char* lastSlash = strrchr(raw, '/');
	const char* lastBackslash = strrchr(raw, '\\');
	const char* split = lastSlash;
	if (split == nullptr || (lastBackslash != nullptr && lastBackslash > split))
	{
		split = lastBackslash;
	}
	const char* baseName = (split != nullptr) ? (split + 1) : raw;
	if (baseName[0] == '!' || baseName[0] == '@')
	{
		return TRUE;
	}

	// 3. Archives loaded explicitly via -mod command line parameter (directory or direct .big).
	if (TheGlobalData)
	{
		if (TheGlobalData->m_modDir.isNotEmpty())
		{
			const char* match = strstr(archiveName.str(), TheGlobalData->m_modDir.str());
			if (match != nullptr)
			{
				char nextChar = match[TheGlobalData->m_modDir.getLength()];
				if (nextChar == '/' || nextChar == '\\' || nextChar == '\0')
				{
					return TRUE;
				}
			}
		}
		if (TheGlobalData->m_modBIG.isNotEmpty())
		{
			if (archiveName.compareNoCase(TheGlobalData->m_modBIG) == 0)
			{
				return TRUE;
			}
			const char* modBigRaw = TheGlobalData->m_modBIG.str();
			const char* modBigSlash = strrchr(modBigRaw, '/');
			const char* modBigBackslash = strrchr(modBigRaw, '\\');
			const char* modBigSplit = modBigSlash;
			if (modBigSplit == nullptr || (modBigBackslash != nullptr && modBigBackslash > modBigSplit))
			{
				modBigSplit = modBigBackslash;
			}
			const char* modBigBase = (modBigSplit != nullptr) ? (modBigSplit + 1) : modBigRaw;
			if (stricmp(baseName, modBigBase) == 0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

Bool ArchiveFileSystem::hasModArchiveOverride(const Char *filename) const
{
	return getFileCount(filename, ARCHIVE_FILTER_MOD_ONLY) > 0;
}

FileInstance ArchiveFileSystem::getFileCount(const Char *filename, ArchiveFilter filter) const
{
	if (filename == nullptr || filename[0] == '\0')
	{
		return 0;
	}

	ArchivedDirectoryInfoResult result = const_cast<ArchiveFileSystem*>(this)->getArchivedDirectoryInfo(filename);
	if (!result.valid())
	{
		return 0;
	}

	if (filter == ARCHIVE_FILTER_ALL)
	{
		return static_cast<FileInstance>(result.dirInfo->m_files.count(result.lastToken));
	}

	std::pair<ArchivedFileLocationMap::const_iterator, ArchivedFileLocationMap::const_iterator> range =
		result.dirInfo->m_files.equal_range(result.lastToken);

	FileInstance count = 0;
	for (ArchivedFileLocationMap::const_iterator it = range.first; it != range.second; ++it)
	{
		ArchiveFile* archive = it->second;
		if (filter == ARCHIVE_FILTER_MOD_ONLY && !isModArchive(archive, filename))
		{
			continue;
		}
		if (filter == ARCHIVE_FILTER_NON_MOD_ONLY && isModArchive(archive, filename))
		{
			continue;
		}
		++count;
	}

	return count;
}



