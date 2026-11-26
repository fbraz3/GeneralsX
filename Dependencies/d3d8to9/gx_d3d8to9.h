// GeneralsX: the two entry points of the d3d8to9 static library compiled into generalszh.exe (compatibility graphics:
// a PC with no Vulkan renders through Windows' own d3d9.dll). Plain C types only, so a caller that includes the
// engine's d3d8.h never meets d3d8to9's own d3d8.hpp in the same translation unit.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// An IDirect3D8* (as void*) that translates to the System32 d3d9.dll, or null with the reason written into why.
void *gx_d3d8to9_Direct3DCreate8(unsigned int sdkVersion, char *why, unsigned int whyLen);

// The full path of the d3d9.dll the call above loaded, or an empty string before it ran or when it failed to load.
const wchar_t *gx_d3d8to9_LoadedD3D9Path(void);

#ifdef __cplusplus
}
#endif
