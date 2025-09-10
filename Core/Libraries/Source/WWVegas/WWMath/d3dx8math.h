#pragma once

/*
 * DirectX 8 Math Compatibility Header for macOS/Unix
 * This file provides compatibility definitions for DirectX 8 math functions
 */

#ifndef _WIN32

// Include necessary forward declarations and math types
#ifndef __VECTOR3_H__
#include "vector3.h"
#endif

#ifndef __MATRIX4_H__
#include "matrix4.h"
#endif

// D3DX Math constants
#define D3DX_PI 3.14159265358979323846f

// Forward declarations for Matrix types
class Matrix3D;

#ifndef D3DXVECTOR3_DEFINED
#define D3DXVECTOR3_DEFINED
typedef struct D3DXVECTOR3 {
    float x, y, z;
} D3DXVECTOR3;
#endif

#ifndef D3DXVECTOR4_DEFINED
#define D3DXVECTOR4_DEFINED
// D3DXVECTOR4 structure with array access
struct D3DXVECTOR4 {
    float x, y, z, w;
    
    float& operator[](int index) {
        return (&x)[index];
    }
    
    const float& operator[](int index) const {
        return (&x)[index];
    }
};
#endif

#ifndef D3DXMATRIX_DEFINED
#define D3DXMATRIX_DEFINED
// D3DXMATRIX structure with operators
struct D3DXMATRIX {
    float m[4][4];
    
    D3DXMATRIX() {}
    D3DXMATRIX(const Matrix4x4& mat) {
        // Convert from Matrix4x4 to D3DXMATRIX
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = mat[i][j];
            }
        }
    }
    
    // Matrix multiplication operator
    D3DXMATRIX operator*(const D3DXMATRIX& other) const {
        D3DXMATRIX result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
};

// D3DX Math functions
inline void D3DXVec3Transform(D3DXVECTOR4* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM) {
    if (pOut && pV && pM) {
        pOut->x = pV->x * pM->m[0][0] + pV->y * pM->m[1][0] + pV->z * pM->m[2][0] + pM->m[3][0];
        pOut->y = pV->x * pM->m[0][1] + pV->y * pM->m[1][1] + pV->z * pM->m[2][1] + pM->m[3][1];
        pOut->z = pV->x * pM->m[0][2] + pV->y * pM->m[1][2] + pV->z * pM->m[2][2] + pM->m[3][2];
        pOut->w = pV->x * pM->m[0][3] + pV->y * pM->m[1][3] + pV->z * pM->m[2][3] + pM->m[3][3];
    }
}

// Overload for Vector3
inline void D3DXVec3Transform(D3DXVECTOR4* pOut, const Vector3* pV, const D3DXMATRIX* pM) {
    if (pOut && pV && pM) {
        pOut->x = pV->X * pM->m[0][0] + pV->Y * pM->m[1][0] + pV->Z * pM->m[2][0] + pM->m[3][0];
        pOut->y = pV->X * pM->m[0][1] + pV->Y * pM->m[1][1] + pV->Z * pM->m[2][1] + pM->m[3][1];
        pOut->z = pV->X * pM->m[0][2] + pV->Y * pM->m[1][2] + pV->Z * pM->m[2][2] + pM->m[3][2];
        pOut->w = pV->X * pM->m[0][3] + pV->Y * pM->m[1][3] + pV->Z * pM->m[2][3] + pM->m[3][3];
    }
}

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

#endif // D3DXMATRIX_DEFINED

#endif // !_WIN32
