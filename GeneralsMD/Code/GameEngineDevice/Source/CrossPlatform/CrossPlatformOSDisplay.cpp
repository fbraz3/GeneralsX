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
//  Phase 54: Cross-platform OSDisplay implementation for Linux/macOS         //
//																																						//
////////////////////////////////////////////////////////////////////////////////

// CrossPlatformOSDisplay.cpp /////////////////////////////////
// Cross-platform stubs for OS display functions
// Linux/macOS: Returns OSDBT_CANCEL to break retry loops (e.g., CD loading)
///////////////////////////////////////////////////////////////

#include "Common/OSDisplay.h"
#include "Common/AsciiString.h"

#include <cstdio>

//-------------------------------------------------------------------------------------------------
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
	// Log the warning to console instead of showing a dialog
	fprintf(stderr, "[OSDisplayWarningBox] %s: %s\n", p.str(), m.str());
	
	// Return CANCEL to break any retry loops (e.g., CD loading loop)
	// This matches the macOS behavior from Phase 051.1
	return OSDBT_CANCEL;
}

//-------------------------------------------------------------------------------------------------
void OSDisplaySetBusyState(Bool busyDisplay, Bool busySystem)
{
	// On Linux/macOS, we don't need to prevent screen sleep or system idle
	// SDL2 handles this automatically when in fullscreen mode
	// This is a no-op stub
	(void)busyDisplay;
	(void)busySystem;
}
