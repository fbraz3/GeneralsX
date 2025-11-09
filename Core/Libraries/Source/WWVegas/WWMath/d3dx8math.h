#pragma once

/*
 * DirectX 8 Math Compatibility Header for macOS/Unix
 * This file provides compatibility definitions for DirectX 8 math functions
 * Available on all platforms (Windows, macOS, Linux)
 */

#ifdef _WIN32
  // On Windows, use system DirectX headers
  #include <d3dx8.h>
#else
  // On macOS/Linux, provide our own implementation
  
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
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
    
    D3DXMATRIX() {}
    D3DXMATRIX(const Matrix4x4& mat) {
        // Convert from Matrix4x4 to D3DXMATRIX
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = mat[i][j];
            }
        }
    }
    
    D3DXMATRIX(float m11, float m12, float m13, float m14,
               float m21, float m22, float m23, float m24,
               float m31, float m32, float m33, float m34,
               float m41, float m42, float m43, float m44) {
        _11 = m11; _12 = m12; _13 = m13; _14 = m14;
        _21 = m21; _22 = m22; _23 = m23; _24 = m24;
        _31 = m31; _32 = m32; _33 = m33; _34 = m34;
        _41 = m41; _42 = m42; _43 = m43; _44 = m44;
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
    
    // Matrix multiplication assignment operator
    D3DXMATRIX& operator*=(const D3DXMATRIX& other) {
        D3DXMATRIX temp = *this * other;
        *this = temp;
        return *this;
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

// Vector4 transformation
inline void D3DXVec4Transform(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV, const D3DXMATRIX* pM) {
    if (pOut && pV && pM) {
        pOut->x = pV->x * pM->m[0][0] + pV->y * pM->m[1][0] + pV->z * pM->m[2][0] + pV->w * pM->m[3][0];
        pOut->y = pV->x * pM->m[0][1] + pV->y * pM->m[1][1] + pV->z * pM->m[2][1] + pV->w * pM->m[3][1];
        pOut->z = pV->x * pM->m[0][2] + pV->y * pM->m[1][2] + pV->z * pM->m[2][2] + pV->w * pM->m[3][2];
        pOut->w = pV->x * pM->m[0][3] + pV->y * pM->m[1][3] + pV->z * pM->m[2][3] + pV->w * pM->m[3][3];
    }
}

// Vector4 dot product
inline float D3DXVec4Dot(const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2) {
    if (pV1 && pV2) {
        return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
    }
    return 0.0f;
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
