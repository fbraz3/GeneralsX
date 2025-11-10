/* Minimal d3d8caps.h stub for Phase 50 builds on non-Windows platforms.
   This supplies tiny definitions for D3DCAPS8 and D3DADAPTER_IDENTIFIER8
   used as members in RenderDeviceDescClass. Keep minimal to avoid
   polluting real DirectX definitions.
*/
#ifndef D3D8CAPS_STUB_H
#define D3D8CAPS_STUB_H

#include "d3d8types.h" // ensure DWORD, LONG, etc. are available

// Minimal adapter identifier structure. Real code may ignore most fields.
#ifndef D3DADAPTER_IDENTIFIER8_DEFINED
#define D3DADAPTER_IDENTIFIER8_DEFINED
struct D3DADAPTER_IDENTIFIER8 {
    char Driver[512];
    char Description[512];
    char DeviceName[512];
    // Other fields intentionally omitted
};
#endif

// Minimal caps structure. Real code reads specific fields; add common ones.
#ifndef D3DCAPS8_DEFINED
#define D3DCAPS8_DEFINED
struct D3DCAPS8 {
    DWORD DeviceType;
    DWORD Caps;
    DWORD PrimitiveMiscCaps;
    DWORD RasterCaps;
    DWORD ZCmpCaps;
    DWORD SrcBlendCaps;
    DWORD DestBlendCaps;
    // Keep compact; additional fields can be added if a TU requires them
};
#endif

#endif // D3D8CAPS_STUB_H
