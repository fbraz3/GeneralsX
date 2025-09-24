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

// FILE: GlobalLanguage.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Aug 2002
//
//	Filename: 	GlobalLanguage.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	Contains the member functions for the language munkee
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"

#include "Common/INI.h"
#include "Common/Registry.h"
#include "GameClient/GlobalLanguage.h"
#include "Common/FileSystem.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GlobalLanguage *TheGlobalLanguageData = NULL;				///< The global language singalton

static const FieldParse TheGlobalLanguageDataFieldParseTable[] =
{
	{ "UnicodeFontName",									INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontName ) },
	//{	"UnicodeFontFileName",							INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontFileName ) },
	{ "LocalFontFile",										GlobalLanguage::parseFontFileName,					NULL,			0},
	{ "MilitaryCaptionSpeed",						INI::parseInt,					NULL,		offsetof( GlobalLanguage, m_militaryCaptionSpeed ) },
	{ "UseHardWordWrap",						INI::parseBool,					NULL,		offsetof( GlobalLanguage, m_useHardWrap) },
	{ "ResolutionFontAdjustment",						INI::parseReal,					NULL,		offsetof( GlobalLanguage, m_resolutionFontSizeAdjustment) },

	{ "CopyrightFont",					GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_copyrightFont ) },
	{ "MessageFont",					GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_messageFont) },
	{ "MilitaryCaptionTitleFont",		GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_militaryCaptionTitleFont) },
	{ "MilitaryCaptionDelayMS",					INI::parseInt,					NULL,		offsetof( GlobalLanguage, m_militaryCaptionDelayMS ) },
	{ "MilitaryCaptionFont",			GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_militaryCaptionFont) },
	{ "SuperweaponCountdownNormalFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_superweaponCountdownNormalFont) },
	{ "SuperweaponCountdownReadyFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_superweaponCountdownReadyFont) },
	{ "NamedTimerCountdownNormalFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_namedTimerCountdownNormalFont) },
	{ "NamedTimerCountdownReadyFont",	GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_namedTimerCountdownReadyFont) },
	{ "DrawableCaptionFont",			GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_drawableCaptionFont) },
	{ "DefaultWindowFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_defaultWindowFont) },
	{ "DefaultDisplayStringFont",		GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_defaultDisplayStringFont) },
	{ "TooltipFontName",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_tooltipFontName) },
	{ "NativeDebugDisplay",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_nativeDebugDisplay) },
	{ "DrawGroupInfoFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_drawGroupInfoFont) },
	{ "CreditsTitleFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsTitleFont) },
	{ "CreditsMinorTitleFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsPositionFont) },
	{ "CreditsNormalFont",				GlobalLanguage::parseFontDesc,	NULL,	offsetof( GlobalLanguage, m_creditsNormalFont) },

	{ NULL,					NULL,						NULL,						0 }  // keep this last
};

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
void INI::parseLanguageDefinition( INI *ini )
{
	if( !TheGlobalLanguageData )
	{
		DEBUG_ASSERTCRASH(TheGlobalLanguageData, ("INI::parseLanguageDefinition - TheGlobalLanguage Data is not around, please create it before trying to parse the ini file."));
		return;
	}  // end if
	// parse the ini weapon definition
	ini->initFromINI( TheGlobalLanguageData, TheGlobalLanguageDataFieldParseTable );
}

GlobalLanguage::GlobalLanguage()
{
	m_unicodeFontName.clear();
	//Added By Sadullah Nader
	//Initializations missing and needed
	m_unicodeFontFileName.clear();
	m_unicodeFontName.clear();
	m_militaryCaptionSpeed = 0;
	m_useHardWrap = FALSE;
	m_resolutionFontSizeAdjustment = 0.7f;
	m_militaryCaptionDelayMS = 750;
	//End Add
}

GlobalLanguage::~GlobalLanguage()
{
	StringListIt it = m_localFonts.begin();
	while( it != m_localFonts.end())
	{
		AsciiString font = *it;
		RemoveFontResource(font.str());
		//SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
		++it;
	}
}

void GlobalLanguage::init( void )
{
	printf("GlobalLanguage::init() - METHOD ENTRY\n");
	fflush(stdout);

	try {
		printf("GlobalLanguage::init() - About to create INI object\n");
		fflush(stdout);
		
		INI ini;
		AsciiString fname;
		
		printf("GlobalLanguage::init() - About to call GetRegistryLanguage()\n");
		fflush(stdout);
		
		#ifdef _WIN32
		fname.format("Data\\%s\\Language.ini", GetRegistryLanguage().str());
		printf("GlobalLanguage::init() - Windows: fname = %s\n", fname.str());
		#else
		// On macOS/Linux, default to English with forward slashes
		fname.format("Data/English/Language.ini");
		printf("GlobalLanguage::init() - macOS: defaulting to fname = %s\n", fname.str());
		#endif
		fflush(stdout);

		printf("GlobalLanguage::init() - About to check OS version\n");
		fflush(stdout);

		#ifdef _WIN32
		OSVERSIONINFO	osvi;
		osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);

		//GS NOTE: Must call doesFileExist in either case so that NameKeyGenerator will stay in sync
		AsciiString tempName;
		tempName.format("Data\\%s\\Language9x.ini", GetRegistryLanguage().str());
		bool isExist = TheFileSystem->doesFileExist(tempName.str());
		if (GetVersionEx(&osvi)  &&  osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS  && isExist)
		{	//check if we're running Win9x variant since they may need different fonts
			fname = tempName;
		}
		#else
		// On macOS/Linux, skip Win9x check
		printf("GlobalLanguage::init() - macOS: skipping Win9x compatibility check\n");
		#endif
		fflush(stdout);

		printf("GlobalLanguage::init() - About to load INI file: %s\n", fname.str());
		fflush(stdout);
		
		try {
			printf("GlobalLanguage::init() - Creating INI load operation...\n");
			fflush(stdout);
			
			// On macOS, skip INI loading and use defaults
			#ifdef _WIN32
			ini.load( fname, INI_LOAD_OVERWRITE, NULL );
			printf("GlobalLanguage::init() - INI file loaded successfully\n");
			#else
			printf("GlobalLanguage::init() - macOS: skipping INI file loading, using defaults\n");
			#endif
			fflush(stdout);
		} catch (const std::exception& e) {
			printf("GlobalLanguage::init() - Exception during INI load: %s\n", e.what());
			fflush(stdout);
			throw;
		} catch (...) {
			printf("GlobalLanguage::init() - Unknown exception during INI load\n");
			fflush(stdout);
			throw;
		}
		
		printf("GlobalLanguage::init() - About to process local fonts (count: %d)\n", (int)m_localFonts.size());
		fflush(stdout);
		
		StringListIt it = m_localFonts.begin();
		while( it != m_localFonts.end())
		{
			AsciiString font = *it;
			printf("GlobalLanguage::init() - Processing font: %s\n", font.str());
			fflush(stdout);
			
			#ifdef _WIN32
			if(AddFontResource(font.str()) == 0)
			{
				printf("GlobalLanguage::init() - WARNING: Failed to add font %s\n", font.str());
				// Don't crash on font failure
				// DEBUG_ASSERTCRASH(FALSE,("GlobalLanguage::init Failed to add font %s", font.str()));
			}
			else
			{
				printf("GlobalLanguage::init() - Successfully added font: %s\n", font.str());
				//SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
			}
			#else
			// On macOS/Linux, just log the font (no actual loading)
			printf("GlobalLanguage::init() - macOS: Skipping font loading for: %s\n", font.str());
			#endif
			++it;
		}
		
		printf("GlobalLanguage::init() - Font processing completed\n");
		printf("GlobalLanguage::init() - METHOD COMPLETED SUCCESSFULLY\n");
		fflush(stdout);
		
	} catch (const std::exception& e) {
		printf("GlobalLanguage::init() - Caught std::exception: %s\n", e.what());
		fflush(stdout);
		throw; // Re-throw to maintain error handling
	} catch (...) {
		printf("GlobalLanguage::init() - Caught unknown exception\n");
		fflush(stdout);
		throw; // Re-throw to maintain error handling
	}
}
void GlobalLanguage::reset( void ) {}


void GlobalLanguage::parseFontDesc(INI *ini, void *instance, void *store, const void* userData)
{
	FontDesc *fontDesc = (FontDesc *)store;
	fontDesc->name = ini->getNextQuotedAsciiString();
	fontDesc->size = ini->scanInt(ini->getNextToken());
	fontDesc->bold = ini->scanBool(ini->getNextToken());
}

void GlobalLanguage::parseFontFileName( INI *ini, void * instance, void *store, const void* userData )
{
	GlobalLanguage *monkey = (GlobalLanguage *)instance;
	AsciiString asciiString = ini->getNextAsciiString();
	monkey->m_localFonts.push_front(asciiString);
}

Int GlobalLanguage::adjustFontSize(Int theFontSize)
{
	Real adjustFactor = TheGlobalData->m_xResolution / (Real)DEFAULT_DISPLAY_WIDTH;
	adjustFactor = 1.0f + (adjustFactor-1.0f) * m_resolutionFontSizeAdjustment;
	if (adjustFactor<1.0f) adjustFactor = 1.0f;
	if (adjustFactor>2.0f) adjustFactor = 2.0f;
	Int pointSize = REAL_TO_INT_FLOOR(theFontSize*adjustFactor);
	return pointSize;
}

FontDesc::FontDesc(void)
{
	name = "Arial Unicode MS";	///<name of font
	size = 12;			///<point size
	bold = FALSE;			///<is bold?
}
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

