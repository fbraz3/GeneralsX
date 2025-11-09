#pragma once

/*
 * DirectX 8 Math Compatibility Header
 * Available on Windows and non-Windows platforms
 */

#ifdef _WIN32
  // On Windows, use system DirectX 8 headers
  #include <d3dx8.h>
#else
  // On non-Windows (macOS, Linux), provide cross-platform implementation
  
  #include <cstdint>
  #include <cmath>

  // Forward declarations
  #ifndef __VECTOR3_H__
  #include "vector3.h"
  #endif

  #ifndef __MATRIX4_H__
  #include "matrix4.h"
  #endif

  // D3DX constants
  #define D3DX_PI 3.14159265358979323846f

  // Vector3 structure (for D3D compatibility)
  #ifndef D3DXVECTOR3_DEFINED
  #define D3DXVECTOR3_DEFINED
  typedef struct D3DXVECTOR3 {
    float x, y, z;
  } D3DXVECTOR3;
  #endif

  // Vector4 structure
  #ifndef D3DXVECTOR4_DEFINED
  #define D3DXVECTOR4_DEFINED
  struct D3DXVECTOR4 {
    float x, y, z, w;
    
    // Array access operators
    float& operator[](int index) {
      return (&x)[index];
    }
    
    const float& operator[](int index) const {
      return (&x)[index];
    }
  };
  #endif

  // Matrix structure
  #ifndef D3DXMATRIX_DEFINED
  #define D3DXMATRIX_DEFINED
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
    
    // Default constructor
    D3DXMATRIX() {
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
      }
    }
    
    // 16-value constructor
    D3DXMATRIX(float m11, float m12, float m13, float m14,
               float m21, float m22, float m23, float m24,
               float m31, float m32, float m33, float m34,
               float m41, float m42, float m43, float m44) {
      _11 = m11; _12 = m12; _13 = m13; _14 = m14;
      _21 = m21; _22 = m22; _23 = m23; _24 = m24;
      _31 = m31; _32 = m32; _33 = m33; _34 = m34;
      _41 = m41; _42 = m42; _43 = m43; _44 = m44;
    }
    
    // Matrix multiplication
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
  #endif

  // ============= D3DX Math Functions =============

  // Vector4 transformation by matrix
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

  // Vector3 transformation
  inline void D3DXVec3Transform(D3DXVECTOR4* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM) {
    if (pOut && pV && pM) {
      pOut->x = pV->x * pM->m[0][0] + pV->y * pM->m[1][0] + pV->z * pM->m[2][0] + pM->m[3][0];
      pOut->y = pV->x * pM->m[0][1] + pV->y * pM->m[1][1] + pV->z * pM->m[2][1] + pM->m[3][1];
      pOut->z = pV->x * pM->m[0][2] + pV->y * pM->m[1][2] + pV->z * pM->m[2][2] + pM->m[3][2];
      pOut->w = pV->x * pM->m[0][3] + pV->y * pM->m[1][3] + pV->z * pM->m[2][3] + pM->m[3][3];
    }
  }

  // Vector3 dot product
  inline float D3DXVec3Dot(const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) {
    if (pV1 && pV2) {
      return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
    }
    return 0.0f;
  }

  // Matrix inverse (stub for now)
  inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pM) {
    if (!pOut || !pM) return nullptr;
    // Basic implementation: just copy (TODO: proper matrix inversion)
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        pOut->m[i][j] = pM->m[i][j];
      }
    }
    if (pDeterminant) {
      *pDeterminant = 1.0f;
    }
    return pOut;
  }

#endif // _WIN32
