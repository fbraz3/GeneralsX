#pragma once

#ifndef D3DX8TEX_H_INCLUDED
#define D3DX8TEX_H_INCLUDED

// Simple compatibility header for d3dx8tex.h on non-Windows platforms
// This provides minimal stubs for D3DX texture functions

#ifndef _WIN32

// Include our DirectX compatibility layer
#include "d3d8.h"

// D3DX texture function stubs (already defined in d3d8.h)
// This header exists to provide compatibility when d3dx8tex.h is included

#else
// On Windows, include the real D3DX texture header
#include <d3dx8tex.h>
#endif // !_WIN32

#endif // D3DX8TEX_H_INCLUDED
