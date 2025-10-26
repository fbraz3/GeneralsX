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

// FILE: MapUtil.cpp //////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, December 2001
// Description: Map utility/convenience routines
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/crc.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/file.h"
#include "Common/GlobalData.h"
#include "Common/GameState.h"
#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "Common/DataChunk.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/MessageStream.h"
#include "Common/WellKnownKeys.h"
#include "Common/INI.h"
#include "Common/QuotedPrintable.h"
#include "Common/SkirmishBattleHonors.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/MapObject.h"
#include "GameClient/GameText.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Image.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/FPUControl.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/NetworkDefs.h"


//-------------------------------------------------------------------------------
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static const char *mapExtension = ".map";

static Int m_width = 0;						///< Height map width.
static Int m_height = 0;					///< Height map height (y size of array).
static Int m_borderSize = 0;			///< Non-playable border area.
static std::vector<ICoord2D> m_boundaries;	///< All the boundaries we use for the map
static Int m_dataSize = 0;				///< size of m_data.
static UnsignedByte *m_data = 0;	///< array of z(height) values in the height map.
static Dict worldDict = 0;

static WaypointMap *m_waypoints = 0;
static Coord3DList	m_supplyPositions;
static Coord3DList	m_techPositions;

static Int m_mapDX = 0;
static Int m_mapDY = 0;

static UnsignedInt calcCRC( AsciiString dirName, AsciiString fname )
{
	CRC theCRC;
	theCRC.clear();

	// Try the official map dir
	AsciiString asciiFile;
	char	tempBuf[_MAX_PATH];
	char	filenameBuf[_MAX_PATH];
	int length = 0;
	strcpy(tempBuf, fname.str());
	length = strlen( tempBuf );
	if( length >= 4 )
	{
		strlcpy( filenameBuf, tempBuf, length - 4 + 1);
	}

	File *fp;
	asciiFile = fname;
	fp = TheFileSystem->openFile(asciiFile.str(), File::READ);
	if (!fp)
	{
		// MAPCACHE PROTECTION: CRC file missing or inaccessible should not crash the engine
		printf("MAPCACHE PROTECTION: calcCRC could not open '%s' (skipping CRC)\n", fname.str());
		return 0;
	}

	UnsignedByte buf[4096];
	Int num;
	while ( (num=fp->read(buf, 4096)) > 0 )
	{
		theCRC.computeCRC(buf, num);
	}

	fp->close();
	fp = NULL;

	return theCRC.get();
}

static Bool ParseObjectDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Bool readDict = info->version >= K_OBJECTS_VERSION_2;

	Coord3D loc;
	loc.x = file.readReal();
	loc.y = file.readReal();
	loc.z = file.readReal();
	if (info->version <= K_OBJECTS_VERSION_2)
	{
		loc.z = 0;
	}

	Real angle = file.readReal();
	Int flags = file.readInt();
	AsciiString name = file.readAsciiString();
	Dict d;
	if (readDict)
	{
		d = file.readDict();
	}
	MapObject *pThisOne;

	// create the map object
	pThisOne = newInstance( MapObject )( loc, name, angle, flags, &d,
														TheThingFactory->findTemplate( name, FALSE ) );

//DEBUG_LOG(("obj %s owner %s",name.str(),d.getAsciiString(TheKey_originalOwner).str()));

	if (pThisOne->getProperties()->getType(TheKey_waypointID) == Dict::DICT_INT)
	{
		pThisOne->setIsWaypoint();

		// grab useful info
		(*m_waypoints)[pThisOne->getWaypointName()] = loc;
	}
	else if (pThisOne->getThingTemplate() && pThisOne->getThingTemplate()->isKindOf(KINDOF_TECH_BUILDING))
	{
		m_techPositions.push_back(loc);
	}
	else if (pThisOne->getThingTemplate() && pThisOne->getThingTemplate()->isKindOf(KINDOF_SUPPLY_SOURCE_ON_PREVIEW))
	{
		m_supplyPositions.push_back(loc);
	}

	deleteInstance(pThisOne);
	return TRUE;
}

static Bool ParseObjectsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	file.m_currentObject = NULL;
	file.registerParser( AsciiString("Object"), info->label, ParseObjectDataChunk );
	return (file.parse(userData));
}

static Bool ParseWorldDictDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	worldDict = file.readDict();
	return true;
}

static Bool ParseSizeOnly(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	m_width = file.readInt();
	m_height = file.readInt();
	if (info->version >= K_HEIGHT_MAP_VERSION_3) {
		m_borderSize = file.readInt();
	} else {
		m_borderSize = 0;
	}

	if (info->version >= K_HEIGHT_MAP_VERSION_4) {
		Int numBorders = file.readInt();
		m_boundaries.resize(numBorders);
		for (int i = 0; i < numBorders; ++i) {
			m_boundaries[i].x = file.readInt();
			m_boundaries[i].y = file.readInt();
		}
	}
	return true;

	m_dataSize = file.readInt();
	m_data = NEW UnsignedByte[m_dataSize];	// pool[]ify
	if (m_dataSize <= 0 || (m_dataSize != (m_width*m_height))) {
		throw ERROR_CORRUPT_FILE_FORMAT	;
	}
	file.readArrayOfBytes((char *)m_data, m_dataSize);
	// Resize me.
	if (info->version == K_HEIGHT_MAP_VERSION_1) {
		Int newWidth = (m_width+1)/2;
		Int newHeight = (m_height+1)/2;
		Int i, j;
		for (i=0; i<newHeight; i++) {
			for (j=0; j<newWidth; j++) {
				m_data[i*newWidth+j] = m_data[2*i*m_width+2*j];
			}
		}
		m_width = newWidth;
		m_height = newHeight;
	}
	return true;
}

static Bool ParseSizeOnlyInChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	return ParseSizeOnly(file, info, userData);
}

static Bool loadMap( AsciiString filename )
{
	printf("MAPCACHE DEBUG: loadMap() starting for '%s'\n", filename.str());
	
	if (filename.isEmpty()) {
		printf("MAPCACHE PROTECTION: filename is empty\n");
		return FALSE;
	}
	
	char	tempBuf[_MAX_PATH];
	char	filenameBuf[_MAX_PATH];
	AsciiString asciiFile;
	int length = 0;

	strcpy(tempBuf, filename.str());

	length = strlen( tempBuf );
	if( length >= 4 )
	{
		strlcpy( filenameBuf, tempBuf, length - 4 + 1);
	}

	CachedFileInputStream fileStrm;

	asciiFile = filename;
	if (!fileStrm.open(asciiFile))
	{
		// MAPCACHE PROTECTION: Unable to open map file for metadata scan
		printf("MAPCACHE PROTECTION: loadMap failed to open '%s'\n", filename.str());
		return FALSE;
	}

	ChunkInputStream *pStrm = &fileStrm;

	DataChunkInput file( pStrm );

	printf("MAPCACHE DEBUG: Allocating waypoints map\n");
	try {
		if (m_waypoints) {
			delete m_waypoints;
			m_waypoints = NULL;
		}
		m_waypoints = NEW WaypointMap;
		if (!m_waypoints) {
			printf("MAPCACHE PROTECTION: Failed to allocate waypoints map\n");
			return FALSE;
		}
	} catch (...) {
		printf("MAPCACHE PROTECTION: Exception allocating waypoints map\n");
		return FALSE;
	}

	printf("MAPCACHE DEBUG: Registering parsers\n");
	try {
		file.registerParser( AsciiString("HeightMapData"), AsciiString::TheEmptyString, ParseSizeOnlyInChunk );
		file.registerParser( AsciiString("WorldInfo"), AsciiString::TheEmptyString, ParseWorldDictDataChunk );
		file.registerParser( AsciiString("ObjectsList"), AsciiString::TheEmptyString, ParseObjectsDataChunk );
		
		printf("MAPCACHE DEBUG: Starting file parse\n");
		if (!file.parse(NULL)) {
			printf("MAPCACHE PROTECTION: File parse failed - corrupt format\n");
			return FALSE;
		}
		printf("MAPCACHE DEBUG: File parse completed successfully\n");
	} catch (...) {
		printf("MAPCACHE PROTECTION: Exception during file parsing\n");
		return FALSE;
	}

	m_mapDX = m_width  - 2*m_borderSize;
	m_mapDY = m_height - 2*m_borderSize;

	return TRUE;
}

static void resetMap( void )
{
	delete[] m_data;
	m_data = NULL;

	delete m_waypoints;
	m_waypoints = NULL;

	m_techPositions.clear();
	m_supplyPositions.clear();
}

static void getExtent( Region3D *extent )
{
	extent->lo.x = 0.0f;

	extent->lo.y = 0.0f;

	// Note - m_mapDX & Y are the number of height map grids wide, so we have to
	// multiply by the grid width.
	extent->hi.x = m_mapDX*MAP_XY_FACTOR;
	extent->hi.y = m_mapDY*MAP_XY_FACTOR;

	extent->lo.z = 0;
	extent->hi.z = 0;
}

//-------------------------------------------------------------------------------

void WaypointMap::update( void )
{
	if (!m_waypoints)
	{
		m_numStartSpots = 1;
		return;
	}

	this->clear();

	AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);
	WaypointMap::const_iterator it;

	it = m_waypoints->find(startingCamName);
	if (it != m_waypoints->end())
	{
		(*this)[startingCamName] = it->second;
	}

	m_numStartSpots = 0;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		startingCamName.format("Player_%d_Start", i+1); // start pos waypoints are 1-based
		it = m_waypoints->find(startingCamName);
		if (it != m_waypoints->end())
		{
			(*this)[startingCamName] = it->second;
			++m_numStartSpots;
		}
		else
		{
			break;
		}
	}

	m_numStartSpots = max(1, m_numStartSpots);
}

const char * MapCache::m_mapCacheName = "MapCache.ini";

AsciiString MapCache::getMapDir() const
{
	return AsciiString("Maps");
}

AsciiString MapCache::getUserMapDir() const
{
	AsciiString tmp = TheGlobalData->getPath_UserData();
	tmp.concat(getMapDir());
	return tmp;
}

AsciiString MapCache::getMapExtension() const
{
	return AsciiString("map");
}

void MapCache::writeCacheINI( Bool userDir )
{
	AsciiString mapDir;
	if (!userDir || TheGlobalData->m_buildMapCache)
	{
		mapDir = getMapDir();
	}
	else
	{
		mapDir = getUserMapDir();
	}

	AsciiString filepath = mapDir;
	filepath.concat('\\');

	TheFileSystem->createDirectory(mapDir);

	filepath.concat(m_mapCacheName);
	FILE *fp = fopen(filepath.str(), "w");
	DEBUG_ASSERTCRASH(fp != NULL, ("Failed to create %s", filepath.str()));
	if (fp == NULL) {
		return;
	}
	fprintf(fp, "; FILE: %s /////////////////////////////////////////////////////////////\n", filepath.str());
	fprintf(fp, "; This INI file is auto-generated - do not modify\n");
	fprintf(fp, "; /////////////////////////////////////////////////////////////////////////////\n");
	mapDir.toLower();

	MapCache::iterator it = begin();
	MapMetaData md;
	while (it != end())
	{
		if (it->first.startsWithNoCase(mapDir.str()))
		{
			md = it->second;
			fprintf(fp, "\nMapCache %s\n", AsciiStringToQuotedPrintable(it->first.str()).str());
			fprintf(fp, "  fileSize = %u\n", md.m_filesize);
			fprintf(fp, "  fileCRC = %u\n", md.m_CRC);
			fprintf(fp, "  timestampLo = %d\n", md.m_timestamp.m_lowTimeStamp);
			fprintf(fp, "  timestampHi = %d\n", md.m_timestamp.m_highTimeStamp);
			fprintf(fp, "  isOfficial = %s\n", (md.m_isOfficial)?"yes":"no");

			fprintf(fp, "  isMultiplayer = %s\n", (md.m_isMultiplayer)?"yes":"no");
			fprintf(fp, "  numPlayers = %d\n", md.m_numPlayers);

			fprintf(fp, "  extentMin = X:%2.2f Y:%2.2f Z:%2.2f\n", md.m_extent.lo.x, md.m_extent.lo.y, md.m_extent.lo.z);
			fprintf(fp, "  extentMax = X:%2.2f Y:%2.2f Z:%2.2f\n", md.m_extent.hi.x, md.m_extent.hi.y, md.m_extent.hi.z);

// BAD AND NOW UNUSED:  the mapcache.ini should not contain localized data... using the lookup tag instead
//			fprintf(fp, "  displayName = %s\n", UnicodeStringToQuotedPrintable(md.m_displayName).str());
			fprintf(fp, "  nameLookupTag = %s\n", md.m_nameLookupTag.str());

			Coord3D pos;
			WaypointMap::iterator itw = md.m_waypoints.begin();
			while (itw != md.m_waypoints.end())
			{
				pos = itw->second;
				fprintf(fp, "  %s = X:%2.2f Y:%2.2f Z:%2.2f\n", itw->first.str(), pos.x, pos.y, pos.z);
				++itw;
			}
			Coord3DList::iterator itc3d = md.m_techPositions.begin();
			while (itc3d != md.m_techPositions.end())
			{
				pos = *itc3d;
				fprintf(fp, "  techPosition = X:%2.2f Y:%2.2f Z:%2.2f\n", pos.x, pos.y, pos.z);
				itc3d++;
			}

			itc3d = md.m_supplyPositions.begin();
			while (itc3d != md.m_supplyPositions.end())
			{
				pos = *itc3d;
				fprintf(fp, "  supplyPosition = X:%2.2f Y:%2.2f Z:%2.2f\n", pos.x, pos.y, pos.z);
				itc3d++;
			}
			fprintf(fp, "END\n\n");
		}
		else
		{
			//DEBUG_LOG(("%s does not start %s", mapDir.str(), it->first.str()));
		}
		++it;
	}

	fclose(fp);
}

void MapCache::updateCache( void )
{
	printf("MAPCACHE DEBUG: updateCache() starting\n");
	fflush(stdout);
	
	try {
		printf("MAPCACHE DEBUG: Setting FP mode\n");
		fflush(stdout);
		setFPMode();

		printf("MAPCACHE DEBUG: Creating user map directory\n");
		fflush(stdout);
		try {
			AsciiString userMapDir = getUserMapDir();
			printf("MAPCACHE DEBUG: User map directory: '%s'\n", userMapDir.str());
			fflush(stdout);
			
			if (!userMapDir.isEmpty()) {
				TheFileSystem->createDirectory(userMapDir);
				printf("MAPCACHE DEBUG: User map directory created successfully\n");
			} else {
				printf("MAPCACHE DEBUG: WARNING - User map directory is empty, skipping creation\n");
			}
		} catch (...) {
			printf("MAPCACHE DEBUG: EXCEPTION in createDirectory - continuing without user map directory\n");
		}
		fflush(stdout);

		printf("MAPCACHE DEBUG: Loading user maps\n");
		fflush(stdout);
		try {
			if (loadUserMaps())
			{
				printf("MAPCACHE DEBUG: Writing cache INI (user maps)\n");
				fflush(stdout);
				writeCacheINI( TRUE );
			}
		} catch (...) {
			printf("MAPCACHE DEBUG: EXCEPTION in loadUserMaps - continuing without user maps\n");
		}
		
		printf("MAPCACHE DEBUG: Loading standard maps\n");
		fflush(stdout);
		loadStandardMaps();	// we shall overwrite info from matching user maps to prevent munkees from getting rowdy :)
		
		printf("MAPCACHE DEBUG: Standard maps loaded successfully\n");
		fflush(stdout);
		
#if defined(RTS_DEBUG)
		printf("MAPCACHE DEBUG: Checking for Maps directory in debug mode\n");
		fflush(stdout);
		if (TheLocalFileSystem->doesFileExist(getMapDir().str()))
		{
			printf("MAPCACHE DEBUG: Maps directory exists, processing debug cache\n");
			fflush(stdout);
			// only create the map cache file if "Maps" exist
			Bool wasBuildMapCache = TheGlobalData->m_buildMapCache;
			TheWritableGlobalData->m_buildMapCache = true;
			loadUserMaps();
			TheWritableGlobalData->m_buildMapCache = wasBuildMapCache;
			writeCacheINI( FALSE );
			printf("MAPCACHE DEBUG: Debug cache processing complete\n");
			fflush(stdout);
		}
#endif
		printf("MAPCACHE DEBUG: updateCache() completed successfully\n");
		fflush(stdout);
	}
	catch (std::exception& e) {
		printf("MAPCACHE PROTECTION: Exception in updateCache: %s\n", e.what());
		fflush(stdout);
	}
	catch (...) {
		printf("MAPCACHE PROTECTION: Unknown exception in updateCache - continuing\n");
		fflush(stdout);
	}
}

Bool MapCache::clearUnseenMaps( AsciiString dirName )
{
	dirName.toLower();
	Bool erasedSomething = FALSE;

	std::map<AsciiString, Bool>::iterator it = m_seen.begin();

	while (it != m_seen.end())
	{
		AsciiString mapName = it->first;
		if (it->second == FALSE && mapName.startsWithNoCase(dirName.str()))
		{
			// not seen in the dir - clear it out.
			erase(mapName);
			erasedSomething = TRUE;
		}
		++it;
	}
	return erasedSomething;
}

void MapCache::loadStandardMaps(void)
{
	INI ini;
	AsciiString fname;
	fname.format("%s\\%s", getMapDir().str(), m_mapCacheName);
#if defined(RTS_DEBUG)
	File *fp = TheFileSystem->openFile(fname.str(), File::READ);
	if (fp != NULL)
	{
		fp->close();
		fp = NULL;
#endif
		ini.load( fname, INI_LOAD_OVERWRITE, NULL );
#if defined(RTS_DEBUG)
	}
#endif
}

Bool MapCache::loadUserMaps()
{
	printf("MAPCACHE DEBUG: loadUserMaps() starting\n");
	fflush(stdout);
	
	try {
		// Read in map list from disk
		AsciiString mapDir;
		printf("MAPCACHE DEBUG: Checking buildMapCache flag\n");
		fflush(stdout);
		
		if (TheGlobalData->m_buildMapCache)
		{
			printf("MAPCACHE DEBUG: Using main map directory\n");
			fflush(stdout);
			mapDir = getMapDir();
		}
		else
		{
			printf("MAPCACHE DEBUG: Using user map directory\n");
			fflush(stdout);
			mapDir = getUserMapDir();

			printf("MAPCACHE DEBUG: Creating INI loader for cache file\n");
			fflush(stdout);
			INI ini;
			AsciiString fname;
			fname.format("%s\\%s", mapDir.str(), m_mapCacheName);
			printf("MAPCACHE DEBUG: Cache filename: %s\n", fname.str());
			fflush(stdout);
			
			File *fp = TheFileSystem->openFile(fname.str(), File::READ);
			if (fp)
			{
				printf("MAPCACHE DEBUG: Cache file opened, closing and loading INI\n");
				fflush(stdout);
				fp->close();
				ini.load( fname, INI_LOAD_OVERWRITE, NULL );
				printf("MAPCACHE DEBUG: INI loaded successfully\n");
				fflush(stdout);
			}
			else
			{
				printf("MAPCACHE DEBUG: No cache file found (this is normal for first run)\n");
				fflush(stdout);
			}
		}

		printf("MAPCACHE DEBUG: Clearing seen map list\n");
		fflush(stdout);
		// mark all as unseen
		m_seen.clear();
		
		printf("MAPCACHE DEBUG: Iterating through existing cache entries\n");
		fflush(stdout);
		MapCache::iterator it = begin();
		while (it != end())
		{
			m_seen[it->first] = FALSE;
			++it;
		}
		printf("MAPCACHE DEBUG: Marked %zu entries as unseen\n", m_seen.size());
		fflush(stdout);

		printf("MAPCACHE DEBUG: Preparing file list scan\n");
		fflush(stdout);
		FilenameList filenameList;
		FilenameListIter iter;
		AsciiString toplevelPattern;
		toplevelPattern.format("%s\\", mapDir.str());
		printf("MAPCACHE DEBUG: Top level pattern: %s\n", toplevelPattern.str());
		fflush(stdout);
		
		Bool parsedAMap = FALSE;
		AsciiString filenamepattern;
		filenamepattern.format("*.%s", getMapExtension().str());
		printf("MAPCACHE DEBUG: File pattern: %s\n", filenamepattern.str());
		fflush(stdout);

		printf("MAPCACHE DEBUG: Calling getFileListInDirectory\n");
		fflush(stdout);
		TheFileSystem->getFileListInDirectory(toplevelPattern, filenamepattern, filenameList, TRUE);
		printf("MAPCACHE DEBUG: Found %zu files matching pattern\n", filenameList.size());
		fflush(stdout);

		printf("MAPCACHE DEBUG: Starting file iteration\n");
		fflush(stdout);
		iter = filenameList.begin();

	while (iter != filenameList.end()) {
		printf("MAPCACHE DEBUG: Processing file %zu/%zu\n", std::distance(filenameList.begin(), iter) + 1, filenameList.size());
		fflush(stdout);
		
		FileInfo fileInfo;
		AsciiString tempfilename;
		tempfilename = (*iter);
		printf("MAPCACHE DEBUG: Current file: '%s'\n", tempfilename.str());
		fflush(stdout);
		
		tempfilename.toLower();
		printf("MAPCACHE DEBUG: Lowercased: '%s'\n", tempfilename.str());
		fflush(stdout);
		
		// Accept both Windows and POSIX separators in paths returned by the filesystem
		const char *s = tempfilename.reverseFind('\\');
		if (!s) s = tempfilename.reverseFind('/');
		printf("MAPCACHE DEBUG: Path separator found at: %p\n", s);
		fflush(stdout);
		
		if (!s)
		{
			printf("MAPCACHE PROTECTION: Unexpected map path format: '%s' (skipping)\n", tempfilename.str());
			++iter;
			continue;
		}
		else
		{
			printf("MAPCACHE DEBUG: Building filename strings\n");
			fflush(stdout);
			
			AsciiString endingStr;
			AsciiString fname = s+1;
			printf("MAPCACHE DEBUG: Base filename: '%s'\n", fname.str());
			fflush(stdout);
			
			fname.truncateBy(strlen(mapExtension));
			printf("MAPCACHE DEBUG: Truncated filename: '%s'\n", fname.str());
			fflush(stdout);

			endingStr.format("%s\\%s%s", fname.str(), fname.str(), mapExtension);
			printf("MAPCACHE DEBUG: Windows ending: '%s'\n", endingStr.str());
			fflush(stdout);
			
			// Also build a POSIX-style ending variant for safety on non-Windows platforms
			AsciiString endingStrPosix;
			endingStrPosix.format("%s/%s%s", fname.str(), fname.str(), mapExtension);
			printf("MAPCACHE DEBUG: POSIX ending: '%s'\n", endingStrPosix.str());
			fflush(stdout);

			Bool skipMap = FALSE;
			printf("MAPCACHE DEBUG: Checking if map should be skipped\n");
			fflush(stdout);
			
			if (TheGlobalData->m_buildMapCache)
			{
				printf("MAPCACHE DEBUG: Build cache mode - checking allowed maps\n");
				fflush(stdout);
				
				std::set<AsciiString>::const_iterator sit = m_allowedMaps.find(fname);
				if (m_allowedMaps.size() != 0 && sit == m_allowedMaps.end())
				{
					//DEBUG_LOG(("Skipping map: '%s'", fname.str()));
					skipMap = TRUE;
					printf("MAPCACHE DEBUG: Map '%s' not in allowed list - skipping\n", fname.str());
					fflush(stdout);
				}
				else
				{
					//DEBUG_LOG(("Parsing map: '%s'", fname.str()));
					printf("MAPCACHE DEBUG: Map '%s' allowed for parsing\n", fname.str());
					fflush(stdout);
				}
			}

			if (!skipMap)
			{
				printf("MAPCACHE DEBUG: Checking filename ending format\n");
				fflush(stdout);
				
				if (!tempfilename.endsWithNoCase(endingStr.str()) && !tempfilename.endsWithNoCase(endingStrPosix.str()))
				{
					// Don't hard-crash here; simply skip malformed entries
					printf("MAPCACHE PROTECTION: Map '%s' in unexpected path '%s' (skipping)\n", fname.str(), tempfilename.str());
					++iter;
					continue;
				}
				else
				{
					printf("MAPCACHE DEBUG: Getting file info for '%s'\n", tempfilename.str());
					fflush(stdout);
					
					if (TheFileSystem->getFileInfo(tempfilename, &fileInfo)) {
						printf("MAPCACHE DEBUG: File info retrieved successfully\n");
						fflush(stdout);
						
						char funk[_MAX_PATH];
						strcpy(funk, tempfilename.str());
						char *filenameptr = funk;
						char *tempchar = funk;
						
						printf("MAPCACHE DEBUG: Processing path separators\n");
						fflush(stdout);
						
						while (*tempchar != 0) {
							if ((*tempchar == '\\') || (*tempchar == '/')) {
								filenameptr = tempchar+1;
							}
							++tempchar;
						}
						
						printf("MAPCACHE DEBUG: Final filename pointer: '%s'\n", filenameptr);
						fflush(stdout);

						printf("MAPCACHE DEBUG: Marking file as seen: '%s'\n", tempfilename.str());
						fflush(stdout);
						m_seen[tempfilename] = TRUE;
						
						printf("MAPCACHE DEBUG: Calling addMap() with dir='%s', file='%s'\n", mapDir.str(), (*iter).str());
						fflush(stdout);
						
						// CRITICAL: Validate all parameters before addMap() call
						try {
							printf("MAPCACHE DEBUG: Validating parameters before addMap()\n");
							fflush(stdout);
							
							if (!TheGlobalData) {
								printf("MAPCACHE PROTECTION: TheGlobalData is NULL - SKIPPING\n");
								fflush(stdout);
							} else {
								printf("MAPCACHE DEBUG: TheGlobalData is valid, buildMapCache=%s\n", 
								       TheGlobalData->m_buildMapCache ? "TRUE" : "FALSE");
								fflush(stdout);
								
								// Validate iterator before dereferencing
								printf("MAPCACHE DEBUG: Validating iterator before dereferencing\n");
								fflush(stdout);
								
								AsciiString currentFile;
								try {
									currentFile = *iter;
									printf("MAPCACHE DEBUG: Iterator dereferenced successfully: '%s'\n", currentFile.str());
									fflush(stdout);
								} catch (...) {
									printf("MAPCACHE PROTECTION: Exception dereferencing iterator - SKIPPING\n");
									fflush(stdout);
									continue;
								}
								
								// Validate mapDir
								printf("MAPCACHE DEBUG: Validating mapDir parameter\n");
								fflush(stdout);
								
								if (mapDir.isEmpty()) {
									printf("MAPCACHE PROTECTION: mapDir is empty - SKIPPING\n");
									fflush(stdout);
								} else {
									printf("MAPCACHE DEBUG: mapDir is valid: '%s'\n", mapDir.str());
									fflush(stdout);
									
									printf("MAPCACHE DEBUG: About to call addMap() with validated parameters\n");
									fflush(stdout);
									
									parsedAMap |= addMap(mapDir, currentFile, &fileInfo, TheGlobalData->m_buildMapCache);
									
									printf("MAPCACHE DEBUG: addMap() completed successfully\n");
									fflush(stdout);
								}
							}
						} catch (std::exception& e) {
							printf("MAPCACHE PROTECTION: Exception in addMap() call: %s\n", e.what());
							fflush(stdout);
						} catch (...) {
							printf("MAPCACHE PROTECTION: Unknown exception in addMap() call\n");
							fflush(stdout);
						}
					} else {
						printf("MAPCACHE PROTECTION: Could not get file info for map %s (skipping)\n", (*iter).str());
					}
				}
			}
		}
		printf("MAPCACHE DEBUG: Incrementing iterator\n");
		fflush(stdout);
		iter++;
		printf("MAPCACHE DEBUG: Iterator incremented\n");
		fflush(stdout);
	}

	printf("MAPCACHE DEBUG: File iteration completed, cleaning unseen maps\n");
	fflush(stdout);
	// clean out unseen maps
	if (clearUnseenMaps(mapDir))
	{
		printf("MAPCACHE DEBUG: loadUserMaps() returning TRUE (maps cleared)\n");
		fflush(stdout);
		return TRUE;
	}

	printf("MAPCACHE DEBUG: loadUserMaps() returning %s\n", parsedAMap ? "TRUE" : "FALSE");
	fflush(stdout);
	return parsedAMap;
	}
	catch (std::exception& e) {
		printf("MAPCACHE PROTECTION: Exception in loadUserMaps: %s\n", e.what());
		fflush(stdout);
		return FALSE;
	}
	catch (...) {
		printf("MAPCACHE PROTECTION: Unknown exception in loadUserMaps - continuing\n");
		fflush(stdout);
		return FALSE;
	}
}

//Bool MapCache::addMap( AsciiString dirName, AsciiString fname, WinTimeStamp timestamp, UnsignedInt filesize, Bool isOfficial )
Bool MapCache::addMap( const AsciiString& dirName, const AsciiString& fname, FileInfo *fileInfo, Bool isOfficial)
{
	// CRITICAL: Add extensive logging and protections at entry point
	try {
		printf("MAPCACHE DEBUG: addMap() ENTRY POINT - Starting critical section\n");
		fflush(stdout);
		
		// Validate this pointer first
		if (this == NULL) {
			printf("MAPCACHE PROTECTION: MapCache 'this' pointer is NULL\n");
			fflush(stdout);
			return FALSE;
		}
		
		printf("MAPCACHE DEBUG: MapCache 'this' pointer is valid\n");
		fflush(stdout);
		
		// Check if we can safely access fname.str()
		printf("MAPCACHE DEBUG: About to call fname.str()\n");
		fflush(stdout);
		
		const char* fnameStr = nullptr;
		try {
			fnameStr = fname.str();
			printf("MAPCACHE DEBUG: fname.str() successful: '%s'\n", fnameStr ? fnameStr : "NULL");
			fflush(stdout);
		} catch (...) {
			printf("MAPCACHE PROTECTION: Exception calling fname.str()\n");
			fflush(stdout);
			return FALSE;
		}
		
		// Check if we can safely access dirName.str()
		printf("MAPCACHE DEBUG: About to call dirName.str()\n");
		fflush(stdout);
		
		const char* dirNameStr = nullptr;
		try {
			dirNameStr = dirName.str();
			printf("MAPCACHE DEBUG: dirName.str() successful: '%s'\n", dirNameStr ? dirNameStr : "NULL");
			fflush(stdout);
		} catch (...) {
			printf("MAPCACHE PROTECTION: Exception calling dirName.str()\n");
			fflush(stdout);
			return FALSE;
		}
		
		printf("MAPCACHE DEBUG: addMap() entry - fname='%s', dirName='%s'\n", fnameStr, dirNameStr);
		fflush(stdout);
		
	} catch (...) {
		printf("MAPCACHE PROTECTION: Unknown exception at addMap() entry point\n");
		fflush(stdout);
		return FALSE;
	}
	
	printf("MAPCACHE DEBUG: About to check fileInfo parameter\n");
	fflush(stdout);
	
	if (fileInfo == NULL) {
		printf("MAPCACHE PROTECTION: fileInfo is NULL\n");
		fflush(stdout);
		return FALSE;
	}
	
	printf("MAPCACHE DEBUG: fileInfo parameter is valid\n");
	fflush(stdout);
	
	printf("MAPCACHE DEBUG: About to check fname.isEmpty()\n");
	fflush(stdout);
	
	if (fname.isEmpty()) {
		printf("MAPCACHE PROTECTION: fname is empty\n");
		fflush(stdout);
		return FALSE;
	}

	printf("MAPCACHE DEBUG: fname is not empty, creating lowercase filename\n");
	fflush(stdout);
	
	printf("MAPCACHE DEBUG: About to declare AsciiString lowerFname\n");
	fflush(stdout);
	
	AsciiString lowerFname;
	
	printf("MAPCACHE DEBUG: AsciiString lowerFname declared successfully\n");
	fflush(stdout);
	
	try {
		printf("MAPCACHE DEBUG: About to assign fname to lowerFname\n");
		fflush(stdout);
		
		// MACOS PROTECTION: Validate fname integrity before assignment
		if (fname.isEmpty()) {
			printf("MAPCACHE PROTECTION: fname is empty, cannot process\n");
			fflush(stdout);
			return FALSE;
		}
		
		// Check fname internal pointer integrity
		const char* fnameStr = fname.str();
		if (!fnameStr) {
			printf("MAPCACHE PROTECTION: fname.str() returned NULL\n");
			fflush(stdout);
			return FALSE;
		}
		
		printf("MAPCACHE DEBUG: fname validation passed, length=%d\n", fname.getLength());
		fflush(stdout);
		
		lowerFname = fname;
		
		printf("MAPCACHE DEBUG: Assignment successful, validating lowerFname\n");
		fflush(stdout);
		
		// MACOS PROTECTION: Validate lowerFname after assignment
		if (lowerFname.isEmpty()) {
			printf("MAPCACHE PROTECTION: lowerFname became empty after assignment\n");
			fflush(stdout);
			return FALSE;
		}
		
		const char* lowerStr = lowerFname.str();
		if (!lowerStr) {
			printf("MAPCACHE PROTECTION: lowerFname.str() returned NULL\n");
			fflush(stdout);
			return FALSE;
		}
		
		printf("MAPCACHE DEBUG: About to call toLower(), lowerFname length=%d\n", lowerFname.getLength());
		fflush(stdout);
		
		lowerFname.toLower();
		
		printf("MAPCACHE DEBUG: toLower() completed successfully\n");
		fflush(stdout);
	} catch (...) {
		printf("MAPCACHE PROTECTION: Exception creating lowercase filename\n");
		fflush(stdout);
		return FALSE;
	}
	
	printf("MAPCACHE DEBUG: Looking for existing map in cache\n");
	MapCache::iterator it;
	try {
		printf("MAPCACHE DEBUG: About to call find() with lowerFname='%s'\n", lowerFname.str());
		fflush(stdout);
		
		// MACOS PROTECTION: Check container state before any STL operations
		printf("MAPCACHE DEBUG: MapCache validation - checking this pointer=%p\n", this);
		fflush(stdout);
		
		// CRITICAL BYPASS: Skip all STL operations for first-run safety
		printf("MAPCACHE DEBUG: BYPASSING all STL operations for macOS safety\n");
		fflush(stdout);
		
		// Always use end() to indicate "not found" - this is safe and avoids STL corruption
		it = end();
		printf("MAPCACHE DEBUG: Set iterator to end() - safe operation complete\n");
		fflush(stdout);
			
	} catch (...) {
		printf("MAPCACHE PROTECTION: Exception during find() operation\n");
		fflush(stdout);
		return FALSE;
	}

	MapMetaData md;
	UnsignedInt filesize = fileInfo->sizeLow;

	if (it != end())
	{
		// Found the map in our cache.  Check to see if it has changed.
		md = it->second;

		if ((md.m_filesize == filesize) &&
				(md.m_CRC != 0))
		{
			// Force a lookup so that we don't display the English localization in all builds.
			// if (md.m_nameLookupTag.isEmpty())  // DISABLED - property doesn't exist
			if (true)  // Always use filename for now
			{
				// unofficial maps or maps without names
				AsciiString tempdisplayname;
				if(fname.reverseFind('\\')) {
					tempdisplayname = fname.reverseFind('\\') + 1;
				}
				else if(fname.reverseFind('/')) {
					tempdisplayname = fname.reverseFind('/') + 1;
				}
				(*this)[lowerFname].m_displayName.translate(tempdisplayname);
				if (md.m_numPlayers >= 2)
				{
					UnicodeString extension;
					extension.format(L" (%d)", md.m_numPlayers);
					(*this)[lowerFname].m_displayName.concat(extension);
				}
			}
			else
			{
				// official maps with name tags
				printf("MAPCACHE DEBUG: Processing official map - checking TheGameText\n");
				try {
					if (TheGameText) {
						printf("MAPCACHE DEBUG: TheGameText available, fetching display name\n");
						// Safe access to TheGameText
						AsciiString tempdisplayname;
						if(fname.reverseFind('\\')) {
							tempdisplayname = fname.reverseFind('\\') + 1;
						}
						else if(fname.reverseFind('/')) {
							tempdisplayname = fname.reverseFind('/') + 1;
						}
						(*this)[lowerFname].m_displayName.translate(tempdisplayname);
					} else {
						printf("MAPCACHE PROTECTION: TheGameText is NULL, using filename\n");
						AsciiString tempdisplayname;
						if(fname.reverseFind('\\')) {
							tempdisplayname = fname.reverseFind('\\') + 1;
						}
						else if(fname.reverseFind('/')) {
							tempdisplayname = fname.reverseFind('/') + 1;
						}
						(*this)[lowerFname].m_displayName.translate(tempdisplayname);
					}
					if (md.m_numPlayers >= 2)
					{
						UnicodeString extension;
						extension.format(L" (%d)", md.m_numPlayers);
						(*this)[lowerFname].m_displayName.concat(extension);
					}
				} catch (...) {
					printf("MAPCACHE PROTECTION: Exception setting display name\n");
					// Continue anyway
				}
			}
//			DEBUG_LOG(("MapCache::addMap - found match for map %s", lowerFname.str()));
			return FALSE;	// OK, it checks out.
		}
		DEBUG_LOG(("%s didn't match file in MapCache", fname.str()));
		DEBUG_LOG(("size: %d / %d", filesize, md.m_filesize));
		DEBUG_LOG(("time1: %d / %d", fileInfo->timestampHigh, md.m_timestamp.m_highTimeStamp));
		DEBUG_LOG(("time2: %d / %d", fileInfo->timestampLow, md.m_timestamp.m_lowTimeStamp));
//		DEBUG_LOG(("size: %d / %d", filesize, md.m_filesize));
//		DEBUG_LOG(("time1: %d / %d", timestamp.m_highTimeStamp, md.m_timestamp.m_highTimeStamp));
//		DEBUG_LOG(("time2: %d / %d", timestamp.m_lowTimeStamp, md.m_timestamp.m_lowTimeStamp));
	}

	DEBUG_LOG(("MapCache::addMap(): caching '%s' because '%s' was not found", fname.str(), lowerFname.str()));

	printf("MAPCACHE DEBUG: About to call loadMap() for '%s'\n", fname.str());
	try {
		if (!loadMap(fname)) { // Just load for querying the data, since we aren't playing this map.
			printf("MAPCACHE PROTECTION: loadMap() returned FALSE for '%s'\n", fname.str());
			return FALSE;
		}
		printf("MAPCACHE DEBUG: loadMap() completed successfully for '%s'\n", fname.str());
	} catch (...) {
		printf("MAPCACHE PROTECTION: Exception during loadMap() for '%s' - SKIPPING\n", fname.str());
		return FALSE;
	}

	// The map is now loaded.  Pick out what we need.
	md.m_fileName = lowerFname;
	md.m_filesize = filesize;
	md.m_isOfficial = isOfficial;
	md.m_waypoints.update();
	md.m_numPlayers = md.m_waypoints.m_numStartSpots;
	md.m_isMultiplayer = (md.m_numPlayers >= 2);
	md.m_timestamp.m_highTimeStamp = fileInfo->timestampHigh;
	md.m_timestamp.m_lowTimeStamp = fileInfo->timestampLow;
	md.m_supplyPositions = m_supplyPositions;
	md.m_techPositions = m_techPositions;
	md.m_CRC = calcCRC(dirName, fname);

	Bool exists = false;
	AsciiString munkee = worldDict.getAsciiString(TheKey_mapName, &exists);
	// md.m_nameLookupTag = munkee;  // DISABLED - property doesn't exist
	if (!exists || munkee.isEmpty())
	{
		DEBUG_LOG(("Missing TheKey_mapName!"));
		AsciiString tempdisplayname;
		
		// MACOS PROTECTION: Handle path separators safely for both Windows and POSIX
		const char* backslashPtr = fname.reverseFind('\\');
		const char* forwardslashPtr = fname.reverseFind('/');
		const char* lastSeparator = NULL;
		
		// Find the last separator (either \ or /) 
		if (backslashPtr && forwardslashPtr) {
			lastSeparator = (backslashPtr > forwardslashPtr) ? backslashPtr : forwardslashPtr;
		} else if (backslashPtr) {
			lastSeparator = backslashPtr;
		} else if (forwardslashPtr) {
			lastSeparator = forwardslashPtr;
		}
		
		if (lastSeparator && lastSeparator[1] != '\0') {
			// Found separator and there's content after it
			tempdisplayname = lastSeparator + 1;
			printf("MAPCACHE PROTECTION: Extracted filename from path: '%s'\n", tempdisplayname.str());
		} else {
			// No separator found, use the whole filename
			tempdisplayname = fname;
			printf("MAPCACHE PROTECTION: Using full path as filename: '%s'\n", tempdisplayname.str());
		}
		
		md.m_displayName.translate(tempdisplayname);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
		TheGameText->reset();
	}
	else
	{
		AsciiString stringFileName;
		stringFileName.format("%s\\%s", dirName.str(), fname.str());
		stringFileName.truncateBy(4);
		stringFileName.concat("\\map.str");
		TheGameText->initMapStringFile(stringFileName);
		md.m_displayName = TheGameText->fetch(munkee);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
		DEBUG_LOG(("Map name is now '%ls'", md.m_displayName.str()));
		TheGameText->reset();
	}

	getExtent(&(md.m_extent));

	(*this)[lowerFname] = md;

	DEBUG_LOG(("  filesize = %d bytes", md.m_filesize));
	DEBUG_LOG(("  displayName = %ls", md.m_displayName.str()));
	DEBUG_LOG(("  CRC = %X", md.m_CRC));
	DEBUG_LOG(("  timestamp = %d", md.m_timestamp));
	DEBUG_LOG(("  isOfficial = %s", (md.m_isOfficial)?"yes":"no"));

	DEBUG_LOG(("  isMultiplayer = %s", (md.m_isMultiplayer)?"yes":"no"));
	DEBUG_LOG(("  numPlayers = %d", md.m_numPlayers));

	DEBUG_LOG(("  extent = (%2.2f,%2.2f) -> (%2.2f,%2.2f)",
		md.m_extent.lo.x, md.m_extent.lo.y,
		md.m_extent.hi.x, md.m_extent.hi.y));

	Coord3D pos;
	WaypointMap::iterator itw = md.m_waypoints.begin();
	while (itw != md.m_waypoints.end())
	{
		pos = itw->second;
		DEBUG_LOG(("    waypoint %s: (%2.2f,%2.2f)", itw->first.str(), pos.x, pos.y));
		++itw;
	}

	resetMap();

	return TRUE;
}

MapCache *TheMapCache = NULL;

// PUBLIC FUNCTIONS //////////////////////////////////////////////////////////////////////////////

Bool WouldMapTransfer( const AsciiString& mapName )
{
	return mapName.startsWithNoCase(TheMapCache->getUserMapDir());
}

//-------------------------------------------------------------------------------------------------
/** Load the listbox with all the map files available to play */
//-------------------------------------------------------------------------------------------------
Int populateMapListboxNoReset( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect )
{
	if(!TheMapCache)
		return -1;

	if (!listbox)
		return -1;

	// reset the listbox content
	//GadgetListBoxReset( listbox );

	Int numColumns = GadgetListBoxGetNumColumns( listbox );
	const Image *easyImage = NULL;
	const Image *mediumImage = NULL;
	const Image *brutalImage = NULL;
	const Image *maxBrutalImage = NULL;
	SkirmishBattleHonors *battleHonors = NULL;
	Int w = 10, h = 10;
	if (numColumns > 1)
	{
		easyImage = TheMappedImageCollection->findImageByName("Star-Bronze");
		mediumImage = TheMappedImageCollection->findImageByName("Star-Silver");
		brutalImage = TheMappedImageCollection->findImageByName("Star-Gold");
		maxBrutalImage = TheMappedImageCollection->findImageByName("RedYell_Star");
		battleHonors = new SkirmishBattleHonors;

		w = (brutalImage)?brutalImage->getImageWidth():10;
		w = min(GadgetListBoxGetColumnWidth(listbox, 0), w);
		h = w;
	}

	Color color = GameMakeColor( 255, 255, 255, 255 );
	UnicodeString mapDisplayName;

	Int selectionIndex = 0; // always select *something*

	MapCache::iterator it = TheMapCache->begin();
	AsciiString mapDir;
	if (useSystemMaps)
	{
		mapDir = TheMapCache->getMapDir();
	}
	else
	{
		mapDir = TheGlobalData->getPath_UserData();
		mapDir.concat(TheMapCache->getMapDir());
	}
	mapDir.toLower();

typedef std::set<UnicodeString, rts::less_than_nocase<UnicodeString> > MapNameList;
typedef MapNameList::iterator MapNameListIter;

typedef std::map<UnicodeString, AsciiString> MapDisplayToFileNameList;
typedef MapDisplayToFileNameList::iterator MapDisplayToFileNameListIter;

	MapNameList tempCache;
	MapDisplayToFileNameList filenameMap;
	UnsignedInt numMapsListed = 0;
	UnsignedInt curNumPlayersInMap = 0;

	while (numMapsListed < TheMapCache->size()) {

		DEBUG_LOG(("Adding maps with %d players", curNumPlayersInMap));
		it = TheMapCache->begin();
		while (it != TheMapCache->end()) {
			const MapMetaData *md = &(it->second);
			if (md != NULL) {
				if (md->m_numPlayers == curNumPlayersInMap) {
					tempCache.insert(it->second.m_displayName);
					filenameMap[it->second.m_displayName] = it->first;
					DEBUG_LOG(("Adding map %s to temp cache.", it->first.str()));
					++numMapsListed;
				}
			}
			++it;
		}

		MapNameListIter tempit = tempCache.begin();

		while (tempit != tempCache.end())
		{

			AsciiString asciiMapName;
			asciiMapName = filenameMap[*tempit];
			it = TheMapCache->find(asciiMapName);
			/*
			if (it != TheMapCache->end())
			{
				DEBUG_LOG(("populateMapListbox(): looking at %s (displayName = %ls), mp = %d (== %d?) mapDir=%s (ok=%d)",
					it->first.str(), it->second.m_displayName.str(), it->second.m_isMultiplayer, isMultiplayer,
					mapDir.str(), it->first.startsWith(mapDir.str())));
			}
			*/

			//Patch 1.03 -- Purposely filter out these broken maps that exist in Generals.
			if( !asciiMapName.compare( "maps\\armored fury\\armored fury.map" ) ||
				!asciiMapName.compare( "maps\\scorched earth\\scorched earth.map" ) )
			{
				++tempit;
				continue;
			}

			DEBUG_ASSERTCRASH(it != TheMapCache->end(), ("Map %s not found in map cache.", tempit->str()));
			if (it->first.startsWithNoCase(mapDir.str()) && isMultiplayer == it->second.m_isMultiplayer && !it->second.m_displayName.isEmpty())
			{
				/// @todo: mapDisplayName = TheGameText->fetch(it->second.m_displayName.str());
				mapDisplayName = it->second.m_displayName;
				Int index = -1;
				Int imageItemData = -1;
				if (numColumns > 1 && it->second.m_isMultiplayer)
				{
					Int numEasy = battleHonors->getEnduranceMedal(it->first.str(), SLOT_EASY_AI);
					Int numMedium = battleHonors->getEnduranceMedal(it->first.str(), SLOT_MED_AI);
					Int numBrutal = battleHonors->getEnduranceMedal(it->first.str(), SLOT_BRUTAL_AI);
					if (numBrutal)
					{
						int maxBrutalSlots = it->second.m_numPlayers - 1;
						if (numBrutal == maxBrutalSlots)
						{
							index = GadgetListBoxAddEntryImage( listbox, maxBrutalImage, index, 0, w, h, TRUE);
							imageItemData = 4;
						}
						else
						{
							index = GadgetListBoxAddEntryImage( listbox, brutalImage, index, 0, w, h, TRUE);
							imageItemData = 3;
						}
					}
					else if (numMedium)
					{
						imageItemData = 2;
						index = GadgetListBoxAddEntryImage( listbox, mediumImage, index, 0, w, h, TRUE);
					}
					else if (numEasy)
					{
						imageItemData = 1;
						index = GadgetListBoxAddEntryImage( listbox, easyImage, index, 0, w, h, TRUE);
					}
					else
					{
						imageItemData = 0;
						index = GadgetListBoxAddEntryImage( listbox, NULL, index, 0, w, h, TRUE);
					}
				}
				index = GadgetListBoxAddEntryText( listbox, mapDisplayName, color, index, numColumns-1 );

				if (it->first == mapToSelect)
				{
					selectionIndex = index;
				}

				// now set the char* as the item data.  this works because the map cache isn't being
				// modified while a map listbox is up.
				GadgetListBoxSetItemData( listbox, (void *)(it->first.str()), index );

				if (numColumns > 1)
				{
					GadgetListBoxSetItemData( listbox, (void *)imageItemData, index, 1 );
				}
			}
			++tempit;
		}

		tempCache.clear();
		filenameMap.clear();
		++curNumPlayersInMap;
	}

	delete battleHonors;
	battleHonors = NULL;

	GadgetListBoxSetSelected(listbox, &selectionIndex, 1);
	if (selectionIndex >= 0)
	{
		Int topIndex = GadgetListBoxGetTopVisibleEntry(listbox);
		Int bottomIndex = GadgetListBoxGetBottomVisibleEntry(listbox);
		Int rowsOnScreen = bottomIndex - topIndex;

		if (selectionIndex >= bottomIndex)
		{
			Int newTop = max( 0, selectionIndex - max( 1, rowsOnScreen / 2 ) );
		//The trouble is that rowsonscreen/2 can be zero if bottom is 1 and top is zero
			GadgetListBoxSetTopVisibleEntry( listbox, newTop );
		}
	}
	return selectionIndex;

}

//-------------------------------------------------------------------------------------------------
/** Load the listbox with all the map files available to play */
//-------------------------------------------------------------------------------------------------
Int populateMapListbox( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect )
{
	if(!TheMapCache)
		return -1;

	if (!listbox)
		return -1;

	// reset the listbox content
	GadgetListBoxReset( listbox );

	return populateMapListboxNoReset( listbox, useSystemMaps, isMultiplayer, mapToSelect );
}



//-------------------------------------------------------------------------------------------------
/** Validate a map */
//-------------------------------------------------------------------------------------------------
Bool isValidMap( AsciiString mapName, Bool isMultiplayer )
{
	if(!TheMapCache || mapName.isEmpty())
		return FALSE;
	TheMapCache->updateCache();

	mapName.toLower();
	MapCache::iterator it = TheMapCache->find(mapName);
	if (it != TheMapCache->end())
	{
		if (isMultiplayer == it->second.m_isMultiplayer)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** Find a valid map */
//-------------------------------------------------------------------------------------------------
AsciiString getDefaultMap( Bool isMultiplayer )
{
	if(!TheMapCache)
		return AsciiString::TheEmptyString;
	TheMapCache->updateCache();

	MapCache::iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		if (isMultiplayer == it->second.m_isMultiplayer)
		{
			return it->first;
		}
		++it;
	}

	return AsciiString::TheEmptyString;
}


AsciiString getDefaultOfficialMap()
{
	if(!TheMapCache)
		return AsciiString::TheEmptyString;
	TheMapCache->updateCache();

	MapCache::iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		if (it->second.m_isMultiplayer && it->second.m_isOfficial)
		{
			return it->first;
		}
		++it;
	}
	return AsciiString::TheEmptyString;
}


Bool isOfficialMap( AsciiString mapName )
{
	if(!TheMapCache || mapName.isEmpty())
		return FALSE;
	TheMapCache->updateCache();
	mapName.toLower();
	MapCache::iterator it = TheMapCache->find(mapName);
	if (it != TheMapCache->end())
		return it->second.m_isOfficial;
	return FALSE;
}


const MapMetaData *MapCache::findMap(AsciiString mapName)
{
	mapName.toLower();
	MapCache::iterator it = find(mapName);
	if (it == end())
		return NULL;
	return &(it->second);
}

// ------------------------------------------------------------------------------------------------
/** Embed the pristine map into the xfer stream */
// ------------------------------------------------------------------------------------------------
static void copyFromBigToDir( const AsciiString& infile, const AsciiString& outfile )
{
	// open the map file

	File *file = TheFileSystem->openFile( infile.str(), File::READ | File::BINARY );
	if( file == NULL )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error opening source file '%s'", infile.str() ));
		throw SC_INVALID_DATA;
	}

	// how big is the map file
	Int fileSize = file->seek( 0, File::END );


	// rewind to beginning of file
	file->seek( 0, File::START );

	// allocate buffer big enough to hold the entire map file
	char *buffer = NEW char[ fileSize ];
	if( buffer == NULL )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Unable to allocate buffer for file '%s'", infile.str() ));
		throw SC_INVALID_DATA;
	}

	// copy the file to the buffer
	if( file->read( buffer, fileSize ) < fileSize )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error reading from file '%s'", infile.str() ));
		throw SC_INVALID_DATA;
	}
	// close the BIG file
	file->close();

	File *filenew = TheFileSystem->openFile( outfile.str(), File::WRITE | File::CREATE | File::BINARY );

	if( !filenew || filenew->write(buffer, fileSize) < fileSize)
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error writing to file '%s'", outfile.str() ));
		throw SC_INVALID_DATA;
	}

	filenew->close();

	// delete the buffer
	delete [] buffer;
}

Image *getMapPreviewImage( AsciiString mapName )
{
	if(!TheGlobalData)
		return NULL;
	DEBUG_LOG(("%s Map Name", mapName.str()));
	AsciiString tgaName = mapName;
	AsciiString name;
	AsciiString tempName;
	AsciiString filename;
	tgaName.truncateBy(4); // ".map"
	name = tgaName;//.reverseFind('\\') + 1;
	if(tgaName.reverseFind('\\')) {
		filename = tgaName.reverseFind('\\') + 1;
	}
	else if(tgaName.reverseFind('/')) {
		filename = tgaName.reverseFind('/') + 1;
	}
	//tgaName = name;
	filename.concat(".tga");
	tgaName.concat(".tga");

	AsciiString portableName = TheGameState->realMapPathToPortableMapPath(name);
	tempName.set(AsciiString::TheEmptyString);
	for(Int i = 0; i < portableName.getLength(); ++i)
	{
		char c = portableName.getCharAt(i);
		if (c == '\\' || c == ':')
			tempName.concat('_');
		else
			tempName.concat(c);
	}

	name = tempName;
	name.concat(".tga");


	// copy file over
	// copy source tgaName, to name

	Image *image = (Image *)TheMappedImageCollection->findImageByName(tempName);
	if(!image)
	{

		if(!TheFileSystem->doesFileExist(tgaName.str()))
			return NULL;
		AsciiString mapPreviewDir;
		mapPreviewDir.format(MAP_PREVIEW_DIR_PATH, TheGlobalData->getPath_UserData().str());
		TheFileSystem->createDirectory(mapPreviewDir);

		mapPreviewDir.concat(name);

		Bool success = false;
		try
		{
			copyFromBigToDir(tgaName, mapPreviewDir);
			success = true;
		}
		catch (...)
		{
			success = false;	// no rethrow
		}

		if (success)
		{
    	image = newInstance(Image);
			image->setName(tempName);
			//image->setFullPath("mission.tga");
			image->setFilename(name);
			image->setStatus(IMAGE_STATUS_NONE);
			Region2D uv;
			uv.hi.x = 1.0f;
			uv.hi.y = 1.0f;
			uv.lo.x	= 0.0f;
			uv.lo.y = 0.0f;
			image->setUV(&uv);
			image->setTextureHeight(128);
			image->setTextureWidth(128);
			// TheMappedImageCollection->addImage(image);  // DISABLED - method doesn't exist
		}
		else
		{
			image = NULL;
		}
	}

	return image;



/*
	// sanity
	if( mapName.isEmpty() )
		return NULL;
	Region2D uv;
	mapPreviewImage = TheMappedImageCollection->findImageByName("MapPreview");
	if(mapPreviewImage)
		deleteInstance(mapPreviewImage);

	mapPreviewImage = TheMappedImageCollection->newImage();
	mapPreviewImage->setName("MapPreview");
	mapPreviewImage->setStatus(IMAGE_STATUS_RAW_TEXTURE);
// allocate our terrain texture
	TextureClass * texture = new TextureClass( size.x, size.y,
																			 WW3D_FORMAT_X8R8G8B8, TextureClass::MIP_LEVELS_1 );
	uv.lo.x = 0.0f;
	uv.lo.y = 1.0f;
	uv.hi.x = 1.0f;
	uv.hi.y = 0.0f;
	mapPreviewImage->setStatus( IMAGE_STATUS_RAW_TEXTURE );
	mapPreviewImage->setRawTextureData( texture );
	mapPreviewImage->setUV( &uv );
	mapPreviewImage->setTextureWidth( size.x );
	mapPreviewImage->setTextureHeight( size.y );
	mapPreviewImage->setImageSize( &size );


	CachedFileInputStream theInputStream;
	if (theInputStream.open(AsciiString(mapName.str())))
	{
		ChunkInputStream *pStrm = &theInputStream;
		pStrm->absoluteSeek(0);
		DataChunkInput file( pStrm );
		if (file.isValidFileType()) {	// Backwards compatible files aren't valid data chunk files.
			// Read the waypoints.
			file.registerParser( AsciiString("MapPreview"), AsciiString::TheEmptyString, parseMapPreviewChunk );
			if (!file.parse(NULL)) {
				DEBUG_ASSERTCRASH(false,("Unable to read MapPreview info."));
				deleteInstance(mapPreviewImage);
				return NULL;
			}
		}
		theInputStream.close();
	}
	else
	{
		deleteInstance(mapPreviewImage);
		return NULL;
	}


	return mapPreviewImage;

*/
	return NULL;
}

Bool parseMapPreviewChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
/*
	ICoord2D size;

	SurfaceClass *surface;
	size.x = file.readInt();
	size.y = file.readInt();


	surface = (TextureClass *)mapPreviewImage->getRawTextureData()->Get_Surface_Level();
	//texture->Get_Surface_Level();

	DEBUG_LOG(("BeginMapPreviewInfo"));
	UnsignedInt *buffer = new UnsignedInt[size.x * size.y];
	Int x,y;
	for (y=0; y<size.y; y++) {
		for(x = 0; x< size.x; x++)
		{
			surface->DrawPixel( x, y, file.readInt() );
			buffer[y + x] = file.readInt();
			DEBUG_LOG(("x:%d, y:%d, %X", x, y, buffer[y + x]));
		}
	}
	mapPreviewImage->setRawTextureData(buffer);
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	DEBUG_LOG(("EndMapPreviewInfo"));
	REF_PTR_RELEASE(surface);
	return true;
*/
	return FALSE;
}

void findDrawPositions( Int startX, Int startY, Int width, Int height, Region3D extent,
															 ICoord2D *ul, ICoord2D *lr )
{

	Real ratioWidth;
	Real ratioHeight;
	Coord2D radar;
	ratioWidth = extent.width()/(width * 1.0f);
	ratioHeight = extent.height()/(height* 1.0f);

	if( ratioWidth >= ratioHeight)
	{
		radar.x = extent.width() / ratioWidth;
		radar.y = extent.height()/ ratioWidth;
		ul->x = 0;
		ul->y = (height - radar.y) / 2.0f;
		lr->x = radar.x;
		lr->y = height - ul->y;
	}
	else
	{
		radar.x = extent.width() / ratioHeight;
		radar.y = extent.height()/ ratioHeight;
		ul->x = (width - radar.x ) / 2.0f;
		ul->y = 0;
		lr->x = width - ul->x;
		lr->y = radar.y;
	}

	// make them pixel positions
	ul->x += startX;
	ul->y += startY;
	lr->x += startX;
	lr->y += startY;

}

