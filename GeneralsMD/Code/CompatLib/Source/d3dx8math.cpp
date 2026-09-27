// GLM_ENABLE_EXPERIMENTAL is passed via target_compile_definitions in CMakeLists.txt.
// Do NOT redefine it here -- that causes -Wmacro-redefined with Clang.

#ifdef _WIN32
#include <windows.h>
#else
#include "windows_compat.h"
#endif
#include "d3dx8core.h"

#include "d3dx8math.h"

#include <cmath>
#include <glm/glm.hpp>

// GeneralsX @bugfix fbraz 27/09/2026 Direct D3DX row-major matrix operations to fix cloud shadows, UV animation, and transformations

static void ConvertGLMToD3DX (const glm::mat4x4 &glm, D3DXMATRIX &d3dx)
{
	d3dx._11 = glm[0][0];
	d3dx._12 = glm[1][0];
	d3dx._13 = glm[2][0];
	d3dx._14 = glm[3][0];

	d3dx._21 = glm[0][1];
	d3dx._22 = glm[1][1];
	d3dx._23 = glm[2][1];
	d3dx._24 = glm[3][1];

	d3dx._31 = glm[0][2];
	d3dx._32 = glm[1][2];
	d3dx._33 = glm[2][2];
	d3dx._34 = glm[3][2];

	d3dx._41 = glm[0][3];
	d3dx._42 = glm[1][3];
	d3dx._43 = glm[2][3];
	d3dx._44 = glm[3][3];
}

static void ConvertD3DXToGLM (const D3DXMATRIX &d3dx, glm::mat4x4 &glm)
{
	glm[0][0] = d3dx._11;
	glm[1][0] = d3dx._12;
	glm[2][0] = d3dx._13;
	glm[3][0] = d3dx._14;

	glm[0][1] = d3dx._21;
	glm[1][1] = d3dx._22;
	glm[2][1] = d3dx._23;
	glm[3][1] = d3dx._24;

	glm[0][2] = d3dx._31;
	glm[1][2] = d3dx._32;
	glm[2][2] = d3dx._33;
	glm[3][2] = d3dx._34;

	glm[0][3] = d3dx._41;
	glm[1][3] = d3dx._42;
	glm[2][3] = d3dx._43;
	glm[3][3] = d3dx._44;
}

D3DXMATRIX *WINAPI D3DXMatrixIdentity(D3DXMATRIX *pOut)
{
	if (!pOut) return nullptr;
	pOut->m[0][0] = 1.0f; pOut->m[0][1] = 0.0f; pOut->m[0][2] = 0.0f; pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = 0.0f; pOut->m[1][1] = 1.0f; pOut->m[1][2] = 0.0f; pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f; pOut->m[2][1] = 0.0f; pOut->m[2][2] = 1.0f; pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = 0.0f; pOut->m[3][1] = 0.0f; pOut->m[3][2] = 0.0f; pOut->m[3][3] = 1.0f;
	return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM)
{
	if (!pOut || !pM) return nullptr;
	glm::mat4x4 m;
	ConvertD3DXToGLM(*pM, m);

	if (pDeterminant)
		*pDeterminant = glm::determinant(m);

	glm::mat4x4 inv = glm::inverse(m);
	ConvertGLMToD3DX(inv, *pOut);

	return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixScaling(D3DXMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz)
{
	if (!pOut) return nullptr;
	pOut->m[0][0] = sx;   pOut->m[0][1] = 0.0f; pOut->m[0][2] = 0.0f; pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = 0.0f; pOut->m[1][1] = sy;   pOut->m[1][2] = 0.0f; pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f; pOut->m[2][1] = 0.0f; pOut->m[2][2] = sz;   pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = 0.0f; pOut->m[3][1] = 0.0f; pOut->m[3][2] = 0.0f; pOut->m[3][3] = 1.0f;
	return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z)
{
	if (!pOut) return nullptr;
	pOut->m[0][0] = 1.0f; pOut->m[0][1] = 0.0f; pOut->m[0][2] = 0.0f; pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = 0.0f; pOut->m[1][1] = 1.0f; pOut->m[1][2] = 0.0f; pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f; pOut->m[2][1] = 0.0f; pOut->m[2][2] = 1.0f; pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = x;    pOut->m[3][1] = y;    pOut->m[3][2] = z;    pOut->m[3][3] = 1.0f;
	return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2)
{
	if (!pOut || !pM1 || !pM2) return nullptr;
	D3DXMATRIX temp;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			temp.m[i][j] = pM1->m[i][0] * pM2->m[0][j] +
			               pM1->m[i][1] * pM2->m[1][j] +
			               pM1->m[i][2] * pM2->m[2][j] +
			               pM1->m[i][3] * pM2->m[3][j];
		}
	}
	*pOut = temp;
	return pOut;
}

D3DXVECTOR4 *WINAPI D3DXVec3Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM)
{
	if (!pOut || !pV || !pM) return nullptr;
	D3DXVECTOR4 temp;
	temp.x = pV->x * pM->m[0][0] + pV->y * pM->m[1][0] + pV->z * pM->m[2][0] + pM->m[3][0];
	temp.y = pV->x * pM->m[0][1] + pV->y * pM->m[1][1] + pV->z * pM->m[2][1] + pM->m[3][1];
	temp.z = pV->x * pM->m[0][2] + pV->y * pM->m[1][2] + pV->z * pM->m[2][2] + pM->m[3][2];
	temp.w = pV->x * pM->m[0][3] + pV->y * pM->m[1][3] + pV->z * pM->m[2][3] + pM->m[3][3];
	*pOut = temp;
	return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM)
{
	if (!pOut || !pM) return nullptr;
	D3DXMATRIX temp;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			temp.m[i][j] = pM->m[j][i];
		}
	}
	*pOut = temp;
	return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixRotationZ(D3DXMATRIX *pOut, FLOAT Angle)
{
	if (!pOut) return nullptr;
	FLOAT fSin = sinf(Angle);
	FLOAT fCos = cosf(Angle);

	pOut->m[0][0] = fCos;  pOut->m[0][1] = fSin; pOut->m[0][2] = 0.0f; pOut->m[0][3] = 0.0f;
	pOut->m[1][0] = -fSin; pOut->m[1][1] = fCos; pOut->m[1][2] = 0.0f; pOut->m[1][3] = 0.0f;
	pOut->m[2][0] = 0.0f;  pOut->m[2][1] = 0.0f; pOut->m[2][2] = 1.0f; pOut->m[2][3] = 0.0f;
	pOut->m[3][0] = 0.0f;  pOut->m[3][1] = 0.0f; pOut->m[3][2] = 0.0f; pOut->m[3][3] = 1.0f;
	return pOut;
}

D3DXVECTOR4 *WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM)
{
	if (!pOut || !pV || !pM) return nullptr;
	D3DXVECTOR4 temp;
	temp.x = pV->x * pM->m[0][0] + pV->y * pM->m[1][0] + pV->z * pM->m[2][0] + pV->w * pM->m[3][0];
	temp.y = pV->x * pM->m[0][1] + pV->y * pM->m[1][1] + pV->z * pM->m[2][1] + pV->w * pM->m[3][1];
	temp.z = pV->x * pM->m[0][2] + pV->y * pM->m[1][2] + pV->z * pM->m[2][2] + pV->w * pM->m[3][2];
	temp.w = pV->x * pM->m[0][3] + pV->y * pM->m[1][3] + pV->z * pM->m[2][3] + pV->w * pM->m[3][3];
	*pOut = temp;
	return pOut;
}