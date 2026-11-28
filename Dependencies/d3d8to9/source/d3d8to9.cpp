/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3dx9.hpp"
#include "d3d8to9.hpp"

PFN_D3DXAssembleShader D3DXAssembleShader = nullptr;
PFN_D3DXDisassembleShader D3DXDisassembleShader = nullptr;
PFN_D3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface = nullptr;

#ifndef D3D8TO9NOLOG
 // Very simple logging for the purpose of debugging only.
std::ofstream LOG;
#endif

extern "C" HRESULT WINAPI ValidatePixelShader(const DWORD* pPixelShader, const D3DCAPS8* pCaps, BOOL ReturnErrors, char** pErrorsString)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "ValidatePixelShader " << "(" << pPixelShader << ", " << pCaps << ", " << ReturnErrors << ", " << pErrorsString << ")' ..." << std::endl;
#endif

	HRESULT hr = E_FAIL;
	const char* errorMessage = "";

	if (!pPixelShader)
	{
		errorMessage = "Invalid code pointer.\n";
	}
	else
	{
		switch (*pPixelShader)
		{
		case D3DPS_VERSION(1, 0):
		case D3DPS_VERSION(1, 1):
		case D3DPS_VERSION(1, 2):
		case D3DPS_VERSION(1, 3):
		case D3DPS_VERSION(1, 4):
			if (pCaps && *pPixelShader > pCaps->PixelShaderVersion)
			{
				errorMessage = "Shader version not supported by caps.\n";
				break;
			}
			hr = S_OK;
			break;

		default:
			errorMessage = "Unsupported shader version.\n";
		}
	}

	if (!ReturnErrors)
	{
		errorMessage = "";
	}

	if (pErrorsString)
	{
		const size_t size = strlen(errorMessage) + 1;

		*pErrorsString = (char*) HeapAlloc(GetProcessHeap(), 0, size);
		if (*pErrorsString)
		{
			memcpy(*pErrorsString, errorMessage, size);
		}
	}

	return hr;
}

extern "C" HRESULT WINAPI ValidateVertexShader(const DWORD* pVertexShader, const DWORD* pVertexDecl, const D3DCAPS8* pCaps, BOOL ReturnErrors, char** pErrorsString)
{
	UNREFERENCED_PARAMETER(pVertexDecl);

#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "ValidateVertexShader " << "(" << pVertexShader << ", " << pVertexDecl << ", " << pCaps << ", " << ReturnErrors << ", " << pErrorsString << ")' ..." << std::endl;
#endif

	HRESULT hr = E_FAIL;
	const char* errorMessage = "";

	if (!pVertexShader)
	{
		errorMessage = "Invalid code pointer.\n";
	}
	else
	{
		switch (*pVertexShader)
		{
		case D3DVS_VERSION(1, 0):
		case D3DVS_VERSION(1, 1):
			if (pCaps && *pVertexShader > pCaps->VertexShaderVersion)
			{
				errorMessage = "Shader version not supported by caps.\n";
				break;
			}
			hr = S_OK;
			break;

		default:
			errorMessage = "Unsupported shader version.\n";
		}
	}

	if (!ReturnErrors)
	{
		errorMessage = "";
	}

	if (pErrorsString)
	{
		const size_t size = strlen(errorMessage) + 1;

		*pErrorsString = (char*) HeapAlloc(GetProcessHeap(), 0, size);
		if (*pErrorsString)
		{
			memcpy(*pErrorsString, errorMessage, size);
		}
	}

	return hr;
}

extern "C" void WINAPI DebugSetMute()
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "DebugSetMute" << "(" << ")' ..." << std::endl;
#endif
}

// GeneralsX: compiled into generalszh.exe, not a d3d8.dll. The entry point is renamed and nothing is exported, Windows'
// own d3d9.dll is loaded by its full System32 path (the app-local d3d9.dll beside the exe is DXVK, which needs Vulkan),
// and nothing here may show a box, open a browser or write a file into the install folder (patch 0001).
#include <cstdio>
#include <cwchar>

static wchar_t s_gxD3D9Path[MAX_PATH] = L"";
extern "C" const wchar_t *gx_d3d8to9_LoadedD3D9Path(void) { return s_gxD3D9Path; }

// System32\<name>, its dependencies searched in System32 only. outPath gets the path of the module actually loaded.
static HMODULE gxLoadSystemDll(const wchar_t *name, wchar_t *outPath)
{
	wchar_t dir[MAX_PATH], path[MAX_PATH];
	const UINT n = GetSystemDirectoryW(dir, MAX_PATH);
	if (n == 0 || n >= MAX_PATH || n + 1 + wcslen(name) >= MAX_PATH)
		return nullptr;
	swprintf_s(path, MAX_PATH, L"%ls\\%ls", dir, name);
	const HMODULE module = LoadLibraryExW(path, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (module != nullptr && outPath != nullptr && GetModuleFileNameW(module, outPath, MAX_PATH) == 0)
		wcscpy_s(outPath, MAX_PATH, path);
	return module;
}

extern "C" void *gx_d3d8to9_Direct3DCreate8(UINT SDKVersion, char *why, unsigned int whyLen)
{
	UNREFERENCED_PARAMETER(SDKVersion);

	typedef IDirect3D9 *(WINAPI *PFN_Direct3DCreate9)(UINT);
	const HMODULE d3d9 = gxLoadSystemDll(L"d3d9.dll", s_gxD3D9Path);
	const PFN_Direct3DCreate9 create9 = d3d9 != nullptr ? reinterpret_cast<PFN_Direct3DCreate9>(GetProcAddress(d3d9, "Direct3DCreate9")) : nullptr;
	if (create9 == nullptr)
	{
		snprintf(why, whyLen, "system d3d9.dll not loadable (error %lu)", GetLastError());
		return nullptr;
	}

	IDirect3D9 *const d3d = create9(D3D_SDK_VERSION);

	if (d3d == nullptr)
	{
		snprintf(why, whyLen, "Direct3DCreate9 returned null");
		return nullptr;
	}

	// D3DX is optional and used only for D3DXLoadSurfaceFromSurface, CopyRects' fallback between formats. Shaders never
	// need it (patch 0003). Missing is normal on a fresh Windows: CopyRects then fails for those copies alone.
	if (D3DXLoadSurfaceFromSurface == nullptr)
	{
		wchar_t d3dxPath[MAX_PATH];
		const HMODULE module = gxLoadSystemDll(L"d3dx9_43.dll", d3dxPath);

		if (module != nullptr)
			D3DXLoadSurfaceFromSurface = reinterpret_cast<PFN_D3DXLoadSurfaceFromSurface>(GetProcAddress(module, "D3DXLoadSurfaceFromSurface"));
	}

	return new Direct3D8(d3d);
}
