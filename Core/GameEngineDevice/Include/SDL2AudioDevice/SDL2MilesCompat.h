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

#pragma once
#ifndef SDL2_MILES_COMPAT_H
#define SDL2_MILES_COMPAT_H

/**
 * @file SDL2MilesCompat.h
 * @brief Compatibility layer between win32_compat.h and Miles Sound System headers
 * 
 * This header resolves typedef conflicts between the game's Windows compatibility
 * layer and the Miles Sound System stub library by ensuring Miles types are
 * defined before including win32_compat.h.
 */

// Include Miles Sound System FIRST to establish its types
// Define sentinel macro before including Miles SDK to prevent typedef conflicts
#define MILES_SOUND_SYSTEM_TYPES_DEFINED
#include <mss/mss.h>

// Then include the game's Windows compatibility layer
// (which will respect Miles' existing typedefs)
#include "Common/AsciiString.h"

#endif // SDL2_MILES_COMPAT_H
