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

#include <cstdarg>
#include <cstdio>

#include "Common/AddonCompat.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/Registry.h"
#include "Common/FileSystem.h"
#include "Common/UserPreferences.h"

#include "GameClient/Display.h"
#include "GameClient/GlobalLanguage.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GlobalLanguage *TheGlobalLanguageData = NULL;				///< The global language singleton

static void boot_trace_log(const char *fmt, ...)
{
	FILE *fp = std::fopen("boot_trace.log", "a");
	if (!fp) {
		return;
	}

	SYSTEMTIME st;
	::GetLocalTime(&st);
	std::fprintf(fp, "[%02u:%02u:%02u.%03u] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	va_list args;
	va_start(args, fmt);
	std::vfprintf(fp, fmt, args);
	va_end(args);

	std::fprintf(fp, "\n");
	std::fflush(fp);
	std::fclose(fp);
}

static const LookupListRec ResolutionFontSizeMethodNames[] =
{
	{ "CLASSIC", GlobalLanguage::ResolutionFontSizeMethod_Classic },
	{ "CLASSIC_NO_CEILING", GlobalLanguage::ResolutionFontSizeMethod_ClassicNoCeiling },
	{ "STRICT", GlobalLanguage::ResolutionFontSizeMethod_Strict },
	{ "BALANCED", GlobalLanguage::ResolutionFontSizeMethod_Balanced },
	{ NULL, 0 }
};

static const FieldParse TheGlobalLanguageDataFieldParseTable[] =
{
	{ "UnicodeFontName",									INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontName ) },
	//{	"UnicodeFontFileName",							INI::parseAsciiString,NULL,									offsetof( GlobalLanguage, m_unicodeFontFileName ) },
	{ "LocalFontFile",										GlobalLanguage::parseFontFileName,					NULL,			0},
	{ "MilitaryCaptionSpeed",						INI::parseInt,					NULL,		offsetof( GlobalLanguage, m_militaryCaptionSpeed ) },
	{ "UseHardWordWrap",						INI::parseBool,					NULL,		offsetof( GlobalLanguage, m_useHardWrap) },
	{ "ResolutionFontAdjustment",						INI::parseReal,					NULL,		offsetof( GlobalLanguage, m_resolutionFontSizeAdjustment) },
	{ "ResolutionFontSizeMethod", INI::parseLookupList, ResolutionFontSizeMethodNames, offsetof( GlobalLanguage, m_resolutionFontSizeMethod) },
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

	{ NULL,					NULL,						NULL,						0 }
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
	}
	// parse the ini weapon definition
	ini->initFromINI( TheGlobalLanguageData, TheGlobalLanguageDataFieldParseTable );
}

GlobalLanguage::GlobalLanguage()
{
	boot_trace_log("[GlobalLanguage::GlobalLanguage] CONSTRUCTOR START");

	m_unicodeFontName.clear();
	m_unicodeFontFileName.clear();
	m_unicodeFontName.clear();
	m_militaryCaptionSpeed = 0;
	m_useHardWrap = FALSE;
	m_resolutionFontSizeAdjustment = 0.7f;
	m_resolutionFontSizeMethod = ResolutionFontSizeMethod_Default;
	m_militaryCaptionDelayMS = 750;

	m_userResolutionFontSizeAdjustment = -1.0f;

	boot_trace_log("[GlobalLanguage::GlobalLanguage] CONSTRUCTOR END");
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
	boot_trace_log("[GlobalLanguage::init] STARTING");

	{
		const AsciiString requestedLanguage = GetRegistryLanguage();
		boot_trace_log("[GlobalLanguage::init] Requested language: %s", requestedLanguage.str());

		const AsciiString fallbacks[] = {
			requestedLanguage,
			"Brazilian",
			"English",
		};

		Bool loaded = FALSE;
		AsciiString lastTried;

		for (UnsignedInt i = 0; i < ARRAY_SIZE(fallbacks); ++i)
		{
			const AsciiString lang = fallbacks[i];
			if (lang.isEmpty()) {
				continue;
			}
			if (lang.compareNoCase(lastTried) == 0) {
				continue;
			}
			lastTried = lang;

			AsciiString fname;
			fname.format("Data\\%s\\Language", lang.str());
			boot_trace_log("[GlobalLanguage::init] Attempting to load Language INI from: %s", fname.str());

			INI ini;
			try {
				ini.loadFileDirectory( fname, INI_LOAD_OVERWRITE, NULL );
				boot_trace_log("[GlobalLanguage::init] INI loaded successfully (language=%s)", lang.str());
				loaded = TRUE;
				break;
			}
			catch (...) {
				boot_trace_log("[GlobalLanguage::init] INI load failed (language=%s)", lang.str());
			}
		}

		if (!loaded)
		{
			boot_trace_log("[GlobalLanguage::init] ERROR: No Language INI could be loaded for requested language '%s'", requestedLanguage.str());
			throw INIException("Could not load any Language INI. Check that language assets exist under Data\\<Language> (e.g. Data\\Brazilian or Data\\English).");
		}
	}
	boot_trace_log("[GlobalLanguage::init] About to iterate m_localFonts (count=%zu)", m_localFonts.size());

	StringListIt it = m_localFonts.begin();
	while( it != m_localFonts.end())
	{
		AsciiString font = *it;
		boot_trace_log("[GlobalLanguage::init] Processing font: %s", font.str());

		if(AddFontResource(font.str()) == 0)
		{
			DWORD lastErr = ::GetLastError();
			boot_trace_log("[GlobalLanguage::init] AddFontResource FAILED for: %s (GetLastError=%lu)", font.str(), (unsigned long)lastErr);
			DEBUG_ASSERTCRASH(FALSE,("GlobalLanguage::init Failed to add font %s", font.str()));
		}
		else
		{
			boot_trace_log("[GlobalLanguage::init] AddFontResource SUCCESS for: %s", font.str());
			//SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
		}
		++it;
	}
	boot_trace_log("[GlobalLanguage::init] Loading user preferences");

	// override values with user preferences
	OptionPreferences optionPref;
	m_userResolutionFontSizeAdjustment = optionPref.getResolutionFontAdjustment();
	boot_trace_log("[GlobalLanguage::init] COMPLETED");
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

float GlobalLanguage::getResolutionFontSizeAdjustment( void ) const
{
	if (m_userResolutionFontSizeAdjustment >= 0.0f)
		return m_userResolutionFontSizeAdjustment;
	else
		return m_resolutionFontSizeAdjustment;
}

Int GlobalLanguage::adjustFontSize(Int theFontSize)
{
	// TheSuperHackers @todo This function is called very often.
	// Therefore cache the adjustFactor on resolution change to not recompute it on every call.
	Real adjustFactor;

	switch (m_resolutionFontSizeMethod)
	{
	default:
	case ResolutionFontSizeMethod_Classic:
	{
		// TheSuperHackers @info The original font scaling for this game.
		// Useful for not breaking legacy Addons and Mods. Scales poorly with large resolutions.
		adjustFactor = TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH;
		adjustFactor = 1.0f + (adjustFactor - 1.0f) * getResolutionFontSizeAdjustment();
		if (adjustFactor > 2.0f)
			adjustFactor = 2.0f;
		break;
	}
	case ResolutionFontSizeMethod_ClassicNoCeiling:
	{
		// TheSuperHackers @feature The original font scaling, but without ceiling.
		// Useful for not changing the original look of the game. Scales alright with large resolutions.
		adjustFactor = TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH;
		adjustFactor = 1.0f + (adjustFactor - 1.0f) * getResolutionFontSizeAdjustment();
		break;
	}
	case ResolutionFontSizeMethod_Strict:
	{
		// TheSuperHackers @feature The strict method scales fonts based on the smallest screen
		// dimension so they scale independent of aspect ratio.
		const Real wScale = TheDisplay->getWidth() / (Real)DEFAULT_DISPLAY_WIDTH;
		const Real hScale = TheDisplay->getHeight() / (Real)DEFAULT_DISPLAY_HEIGHT;
		adjustFactor = min(wScale, hScale);
		adjustFactor = 1.0f + (adjustFactor - 1.0f) * getResolutionFontSizeAdjustment();
		break;
	}
	case ResolutionFontSizeMethod_Balanced:
	{
		// TheSuperHackers @feature The balanced method evenly weighs the display width and height
		// for a balanced rescale on non 4:3 resolutions. The aspect ratio scaling is clamped to
		// prevent oversizing.
		constexpr const Real maxAspect = 1.8f;
		constexpr const Real minAspect = 1.0f;
		Real w = TheDisplay->getWidth();
		Real h = TheDisplay->getHeight();
		const Real aspect = w / h;
		Real wScale = w / (Real)DEFAULT_DISPLAY_WIDTH;
		Real hScale = h / (Real)DEFAULT_DISPLAY_HEIGHT;

		if (aspect > maxAspect)
		{
			// Recompute width at max aspect
			w = maxAspect * h;
			wScale = w / (Real)DEFAULT_DISPLAY_WIDTH;
		}
		else if (aspect < minAspect)
		{
			// Recompute height at min aspect
			h = minAspect * w;
			hScale = h / (Real)DEFAULT_DISPLAY_HEIGHT;
		}
		adjustFactor = (wScale + hScale) * 0.5f;
		adjustFactor = 1.0f + (adjustFactor - 1.0f) * getResolutionFontSizeAdjustment();
		break;
	}
	}

	if (adjustFactor < 1.0f)
		adjustFactor = 1.0f;
	Int pointSize = REAL_TO_INT_FLOOR(theFontSize*adjustFactor);
	return pointSize;
}

void GlobalLanguage::parseCustomDefinition()
{
	if (addon::HasFullviewportDat())
	{
		// TheSuperHackers @tweak xezon 19/08/2025 Force the classic font size adjustment for the old
		// 'Control Bar Pro' Addons because they use manual font upscaling in higher resolution packages.
		m_resolutionFontSizeMethod = ResolutionFontSizeMethod_Classic;
	}
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

