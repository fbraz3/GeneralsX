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

// FILE: W3DPostProcess.h /////////////////////////////////////////////////////
// GeneralsX @feature fbraz3 08/10/2026 Post-processing pipeline manager for GeneralsX:
//              Offscreen scene color target management, bloom bright-pass
//              extraction & separable Gaussian blur, HDR tone mapping, and
//              FXAA edge antialiasing before UI rendering.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/SubsystemInterface.h"
#include "WW3D2/dx8wrapper.h"

class W3DPostProcess : public SubsystemInterface, public DX8_CleanupHook
{
public:
	W3DPostProcess();
	virtual ~W3DPostProcess();

	virtual void init() override;
	virtual void reset() override;
	virtual void update() override;

	// DX8_CleanupHook interface
	virtual void ReleaseResources() override { releaseResources(); }
	virtual void ReAcquireResources() override {}

	void releaseResources();
	bool reacquireResources();

	// Called in W3DDisplay::draw() right before 3D world rendering begins
	bool beginScene();

	// Called in W3DDisplay::draw() right after drawViews(), before TheInGameUI->DRAW()
	bool endSceneAndApply();

	bool isPostProcessActive() const { return m_isSceneActive; }

private:
	bool initShaders();
	void renderQuad(IDirect3DTexture8 *tex0, DWORD pixelShader, int width, int height,
	                IDirect3DTexture8 *tex1 = nullptr);
	void renderQuad4Tap(IDirect3DTexture8 *tex, DWORD pixelShader, int width, int height,
	                    float offsetU, float offsetV);

	bool m_initialized;
	bool m_isSceneActive;

	int m_sceneWidth;
	int m_sceneHeight;
	int m_bloomWidth;
	int m_bloomHeight;

	IDirect3DTexture8 *m_sceneTexture;
	IDirect3DSurface8 *m_sceneSurface;
	IDirect3DSurface8 *m_sceneDepthSurface;

	IDirect3DTexture8 *m_bloomTexture[2];
	IDirect3DSurface8 *m_bloomSurface[2];

	IDirect3DTexture8 *m_compositeTexture;
	IDirect3DSurface8 *m_compositeSurface;

	IDirect3DSurface8 *m_savedRenderTarget;
	IDirect3DSurface8 *m_savedDepthBuffer;
	D3DVIEWPORT8 m_savedViewport;

	DWORD m_psCopy;
	DWORD m_psBloomExtract;
	DWORD m_psBloomBlur4Tap;
	DWORD m_psBloomComposite;
	DWORD m_psBloomCompositeHDR;
	DWORD m_psToneMapOnly;
	DWORD m_psFXAA;
};

extern W3DPostProcess *TheW3DPostProcess;
