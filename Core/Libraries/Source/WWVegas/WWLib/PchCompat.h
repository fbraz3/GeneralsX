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
 * PchCompat.h - Precompiled Header Compatibility Layer (Core/Libraries)
 * ====================================================================
 * 
 * This file provides platform-safe headers for the WWLib precompiled header.
 * On Windows, it includes necessary Windows-specific headers.
 * On Linux/Unix, these headers are replaced with stubs or platform compatibility shims.
 * 
 * TheSuperHackers @build 08/02/2026
 * Phase 1 Linux Port - Graphics Layer (DXVK)
 */

#pragma once

#ifdef _WIN32
    // Windows-specific precompiled headers
    #include <windows.h>
#else
    // Linux/Unix platform - provide compatibility stubs
    // TheSuperHackers @build BenderAI 13/02/2026 Include windows_compat.h for Linux builds
    // Provides __max/__min macros, threading APIs, file I/O stubs, etc.
    #include "windows_compat.h"
#endif
