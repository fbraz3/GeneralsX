#pragma once

// CRITICAL: windows.h MUST come before d3d8.h
// On Linux: windows.h → windows_base.h (DXVK) → windows_compat.h
// On Windows: windows.h → Windows SDK
#include <windows.h>

#include <d3d8.h>

#ifdef __cplusplus
extern "C"
{
#endif

// D3DXMATRIX: Wrapper around D3DMATRIX with operator overloads
// DXVK D3DMATRIX uses union { struct { _11, _12, ... }; float m[4][4]; }
// Access via m[row][col] for portability
typedef struct D3DXMATRIX : D3DMATRIX
{
#ifdef __cplusplus
  D3DXMATRIX() = default;

  D3DXMATRIX(
      FLOAT m00, FLOAT m01, FLOAT m02, FLOAT m03,
      FLOAT m10, FLOAT m11, FLOAT m12, FLOAT m13,
      FLOAT m20, FLOAT m21, FLOAT m22, FLOAT m23,
      FLOAT m30, FLOAT m31, FLOAT m32, FLOAT m33)
  {
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
  }

  D3DXMATRIX operator *(const D3DXMATRIX &other) const;
  D3DXMATRIX operator *= (const D3DXMATRIX& other);
#endif
} D3DXMATRIX;

typedef D3DVECTOR D3DXVECTOR3;

typedef struct D3DXVECTOR4
{
#ifdef __cplusplus
  D3DXVECTOR4() {}
  D3DXVECTOR4(FLOAT x, FLOAT y, FLOAT z, FLOAT w) : x(x), y(y), z(z), w(w) {}

  operator FLOAT* () { return &x; }
#endif
  FLOAT x, y, z, w;
} D3DXVECTOR4;

#define D3DX_PI 3.141592654f

D3DXMATRIX *WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM);
D3DXMATRIX *WINAPI D3DXMatrixScaling(D3DXMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz);
D3DXMATRIX *WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z);
D3DXMATRIX *WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2);
D3DXVECTOR4 *WINAPI D3DXVec3Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM);
D3DXMATRIX *WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM);
D3DXMATRIX *WINAPI D3DXMatrixRotationZ(D3DXMATRIX *pOut, FLOAT angle);
D3DXVECTOR4 *WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM);

#ifdef __cplusplus

inline D3DXMATRIX D3DXMATRIX::operator*(const D3DXMATRIX &other) const
{
  D3DXMATRIX result;
  D3DXMatrixMultiply(&result, this, &other);
  return result;
}
inline D3DXMATRIX D3DXMATRIX::operator *= (const D3DXMATRIX& other)
{
  D3DXMatrixMultiply(this, this, &other);
  return *this;
}

}
#endif