/*
**	Command & Conquer Generals(tm)
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
** Compatibility shim for Generals (base game)
** This provides stub definitions for WWShade functionality
** The actual implementation exists only in GeneralsMD (Zero Hour expansion)
*/

#pragma once

// Stub definitions for WWShade compatibility
// All macros expand to empty since USE_WWSHADE is not used in base Generals

#define SHD_INIT
#define SHD_SHUTDOWN
#define SHD_INIT_SHADERS
#define SHD_SHUTDOWN_SHADERS
#define SHD_FLUSH
#define SHD_REG_LOADER
