/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "d3dx9.hpp"
#include "d3d8to9.hpp"
#include "gx_shader_tokens.hpp"
#include <cstring>
#include <regex>
#include <assert.h>

struct VertexShaderInfo
{
	IDirect3DVertexShader9 *Shader = nullptr;
	IDirect3DVertexDeclaration9 *Declaration = nullptr;
};

Direct3DDevice8::Direct3DDevice8(Direct3D8 *d3d, IDirect3DDevice9 *ProxyInterface, DWORD BehaviorFlags, D3DFORMAT ZBufferFormat, BOOL EnableZBufferDiscarding) :
	D3D(d3d), ProxyInterface(ProxyInterface), ZBufferDiscarding(EnableZBufferDiscarding)
{
	ProxyAddressLookupTable = new AddressLookupTable(this);

	const HDC hDC = GetDC(nullptr);
	IsPaletteSupported = (::GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) != 0;
	ReleaseDC(nullptr, hDC);

	IsMixedVertexProcessingDevice = (BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0;

	CurrentZBufferBitCount = GetDepthStencilBitCount(ZBufferFormat);

	// The default value of D3DRS_POINTSIZE_MIN is 0.0f in D3D8,
	// whereas in D3D9 it is 1.0f, so adjust it as needed
	ProxyInterface->SetRenderState(D3DRS_POINTSIZE_MIN, (DWORD)0.0f);
	// The DEPTHBIAS value of -0.0f works differently than 0.0f
	// Some games require defaulting to -0.0f to work correctly
	const float DepthBias = -0.0f;
	ProxyInterface->SetRenderState(D3DRS_DEPTHBIAS, *(const DWORD *)&DepthBias);
}
Direct3DDevice8::~Direct3DDevice8()
{
	delete ProxyAddressLookupTable;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
		return E_POINTER;

	if (riid == __uuidof(IDirect3DDevice8) ||
		riid == __uuidof(IUnknown))
	{
		AddRef();
		*ppvObj = static_cast<IDirect3DDevice8 *>(this);

		return S_OK;
	}

	const HRESULT hr = ProxyInterface->QueryInterface(ConvertREFIID(riid), ppvObj);
	if (SUCCEEDED(hr))
		GenericQueryInterface(riid, ppvObj, this);

	return hr;
}
ULONG STDMETHODCALLTYPE Direct3DDevice8::AddRef()
{
	ULONG LastRefCount = ProxyInterface->AddRef();

	// Shaders and state blocks increase ref counter in d3d9 but not in d3d8
	DWORD ExtraRefs = VertexShaderAndDeclarationCount + PixelShaderHandles.size() + StateBlockTokens.size();
	if (ExtraRefs <= LastRefCount)
	{
		LastRefCount = LastRefCount - ExtraRefs;
	}

	return LastRefCount;
}

ULONG STDMETHODCALLTYPE Direct3DDevice8::Release()
{
	// Get current value before releasing the device reference
	ULONG LastRefCount = ProxyInterface->AddRef();
	LastRefCount = ProxyInterface->Release();

	// Shaders and StateBlocks are destroyed alongside the device that created them in D3D8 but not in D3D9
	// so we need to Release any remaining shaders or state blocks when the device is released to mirror that behaviour
	DWORD ExtraRefs = VertexShaderAndDeclarationCount + PixelShaderHandles.size() + StateBlockTokens.size();
	if (ExtraRefs <= LastRefCount)
	{
		LastRefCount = LastRefCount - ExtraRefs;
		if (LastRefCount == 1)
		{
			// Release shaders and state blocks when only one reference is left
			ReleaseShadersAndStateBlocks();
		}
	}

	// Release device reference
	LastRefCount = ProxyInterface->Release();

	if (LastRefCount == 0)
		delete this;

	return LastRefCount;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::TestCooperativeLevel()
{
	return ProxyInterface->TestCooperativeLevel();
}
UINT STDMETHODCALLTYPE Direct3DDevice8::GetAvailableTextureMem()
{
	return ProxyInterface->GetAvailableTextureMem();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ResourceManagerDiscardBytes(DWORD Bytes)
{
	UNREFERENCED_PARAMETER(Bytes);

	return ProxyInterface->EvictManagedResources();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDirect3D(IDirect3D8 **ppD3D8)
{
	if (ppD3D8 == nullptr)
		return D3DERR_INVALIDCALL;

	D3D->AddRef();
	*ppD3D8 = D3D;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDeviceCaps(D3DCAPS8 *pCaps)
{
	if (pCaps == nullptr)
		return D3DERR_INVALIDCALL;

	D3DCAPS9 DeviceCaps;

	const HRESULT hr = ProxyInterface->GetDeviceCaps(&DeviceCaps);
	if (FAILED(hr))
		return hr;

	ConvertCaps(DeviceCaps, *pCaps);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
	return ProxyInterface->GetDisplayMode(0, pMode);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return ProxyInterface->GetCreationParameters(pParameters);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8 *pCursorBitmap)
{
	if (pCursorBitmap == nullptr)
		return D3DERR_INVALIDCALL;

	auto pCursorBitmapImpl = static_cast<Direct3DSurface8 *>(pCursorBitmap);
	return ProxyInterface->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmapImpl->GetProxyInterface());
}
void STDMETHODCALLTYPE Direct3DDevice8::SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags)
{
	ProxyInterface->SetCursorPosition(XScreenSpace, YScreenSpace, Flags);
}
BOOL STDMETHODCALLTYPE Direct3DDevice8::ShowCursor(BOOL bShow)
{
	return ProxyInterface->ShowCursor(bShow);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS8 *pPresentationParameters, IDirect3DSwapChain8 **ppSwapChain)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateAdditionalSwapChain" << "(" << this << ", " << pPresentationParameters << ", " << ppSwapChain << ")' ..." << std::endl;
#endif

	if (pPresentationParameters == nullptr || ppSwapChain == nullptr)
		return D3DERR_INVALIDCALL;

	*ppSwapChain = nullptr;

	D3DPRESENT_PARAMETERS PresentParams;
	ConvertPresentParameters(*pPresentationParameters, PresentParams);

	IDirect3DSwapChain9 *SwapChainInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateAdditionalSwapChain(&PresentParams, &SwapChainInterface);
	if (FAILED(hr))
		return hr;

	*ppSwapChain = ProxyAddressLookupTable->FindAddress<Direct3DSwapChain8>(SwapChainInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::Reset(D3DPRESENT_PARAMETERS8 *pPresentationParameters)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::Reset" << "(" << this << ", " << pPresentationParameters << ")' ..." << std::endl;
#endif

	if (pPresentationParameters == nullptr)
		return D3DERR_INVALIDCALL;

	CurrentZBiasRenderState = 0;

	const HRESULT deviceState = ProxyInterface->TestCooperativeLevel();

	if (deviceState == D3DERR_DEVICENOTRESET) {
		while (!StateBlockTokens.empty())
		{
			DWORD Token = *StateBlockTokens.begin();
			DeleteStateBlock(Token);
		}
	}

	D3DPRESENT_PARAMETERS PresentParams;
	ConvertPresentParameters(*pPresentationParameters, PresentParams);

	const HRESULT hr = ProxyInterface->Reset(&PresentParams);

	if (SUCCEEDED(hr))
	{
		// The default value of D3DRS_POINTSIZE_MIN is 0.0f in D3D8,
		// whereas in D3D9 it is 1.0f, so adjust it as needed
		ProxyInterface->SetRenderState(D3DRS_POINTSIZE_MIN, (DWORD) 0.0f);
		// The DEPTHBIAS value of -0.0f works differently than 0.0f
		// Some games require defaulting to -0.0f to work correctly
		float DepthBias = -0.0f;
		ProxyInterface->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)&DepthBias);
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	UNREFERENCED_PARAMETER(pDirtyRegion);

	return ProxyInterface->Present(pSourceRect, pDestRect, hDestWindowOverride, nullptr);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer)
{
	if (ppBackBuffer == nullptr)
		return D3DERR_INVALIDCALL;

	*ppBackBuffer = nullptr;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetBackBuffer(0, iBackBuffer, Type, &SurfaceInterface);
	if (FAILED(hr))
		return hr;

	*ppBackBuffer = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
	return ProxyInterface->GetRasterStatus(0, pRasterStatus);
}
void STDMETHODCALLTYPE Direct3DDevice8::SetGammaRamp(DWORD Flags, const D3DGAMMARAMP *pRamp)
{
	ProxyInterface->SetGammaRamp(0, Flags, pRamp);
}
void STDMETHODCALLTYPE Direct3DDevice8::GetGammaRamp(D3DGAMMARAMP *pRamp)
{
	ProxyInterface->GetGammaRamp(0, pRamp);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8 **ppTexture)
{
	if (ppTexture == nullptr)
		return D3DERR_INVALIDCALL;

	if (Format == D3DFMT_UNKNOWN)
		return D3DERR_INVALIDCALL;

	*ppTexture = nullptr;

	if (Pool == D3DPOOL_DEFAULT)
	{
		D3DDEVICE_CREATION_PARAMETERS CreationParams;
		ProxyInterface->GetCreationParameters(&CreationParams);

		if ((Usage & D3DUSAGE_DYNAMIC) == 0 &&
			SUCCEEDED(D3D->GetProxyInterface()->CheckDeviceFormat(CreationParams.AdapterOrdinal, CreationParams.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, Format)))
		{
			Usage |= D3DUSAGE_RENDERTARGET;
		}
		else if (Usage != D3DUSAGE_DEPTHSTENCIL)
		{
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	IDirect3DTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateTexture(Width, Height, Levels, Usage, Format, Pool, &TextureInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DTexture8>(TextureInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8 **ppVolumeTexture)
{
	if (ppVolumeTexture == nullptr)
		return D3DERR_INVALIDCALL;

	if (Format == D3DFMT_UNKNOWN)
		return D3DERR_INVALIDCALL;

	*ppVolumeTexture = nullptr;

	IDirect3DVolumeTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, &TextureInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppVolumeTexture = ProxyAddressLookupTable->FindAddress<Direct3DVolumeTexture8>(TextureInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture)
{
	if (ppCubeTexture == nullptr)
		return D3DERR_INVALIDCALL;

	if (Format == D3DFMT_UNKNOWN)
		return D3DERR_INVALIDCALL;

	*ppCubeTexture = nullptr;

	IDirect3DCubeTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, &TextureInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppCubeTexture = ProxyAddressLookupTable->FindAddress<Direct3DCubeTexture8>(TextureInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8 **ppVertexBuffer)
{
	if (ppVertexBuffer == nullptr)
		return D3DERR_INVALIDCALL;

	*ppVertexBuffer = nullptr;

	IDirect3DVertexBuffer9 *BufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateVertexBuffer(Length, Usage, FVF, Pool, &BufferInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppVertexBuffer = ProxyAddressLookupTable->FindAddress<Direct3DVertexBuffer8>(BufferInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8 **ppIndexBuffer)
{
	if (ppIndexBuffer == nullptr)
		return D3DERR_INVALIDCALL;

	*ppIndexBuffer = nullptr;

	IDirect3DIndexBuffer9 *BufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateIndexBuffer(Length, Usage, Format, Pool, &BufferInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppIndexBuffer = ProxyAddressLookupTable->FindAddress<Direct3DIndexBuffer8>(BufferInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
		return D3DERR_INVALIDCALL;

	if (Format == D3DFMT_UNKNOWN)
		return D3DERR_INVALIDCALL;

	*ppSurface = nullptr;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateRenderTarget(Width, Height, Format, MultiSample, 0, Lockable, &SurfaceInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppSurface = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8 **ppSurface)
{
	if (ppSurface == nullptr)
		return D3DERR_INVALIDCALL;

	if (Format == D3DFMT_UNKNOWN)
		return D3DERR_INVALIDCALL;

	*ppSurface = nullptr;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateDepthStencilSurface(Width, Height, Format, MultiSample, 0, ZBufferDiscarding, &SurfaceInterface, nullptr);
	if (FAILED(hr))
		return hr;

	*ppSurface = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8 **ppSurface)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateImageSurface" << "(" << this << ", " << Width << ", " << Height << ", " << Format << ", " << ppSurface << ")' ..." << std::endl;
#endif

	if (ppSurface == nullptr)
		return D3DERR_INVALIDCALL;

	// Only 'CreateImageSurface' clears the content of ppSurface before checking if Format is equal to D3DFMT_UNKNOWN.
	*ppSurface = nullptr;

	if (Format == D3DFMT_UNKNOWN)
		return D3DERR_INVALIDCALL;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_SYSTEMMEM, &SurfaceInterface, nullptr);

	if (FAILED(hr) && FAILED(ProxyInterface->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_SCRATCH, &SurfaceInterface, nullptr)))
	{
#ifndef D3D8TO9NOLOG
		LOG << "> 'IDirect3DDevice9::CreateOffscreenPlainSurface' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
		return hr;
	}

	*ppSurface = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
// GeneralsX (patch 0004): CopyRects where D3D9 has no GPU copy - into a managed texture level (every font upload) or out
// of a render target into system memory (screenshots) - by locking both sides. Upstream needed d3dx9_43.dll for these,
// which a fresh Windows does not have. Same format on both sides (the caller checks). A render-target source is read
// back through system memory first; a destination D3D9 cannot lock is staged in system memory and uploaded.
static HRESULT GxCopyRectByLock(IDirect3DDevice9 *Device, IDirect3DSurface9 *Source, const D3DSURFACE_DESC &SourceDesc, const RECT &SourceRect,
	IDirect3DSurface9 *Destination, const D3DSURFACE_DESC &DestinationDesc, const RECT &DestinationRect)
{
	const UINT Width = SourceRect.right - SourceRect.left, Height = SourceRect.bottom - SourceRect.top;
	const bool Blocks = SourceDesc.Format == D3DFMT_DXT1 || SourceDesc.Format == D3DFMT_DXT2 || SourceDesc.Format == D3DFMT_DXT3 ||
		SourceDesc.Format == D3DFMT_DXT4 || SourceDesc.Format == D3DFMT_DXT5;
	const UINT RowBytes = CalcTextureSize(Width, Blocks ? 4 : 1, 1, SourceDesc.Format);
	const UINT Rows = Blocks ? (Height + 3) / 4 : Height;
	if (RowBytes == 0 || Rows == 0)
		return D3DERR_INVALIDCALL;

	IDirect3DSurface9 *Read = Source, *ReadTemp = nullptr, *Write = Destination, *WriteTemp = nullptr;
	RECT WriteRect = DestinationRect;
	HRESULT hr = D3D_OK;
	if (SourceDesc.Pool == D3DPOOL_DEFAULT)
	{
		hr = Device->CreateOffscreenPlainSurface(SourceDesc.Width, SourceDesc.Height, SourceDesc.Format, D3DPOOL_SYSTEMMEM, &ReadTemp, nullptr);
		if (SUCCEEDED(hr))
			hr = Device->GetRenderTargetData(Source, ReadTemp);
		Read = ReadTemp;
	}
	if (SUCCEEDED(hr) && DestinationDesc.Pool == D3DPOOL_DEFAULT)
	{
		hr = Device->CreateOffscreenPlainSurface(Width, Height, DestinationDesc.Format, D3DPOOL_SYSTEMMEM, &WriteTemp, nullptr);
		Write = WriteTemp;
		WriteRect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
	}
	if (SUCCEEDED(hr))
	{
		D3DLOCKED_RECT From, To;
		hr = Read->LockRect(&From, &SourceRect, D3DLOCK_READONLY);
		if (SUCCEEDED(hr))
		{
			hr = Write->LockRect(&To, &WriteRect, 0);
			if (SUCCEEDED(hr))
			{
				for (UINT Row = 0; Row < Rows; ++Row)
					memcpy(static_cast<BYTE *>(To.pBits) + Row * To.Pitch, static_cast<const BYTE *>(From.pBits) + Row * From.Pitch, RowBytes);
				Write->UnlockRect();
			}
			Read->UnlockRect();
		}
	}
	if (SUCCEEDED(hr) && WriteTemp != nullptr)
	{
		const POINT Point = { DestinationRect.left, DestinationRect.top };
		hr = Device->UpdateSurface(WriteTemp, nullptr, Destination, &Point);
	}
	if (ReadTemp != nullptr)
		ReadTemp->Release();
	if (WriteTemp != nullptr)
		WriteTemp->Release();
	return hr;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CopyRects(IDirect3DSurface8 *pSourceSurface, const RECT *pSourceRectsArray, UINT cRects, IDirect3DSurface8 *pDestinationSurface, const POINT *pDestPointsArray)
{
	if (pSourceSurface == nullptr || pDestinationSurface == nullptr || pSourceSurface == pDestinationSurface)
		return D3DERR_INVALIDCALL;

	auto pSourceSurfaceImpl = static_cast<Direct3DSurface8 *>(pSourceSurface);
	auto pDestinationSurfaceImpl = static_cast<Direct3DSurface8 *>(pDestinationSurface);

	D3DSURFACE_DESC SourceDesc, DestinationDesc;
	pSourceSurfaceImpl->GetProxyInterface()->GetDesc(&SourceDesc);
	pDestinationSurfaceImpl->GetProxyInterface()->GetDesc(&DestinationDesc);

	if (SourceDesc.Format != DestinationDesc.Format)
		return D3DERR_INVALIDCALL;

	if (GetDepthStencilBitCount(SourceDesc.Format) != 0)
		return D3DERR_INVALIDCALL;

	HRESULT hr = D3DERR_INVALIDCALL;

	if (cRects == 0)
		cRects  = 1;

	for (UINT i = 0; i < cRects; i++)
	{
		RECT SourceRect, DestinationRect;

		if (pSourceRectsArray != nullptr)
		{
			SourceRect = pSourceRectsArray[i];
		}
		else
		{
			SourceRect.left = 0;
			SourceRect.right = SourceDesc.Width;
			SourceRect.top = 0;
			SourceRect.bottom = SourceDesc.Height;
		}

		if (pDestPointsArray != nullptr)
		{
			DestinationRect.left = pDestPointsArray[i].x;
			DestinationRect.right = DestinationRect.left + (SourceRect.right - SourceRect.left);
			DestinationRect.top = pDestPointsArray[i].y;
			DestinationRect.bottom = DestinationRect.top + (SourceRect.bottom - SourceRect.top);
		}
		else
		{
			DestinationRect = SourceRect;
		}

		if (SourceDesc.Pool == D3DPOOL_MANAGED || DestinationDesc.Pool != D3DPOOL_DEFAULT)
		{
			hr = GxCopyRectByLock(ProxyInterface, pSourceSurfaceImpl->GetProxyInterface(), SourceDesc, SourceRect,
				pDestinationSurfaceImpl->GetProxyInterface(), DestinationDesc, DestinationRect);
			if (FAILED(hr) && D3DXLoadSurfaceFromSurface != nullptr)
			{
				if (SUCCEEDED(D3DXLoadSurfaceFromSurface(pDestinationSurfaceImpl->GetProxyInterface(), nullptr, &DestinationRect, pSourceSurfaceImpl->GetProxyInterface(), nullptr, &SourceRect, D3DX_FILTER_NONE, 0)))
				{
					// Explicitly call AddDirtyRect on the surface
					void *pContainer = nullptr;
					if (SUCCEEDED(pDestinationSurfaceImpl->GetContainer(IID_IDirect3DTexture9, &pContainer)) && pContainer)
					{
						IDirect3DTexture9 *pTexture = (IDirect3DTexture9*)pContainer;
						pTexture->AddDirtyRect(&DestinationRect);
						pTexture->Release();
					}
					hr = D3D_OK;
				}
			}
		}
		else if (SourceDesc.Pool == D3DPOOL_DEFAULT)
		{
			hr = ProxyInterface->StretchRect(pSourceSurfaceImpl->GetProxyInterface(), &SourceRect, pDestinationSurfaceImpl->GetProxyInterface(), &DestinationRect, D3DTEXF_NONE);
		}
		else if (SourceDesc.Pool == D3DPOOL_SYSTEMMEM)
		{
			const POINT pt = { DestinationRect.left, DestinationRect.top };

			hr = ProxyInterface->UpdateSurface(pSourceSurfaceImpl->GetProxyInterface(), &SourceRect, pDestinationSurfaceImpl->GetProxyInterface(), &pt);
		}

		if (FAILED(hr))
		{
#ifndef D3D8TO9NOLOG
			LOG << "Failed to translate 'IDirect3DDevice8::CopyRects' call from '[" << SourceDesc.Width << "x" << SourceDesc.Height << ", " << SourceDesc.Format << ", " << SourceDesc.MultiSampleType << ", " << SourceDesc.Usage << ", " << SourceDesc.Pool << "]' to '[" << DestinationDesc.Width << "x" << DestinationDesc.Height << ", " << DestinationDesc.Format << ", " << DestinationDesc.MultiSampleType << ", " << DestinationDesc.Usage << ", " << DestinationDesc.Pool << "]'!" << std::endl;
#endif
			break;
		}
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::UpdateTexture(IDirect3DBaseTexture8 *pSourceTexture, IDirect3DBaseTexture8 *pDestinationTexture)
{
	if (pSourceTexture == nullptr || pDestinationTexture == nullptr || pSourceTexture->GetType() != pDestinationTexture->GetType())
		return D3DERR_INVALIDCALL;

	IDirect3DBaseTexture9 *SourceBaseTextureInterface, *DestinationBaseTextureInterface;

	switch (pSourceTexture->GetType())
	{
	case D3DRTYPE_TEXTURE:
		SourceBaseTextureInterface = static_cast<Direct3DTexture8 *>(pSourceTexture)->GetProxyInterface();
		DestinationBaseTextureInterface = static_cast<Direct3DTexture8 *>(pDestinationTexture)->GetProxyInterface();
		break;
	case D3DRTYPE_VOLUMETEXTURE:
		SourceBaseTextureInterface = static_cast<Direct3DVolumeTexture8 *>(pSourceTexture)->GetProxyInterface();
		DestinationBaseTextureInterface = static_cast<Direct3DVolumeTexture8 *>(pDestinationTexture)->GetProxyInterface();
		break;
	case D3DRTYPE_CUBETEXTURE:
		SourceBaseTextureInterface = static_cast<Direct3DCubeTexture8 *>(pSourceTexture)->GetProxyInterface();
		DestinationBaseTextureInterface = static_cast<Direct3DCubeTexture8 *>(pDestinationTexture)->GetProxyInterface();
		break;
	default:
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->UpdateTexture(SourceBaseTextureInterface, DestinationBaseTextureInterface);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetFrontBuffer(IDirect3DSurface8 *pDestSurface)
{
	if (pDestSurface == nullptr)
		return D3DERR_INVALIDCALL;

	auto pDestSurfaceImpl = static_cast<Direct3DSurface8 *>(pDestSurface);
	return ProxyInterface->GetFrontBufferData(0, pDestSurfaceImpl->GetProxyInterface());
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetRenderTarget(IDirect3DSurface8 *pRenderTarget, IDirect3DSurface8 *pNewZStencil)
{
	HRESULT hr;

	if (pRenderTarget != nullptr)
	{
		auto pRenderTargetImpl = static_cast<Direct3DSurface8 *>(pRenderTarget);
		hr = ProxyInterface->SetRenderTarget(0, pRenderTargetImpl->GetProxyInterface());
		if (FAILED(hr))
			return hr;
	}

	if (pNewZStencil != nullptr)
	{
		auto pNewZStencilImpl = static_cast<Direct3DSurface8 *>(pNewZStencil);
		hr = ProxyInterface->SetDepthStencilSurface(pNewZStencilImpl->GetProxyInterface());
		if (FAILED(hr))
			return hr;

		D3DSURFACE_DESC8 Desc = {};
		pNewZStencilImpl->GetDesc(&Desc);

		CurrentZBufferBitCount = GetDepthStencilBitCount(Desc.Format);

		ProxyInterface->SetRenderState(D3DRS_DEPTHBIAS, CalcDepthBias(CurrentZBiasRenderState, CurrentZBufferBitCount));
	}
	else
	{
		ProxyInterface->SetDepthStencilSurface(nullptr);
	}

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRenderTarget(IDirect3DSurface8 **ppRenderTarget)
{
	if (ppRenderTarget == nullptr)
		return D3DERR_INVALIDCALL;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetRenderTarget(0, &SurfaceInterface);
	if (FAILED(hr))
		return hr;

	*ppRenderTarget = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDepthStencilSurface(IDirect3DSurface8 **ppZStencilSurface)
{
	if (ppZStencilSurface == nullptr)
		return D3DERR_INVALIDCALL;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetDepthStencilSurface(&SurfaceInterface);
	if (FAILED(hr))
		return hr;

	*ppZStencilSurface = ProxyAddressLookupTable->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::BeginScene()
{
	return ProxyInterface->BeginScene();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::EndScene()
{
	return ProxyInterface->EndScene();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return ProxyInterface->Clear(Count, pRects, Flags, Color, Z, Stencil);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return ProxyInterface->SetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix)
{
	return ProxyInterface->GetTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix)
{
	return ProxyInterface->MultiplyTransform(State, pMatrix);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetViewport(const D3DVIEWPORT8 *pViewport)
{
	IDirect3DSurface9 *pCurrentRenderTarget = nullptr;
	if (SUCCEEDED(ProxyInterface->GetRenderTarget(0, &pCurrentRenderTarget)))
	{
		D3DSURFACE_DESC Desc;
		pCurrentRenderTarget->GetDesc(&Desc);

		pCurrentRenderTarget->Release();

		if (pViewport->Y + pViewport->Height > Desc.Height || pViewport->X + pViewport->Width > Desc.Width)
			return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->SetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetViewport(D3DVIEWPORT8 *pViewport)
{
	return ProxyInterface->GetViewport(pViewport);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetMaterial(const D3DMATERIAL8 *pMaterial)
{
	return ProxyInterface->SetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetMaterial(D3DMATERIAL8 *pMaterial)
{
	return ProxyInterface->GetMaterial(pMaterial);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetLight(DWORD Index, const D3DLIGHT8 *pLight)
{
	if (pLight == nullptr)
		return D3DERR_INVALIDCALL;

	D3DLIGHT8 Light = *pLight;

	// Make spot light work more like it did in Direct3D 8
	if (Light.Type == D3DLIGHTTYPE::D3DLIGHT_SPOT)
	{
		// Theta must be in the range from 0 through the value specified by Phi
		if (Light.Theta <= Light.Phi)
		{
			Light.Theta /= 1.75f;
		}
	}

	return ProxyInterface->SetLight(Index, &Light);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetLight(DWORD Index, D3DLIGHT8 *pLight)
{
	return ProxyInterface->GetLight(Index, pLight);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::LightEnable(DWORD Index, BOOL Enable)
{
	return ProxyInterface->LightEnable(Index, Enable);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetLightEnable(DWORD Index, BOOL *pEnable)
{
	return ProxyInterface->GetLightEnable(Index, pEnable);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetClipPlane(DWORD Index, const float *pPlane)
{
	if (pPlane == nullptr || Index >= MAX_CLIP_PLANES)
		return D3DERR_INVALIDCALL;

	memcpy(StoredClipPlanes[Index], pPlane, sizeof(StoredClipPlanes[0]));
	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetClipPlane(DWORD Index, float *pPlane)
{
	if (pPlane == nullptr || Index >= MAX_CLIP_PLANES)
		return D3DERR_INVALIDCALL;

	memcpy(pPlane, StoredClipPlanes[Index], sizeof(StoredClipPlanes[0]));
	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	HRESULT hr;

	switch (static_cast<DWORD>(State))
	{
	case D3DRS_ZVISIBLE:
	case D3DRS_PATCHSEGMENTS:
	case D3DRS_LINEPATTERN:
		return D3D_OK;
	case D3DRS_SOFTWAREVERTEXPROCESSING:
		// SWVP can be modified by this render state only on devices
		// created with the D3DCREATE_MIXED_VERTEXPROCESSING flag
		if (IsMixedVertexProcessingDevice)
			return ProxyInterface->SetSoftwareVertexProcessing(static_cast<BOOL>(Value));
		return D3D_OK;
	case D3DRS_EDGEANTIALIAS:
		return ProxyInterface->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, Value);
	case D3DRS_CLIPPLANEENABLE:
		hr = ProxyInterface->SetRenderState(State, Value);
		if (SUCCEEDED(hr))
			ClipPlaneRenderState = Value;
		return hr;
	case D3DRS_ZBIAS:
		CurrentZBiasRenderState = Value;
		Value = CalcDepthBias(Value, CurrentZBufferBitCount);
		State = D3DRS_DEPTHBIAS;
	default:
		return ProxyInterface->SetRenderState(State, Value);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue)
{
	if (pValue == nullptr)
		return D3DERR_INVALIDCALL;

	*pValue = 0;

	switch (static_cast<DWORD>(State))
	{
	case D3DRS_ZVISIBLE:
	case D3DRS_LINEPATTERN:
		*pValue = 0;
		return D3D_OK;
	case D3DRS_EDGEANTIALIAS:
		return ProxyInterface->GetRenderState(D3DRS_ANTIALIASEDLINEENABLE, pValue);
	case D3DRS_ZBIAS:
		*pValue = CurrentZBiasRenderState;
		return D3D_OK;
	case D3DRS_SOFTWAREVERTEXPROCESSING:
		*pValue = static_cast<DWORD>(ProxyInterface->GetSoftwareVertexProcessing());
		return D3D_OK;
	case D3DRS_PATCHSEGMENTS:
		*pValue = 1;
		return D3D_OK;
	default:
		return ProxyInterface->GetRenderState(State, pValue);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::BeginStateBlock()
{
	if (IsRecordingState)
		return D3DERR_INVALIDCALL;

	HRESULT hr = ProxyInterface->BeginStateBlock();

	if (SUCCEEDED(hr))
		IsRecordingState = true;

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::EndStateBlock(DWORD *pToken)
{
	if (pToken == nullptr)
		return D3DERR_INVALIDCALL;

	if (!IsRecordingState)
		return D3DERR_INVALIDCALL;

	IDirect3DStateBlock9 *StateBlock = nullptr;
	HRESULT hr = ProxyInterface->EndStateBlock(&StateBlock);

	if (SUCCEEDED(hr))
	{
		*pToken = StateBlockTable.Add(StateBlock);
		StateBlockTokens.insert(*pToken);
		IsRecordingState = false;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ApplyStateBlock(DWORD Token)
{
	if (Token == 0)
		return D3DERR_INVALIDCALL;

	if (IsRecordingState)
		return D3DERR_INVALIDCALL;

	if (StateBlockTokens.find(Token) == StateBlockTokens.end())
		return D3D_OK;

	return StateBlockTable.Get(Token)->Apply();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CaptureStateBlock(DWORD Token)
{
	if (Token == 0)
		return D3DERR_INVALIDCALL;

	if (IsRecordingState)
		return D3DERR_INVALIDCALL;

	if (StateBlockTokens.find(Token) == StateBlockTokens.end())
		return D3D_OK;

	return StateBlockTable.Get(Token)->Capture();
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeleteStateBlock(DWORD Token)
{
	if (Token == 0)
		return D3DERR_INVALIDCALL;

	if (IsRecordingState)
		return D3DERR_INVALIDCALL;

	if (StateBlockTokens.find(Token) == StateBlockTokens.end())
		return D3D_OK;

	StateBlockTable.Get(Token)->Release();
	StateBlockTable.Remove(Token);

	StateBlockTokens.erase(Token);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD *pToken)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateStateBlock" << "(" << Type << ", " << pToken << ")' ..." << std::endl;
#endif

	if (pToken == nullptr)
		return D3DERR_INVALIDCALL;

	if (IsRecordingState)
		return D3DERR_INVALIDCALL;

	IDirect3DStateBlock9 *StateBlock = nullptr;
	HRESULT hr = ProxyInterface->CreateStateBlock(Type, &StateBlock);

	if (SUCCEEDED(hr))
	{
		*pToken = StateBlockTable.Add(StateBlock);
		StateBlockTokens.insert(*pToken);
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetClipStatus(const D3DCLIPSTATUS8 *pClipStatus)
{
	return ProxyInterface->SetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetClipStatus(D3DCLIPSTATUS8 *pClipStatus)
{
	return ProxyInterface->GetClipStatus(pClipStatus);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTexture(DWORD Stage, IDirect3DBaseTexture8 **ppTexture)
{
	if (ppTexture == nullptr)
		return D3DERR_INVALIDCALL;

	*ppTexture = nullptr;

	IDirect3DBaseTexture9 *BaseTextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetTexture(Stage, &BaseTextureInterface);
	if (FAILED(hr))
		return hr;

	if (BaseTextureInterface != nullptr)
	{
		IDirect3DTexture9 *TextureInterface = nullptr;
		IDirect3DCubeTexture9 *CubeTextureInterface = nullptr;
		IDirect3DVolumeTexture9 *VolumeTextureInterface = nullptr;

		switch (BaseTextureInterface->GetType())
		{
		case D3DRTYPE_TEXTURE:
			BaseTextureInterface->QueryInterface(IID_PPV_ARGS(&TextureInterface));
			*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DTexture8>(TextureInterface);
			BaseTextureInterface->Release();
			break;
		case D3DRTYPE_VOLUMETEXTURE:
			BaseTextureInterface->QueryInterface(IID_PPV_ARGS(&VolumeTextureInterface));
			*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DVolumeTexture8>(VolumeTextureInterface);
			BaseTextureInterface->Release();
			break;
		case D3DRTYPE_CUBETEXTURE:
			BaseTextureInterface->QueryInterface(IID_PPV_ARGS(&CubeTextureInterface));
			*ppTexture = ProxyAddressLookupTable->FindAddress<Direct3DCubeTexture8>(CubeTextureInterface);
			BaseTextureInterface->Release();
			break;
		default:
			BaseTextureInterface->Release();
			return D3DERR_INVALIDCALL;
		}
	}

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture)
{
	if (pTexture == nullptr)
		return ProxyInterface->SetTexture(Stage, nullptr);

	IDirect3DBaseTexture9 *BaseTextureInterface;

	switch (pTexture->GetType())
	{
	case D3DRTYPE_TEXTURE:
		BaseTextureInterface = static_cast<Direct3DTexture8 *>(pTexture)->GetProxyInterface();
		break;
	case D3DRTYPE_VOLUMETEXTURE:
		BaseTextureInterface = static_cast<Direct3DVolumeTexture8 *>(pTexture)->GetProxyInterface();
		break;
	case D3DRTYPE_CUBETEXTURE:
		BaseTextureInterface = static_cast<Direct3DCubeTexture8 *>(pTexture)->GetProxyInterface();
		break;
	default:
		return D3DERR_INVALIDCALL;
	}

	return ProxyInterface->SetTexture(Stage, BaseTextureInterface);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue)
{
	switch (static_cast<DWORD>(Type))
	{
	case D3DTSS_ADDRESSU:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_ADDRESSU, pValue);
	case D3DTSS_ADDRESSV:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_ADDRESSV, pValue);
	case D3DTSS_ADDRESSW:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_ADDRESSW, pValue);
	case D3DTSS_BORDERCOLOR:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_BORDERCOLOR, pValue);
	case D3DTSS_MAGFILTER:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MAGFILTER, pValue);
	case D3DTSS_MINFILTER:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MINFILTER, pValue);
	case D3DTSS_MIPFILTER:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MIPFILTER, pValue);
	case D3DTSS_MIPMAPLODBIAS:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, pValue);
	case D3DTSS_MAXMIPLEVEL:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, pValue);
	case D3DTSS_MAXANISOTROPY:
		return ProxyInterface->GetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, pValue);
	default:
		return ProxyInterface->GetTextureStageState(Stage, Type, pValue);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	switch (static_cast<DWORD>(Type))
	{
	case D3DTSS_ADDRESSU:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_ADDRESSU, Value);
	case D3DTSS_ADDRESSV:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_ADDRESSV, Value);
	case D3DTSS_ADDRESSW:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_ADDRESSW, Value);
	case D3DTSS_BORDERCOLOR:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_BORDERCOLOR, Value);
	case D3DTSS_MAGFILTER:
		if (Value == D3DTEXF_FLATCUBIC || Value == D3DTEXF_GAUSSIANCUBIC)
			Value = D3DTEXF_LINEAR;
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MAGFILTER, Value);
	case D3DTSS_MINFILTER:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MINFILTER, Value);
	case D3DTSS_MIPFILTER:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MIPFILTER, Value);
	case D3DTSS_MIPMAPLODBIAS:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, Value);
	case D3DTSS_MAXMIPLEVEL:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, Value);
	case D3DTSS_MAXANISOTROPY:
		return ProxyInterface->SetSamplerState(Stage, D3DSAMP_MAXANISOTROPY, Value);
	default:
		return ProxyInterface->SetTextureStageState(Stage, Type, Value);
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ValidateDevice(DWORD *pNumPasses)
{
	return ProxyInterface->ValidateDevice(pNumPasses);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetInfo(DWORD DevInfoID, void *pDevInfoStruct, DWORD DevInfoStructSize)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetInfo" << "(" << this << ", " << DevInfoID << ", " << pDevInfoStruct << ", " << DevInfoStructSize << ")' ..." << std::endl;
#endif

	if (pDevInfoStruct == nullptr || DevInfoStructSize == 0)
		return D3DERR_INVALIDCALL;

	HRESULT hr;
	IDirect3DQuery9 *pQuery = nullptr;

	switch (DevInfoID)
	{
		case 0:
		case D3DDEVINFOID_TEXTUREMANAGER:
		case D3DDEVINFOID_D3DTEXTUREMANAGER:
		case D3DDEVINFOID_TEXTURING:
			return E_FAIL; // Unsupported query IDs

		case D3DDEVINFOID_VCACHE:
			hr = ProxyInterface->CreateQuery(D3DQUERYTYPE_VCACHE, &pQuery);

			if (FAILED(hr))
			{
				if (DevInfoStructSize != sizeof(D3DDEVINFO_VCACHE))
					return D3DERR_INVALIDCALL;

				// The contents of pDevInfoStruct are zeroed before return
				memset(pDevInfoStruct, 0, sizeof(D3DDEVINFO_VCACHE));
				return S_FALSE;
			}

			break;

		case D3DDEVINFOID_RESOURCEMANAGER:
			hr = ProxyInterface->CreateQuery(D3DQUERYTYPE_RESOURCEMANAGER, &pQuery);
			break;

		case D3DDEVINFOID_VERTEXSTATS:
			hr = ProxyInterface->CreateQuery(D3DQUERYTYPE_VERTEXSTATS, &pQuery);
			break;

		default: // D3DDEVINFOID_UNKNOWN
			return E_FAIL;
	}

	if ((FAILED(hr)))
	{
		if (hr == D3DERR_NOTAVAILABLE)
		{
			return E_FAIL;
		}
		else
		{
			return S_FALSE;
		}
	}

	if (pQuery != nullptr)
	{
		pQuery->Issue(D3DISSUE_END);
		hr = pQuery->GetData(pDevInfoStruct, DevInfoStructSize, D3DGETDATA_FLUSH);

		pQuery->Release();
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY *pEntries)
{
	if (pEntries == nullptr)
		return D3DERR_INVALIDCALL;

	return ProxyInterface->SetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
	if (pEntries == nullptr)
		return D3DERR_INVALIDCALL;

	return ProxyInterface->GetPaletteEntries(PaletteNumber, pEntries);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetCurrentTexturePalette(UINT PaletteNumber)
{
	if (!IsPaletteSupported)
		return D3DERR_INVALIDCALL;

	return ProxyInterface->SetCurrentTexturePalette(PaletteNumber);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetCurrentTexturePalette(UINT *pPaletteNumber)
{
	if (!IsPaletteSupported)
		return D3DERR_INVALIDCALL;

	return ProxyInterface->GetCurrentTexturePalette(pPaletteNumber);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	ApplyClipPlanes();
	ProxyInterface->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	ApplyClipPlanes();
	ProxyInterface->DrawIndexedPrimitive(PrimitiveType, CurrentBaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	ApplyClipPlanes();
	ProxyInterface->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, const void *pIndexData, D3DFORMAT IndexDataFormat, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	ApplyClipPlanes();
	ProxyInterface->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8 *pDestBuffer, DWORD Flags)
{
	if (pDestBuffer == nullptr)
		return D3DERR_INVALIDCALL;

	Direct3DVertexBuffer8 *pDestBufferImpl = static_cast<Direct3DVertexBuffer8 *>(pDestBuffer);
	return ProxyInterface->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBufferImpl->GetProxyInterface(), nullptr, Flags);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVertexShader(const DWORD *pDeclaration, const DWORD *pFunction, DWORD *pHandle, DWORD Usage)
{
	UNREFERENCED_PARAMETER(Usage);

#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreateVertexShader" << "(" << this << ", " << pDeclaration << ", " << pFunction << ", " << pHandle << ", " << Usage << ")' ..." << std::endl;
#endif

	if (pDeclaration == nullptr || pHandle == nullptr)
		return D3DERR_INVALIDCALL;

	*pHandle = 0;

	const DWORD *const pDeclarationStart = pDeclaration;	// GeneralsX: the loop below walks pDeclaration to its end

	UINT ElementIndex = 0;
	const UINT ElementLimit = 32;
	std::string ConstantsCode;
	WORD Stream = 0, Offset = 0;
	DWORD VertexShaderInputs[ElementLimit];
	D3DVERTEXELEMENT9 VertexElements[ElementLimit];

#ifndef D3D8TO9NOLOG
	LOG << "> Translating vertex declaration ..." << std::endl;
#endif

	static const BYTE DeclTypes[][2] =
	{
		{ D3DDECLTYPE_FLOAT1, 4 },
		{ D3DDECLTYPE_FLOAT2, 8 },
		{ D3DDECLTYPE_FLOAT3, 12 },
		{ D3DDECLTYPE_FLOAT4, 16 },
		{ D3DDECLTYPE_D3DCOLOR, 4 },
		{ D3DDECLTYPE_UBYTE4, 4 },
		{ D3DDECLTYPE_SHORT2, 4 },
		{ D3DDECLTYPE_SHORT4, 8 },
		{ D3DDECLTYPE_UBYTE4N, 4 },
		{ D3DDECLTYPE_SHORT2N, 4 },
		{ D3DDECLTYPE_SHORT4N, 8 },
		{ D3DDECLTYPE_USHORT2N, 4 },
		{ D3DDECLTYPE_USHORT4N, 8 },
		{ D3DDECLTYPE_UDEC3, 6 },
		{ D3DDECLTYPE_DEC3N, 6 },
		{ D3DDECLTYPE_FLOAT16_2, 8 },
		{ D3DDECLTYPE_FLOAT16_4, 16 }
	};
	static const BYTE DeclAddressUsages[][2] =
	{
		{ D3DDECLUSAGE_POSITION, 0 },
		{ D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ D3DDECLUSAGE_BLENDINDICES, 0 },
		{ D3DDECLUSAGE_NORMAL, 0 },
		{ D3DDECLUSAGE_PSIZE, 0 },
		{ D3DDECLUSAGE_COLOR, 0 },
		{ D3DDECLUSAGE_COLOR, 1 },
		{ D3DDECLUSAGE_TEXCOORD, 0 },
		{ D3DDECLUSAGE_TEXCOORD, 1 },
		{ D3DDECLUSAGE_TEXCOORD, 2 },
		{ D3DDECLUSAGE_TEXCOORD, 3 },
		{ D3DDECLUSAGE_TEXCOORD, 4 },
		{ D3DDECLUSAGE_TEXCOORD, 5 },
		{ D3DDECLUSAGE_TEXCOORD, 6 },
		{ D3DDECLUSAGE_TEXCOORD, 7 },
		{ D3DDECLUSAGE_POSITION, 1 },
		{ D3DDECLUSAGE_NORMAL, 1 }
	};

	while (ElementIndex < ElementLimit)
	{
		const DWORD Token = *pDeclaration;
		const DWORD TokenType = (Token & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;

		if (Token == D3DVSD_END())
		{
			break;
		}
		else if (TokenType == D3DVSD_TOKEN_STREAM)
		{
			Stream = static_cast<WORD>((Token & D3DVSD_STREAMNUMBERMASK) >> D3DVSD_STREAMNUMBERSHIFT);
			Offset = 0;
		}
		else if (TokenType == D3DVSD_TOKEN_STREAMDATA && !(Token & 0x10000000))
		{
			VertexElements[ElementIndex].Stream = Stream;
			VertexElements[ElementIndex].Offset = Offset;
			const DWORD type = (Token & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT;
			VertexElements[ElementIndex].Type = DeclTypes[type][0];
			Offset += DeclTypes[type][1];
			VertexElements[ElementIndex].Method = D3DDECLMETHOD_DEFAULT;
			const DWORD Address = (Token & D3DVSD_VERTEXREGMASK) >> D3DVSD_VERTEXREGSHIFT;
			VertexElements[ElementIndex].Usage = DeclAddressUsages[Address][0];
			VertexElements[ElementIndex].UsageIndex = DeclAddressUsages[Address][1];

			VertexShaderInputs[ElementIndex++] = Address;
		}
		else if (TokenType == D3DVSD_TOKEN_STREAMDATA && (Token & 0x10000000))
		{
			Offset += ((Token & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT) * sizeof(DWORD);
		}
		else if (TokenType == D3DVSD_TOKEN_TESSELLATOR && !(Token & 0x10000000))
		{
			VertexElements[ElementIndex].Stream = Stream;
			VertexElements[ElementIndex].Offset = Offset;

			const DWORD UsageType = (Token & D3DVSD_VERTEXREGINMASK) >> D3DVSD_VERTEXREGINSHIFT;

			for (UINT r = 0; r < ElementIndex; ++r)
			{
				if (VertexElements[r].Usage == DeclAddressUsages[UsageType][0] && VertexElements[r].UsageIndex == DeclAddressUsages[UsageType][1])
				{
					VertexElements[ElementIndex].Stream = VertexElements[r].Stream;
					VertexElements[ElementIndex].Offset = VertexElements[r].Offset;
					break;
				}
			}

			VertexElements[ElementIndex].Type = D3DDECLTYPE_FLOAT3;
			VertexElements[ElementIndex].Method = D3DDECLMETHOD_CROSSUV;
			const DWORD Address = (Token & 0xF);
			VertexElements[ElementIndex].Usage = DeclAddressUsages[Address][0];
			VertexElements[ElementIndex].UsageIndex = DeclAddressUsages[Address][1];

			if (VertexElements[ElementIndex].Usage == D3DDECLUSAGE_BLENDINDICES)
			{
				VertexElements[ElementIndex].Method = D3DDECLMETHOD_DEFAULT;
			}

			VertexShaderInputs[ElementIndex++] = Address;
		}
		else if (TokenType == D3DVSD_TOKEN_TESSELLATOR && (Token & 0x10000000))
		{
			VertexElements[ElementIndex].Stream = 0;
			VertexElements[ElementIndex].Offset = 0;
			VertexElements[ElementIndex].Type = D3DDECLTYPE_UNUSED;
			VertexElements[ElementIndex].Method = D3DDECLMETHOD_UV;
			const DWORD Address = (Token & 0xF);
			VertexElements[ElementIndex].Usage = DeclAddressUsages[Address][0];
			VertexElements[ElementIndex].UsageIndex = DeclAddressUsages[Address][1];

			if (VertexElements[ElementIndex].Usage == D3DDECLUSAGE_BLENDINDICES)
			{
				VertexElements[ElementIndex].Method = D3DDECLMETHOD_DEFAULT;
			}

			VertexShaderInputs[ElementIndex++] = Address;
		}
		else if (TokenType == D3DVSD_TOKEN_CONSTMEM)
		{
			const DWORD RegisterCount = 4 * ((Token & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT);
			DWORD Address = (Token & D3DVSD_CONSTADDRESSMASK) >> D3DVSD_CONSTADDRESSSHIFT;

			for (DWORD RegisterIndex = 0; RegisterIndex < RegisterCount; RegisterIndex += 4, ++Address)
			{
				ConstantsCode += "    def c" + std::to_string(Address) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 1])) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 2])) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 3])) + ", " +
					std::to_string(*reinterpret_cast<const float *>(&pDeclaration[RegisterIndex + 4])) + " /* vertex declaration constant */\n";
			}

			pDeclaration += RegisterCount;
		}
		else
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed because token type '" << TokenType << "' is not supported!" << std::endl;
#endif

			return D3DERR_INVALIDCALL;
		}

		++pDeclaration;
	}

	const D3DVERTEXELEMENT9 Terminator = D3DDECL_END();
	VertexElements[ElementIndex] = Terminator;

	HRESULT hr;
	VertexShaderInfo *ShaderInfo;

	if (pFunction != nullptr)
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Disassembling shader and translating assembly to Direct3D 9 compatible code ..." << std::endl;
#endif

		if (*pFunction < D3DVS_VERSION(1, 0) || *pFunction > D3DVS_VERSION(1, 1))
		{
#ifndef D3D8TO9NOLOG
			LOG << "> Failed because of version mismatch ('" << std::showbase << std::hex << *pFunction << std::dec << std::noshowbase << "')! Only 'vs_1_x' shaders are supported." << std::endl;
#endif

			return D3DERR_INVALIDCALL;
		}

		// GeneralsX: no D3DX. The D3D8 program gains the dcl inputs its declaration implied (patch 0003, gx_shader_tokens.cpp,
		// adapted from DXVK's D3D8 front end); it is otherwise passed to D3D9 as written.
		std::vector<std::uint32_t> Tokens;
		std::string Why;

		if (!gx_vs8to9(reinterpret_cast<const std::uint32_t *>(pDeclarationStart), reinterpret_cast<const std::uint32_t *>(pFunction), Tokens, &Why))
		{
			return D3DERR_INVALIDCALL;
		}

		ShaderInfo = new VertexShaderInfo();

		hr = ProxyInterface->CreateVertexShader(reinterpret_cast<const DWORD *>(Tokens.data()), &ShaderInfo->Shader);
	}
	else
	{
		ShaderInfo = new VertexShaderInfo();
		ShaderInfo->Shader = nullptr;

		hr = D3D_OK;
	}

	if (SUCCEEDED(hr))
	{
		hr = ProxyInterface->CreateVertexDeclaration(VertexElements, &ShaderInfo->Declaration);

		if (SUCCEEDED(hr))
		{
			*pHandle = VertexShaderTable.Add(ShaderInfo) | 0x80000000;

			VertexShaderHandles.insert(*pHandle);
			VertexShaderAndDeclarationCount++;
			if (ShaderInfo->Shader)
			{
				VertexShaderAndDeclarationCount++;
			}
		}
		else
		{
#ifndef D3D8TO9NOLOG
			LOG << "> 'IDirect3DDevice9::CreateVertexDeclaration' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
			if (ShaderInfo->Shader != nullptr) 
			{
				ShaderInfo->Shader->Release();
			}
		}
	}
	else
	{
#ifndef D3D8TO9NOLOG
		LOG << "> 'IDirect3DDevice9::CreateVertexShader' failed with error code " << std::hex << hr << std::dec << "!" << std::endl;
#endif
	}

	if (FAILED(hr))
	{
		delete ShaderInfo;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetVertexShader(DWORD Handle)
{
	HRESULT hr;

	if ((Handle & 0x80000000) == 0)
	{
		ProxyInterface->SetVertexShader(nullptr);
		ProxyInterface->SetVertexDeclaration(nullptr);
		hr = ProxyInterface->SetFVF(Handle);

		CurrentVertexShaderHandle = 0;
	}
	else
	{
		VertexShaderInfo *const ShaderInfo = VertexShaderTable.Get(Handle & 0x7FFFFFFF);
		if (ShaderInfo == nullptr)
			return D3DERR_INVALIDCALL;

		hr = ProxyInterface->SetVertexShader(ShaderInfo->Shader);
		ProxyInterface->SetVertexDeclaration(ShaderInfo->Declaration);

		if (SUCCEEDED(hr))
			CurrentVertexShaderHandle = Handle;
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShader(DWORD *pHandle)
{
	if (pHandle == nullptr)
		return D3DERR_INVALIDCALL;

	if (CurrentVertexShaderHandle == 0)
	{
		return ProxyInterface->GetFVF(pHandle);
	}
	else
	{
		*pHandle = CurrentVertexShaderHandle;
		return D3D_OK;
	}
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeleteVertexShader(DWORD Handle)
{
	if ((Handle & 0x80000000) == 0)
		return D3DERR_INVALIDCALL;

	if (VertexShaderHandles.erase(Handle) == 0)
		return D3DERR_INVALIDCALL;

	if (CurrentVertexShaderHandle == Handle)
	{
		ProxyInterface->SetVertexShader(nullptr);
		ProxyInterface->SetVertexDeclaration(nullptr);
		CurrentVertexShaderHandle = 0;
	}

	VertexShaderInfo *const ShaderInfo = VertexShaderTable.Get(Handle & 0x7FFFFFFF);
	VertexShaderTable.Remove(Handle & 0x7FFFFFFF);

	if (ShaderInfo->Shader != nullptr) 
	{
		ShaderInfo->Shader->Release();
		VertexShaderAndDeclarationCount--;
	}
	if (ShaderInfo->Declaration != nullptr)
	{
		ShaderInfo->Declaration->Release();
		VertexShaderAndDeclarationCount--;
	}

	delete ShaderInfo;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetVertexShaderConstant(DWORD Register, const void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->SetVertexShaderConstantF(Register, static_cast<const float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->GetVertexShaderConstantF(Register, static_cast<float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderDeclaration(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
	UNREFERENCED_PARAMETER(Handle);
	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(pSizeOfData);

#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetVertexShaderDeclaration" << "(" << this << ", " << Handle << ", " << pData << ", " << pSizeOfData << ")' ..." << std::endl;
	LOG << "> 'IDirect3DDevice8::GetVertexShaderDeclaration' is not implemented!" << std::endl;
#endif

	return D3DERR_INVALIDCALL;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetVertexShaderFunction" << "(" << this << ", " << Handle << ", " << pData << ", " << pSizeOfData << ")' ..." << std::endl;
#endif

	if ((Handle & 0x80000000) == 0)
		return D3DERR_INVALIDCALL;

	VertexShaderInfo *const ShaderInfo = VertexShaderTable.Get(Handle & 0x7FFFFFFF);
	IDirect3DVertexShader9 *VertexShaderInterface = ShaderInfo != nullptr ? ShaderInfo->Shader : nullptr;

	if (VertexShaderInterface == nullptr)
		return D3DERR_INVALIDCALL;

#ifndef D3D8TO9NOLOG
	LOG << "> Returning translated shader byte code." << std::endl;
#endif

	return VertexShaderInterface->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 *pStreamData, UINT Stride)
{
	IDirect3DVertexBuffer9 *pStreamDataImpl = nullptr;
	if (pStreamData != nullptr)
		pStreamDataImpl = static_cast<Direct3DVertexBuffer8 *>(pStreamData)->GetProxyInterface();

	return ProxyInterface->SetStreamSource(StreamNumber, pStreamDataImpl, 0, Stride);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 **ppStreamData, UINT *pStride)
{
	if (ppStreamData == nullptr)
		return D3DERR_INVALIDCALL;

	*ppStreamData = nullptr;

	UINT StreamOffset = 0;
	IDirect3DVertexBuffer9 *VertexBufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetStreamSource(StreamNumber, &VertexBufferInterface, &StreamOffset, pStride);
	if (FAILED(hr))
		return hr;

	if (VertexBufferInterface != nullptr)
		*ppStreamData = ProxyAddressLookupTable->FindAddress<Direct3DVertexBuffer8>(VertexBufferInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetIndices(IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex)
{
	if (BaseVertexIndex > 0x7FFFFFFF)
		return D3DERR_INVALIDCALL;

	IDirect3DIndexBuffer9 *pIndexDataImpl = nullptr;
	if (pIndexData != nullptr)
		pIndexDataImpl = static_cast<Direct3DIndexBuffer8 *>(pIndexData)->GetProxyInterface();

	const HRESULT hr = ProxyInterface->SetIndices(pIndexDataImpl);
	if (FAILED(hr))
		return hr;

	CurrentBaseVertexIndex = static_cast<INT>(BaseVertexIndex);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetIndices(IDirect3DIndexBuffer8 **ppIndexData, UINT *pBaseVertexIndex)
{
	if (ppIndexData == nullptr)
		return D3DERR_INVALIDCALL;

	*ppIndexData = nullptr;

	if (pBaseVertexIndex != nullptr)
		*pBaseVertexIndex = static_cast<UINT>(CurrentBaseVertexIndex);

	IDirect3DIndexBuffer9 *IntexBufferInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetIndices(&IntexBufferInterface);
	if (FAILED(hr))
		return hr;

	if (IntexBufferInterface != nullptr)
		*ppIndexData = ProxyAddressLookupTable->FindAddress<Direct3DIndexBuffer8>(IntexBufferInterface);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreatePixelShader(const DWORD *pFunction, DWORD *pHandle)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::CreatePixelShader" << "(" << this << ", " << pFunction << ", " << pHandle << ")' ..." << std::endl;
#endif

	if (pFunction == nullptr || pHandle == nullptr)
		return D3DERR_INVALIDCALL;

	*pHandle = 0;

#ifndef D3D8TO9NOLOG
	LOG << "> Disassembling shader and translating assembly to Direct3D 9 compatible code ..." << std::endl;
#endif

	if (*pFunction < D3DPS_VERSION(1, 0) || *pFunction > D3DPS_VERSION(1, 4))
	{
#ifndef D3D8TO9NOLOG
		LOG << "> Failed because of version mismatch ('" << std::showbase << std::hex << *pFunction << std::dec << std::noshowbase << "')! Only 'ps_1_x' shaders are supported." << std::endl;
#endif
		return D3DERR_INVALIDCALL;
	}

	// GeneralsX: no D3DX. D3D9 reads ps_1_x tokens as D3D8 wrote them, as DXVK's own D3D8 front end relies on; only ps_1_0
	// is renamed ps_1_1 (patch 0003, gx_shader_tokens.cpp).
	std::vector<std::uint32_t> Tokens;
	std::string Why;

	if (!gx_ps8to9(reinterpret_cast<const std::uint32_t *>(pFunction), Tokens, &Why))
		return D3DERR_INVALIDCALL;

	IDirect3DPixelShader9 *Shader = nullptr;
	const HRESULT hr = ProxyInterface->CreatePixelShader(reinterpret_cast<const DWORD *>(Tokens.data()), &Shader);

	if (SUCCEEDED(hr))
	{
		*pHandle = PixelShaderTable.Add(Shader);
		PixelShaderHandles.insert(*pHandle);
	}

	return hr;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPixelShader(DWORD Handle)
{
	IDirect3DPixelShader9 *const Shader = PixelShaderTable.Get(Handle);
	if (Handle != 0 && Shader == nullptr)
		return D3DERR_INVALIDCALL;

	const HRESULT hr = ProxyInterface->SetPixelShader(Shader);
	if (FAILED(hr))
		return hr;

	CurrentPixelShaderHandle = Handle;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShader(DWORD *pHandle)
{
	if (pHandle == nullptr)
		return D3DERR_INVALIDCALL;

	*pHandle = CurrentPixelShaderHandle;

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeletePixelShader(DWORD Handle)
{
	if (Handle == 0)
		return D3DERR_INVALIDCALL;

	if (PixelShaderHandles.erase(Handle) == 0)
		return D3DERR_INVALIDCALL;

	if (CurrentPixelShaderHandle == Handle)
		SetPixelShader(0);

	PixelShaderTable.Get(Handle)->Release();
	PixelShaderTable.Remove(Handle);

	return D3D_OK;
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPixelShaderConstant(DWORD Register, const void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->SetPixelShaderConstantF(Register, static_cast<const float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShaderConstant(DWORD Register, void *pConstantData, DWORD ConstantCount)
{
	return ProxyInterface->GetPixelShaderConstantF(Register, static_cast<float *>(pConstantData), ConstantCount);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShaderFunction(DWORD Handle, void *pData, DWORD *pSizeOfData)
{
#ifndef D3D8TO9NOLOG
	LOG << "Redirecting '" << "IDirect3DDevice8::GetPixelShaderFunction" << "(" << this << ", " << Handle << ", " << pData << ", " << pSizeOfData << ")' ..." << std::endl;
#endif

	if (Handle == 0)
		return D3DERR_INVALIDCALL;

	IDirect3DPixelShader9 *const PixelShaderInterface = PixelShaderTable.Get(Handle);
	if (PixelShaderInterface == nullptr)
		return D3DERR_INVALIDCALL;

#ifndef D3D8TO9NOLOG
	LOG << "> Returning translated shader byte code." << std::endl;
#endif

	return PixelShaderInterface->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawRectPatch(UINT Handle, const float *pNumSegs, const D3DRECTPATCH_INFO *pRectPatchInfo)
{
	return ProxyInterface->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawTriPatch(UINT Handle, const float *pNumSegs, const D3DTRIPATCH_INFO *pTriPatchInfo)
{
	return ProxyInterface->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeletePatch(UINT Handle)
{
	return ProxyInterface->DeletePatch(Handle);
}

void Direct3DDevice8::ApplyClipPlanes()
{
	DWORD index = 0;
	for (const auto plane : StoredClipPlanes)
	{
		if ((ClipPlaneRenderState & (1 << index)) != 0)
			ProxyInterface->SetClipPlane(index, plane);

		index++;
	}
}

void Direct3DDevice8::ReleaseShadersAndStateBlocks()
{
	while (!PixelShaderHandles.empty())
	{
		DWORD Handle = *PixelShaderHandles.begin();
		DeletePixelShader(Handle);
	}

	while (!VertexShaderHandles.empty())
	{
		DWORD Handle = *VertexShaderHandles.begin();
		DeleteVertexShader(Handle);
	}

	VertexShaderAndDeclarationCount = 0;

	while (!StateBlockTokens.empty())
	{
		DWORD Token = *StateBlockTokens.begin();
		DeleteStateBlock(Token);
	}
}
