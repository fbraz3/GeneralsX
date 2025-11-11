#pragma once

/**
 * @file d3d8_types.h
 * @brief Minimal DirectX 8 Type Definitions - ONLY types needed by d3d8_interfaces.h
 * 
 * This header provides ONLY:
 * - IID: Interface ID structure for COM
 * - RGNDATA: Region data for dirty rectangles
 * 
 * All other types are already defined in d3d8.h or win32_types.h
 * Phase 02.5: DirectX 8 Stub Interface Implementation
 */

#ifndef D3D8_TYPES_H_INCLUDED
#define D3D8_TYPES_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =====================================================================
// COM Interface Identifier
// =====================================================================

#ifndef __IID_DEFINED
#define __IID_DEFINED

struct IID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
};

#endif

// =====================================================================
// Region Data (for dirty rectangles in Present)
// =====================================================================

#ifndef RGNDATA_DEFINED
#define RGNDATA_DEFINED

struct RGNDATA {
    uint32_t rdh_dwSize;
    uint32_t rdh_iType;
    uint32_t rdh_nCount;
    uint32_t rdh_nRgnSize;
    // Rect and data would follow
};

#endif

#ifdef __cplusplus
}
#endif

#endif // D3D8_TYPES_H_INCLUDED
