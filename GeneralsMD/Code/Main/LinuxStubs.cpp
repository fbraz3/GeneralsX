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

/*
** LinuxStubs.cpp
**
** Linux platform stubs for P2-P3 priority features not yet implemented
**
** TheSuperHackers @build felipebraz 13/02/2026
** Phase 1 focuses on getting the game to launch on Linux.
** P2-P3 features like video, web browser, CD manager are stubbed for now.
*/

#ifndef _WIN32

#include <cstdio>
#include "../../../Core/Libraries/Include/Lib/BaseType.h"
#include "../../../Core/GameEngine/Include/Common/AsciiString.h"
#include "../GameEngine/Include/Common/CDManager.h"

// P2: OSDisplay functions (cursor, warning dialogs)

/**
 * OSDisplaySetBusyState - Set cursor busy/not busy (P2)
 * TheSuperHackers @build felipebraz 13/02/2026 - Linux stub
 */
void OSDisplaySetBusyState(bool is_busy, bool force_update)
{
	fprintf(stderr, "INFO: OSDisplaySetBusyState(%s, %s) - Linux stub\n", 
		is_busy ? "true" : "false", force_update ? "true" : "false");
	// On Linux with SDL3, cursor changes are handled via SDL_SetCursor
	// Deferred to future implementation
}

/**
 * OSDisplayWarningBox - Show warning dialog (P2)
 * TheSuperHackers @build felipebraz 13/02/2026 - Linux stub
 */
void OSDisplayWarningBox(AsciiString title, AsciiString message, unsigned int icon_type, unsigned int button_type)
{
	fprintf(stderr, "WARNING: OSDisplayWarningBox('%s', '%s') - Linux stub\n", 
		title.str() ? title.str() : "(null)", 
		message.str() ? message.str() : "(null)");
	// On Linux with SDL3, dialog boxes would use native system dialogs
	// For now, we print to stderr and continue
}

// P3: Legacy features (CD-ROM manager, video player, web browser)

/**
 * CreateCDManager - Initialize CD-ROM manager (P3)
 * TheSuperHackers @build felipebraz 13/02/2026 - Linux stub
 * Legacy feature: Game originally checked for Zero Hour CD in drive
 * On Linux with DXVK, no CD-ROM support needed
 */
CDManagerInterface* CreateCDManager(void)
{
	fprintf(stderr, "WARNING: CreateCDManager() called - returning nullptr (no CD support on Linux)\n");
	return nullptr;  // No CD-ROM support on Linux
}

#endif // !_WIN32
