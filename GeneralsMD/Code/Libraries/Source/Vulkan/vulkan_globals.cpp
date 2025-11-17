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

/**
 * vulkan_globals.cpp
 * 
 * Global variables required by DX8Wrapper and other systems.
 * These are defined here to satisfy linker requirements during Phase 39.3
 * graphics backend migration from DirectX8 to Vulkan.
 * 
 * Phase 39.3 Stage 3: D3D8 Method Implementation
 */

// Global counter for D3D8 API calls
unsigned number_of_DX8_calls = 0;

// Thread hosting flag for multiplayer support
int isThreadHosting = 0;

// Placeholder for AsciiString g_LastErrorDump
// This would normally be defined in StackDump.cpp on Windows only
// We can't define AsciiString here without including complex headers
// Instead, we rely on linker-defined weak symbol or stub it out
extern "C" {
    __attribute__((weak)) void* _g_LastErrorDump = nullptr;
}
