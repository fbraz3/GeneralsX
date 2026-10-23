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
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO
//
// File name: IO_FileSystem.cpp
//
// Created:   4/23/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"
#include "Common/file.h"
#include "Common/FileSystem.h"

#include "Common/ArchiveFileSystem.h"
#include "Common/GameAudio.h"
#include "Common/LocalFileSystem.h"
#include "Common/PerfTimer.h"

#include "Lib/PathUtil.h"


DECLARE_PERF_TIMER(FileSystem)

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

//===============================
// TheFileSystem
//===============================
/**
  *	This is the FileSystem's singleton class. All file access
	* should be through TheFileSystem, unless code needs to use an explicit
	* File or FileSystem derivative.
	*
	* Using TheFileSystem->open and File exclusively for file access, particularly
	* in library or modular code, allows applications to transparently implement
	* file access as they see fit. This is particularly important for code that
	* needs to be shared between applications, such as games and tools.
	*/
//===============================

FileSystem	*TheFileSystem = nullptr;

//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------

// The character as it is written in a path key.
// GeneralsX @tweak cheviiot 24/10/2026 Port path-key helpers from upstream #3531 (keep mod-override open/exist)
// Upstream reference: xezon, PR #3531
// https://github.com/TheSuperHackers/GeneralsGameCode/pull/3531
static UnsignedByte toPathKeyChar(Char ch)
{
	if (isPathSeparator(ch))
		return '\\';

	if (ch >= 'A' && ch <= 'Z')
		return (UnsignedByte)(ch + ('a' - 'A'));

	return (UnsignedByte)ch;
}

// Returns the end of the separator pair that starts a UNC or device path, such as \\server\share.
// Windows keeps this pair apart from the single separator of a rooted path. Other platforms read
// two leading separators as one.
static const Char* getPathKeyPairEnd(const Char* path)
{
#ifdef _WIN32
	if (isPathSeparator(path[0]) && isPathSeparator(path[1]))
		return path + 2;
#endif
	return path;
}

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
// Returns the key under which a file is held in the file exist cache.
// A path that is too long for a path key is held under its own spelling.
static const Char* getFileExistKey(Char* keyBuffer, size_t keyBufferSize, const Char* filename)
{
	return FileSystem::makePathKey(keyBuffer, keyBufferSize, filename) ? keyBuffer : filename;
}
#endif

//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------


//============================================================================
// FileSystem::FileSystem
//============================================================================

FileSystem::FileSystem()
{

}

//============================================================================
// FileSystem::~FileSystem
//============================================================================

FileSystem::~FileSystem()
{

}

//============================================================================
// FileSystem::init
//============================================================================

void		FileSystem::init()
{
	TheLocalFileSystem->init();
	TheArchiveFileSystem->init();
}

//============================================================================
// FileSystem::update
//============================================================================

void		FileSystem::update()
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->update();
	TheArchiveFileSystem->update();
}

//============================================================================
// FileSystem::reset
//============================================================================

void		FileSystem::reset()
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->reset();
	TheArchiveFileSystem->reset();
}

//============================================================================
// FileSystem::open
//============================================================================

File*		FileSystem::openFile( const Char *filename, Int access, size_t bufferSize, FileInstance instance )
{
	USE_PERF_TIMER(FileSystem)
	File *file = nullptr;

	// GeneralsX @bugfix felipebraz 02/10/2026 Allow mod archives (e.g. !*.big, custom SkirmishScripts)
	// to override stock loose files that otherwise shadow mod contents.
	const Bool hasModOverride = (TheArchiveFileSystem != nullptr) &&
	                            !(access & File::WRITE) &&
	                            TheArchiveFileSystem->hasModArchiveOverride(filename);

	FileInstance remainingInstance = instance;

	if (hasModOverride)
	{
		FileInstance modArchiveCount = TheArchiveFileSystem->getFileCount(filename, ArchiveFileSystem::ARCHIVE_FILTER_MOD_ONLY);
		if (remainingInstance < modArchiveCount)
		{
			return TheArchiveFileSystem->openFile( filename, access, remainingInstance, ArchiveFileSystem::ARCHIVE_FILTER_MOD_ONLY );
		}
		remainingInstance -= modArchiveCount;
	}

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
	Char keyBuffer[_MAX_PATH];
	const Char* key = getFileExistKey(keyBuffer, ARRAY_SIZE(keyBuffer), filename);
#endif

	if ( TheLocalFileSystem != nullptr )
	{
		if (remainingInstance != 0)
		{
			if (TheLocalFileSystem->doesFileExist(filename))
			{
				--remainingInstance;
			}
		}
		else
		{
			file = TheLocalFileSystem->openFile( filename, access, bufferSize );

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
			if (file != nullptr && (file->getAccess() & File::CREATE))
			{
				FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
				FileExistMap::iterator it = m_fileExist.find(FileExistMap::key_type::temporary(key));
				if (it != m_fileExist.end())
				{
					++it->second.instanceExists;
					if (it->second.instanceDoesNotExist != ~FileInstance(0))
						++it->second.instanceDoesNotExist;
				}
				else
				{
					m_fileExist[key];
				}
			}
#endif
		}
	}

	if ( (TheArchiveFileSystem != nullptr) && (file == nullptr) )
	{
		// Non-mod (ordinary) archives fallback if mod overrides were active, or all archives if not.
		ArchiveFileSystem::ArchiveFilter filter = hasModOverride ? ArchiveFileSystem::ARCHIVE_FILTER_NON_MOD_ONLY : ArchiveFileSystem::ARCHIVE_FILTER_ALL;
		file = TheArchiveFileSystem->openFile( filename, 0, remainingInstance, filter );
	}

	return file;
}

//============================================================================
// FileSystem::doesFileExist
//============================================================================

Bool FileSystem::doesFileExist(const Char *filename, FileInstance instance) const
{
	USE_PERF_TIMER(FileSystem)

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
	Char keyBuffer[_MAX_PATH];
	const Char* key = getFileExistKey(keyBuffer, ARRAY_SIZE(keyBuffer), filename);

	{
		FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
		FileExistMap::const_iterator it = m_fileExist.find(FileExistMap::key_type::temporary(key));
		if (it != m_fileExist.end())
		{
			// Must test instanceDoesNotExist first!
			if (instance >= it->second.instanceDoesNotExist)
				return FALSE;
			if (instance <= it->second.instanceExists)
				return TRUE;
		}
	}
#endif

	const FileInstance requestedInstance = instance;
	FileInstance remainingInstance = instance;

	// GeneralsX @bugfix felipebraz 02/10/2026 Prioritize mod archive existence if an override is active.
	const Bool hasModOverride = (TheArchiveFileSystem != nullptr) &&
	                            TheArchiveFileSystem->hasModArchiveOverride(filename);

	if (hasModOverride)
	{
		FileInstance modArchiveCount = TheArchiveFileSystem->getFileCount(filename, ArchiveFileSystem::ARCHIVE_FILTER_MOD_ONLY);
		if (remainingInstance < modArchiveCount)
		{
#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
			{
				FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
				FileExistMap::mapped_type& value = m_fileExist[key];
				value.instanceExists = max(value.instanceExists, requestedInstance);
			}
#endif
			return TRUE;
		}
		remainingInstance -= modArchiveCount;
	}

	if (TheLocalFileSystem->doesFileExist(filename))
	{
		if (remainingInstance == 0)
		{
#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
			{
				FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
				FileExistMap::mapped_type& value = m_fileExist[key];
				value.instanceExists = max(value.instanceExists, requestedInstance);
			}
#endif
			return TRUE;
		}

		--remainingInstance;
	}

	ArchiveFileSystem::ArchiveFilter filter = hasModOverride ? ArchiveFileSystem::ARCHIVE_FILTER_NON_MOD_ONLY : ArchiveFileSystem::ARCHIVE_FILTER_ALL;
	if (TheArchiveFileSystem->doesFileExist(filename, remainingInstance, filter))
	{
#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
		{
			FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
			FileExistMap::mapped_type& value = m_fileExist[key];
			value.instanceExists = max(value.instanceExists, requestedInstance);
		}
#endif
		return TRUE;
	}

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
	{
		FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
		FileExistMap::mapped_type& value = m_fileExist[key];
		value.instanceDoesNotExist = min(value.instanceDoesNotExist, requestedInstance);
	}
#endif
	return FALSE;
}

//============================================================================
// FileSystem::getFileListInDirectory
//============================================================================
void FileSystem::getFileListInDirectory(const AsciiString& directory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->getFileListInDirectory(AsciiString::TheEmptyString, directory, searchName, filenameList, searchSubdirectories);
	TheArchiveFileSystem->getFileListInDirectory(AsciiString::TheEmptyString, directory, searchName, filenameList, searchSubdirectories);
}

//============================================================================
// FileSystem::getFileInfo
//============================================================================
Bool FileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance) const
{
	USE_PERF_TIMER(FileSystem)

	// TheSuperHackers @todo Add file info cache?

	if (fileInfo == nullptr) {
		return FALSE;
	}
	memset(fileInfo, 0, sizeof(*fileInfo));

	FileInstance remainingInstance = instance;

	// GeneralsX @bugfix felipebraz 02/10/2026 Prioritize mod archive metadata if an override is active.
	const Bool hasModOverride = (TheArchiveFileSystem != nullptr) &&
	                            TheArchiveFileSystem->hasModArchiveOverride(filename.str());

	if (hasModOverride)
	{
		FileInstance modArchiveCount = TheArchiveFileSystem->getFileCount(filename.str(), ArchiveFileSystem::ARCHIVE_FILTER_MOD_ONLY);
		if (remainingInstance < modArchiveCount)
		{
			return TheArchiveFileSystem->getFileInfo(filename, fileInfo, remainingInstance, ArchiveFileSystem::ARCHIVE_FILTER_MOD_ONLY);
		}
		remainingInstance -= modArchiveCount;
	}

	if (TheLocalFileSystem->getFileInfo(filename, fileInfo)) {
		if (remainingInstance == 0) {
			return TRUE;
		}

		--remainingInstance;
	}

	ArchiveFileSystem::ArchiveFilter filter = hasModOverride ? ArchiveFileSystem::ARCHIVE_FILTER_NON_MOD_ONLY : ArchiveFileSystem::ARCHIVE_FILTER_ALL;
	if (TheArchiveFileSystem->getFileInfo(filename, fileInfo, remainingInstance, filter)) {
		return TRUE;
	}

	return FALSE;
}

//============================================================================
// FileSystem::createDirectory
//============================================================================
Bool FileSystem::createDirectory(AsciiString directory)
{
	USE_PERF_TIMER(FileSystem)
	if (TheLocalFileSystem != nullptr) {
		return TheLocalFileSystem->createDirectory(directory);
	}
	return FALSE;
}

//============================================================================
// FileSystem::normalizePath
//============================================================================
AsciiString FileSystem::normalizePath(const AsciiString& path)
{
	return TheLocalFileSystem->normalizePath(path);
}

//============================================================================
// FileSystem::isPathInDirectory
//============================================================================
Bool FileSystem::isPathInDirectory(const AsciiString& testPath, const AsciiString& basePath)
{
	AsciiString testPathNormalized = TheFileSystem->normalizePath(testPath);
	AsciiString basePathNormalized = TheFileSystem->normalizePath(basePath);

	if (basePathNormalized.isEmpty())
	{
		DEBUG_CRASH(("Unable to normalize base directory path '%s'.", basePath.str()));
		return false;
	}
	else if (testPathNormalized.isEmpty())
	{
		DEBUG_CRASH(("Unable to normalize file path '%s'.", testPath.str()));
		return false;
	}

#ifdef _WIN32
	const char* pathSep = "\\";
#else
	const char* pathSep = "/";
#endif

	if (!basePathNormalized.endsWith(pathSep))
	{
		basePathNormalized.concat(pathSep);
	}

#ifdef _WIN32
	if (!testPathNormalized.startsWithNoCase(basePathNormalized))
#else
	if (!testPathNormalized.startsWith(basePathNormalized))
#endif
	{
		return false;
	}

	return true;
}

//============================================================================
// FileSystem::removeExtension - Ascii handling variant
//============================================================================
bool FileSystem::removeExtension(AsciiString& path)
{
	if (const Char* ext = getExtension(path.str()))
	{
		path.truncateTo(ext - path.str());
		return true;
	}

	return false;
}

//============================================================================
// FileSystem::removeExtension - Unicode handling variant
//============================================================================
bool FileSystem::removeExtension(UnicodeString& path)
{
	if (const WideChar* ext = getExtension(path.str()))
	{
		path.truncateTo(ext - path.str());
		return true;
	}

	return false;
}

//============================================================================
// FileSystem::appendPathSeparator
//============================================================================
void FileSystem::appendPathSeparator(AsciiString& path)
{
	if (path.isNotEmpty() && !isPathSeparator(path.back()))
	{
		path.concat(getNativePathSeparator());
	}
}

//============================================================================
// FileSystem::makePathKey
//============================================================================
Bool FileSystem::makePathKey(Char* key, size_t keySize, const Char* path)
{
	DEBUG_ASSERTCRASH(keySize > 0, ("FileSystem::makePathKey - key buffer is empty"));

	Char* const keyBegin = key;
	const Char* const keyEnd = key + keySize - 1;
	const Char* const pairEnd = getPathKeyPairEnd(path);

	for (;;)
	{
		const UnsignedByte ch = toPathKeyChar(*path);

		if (ch == '\0')
			break;

		if (key == keyEnd)
		{
			*keyBegin = '\0';
			return FALSE;
		}

		*key++ = ch;
		++path;

		if (ch == '\\')
		{
			while (path > pairEnd && isPathSeparator(*path))
				++path;
		}
	}

	*key = '\0';
	return TRUE;
}

//============================================================================
// FileSystem::comparePath
//============================================================================
Int FileSystem::comparePath(const Char* a, const Char* b)
{
	const Char* const pairEndA = getPathKeyPairEnd(a);
	const Char* const pairEndB = getPathKeyPairEnd(b);

	for (;;)
	{
		const UnsignedByte ca = toPathKeyChar(*a);
		const UnsignedByte cb = toPathKeyChar(*b);

		if (ca != cb || ca == '\0')
			return (Int)ca - (Int)cb;

		++a;
		++b;

		if (ca == '\\')
		{
			while (a > pairEndA && isPathSeparator(*a))
				++a;
			while (b > pairEndB && isPathSeparator(*b))
				++b;
		}
	}
}
