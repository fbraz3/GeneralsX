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
** D3D8Stub.h
**
** Minimal DirectX8 stub for Linux builds using DXVK.
**
** TheSuperHackers @build 15/12/2024
** This header provides minimal DirectX8 types when SAGE_USE_OPENAL is defined.
** These are stubs only - actual rendering is handled by DXVK.
*/

#pragma once

#if defined(SAGE_USE_OPENAL) && !defined(_D3D8_STUB_H)
#define _D3D8_STUB_H

// Minimal DirectX types (stubs for texture format detection)
typedef unsigned int D3DFORMAT;
typedef unsigned int D3DTEXTUREFILTERTYPE;
typedef unsigned int D3DTEXTUREADDRESS;
typedef unsigned int D3DBLEND;

// D3D Format constants (subset used by texture.cpp)
#define D3DFMT_R8G8B8        20
#define D3DFMT_A8R8G8B8      21
#define D3DFMT_X8R8G8B8      22
#define D3DFMT_R5G6B5        23
#define D3DFMT_X1R5G5B5      24
#define D3DFMT_A1R5G5B5      25
#define D3DFMT_A4R4G4B4      26
#define D3DFMT_R3G3B2        27
#define D3DFMT_A8P8          28
#define D3DFMT_P8            29
#define D3DFMT_A8            30
#define D3DFMT_L8            31
#define D3DFMT_A8L8          32
#define D3DFMT_L5A3          33
#define D3DFMT_DXT1          0x31545844
#define D3DFMT_DXT2          0x32545844
#define D3DFMT_DXT3          0x33545844
#define D3DFMT_DXT4          0x34545844
#define D3DFMT_DXT5          0x35545844

// Texture filter types
#define D3DTEXF_NONE         0
#define D3DTEXF_POINT        1
#define D3DTEXF_LINEAR       2
#define D3DTEXF_ANISOTROPIC  3

// Texture address modes
#define D3DTADDRESS_WRAP     1
#define D3DTADDRESS_MIRROR   2
#define D3DTADDRESS_CLAMP    3
#define D3DTADDRESS_BORDER   4

// Blend factors
#define D3DBLEND_ZERO        1
#define D3DBLEND_ONE         2
#define D3DBLEND_SRCCOLOR    3
#define D3DBLEND_INVSRCCOLOR 4
#define D3DBLEND_SRCALPHA    5
#define D3DBLEND_INVSRCALPHA 6

// TheSuperHackers @build 09/02/2026 Windows string functions stubs for Linux
// lstrcpyn and lstrcat are Windows-only, map to standard C functions
#ifdef __linux__
#define lstrcpyn(dst, src, len) strncpy(dst, src, len-1); dst[len-1] = '\0'
#define lstrcat(dst, src) strcat(dst, src)
#endif

#endif
