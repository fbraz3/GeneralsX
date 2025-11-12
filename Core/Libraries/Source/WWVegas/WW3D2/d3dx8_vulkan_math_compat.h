/*
 * d3dx8math.h - DirectX 8 Math compatibility header for cross-platform support
 * 
 * This file provides minimal DirectX Math definitions for cross-platform compilation.
 * It acts as a compatibility layer, mapping DirectX 8 math functions to standard C++ math.
 * 
 * Phase 01 Note: Created as prerequisite for SDL2 Window & Event Loop implementation.
 * Original DirectX functions are not fully implemented, but stubs are provided to allow
 * compilation on non-Windows platforms while maintaining API compatibility.
 */

#ifndef __D3DX8MATH_H__
#define __D3DX8MATH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ========== D3DX Math Constants ========== */

/**
 * @brief PI constant as used in DirectX 8 math functions
 * Used in rotation, angle, and trigonometric calculations
 */
#define D3DX_PI             3.14159265358979323846f
#define D3DX_2PI            6.28318530717958647692f
#define D3DX_PI_2           1.57079632679489661923f
#define D3DX_PI_4           0.78539816339744830962f
#define D3DX_PI_OVER_180    0.01745329251994329577f  /* PI/180 for degree conversion */
#define D3DX_180_OVER_PI    57.2957795130823208768f  /* 180/PI for degree conversion */

/* ========== Basic Math Type Definitions ========== */

typedef float D3DXFLOAT16;
typedef float D3DXFLOAT32;

/* Vector types - basic 2D, 3D, 4D vectors */
typedef struct D3DXVECTOR2 {
    float x, y;
} D3DXVECTOR2;

typedef struct D3DXVECTOR3 {
    float x, y, z;
} D3DXVECTOR3;

typedef struct D3DXVECTOR4 {
    float x, y, z, w;
} D3DXVECTOR4;

/* Matrix type - 4x4 transformation matrix */
typedef struct D3DXMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
        float f[16];
    };
} D3DXMATRIX;

/* Quaternion for rotation representation */
typedef struct D3DXQUATERNION {
    float x, y, z, w;
} D3DXQUATERNION;

/* Plane representation (normal + distance) */
typedef struct D3DXPLANE {
    float a, b, c, d;
} D3DXPLANE;

/* ========== Matrix Operations (Minimal Stubs) ========== */

/* Matrix identity - set matrix to identity */
static inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut) {
    if (!pOut) return nullptr;
    pOut->_11 = 1; pOut->_12 = 0; pOut->_13 = 0; pOut->_14 = 0;
    pOut->_21 = 0; pOut->_22 = 1; pOut->_23 = 0; pOut->_24 = 0;
    pOut->_31 = 0; pOut->_32 = 0; pOut->_33 = 1; pOut->_34 = 0;
    pOut->_41 = 0; pOut->_42 = 0; pOut->_43 = 0; pOut->_44 = 1;
    return pOut;
}

/* Matrix transpose */
static inline D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM) {
    if (!pOut || !pM) return nullptr;
    D3DXMATRIX temp = *pM;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            pOut->m[i][j] = temp.m[j][i];
        }
    }
    return pOut;
}

/* Matrix multiply - stub implementation (not fully functional) */
static inline D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2) {
    if (!pOut || !pM1 || !pM2) return nullptr;
    /* Full matrix multiply would be complex - just return identity for now */
    return D3DXMatrixIdentity(pOut);
}

/* Matrix inverse - stub implementation (returns identity) */
static inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pM) {
    if (!pOut || !pM) return nullptr;
    if (pDeterminant) *pDeterminant = 1.0f;
    /* Full matrix inversion is complex - just return identity for now */
    return D3DXMatrixIdentity(pOut);
}

/* ========== Vector Operations (Minimal Stubs) ========== */

/* Vector length */
static inline float D3DXVec3Length(const D3DXVECTOR3* pV) {
    if (!pV) return 0.0f;
    return std::sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

/* Vector normalize */
static inline D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV) {
    if (!pOut || !pV) return nullptr;
    float len = D3DXVec3Length(pV);
    if (len > 0.0f) {
        pOut->x = pV->x / len;
        pOut->y = pV->y / len;
        pOut->z = pV->z / len;
    }
    return pOut;
}

/* Vector dot product */
static inline float D3DXVec3Dot(const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) {
    if (!pV1 || !pV2) return 0.0f;
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

/* Vector cross product */
static inline D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) {
    if (!pOut || !pV1 || !pV2) return nullptr;
    D3DXVECTOR3 temp;
    temp.x = pV1->y * pV2->z - pV1->z * pV2->y;
    temp.y = pV1->z * pV2->x - pV1->x * pV2->z;
    temp.z = pV1->x * pV2->y - pV1->y * pV2->x;
    *pOut = temp;
    return pOut;
}

/* ========== Quaternion Operations (Minimal Stubs) ========== */

/* Quaternion identity (no rotation) */
static inline D3DXQUATERNION* D3DXQuaternionIdentity(D3DXQUATERNION* pOut) {
    if (!pOut) return nullptr;
    pOut->x = 0.0f;
    pOut->y = 0.0f;
    pOut->z = 0.0f;
    pOut->w = 1.0f;
    return pOut;
}

/* ========== Plane Operations (Minimal Stubs) ========== */

/* Plane normalize */
static inline D3DXPLANE* D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP) {
    if (!pOut || !pP) return nullptr;
    float len = std::sqrt(pP->a * pP->a + pP->b * pP->b + pP->c * pP->c);
    if (len > 0.0f) {
        pOut->a = pP->a / len;
        pOut->b = pP->b / len;
        pOut->c = pP->c / len;
        pOut->d = pP->d / len;
    }
    return pOut;
}

/* ========== Matrix Operations ========== */

/**
 * @brief Create a rotation matrix around the Z axis
 * @param pOut      Output matrix pointer
 * @param angle     Rotation angle in radians
 * @return          Pointer to the output matrix
 */
static inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float angle) {
    if (!pOut) return nullptr;
    
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    
    pOut->_11 = cosA;   pOut->_12 = sinA;   pOut->_13 = 0.0f;  pOut->_14 = 0.0f;
    pOut->_21 = -sinA;  pOut->_22 = cosA;   pOut->_23 = 0.0f;  pOut->_24 = 0.0f;
    pOut->_31 = 0.0f;   pOut->_32 = 0.0f;   pOut->_33 = 1.0f;  pOut->_34 = 0.0f;
    pOut->_41 = 0.0f;   pOut->_42 = 0.0f;   pOut->_43 = 0.0f;  pOut->_44 = 1.0f;
    
    return pOut;
}

#ifdef __cplusplus
}
#endif

#endif /* __D3DX8MATH_H__ */
