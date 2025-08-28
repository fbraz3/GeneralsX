#pragma once

/*
 * DirectX 8 Math Compatibility Header for macOS/Unix
 * This file provides compatibility definitions for DirectX 8 math functions
 */

#ifndef _WIN32

// Forward declarations for Matrix types
struct Matrix3D;
struct Matrix4;

// DirectX math compatibility types
typedef struct D3DXMATRIX {
    float m[4][4];
} D3DXMATRIX;

// DirectX math function compatibility
#ifdef __cplusplus
extern "C" {
#endif

// Matrix inverse function - we'll implement this using standard math
static inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pM) {
    if (!pOut || !pM) return NULL;
    
    // Simple 4x4 matrix inverse implementation
    // For now, just copy the matrix (basic implementation)
    // TODO: Implement proper matrix inverse
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            pOut->m[i][j] = pM->m[i][j];
        }
    }
    
    if (pDeterminant) {
        *pDeterminant = 1.0f; // Dummy determinant
    }
    
    return pOut;
}

#ifdef __cplusplus
}
#endif

#endif // !_WIN32
