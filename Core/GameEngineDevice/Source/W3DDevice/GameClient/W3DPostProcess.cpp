/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: W3DPostProcess.cpp ///////////////////////////////////////////////////
// GeneralsX @feature fbraz3 08/10/2026 Post-processing pipeline implementation for GeneralsX.
///////////////////////////////////////////////////////////////////////////////

#include "W3DDevice/GameClient/W3DPostProcess.h"
#include "W3DDevice/GameClient/W3DPostProcessShaders.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/Water.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/ww3d.h"

#include <algorithm>
#include <stdio.h>

W3DPostProcess *TheW3DPostProcess = nullptr;

struct PostQuadVertex1 {
	float x, y, z, rhw;
	float u0, v0;
};

struct PostQuadVertex2 {
	float x, y, z, rhw;
	float u0, v0;
	float u1, v1;
};

struct PostQuadVertex4 {
	float x, y, z, rhw;
	float u0, v0;
	float u1, v1;
	float u2, v2;
	float u3, v3;
};

W3DPostProcess::W3DPostProcess()
	: m_initialized(false)
	, m_isSceneActive(false)
	, m_sceneWidth(0)
	, m_sceneHeight(0)
	, m_bloomWidth(0)
	, m_bloomHeight(0)
	, m_sceneTexture(nullptr)
	, m_sceneSurface(nullptr)
	, m_sceneDepthSurface(nullptr)
	, m_compositeTexture(nullptr)
	, m_compositeSurface(nullptr)
	, m_savedRenderTarget(nullptr)
	, m_savedDepthBuffer(nullptr)
	, m_psCopy(0)
	, m_psBloomExtract(0)
	, m_psBloomBlur4Tap(0)
	, m_psBloomComposite(0)
	, m_psBloomCompositeHDR(0)
	, m_psToneMapOnly(0)
	, m_psFXAA(0)
{
	m_bloomTexture[0] = nullptr;
	m_bloomTexture[1] = nullptr;
	m_bloomSurface[0] = nullptr;
	m_bloomSurface[1] = nullptr;
}

W3DPostProcess::~W3DPostProcess()
{
	DX8Wrapper::RemoveCleanupHook(this);
	releaseResources();

	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (pDev)
	{
		if (m_psCopy) pDev->DeletePixelShader(m_psCopy);
		if (m_psBloomExtract) pDev->DeletePixelShader(m_psBloomExtract);
		if (m_psBloomBlur4Tap) pDev->DeletePixelShader(m_psBloomBlur4Tap);
		if (m_psBloomComposite) pDev->DeletePixelShader(m_psBloomComposite);
		if (m_psBloomCompositeHDR) pDev->DeletePixelShader(m_psBloomCompositeHDR);
		if (m_psToneMapOnly) pDev->DeletePixelShader(m_psToneMapOnly);
		if (m_psFXAA) pDev->DeletePixelShader(m_psFXAA);
	}

	m_psCopy = 0;
	m_psBloomExtract = 0;
	m_psBloomBlur4Tap = 0;
	m_psBloomComposite = 0;
	m_psBloomCompositeHDR = 0;
	m_psToneMapOnly = 0;
	m_psFXAA = 0;
}

void W3DPostProcess::init()
{
	if (m_initialized)
		return;

	if (TheGlobalData && TheGlobalData->m_headless)
		return;

	DX8Wrapper::AddCleanupHook(this);
	initShaders();
	m_initialized = true;
	fprintf(stderr, "[POST_PROCESS] Initialized post-processing pipeline shaders and cleanup hook\n");
}

void W3DPostProcess::reset()
{
	releaseResources();
}

void W3DPostProcess::update()
{
	// No per-frame logic required outside draw
}

bool W3DPostProcess::initShaders()
{
	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (!pDev)
		return false;

	HRESULT hr;
	hr = pDev->CreatePixelShader(W3DPostProcessShaders::Copy, &m_psCopy);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create copy shader (hr=0x%08x)\n", hr);

	hr = pDev->CreatePixelShader(W3DPostProcessShaders::BloomExtract, &m_psBloomExtract);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create bloom extract shader (hr=0x%08x)\n", hr);

	hr = pDev->CreatePixelShader(W3DPostProcessShaders::BloomBlur4Tap, &m_psBloomBlur4Tap);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create bloom blur shader (hr=0x%08x)\n", hr);

	hr = pDev->CreatePixelShader(W3DPostProcessShaders::BloomComposite, &m_psBloomComposite);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create bloom composite shader (hr=0x%08x)\n", hr);

	hr = pDev->CreatePixelShader(W3DPostProcessShaders::BloomCompositeHDR, &m_psBloomCompositeHDR);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create bloom composite HDR shader (hr=0x%08x)\n", hr);

	hr = pDev->CreatePixelShader(W3DPostProcessShaders::ToneMapOnly, &m_psToneMapOnly);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create tone map shader (hr=0x%08x)\n", hr);

	hr = pDev->CreatePixelShader(W3DPostProcessShaders::FXAA, &m_psFXAA);
	if (FAILED(hr)) fprintf(stderr, "[POST_PROCESS] Warning: Failed to create FXAA shader (hr=0x%08x)\n", hr);

	return true;
}

void W3DPostProcess::releaseResources()
{
	if (m_sceneDepthSurface) { m_sceneDepthSurface->Release(); m_sceneDepthSurface = nullptr; }
	if (m_sceneSurface) { m_sceneSurface->Release(); m_sceneSurface = nullptr; }
	if (m_sceneTexture) { m_sceneTexture->Release(); m_sceneTexture = nullptr; }

	for (int i = 0; i < 2; ++i)
	{
		if (m_bloomSurface[i]) { m_bloomSurface[i]->Release(); m_bloomSurface[i] = nullptr; }
		if (m_bloomTexture[i]) { m_bloomTexture[i]->Release(); m_bloomTexture[i] = nullptr; }
	}

	if (m_compositeSurface) { m_compositeSurface->Release(); m_compositeSurface = nullptr; }
	if (m_compositeTexture) { m_compositeTexture->Release(); m_compositeTexture = nullptr; }

	if (m_savedRenderTarget) { m_savedRenderTarget->Release(); m_savedRenderTarget = nullptr; }
	if (m_savedDepthBuffer) { m_savedDepthBuffer->Release(); m_savedDepthBuffer = nullptr; }

	m_isSceneActive = false;
}

bool W3DPostProcess::reacquireResources()
{
	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (!pDev)
		return false;

	releaseResources();

	int width = TheDisplay ? TheDisplay->getWidth() : 0;
	int height = TheDisplay ? TheDisplay->getHeight() : 0;
	if (width <= 0 || height <= 0)
	{
		int bits = 32;
		bool windowed = true;
		WW3D::Get_Render_Target_Resolution(width, height, bits, windowed);
	}
	if (width <= 0 || height <= 0)
	{
		width = 800;
		height = 600;
	}

	m_sceneWidth = width;
	m_sceneHeight = height;
	m_bloomWidth = std::max(1, width / 4);
	m_bloomHeight = std::max(1, height / 4);

	HRESULT hr;

	// 1. Scene color render target
	hr = pDev->CreateTexture(m_sceneWidth, m_sceneHeight, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_sceneTexture);
	if (FAILED(hr))
	{
		fprintf(stderr, "[POST_PROCESS] Failed to create scene render target texture (%dx%d, hr=0x%08x)\n",
			m_sceneWidth, m_sceneHeight, hr);
		return false;
	}
	m_sceneTexture->GetSurfaceLevel(0, &m_sceneSurface);

	// 2. Dedicated non-MSAA depth-stencil buffer matching scene resolution
	D3DFORMAT depthFormat = D3DFMT_D24S8;
	IDirect3DSurface8 *devDepth = nullptr;
	if (SUCCEEDED(pDev->GetDepthStencilSurface(&devDepth)) && devDepth)
	{
		D3DSURFACE_DESC devDepthDesc;
		devDepth->GetDesc(&devDepthDesc);
		depthFormat = devDepthDesc.Format;
		devDepth->Release();
	}

	hr = pDev->CreateDepthStencilSurface(m_sceneWidth, m_sceneHeight, depthFormat, D3DMULTISAMPLE_NONE, &m_sceneDepthSurface);
	if (FAILED(hr))
	{
		hr = pDev->CreateDepthStencilSurface(m_sceneWidth, m_sceneHeight, D3DFMT_D16, D3DMULTISAMPLE_NONE, &m_sceneDepthSurface);
		if (FAILED(hr))
		{
			fprintf(stderr, "[POST_PROCESS] Warning: Failed to create non-MSAA depth stencil surface (%dx%d, hr=0x%08x)\n",
				m_sceneWidth, m_sceneHeight, hr);
			m_sceneDepthSurface = nullptr;
		}
	}

	// 3. Quarter-size Bloom ping-pong textures
	for (int i = 0; i < 2; ++i)
	{
		hr = pDev->CreateTexture(m_bloomWidth, m_bloomHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_bloomTexture[i]);
		if (FAILED(hr))
		{
			fprintf(stderr, "[POST_PROCESS] Failed to create bloom target texture %d (%dx%d, hr=0x%08x)\n",
				i, m_bloomWidth, m_bloomHeight, hr);
			releaseResources();
			return false;
		}
		m_bloomTexture[i]->GetSurfaceLevel(0, &m_bloomSurface[i]);
	}

	// 4. Composite full-screen texture for chaining
	hr = pDev->CreateTexture(m_sceneWidth, m_sceneHeight, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_compositeTexture);
	if (FAILED(hr))
	{
		fprintf(stderr, "[POST_PROCESS] Failed to create composite target texture (%dx%d, hr=0x%08x)\n",
			m_sceneWidth, m_sceneHeight, hr);
		releaseResources();
		return false;
	}
	m_compositeTexture->GetSurfaceLevel(0, &m_compositeSurface);

	return true;
}

bool W3DPostProcess::beginScene()
{
	if (!TheGlobalData || !TheGlobalData->isEnablePostProcessing())
		return false;

	if (!TheDisplay || !TheDisplay->getFirstView())
		return false;

	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (!pDev || pDev->TestCooperativeLevel() != D3D_OK)
		return false;

	int curWidth = TheDisplay ? TheDisplay->getWidth() : 0;
	int curHeight = TheDisplay ? TheDisplay->getHeight() : 0;
	if (curWidth <= 0 || curHeight <= 0)
	{
		int bits = 32;
		bool windowed = true;
		WW3D::Get_Render_Target_Resolution(curWidth, curHeight, bits, windowed);
	}
	if (curWidth <= 0 || curHeight <= 0)
	{
		curWidth = m_sceneWidth > 0 ? m_sceneWidth : 800;
		curHeight = m_sceneHeight > 0 ? m_sceneHeight : 600;
	}

	if (!m_sceneSurface || m_sceneWidth != curWidth || m_sceneHeight != curHeight)
	{
		if (!reacquireResources())
			return false;
	}

	// Save active render target, depth stencil, and viewport
	if (m_savedRenderTarget) { m_savedRenderTarget->Release(); m_savedRenderTarget = nullptr; }
	if (m_savedDepthBuffer) { m_savedDepthBuffer->Release(); m_savedDepthBuffer = nullptr; }

	pDev->GetRenderTarget(&m_savedRenderTarget);
	pDev->GetDepthStencilSurface(&m_savedDepthBuffer);
	pDev->GetViewport(&m_savedViewport);

	if (!m_savedRenderTarget || !m_savedDepthBuffer)
	{
		if (m_savedRenderTarget) { m_savedRenderTarget->Release(); m_savedRenderTarget = nullptr; }
		if (m_savedDepthBuffer) { m_savedDepthBuffer->Release(); m_savedDepthBuffer = nullptr; }
		return false;
	}

	// Select depth surface: prefer dedicated non-MSAA scene depth surface, fallback to saved device depth buffer
	IDirect3DSurface8 *depthToUse = m_sceneDepthSurface ? m_sceneDepthSurface : m_savedDepthBuffer;

	D3DSURFACE_DESC depthDesc;
	depthToUse->GetDesc(&depthDesc);
	if (depthDesc.MultiSampleType != D3DMULTISAMPLE_NONE)
	{
		// Cannot pair multisampled depth buffer with non-multisampled scene target in D3D8
		fprintf(stderr, "[POST_PROCESS] Multisampled depth buffer cannot be paired with non-MSAA render target, skipping post-processing\n");
		if (m_savedRenderTarget) { m_savedRenderTarget->Release(); m_savedRenderTarget = nullptr; }
		if (m_savedDepthBuffer) { m_savedDepthBuffer->Release(); m_savedDepthBuffer = nullptr; }
		return false;
	}

	// Redirect rendering of the 3D world to our offscreen scene target
	HRESULT hr = pDev->SetRenderTarget(m_sceneSurface, depthToUse);
	if (FAILED(hr))
	{
		fprintf(stderr, "[POST_PROCESS] SetRenderTarget to sceneSurface failed (hr=0x%08x)\n", hr);
		if (m_savedRenderTarget) { m_savedRenderTarget->Release(); m_savedRenderTarget = nullptr; }
		if (m_savedDepthBuffer) { m_savedDepthBuffer->Release(); m_savedDepthBuffer = nullptr; }
		return false;
	}

	D3DVIEWPORT8 vpScene = {0, 0, (DWORD)m_sceneWidth, (DWORD)m_sceneHeight, 0.0f, 1.0f};
	pDev->SetViewport(&vpScene);

	// Clear scene surface to opaque black with alpha matching engine water transparency
	// GeneralsX @bugfix fbraz3 09/10/2026 Water blending uses D3DBLEND_DESTALPHA; alpha must be cleared to minWaterOpacity (0xFF)
	float destAlpha = (TheWaterTransparency != nullptr) ? TheWaterTransparency->m_minWaterOpacity : 1.0f;
	if (destAlpha < 0.0f) destAlpha = 0.0f;
	if (destAlpha > 1.0f) destAlpha = 1.0f;
	DWORD alphaVal = (DWORD)(destAlpha * 255.0f);
	D3DCOLOR clearColor = (alphaVal << 24);
	pDev->Clear(0, nullptr, D3DCLEAR_TARGET, clearColor, 1.0f, 0);

	m_isSceneActive = true;
	return true;
}

bool W3DPostProcess::endSceneAndApply()
{
	if (!m_isSceneActive)
		return false;

	m_isSceneActive = false;

	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (!pDev || !m_savedRenderTarget)
		return false;

	bool doBloom = TheGlobalData ? TheGlobalData->isPostProcessBloom() : false;
	bool doHDR = TheGlobalData ? TheGlobalData->isPostProcessHDR() : false;
	bool doFXAA = TheGlobalData ? TheGlobalData->isPostProcessFXAA() : false;

	// 1. Pass: Bloom Extraction & Blur
	if (doBloom && m_bloomSurface[0] && m_bloomSurface[1] && m_psBloomExtract && m_psBloomBlur4Tap)
	{
		// 1a. Extract bright pass to 1/4 size bloom target 0
		pDev->SetRenderTarget(m_bloomSurface[0], nullptr);
		D3DVIEWPORT8 vpBloom = {0, 0, (DWORD)m_bloomWidth, (DWORD)m_bloomHeight, 0.0f, 1.0f};
		pDev->SetViewport(&vpBloom);
		renderQuad(m_sceneTexture, m_psBloomExtract, m_bloomWidth, m_bloomHeight);

		// 1b. Horizontal Gaussian blur: bloom 0 -> bloom 1
		pDev->SetRenderTarget(m_bloomSurface[1], nullptr);
		renderQuad4Tap(m_bloomTexture[0], m_psBloomBlur4Tap, m_bloomWidth, m_bloomHeight,
			1.5f / (float)m_bloomWidth, 0.0f);

		// 1c. Vertical Gaussian blur: bloom 1 -> bloom 0
		pDev->SetRenderTarget(m_bloomSurface[0], nullptr);
		renderQuad4Tap(m_bloomTexture[1], m_psBloomBlur4Tap, m_bloomWidth, m_bloomHeight,
			0.0f, 1.5f / (float)m_bloomHeight);
	}

	D3DVIEWPORT8 vpScene = {0, 0, (DWORD)m_sceneWidth, (DWORD)m_sceneHeight, 0.0f, 1.0f};

	// 2. Pass: Composite / Tone-Mapping / FXAA
	if (doBloom || doHDR)
	{
		IDirect3DSurface8 *destSurface = doFXAA ? m_compositeSurface : m_savedRenderTarget;
		pDev->SetRenderTarget(destSurface, nullptr);
		pDev->SetViewport(&vpScene);

		if (doBloom && doHDR && m_psBloomCompositeHDR && m_bloomTexture[0])
		{
			renderQuad(m_sceneTexture, m_psBloomCompositeHDR, m_sceneWidth, m_sceneHeight, m_bloomTexture[0]);
		}
		else if (doBloom && m_psBloomComposite && m_bloomTexture[0])
		{
			renderQuad(m_sceneTexture, m_psBloomComposite, m_sceneWidth, m_sceneHeight, m_bloomTexture[0]);
		}
		else if (doHDR && m_psToneMapOnly)
		{
			renderQuad(m_sceneTexture, m_psToneMapOnly, m_sceneWidth, m_sceneHeight);
		}
		else
		{
			renderQuad(m_sceneTexture, m_psCopy, m_sceneWidth, m_sceneHeight);
		}

		// 3. Pass: FXAA Anti-Aliasing (if enabled after composite)
		if (doFXAA && m_psFXAA && m_compositeTexture)
		{
			pDev->SetRenderTarget(m_savedRenderTarget, nullptr);
			pDev->SetViewport(&vpScene);
			renderQuad4Tap(m_compositeTexture, m_psFXAA, m_sceneWidth, m_sceneHeight,
				1.0f / (float)m_sceneWidth, 1.0f / (float)m_sceneHeight);
		}
	}
	else if (doFXAA && m_psFXAA)
	{
		// Only FXAA is enabled: run directly from m_sceneTexture to m_savedRenderTarget
		pDev->SetRenderTarget(m_savedRenderTarget, nullptr);
		pDev->SetViewport(&vpScene);
		renderQuad4Tap(m_sceneTexture, m_psFXAA, m_sceneWidth, m_sceneHeight,
			1.0f / (float)m_sceneWidth, 1.0f / (float)m_sceneHeight);
	}
	else
	{
		// Fallback copy
		pDev->SetRenderTarget(m_savedRenderTarget, nullptr);
		pDev->SetViewport(&vpScene);
		renderQuad(m_sceneTexture, m_psCopy, m_sceneWidth, m_sceneHeight);
	}

	// 4. Restore original render target, depth buffer, and viewport for 2D UI rendering
	pDev->SetRenderTarget(m_savedRenderTarget, m_savedDepthBuffer);
	pDev->SetViewport(&m_savedViewport);

	if (m_savedRenderTarget) { m_savedRenderTarget->Release(); m_savedRenderTarget = nullptr; }
	if (m_savedDepthBuffer) { m_savedDepthBuffer->Release(); m_savedDepthBuffer = nullptr; }

	// Restore device state & unbind textures
	pDev->SetTexture(0, nullptr);
	pDev->SetTexture(1, nullptr);
	pDev->SetTexture(2, nullptr);
	pDev->SetTexture(3, nullptr);
	pDev->SetPixelShader(0);

	DX8Wrapper::Invalidate_Cached_Render_States();
	return true;
}

void W3DPostProcess::renderQuad(IDirect3DTexture8 *tex0, DWORD pixelShader, int width, int height,
                               IDirect3DTexture8 *tex1)
{
	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (!pDev || !tex0)
		return;

	// D3D8 half-pixel offset for exact texel-to-pixel mapping
	float x0 = -0.5f;
	float y0 = -0.5f;
	float x1 = (float)width - 0.5f;
	float y1 = (float)height - 0.5f;

	pDev->SetTexture(0, tex0);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

	if (tex1)
	{
		pDev->SetTexture(1, tex1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	}
	else
	{
		pDev->SetTexture(1, nullptr);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	}

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_NONE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0xF);

	if (pixelShader)
		pDev->SetPixelShader(pixelShader);
	else
		pDev->SetPixelShader(0);

	if (tex1)
	{
		PostQuadVertex2 quad[4] = {
			{ x0, y0, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
			{ x1, y0, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },
			{ x0, y1, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f },
			{ x1, y1, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }
		};
		pDev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX2);
		pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(PostQuadVertex2));
	}
	else
	{
		PostQuadVertex1 quad[4] = {
			{ x0, y0, 0.0f, 1.0f, 0.0f, 0.0f },
			{ x1, y0, 0.0f, 1.0f, 1.0f, 0.0f },
			{ x0, y1, 0.0f, 1.0f, 0.0f, 1.0f },
			{ x1, y1, 0.0f, 1.0f, 1.0f, 1.0f }
		};
		pDev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX1);
		pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(PostQuadVertex1));
	}

	pDev->SetPixelShader(0);
	pDev->SetTexture(0, nullptr);
	pDev->SetTexture(1, nullptr);
}

void W3DPostProcess::renderQuad4Tap(IDirect3DTexture8 *tex, DWORD pixelShader, int width, int height,
                                    float offsetU, float offsetV)
{
	LPDIRECT3DDEVICE8 pDev = DX8Wrapper::_Get_D3D_Device8();
	if (!pDev || !tex)
		return;

	float x0 = -0.5f;
	float y0 = -0.5f;
	float x1 = (float)width - 0.5f;
	float y1 = (float)height - 0.5f;

	for (int stage = 0; stage < 4; ++stage)
	{
		pDev->SetTexture(stage, tex);
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	}

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_NONE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_STENCILENABLE, FALSE);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0xF);

	if (pixelShader)
		pDev->SetPixelShader(pixelShader);
	else
		pDev->SetPixelShader(0);

	PostQuadVertex4 quad[4] = {
		// Top-Left (0, 0)
		{ x0, y0, 0.0f, 1.0f,
		  0.0f - 1.5f * offsetU, 0.0f - 1.5f * offsetV,
		  0.0f - 0.5f * offsetU, 0.0f - 0.5f * offsetV,
		  0.0f + 0.5f * offsetU, 0.0f + 0.5f * offsetV,
		  0.0f + 1.5f * offsetU, 0.0f + 1.5f * offsetV },

		// Top-Right (1, 0)
		{ x1, y0, 0.0f, 1.0f,
		  1.0f - 1.5f * offsetU, 0.0f - 1.5f * offsetV,
		  1.0f - 0.5f * offsetU, 0.0f - 0.5f * offsetV,
		  1.0f + 0.5f * offsetU, 0.0f + 0.5f * offsetV,
		  1.0f + 1.5f * offsetU, 0.0f + 1.5f * offsetV },

		// Bottom-Left (0, 1)
		{ x0, y1, 0.0f, 1.0f,
		  0.0f - 1.5f * offsetU, 1.0f - 1.5f * offsetV,
		  0.0f - 0.5f * offsetU, 1.0f - 0.5f * offsetV,
		  0.0f + 0.5f * offsetU, 1.0f + 0.5f * offsetV,
		  0.0f + 1.5f * offsetU, 1.0f + 1.5f * offsetV },

		// Bottom-Right (1, 1)
		{ x1, y1, 0.0f, 1.0f,
		  1.0f - 1.5f * offsetU, 1.0f - 1.5f * offsetV,
		  1.0f - 0.5f * offsetU, 1.0f - 0.5f * offsetV,
		  1.0f + 0.5f * offsetU, 1.0f + 0.5f * offsetV,
		  1.0f + 1.5f * offsetU, 1.0f + 1.5f * offsetV }
	};

	pDev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_TEX4);
	pDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(PostQuadVertex4));

	pDev->SetPixelShader(0);
	for (int stage = 0; stage < 4; ++stage)
		pDev->SetTexture(stage, nullptr);
}
