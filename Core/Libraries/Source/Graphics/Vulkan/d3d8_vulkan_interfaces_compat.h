#pragma once

/**
 * @file d3d8_interfaces.h
 * @brief DirectX 8.0 COM Interface Stub Definitions
 * 
 * This header provides complete struct definitions for DirectX 8 COM interfaces.
 * All methods are no-op stubs that return S_OK, as the Vulkan backend doesn't
 * use actual DirectX rendering.
 * 
 * These stubs exist purely to:
 * 1. Satisfy compiler type-checking for method calls on interface pointers
 * 2. Allow code that uses DirectX patterns to compile on non-Windows platforms
 * 3. Provide a basis for future real implementations if needed
 * 
 * Phase 02.5: DirectX 8 Stub Interface Implementation
 */

#ifndef D3D8_INTERFACES_H_INCLUDED
#define D3D8_INTERFACES_H_INCLUDED

#include "d3d8_vulkan_types_compat.h"  // For IID and RGNDATA

// Note: d3d8_enums.h is not included to avoid circular dependencies
// Enum types are forward-declared or used through their actual definitions in d3d8.h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct IDirect3DBaseTexture8
 * @brief Base texture interface (parent for all texture types)
 */
struct IDirect3DBaseTexture8 {
    virtual ~IDirect3DBaseTexture8() {}
    
    // Resource methods
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // Texture state methods
    virtual DWORD SetLOD(DWORD LODNew) { return 0; }
    virtual DWORD GetLOD() { return 0; }
    virtual DWORD GetLevelCount() { return 1; }
};

/**
 * @struct IDirect3DSurface8
 * @brief Surface interface for texture and backbuffer surfaces
 */
struct IDirect3DSurface8 {
    virtual ~IDirect3DSurface8() {}
    
    // Resource methods
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // Surface methods
    virtual HRESULT GetDesc(D3DSURFACE_DESC *pDesc) { return S_OK; }
    virtual HRESULT LockRect(D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) { return S_OK; }
    virtual HRESULT UnlockRect() { return S_OK; }
    virtual HRESULT GetDC(HDC *phdc) { return S_OK; }
    virtual HRESULT ReleaseDC(HDC hdc) { return S_OK; }
};

/**
 * @struct IDirect3DTexture8
 * @brief 2D texture interface
 */
struct IDirect3DTexture8 : public IDirect3DBaseTexture8 {
    virtual ~IDirect3DTexture8() {}
    
    // Texture methods
    virtual HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) { return S_OK; }
    virtual HRESULT GetSurfaceLevel(UINT Level, IDirect3DSurface8 **ppSurfaceLevel) { return S_OK; }
    virtual HRESULT LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) { return S_OK; }
    virtual HRESULT UnlockRect(UINT Level) { return S_OK; }
    virtual HRESULT AddDirtyRect(const RECT *pDirtyRect) { return S_OK; }
};

/**
 * @struct IDirect3DCubeTexture8
 * @brief Cube texture interface
 */
struct IDirect3DCubeTexture8 : public IDirect3DBaseTexture8 {
    virtual ~IDirect3DCubeTexture8() {}
    
    // Cube texture methods
    virtual HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) { return S_OK; }
    virtual HRESULT GetCubeMapSurface(DWORD FaceType, UINT Level, IDirect3DSurface8 **ppCubeMapSurface) { return S_OK; }
    virtual HRESULT LockRect(DWORD FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) { return S_OK; }
    virtual HRESULT UnlockRect(DWORD FaceType, UINT Level) { return S_OK; }
    virtual HRESULT AddDirtyRect(DWORD FaceType, const RECT *pDirtyRect) { return S_OK; }
};

/**
 * @struct IDirect3DVolumeTexture8
 * @brief 3D volume texture interface
 */
struct IDirect3DVolumeTexture8 : public IDirect3DBaseTexture8 {
    virtual ~IDirect3DVolumeTexture8() {}
    
    // Volume texture methods
    virtual HRESULT GetLevelDesc(UINT Level, void *pDesc) { return S_OK; }
    virtual HRESULT GetVolumeLevel(UINT Level, void **ppVolumeLevel) { return S_OK; }
    virtual HRESULT LockBox(UINT Level, void *pLockedVolume, const void *pBox, DWORD Flags) { return S_OK; }
    virtual HRESULT UnlockBox(UINT Level) { return S_OK; }
    virtual HRESULT AddDirtyBox(const void *pDirtyBox) { return S_OK; }
};

/**
 * @struct IDirect3DVertexBuffer8
 * @brief Vertex buffer interface
 */
struct IDirect3DVertexBuffer8 {
    virtual ~IDirect3DVertexBuffer8() {}
    
    // Resource methods
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // Buffer methods
    virtual HRESULT GetDesc(void *pDesc) { return S_OK; }
    virtual HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags) { return S_OK; }
    virtual HRESULT Unlock() { return S_OK; }
};

/**
 * @struct IDirect3DIndexBuffer8
 * @brief Index buffer interface
 */
struct IDirect3DIndexBuffer8 {
    virtual ~IDirect3DIndexBuffer8() {}
    
    // Resource methods
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // Buffer methods
    virtual HRESULT GetDesc(void *pDesc) { return S_OK; }
    virtual HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags) { return S_OK; }
    virtual HRESULT Unlock() { return S_OK; }
};

/**
 * @struct IDirect3DSwapChain8
 * @brief Swap chain interface for presentation
 */
struct IDirect3DSwapChain8 {
    virtual ~IDirect3DSwapChain8() {}
    
    // Resource methods
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // Swap chain methods
    virtual HRESULT Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion) { return S_OK; }
    virtual HRESULT GetBackBuffer(UINT BackBuffer, DWORD Type, IDirect3DSurface8 **ppBackBuffer) { return S_OK; }
    virtual HRESULT GetRasterStatus(void *pRasterStatus) { return S_OK; }
    virtual HRESULT GetDisplayMode(D3DDISPLAYMODE *pMode) { return S_OK; }
    virtual HRESULT GetDevice(IDirect3DDevice8 **ppDevice) { return S_OK; }
    virtual HRESULT GetPresentParameters(void *pPresentationParameters) { return S_OK; }
};

/**
 * @struct IDirect3DDevice8
 * @brief Main rendering device interface
 * 
 * This is the primary interface used by the game for rendering operations.
 * All methods are stubs that return S_OK because the Vulkan backend
 * handles actual rendering through a different code path.
 */
struct IDirect3DDevice8 {
    virtual ~IDirect3DDevice8() {}
    
    // ============= COM Methods =============
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // ============= Device Information =============
    virtual HRESULT GetDeviceCaps(void *pCaps) { return S_OK; }
    virtual HRESULT GetDisplayMode(D3DDISPLAYMODE *pMode) { return S_OK; }
    virtual HRESULT GetCreationParameters(void *pParameters) { return S_OK; }
    
    // ============= Render State Management =============
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) { return S_OK; }
    virtual HRESULT GetRenderState(D3DRENDERSTATETYPE State, DWORD *pValue) { return S_OK; }
    
    // ============= Texture Stage State Management =============
    virtual HRESULT SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) { return S_OK; }
    virtual HRESULT GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD *pValue) { return S_OK; }
    
    // ============= Sampler State Management =============
    virtual HRESULT SetSamplerState(DWORD Sampler, DWORD Type, DWORD Value) { return S_OK; }
    virtual HRESULT GetSamplerState(DWORD Sampler, DWORD Type, DWORD *pValue) { return S_OK; }
    
    // ============= Transform Management =============
    virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix) { return S_OK; }
    virtual HRESULT GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX *pMatrix) { return S_OK; }
    virtual HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix) { return S_OK; }
    
    // ============= Viewport and Scissor =============
    virtual HRESULT SetViewport(const D3DVIEWPORT8 *pViewport) { return S_OK; }
    virtual HRESULT GetViewport(D3DVIEWPORT8 *pViewport) { return S_OK; }
    virtual HRESULT SetScissorRect(const RECT *pRect) { return S_OK; }
    virtual HRESULT GetScissorRect(RECT *pRect) { return S_OK; }
    
    // ============= Clipping Planes =============
    virtual HRESULT SetClipPlane(DWORD Index, const float *pPlane) { return S_OK; }
    virtual HRESULT GetClipPlane(DWORD Index, float *pPlane) { return S_OK; }
    
    // ============= Material =============
    virtual HRESULT SetMaterial(const D3DMATERIAL8 *pMaterial) { return S_OK; }
    virtual HRESULT GetMaterial(D3DMATERIAL8 *pMaterial) { return S_OK; }
    
    // ============= Lighting =============
    virtual HRESULT SetLight(DWORD Index, const D3DLIGHT8 *pLight) { return S_OK; }
    virtual HRESULT GetLight(DWORD Index, D3DLIGHT8 *pLight) { return S_OK; }
    virtual HRESULT LightEnable(DWORD Index, BOOL Enable) { return S_OK; }
    virtual HRESULT GetLightEnable(DWORD Index, BOOL *pEnable) { return S_OK; }
    
    // ============= Shader Management =============
    virtual HRESULT CreateVertexShader(const DWORD *pDeclaration, const DWORD *pFunction, DWORD *pHandle, DWORD Usage) { return S_OK; }
    virtual HRESULT SetVertexShader(DWORD Handle) { return S_OK; }
    virtual HRESULT GetVertexShader(DWORD *pHandle) { return S_OK; }
    virtual HRESULT DeleteVertexShader(DWORD Handle) { return S_OK; }
    
    virtual HRESULT CreatePixelShader(const DWORD *pFunction, DWORD *pHandle) { return S_OK; }
    virtual HRESULT SetPixelShader(DWORD Handle) { return S_OK; }
    virtual HRESULT GetPixelShader(DWORD *pHandle) { return S_OK; }
    virtual HRESULT DeletePixelShader(DWORD Handle) { return S_OK; }
    
    // ============= Shader Constants =============
    virtual HRESULT SetVertexShaderConstant(UINT Register, const void *pConstantData, UINT ConstantCount) { return S_OK; }
    virtual HRESULT GetVertexShaderConstant(UINT Register, void *pConstantData, UINT ConstantCount) { return S_OK; }
    
    virtual HRESULT SetPixelShaderConstant(UINT Register, const void *pConstantData, UINT ConstantCount) { return S_OK; }
    virtual HRESULT GetPixelShaderConstant(UINT Register, void *pConstantData, UINT ConstantCount) { return S_OK; }
    
    // ============= Texture and Sampler Binding =============
    virtual HRESULT SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture) { return S_OK; }
    virtual HRESULT GetTexture(DWORD Stage, IDirect3DBaseTexture8 **ppTexture) { return S_OK; }
    
    // ============= Stream Source (Vertex Buffer) Binding =============
    virtual HRESULT SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 *pStreamData, UINT Stride) { return S_OK; }
    virtual HRESULT GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 **ppStreamData, UINT *pStride) { return S_OK; }
    
    // ============= Index Buffer Binding =============
    virtual HRESULT SetIndices(IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex) { return S_OK; }
    virtual HRESULT GetIndices(IDirect3DIndexBuffer8 **ppIndexData, UINT *pBaseVertexIndex) { return S_OK; }
    
    // ============= Vertex Declaration =============
    virtual HRESULT SetVertexDeclaration(const DWORD *pDecl) { return S_OK; }
    virtual HRESULT GetVertexDeclaration(DWORD *pDecl) { return S_OK; }
    
    // ============= Rendering Operations =============
    virtual HRESULT Clear(DWORD Count, const D3DRECT *pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) { return S_OK; }
    virtual HRESULT BeginScene() { return S_OK; }
    virtual HRESULT EndScene() { return S_OK; }
    virtual HRESULT Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion) { return S_OK; }
    
    // ============= Drawing Operations =============
    virtual HRESULT DrawPrimitive(DWORD PrimitiveType, UINT StartVertex, UINT PrimitiveCount) { return S_OK; }
    virtual HRESULT DrawIndexedPrimitive(DWORD Type, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) { return S_OK; }
    virtual HRESULT DrawPrimitiveUP(DWORD PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { return S_OK; }
    virtual HRESULT DrawIndexedPrimitiveUP(DWORD Type, UINT minIndex, UINT NumVertices, UINT PrimitiveCount, const void *pIndexData, DWORD IndexDataFormat, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride) { return S_OK; }
    
    // ============= Surface Operations =============
    virtual HRESULT CopyRects(IDirect3DSurface8 *pSourceSurface, const RECT *pSourceRectsArray, UINT cRects, IDirect3DSurface8 *pDestinationSurface, const POINT *pDestPointsArray) { return S_OK; }
    virtual HRESULT UpdateTexture(IDirect3DBaseTexture8 *pSourceTexture, IDirect3DBaseTexture8 *pDestinationTexture) { return S_OK; }
    virtual HRESULT GetFrontBuffer(IDirect3DSurface8 *pDestSurface) { return S_OK; }
    
    // ============= Palette (Rarely Used) =============
    virtual HRESULT SetPaletteEntries(UINT PaletteNumber, const void *pEntries) { return S_OK; }
    virtual HRESULT GetPaletteEntries(UINT PaletteNumber, void *pEntries) { return S_OK; }
    virtual HRESULT SetCurrentTexturePalette(UINT PaletteNumber) { return S_OK; }
    virtual HRESULT GetCurrentTexturePalette(UINT *PaletteNumber) { return S_OK; }
    
    // ============= Rendering Target Management =============
    virtual HRESULT SetRenderTarget(IDirect3DSurface8 *pRenderTarget, IDirect3DSurface8 *pNewZStencil) { return S_OK; }
    virtual HRESULT GetRenderTarget(IDirect3DSurface8 **ppRenderTarget) { return S_OK; }
    virtual HRESULT GetDepthStencilSurface(IDirect3DSurface8 **ppZStencilSurface) { return S_OK; }
    
    // ============= Device Loss and Reset =============
    virtual HRESULT GetRasterStatus(void *pRasterStatus) { return S_OK; }
    virtual HRESULT SetDialogBoxMode(BOOL bEnableDialogs) { return S_OK; }
    virtual HRESULT TestCooperativeLevel() { return S_OK; }
    virtual HRESULT Reset(void *pPresentationParameters) { return S_OK; }
    
    // ============= Resource Creation =============
    virtual HRESULT CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8 **ppTexture) { return S_OK; }
    virtual HRESULT CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8 **ppVolumeTexture) { return S_OK; }
    virtual HRESULT CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture) { return S_OK; }
    virtual HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8 **ppVertexBuffer) { return S_OK; }
    virtual HRESULT CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8 **ppIndexBuffer) { return S_OK; }
    virtual HRESULT CreateSurface(UINT Width, UINT Height, D3DFORMAT Format, BOOL Lockable, BOOL Discard, IDirect3DSurface8 **ppSurface) { return S_OK; }
    
    /**
     * @brief Create an image surface (off-screen rendering)
     * 
     * Creates a surface that can be used for rendering or manipulation
     * without being tied to the display. This is called from dx8wrapper.cpp
     * for creating textures and off-screen render targets.
     * 
     * @param Width         Surface width in pixels
     * @param Height        Surface height in pixels
     * @param Format        Surface format (D3DFORMAT)
     * @param ppSurface     Output pointer to created surface
     * @return HRESULT (S_OK if successful)
     */
    virtual HRESULT CreateImageSurface(DWORD Width, DWORD Height, D3DFORMAT Format, IDirect3DSurface8 **ppSurface) { 
        if (ppSurface) *ppSurface = nullptr; 
        return S_OK; 
    }
    
    virtual HRESULT CreateSwapChain(void *pPresentationParameters, IDirect3DSwapChain8 **ppSwapChain) { return S_OK; }
    
    // ============= Strided Data =============
    virtual HRESULT DrawRectPatch(UINT Handle, const float *pNumSegs, const void *pRectPatchInfo) { return S_OK; }
    virtual HRESULT DrawTriPatch(UINT Handle, const float *pNumSegs, const void *pTriPatchInfo) { return S_OK; }
    virtual HRESULT DeletePatch(UINT Handle) { return S_OK; }
};

/**
 * @struct IDirect3D8
 * @brief Direct3D interface for device creation and enumeration
 */
struct IDirect3D8 {
    virtual ~IDirect3D8() {}
    
    // ============= COM Methods =============
    virtual HRESULT QueryInterface(const IID &riid, void **ppvObj) { return S_OK; }
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 0; }
    
    // ============= Adapter Enumeration =============
    virtual UINT GetAdapterCount() { return 1; }
    virtual HRESULT GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier) { return S_OK; }
    virtual UINT GetAdapterModeCount(UINT Adapter) { return 1; }
    virtual HRESULT EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE *pMode) { return S_OK; }
    virtual HRESULT GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE *pMode) { return S_OK; }
    virtual HRESULT CheckDeviceType(UINT Adapter, DWORD CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL Windowed) { return S_OK; }
    virtual HRESULT CheckDeviceFormat(UINT Adapter, DWORD DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, DWORD RType, D3DFORMAT CheckFormat) { return S_OK; }
    virtual HRESULT CheckDeviceMultiSampleType(UINT Adapter, DWORD DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, DWORD MultiSampleType) { return S_OK; }
    virtual HRESULT CheckDepthStencilMatch(UINT Adapter, DWORD DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) { return S_OK; }
    virtual HRESULT CheckDeviceCaps(UINT Adapter, DWORD DeviceType, void *pCaps) { return S_OK; }
    
    // ============= Device Creation =============
    virtual HRESULT CreateDevice(UINT Adapter, DWORD DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, void *pPresentationParameters, IDirect3DDevice8 **ppReturnedDeviceInterface) { return S_OK; }
    virtual HRESULT GetDeviceCaps(UINT Adapter, DWORD DeviceType, void *pCaps) { return S_OK; }
};

#ifdef __cplusplus
}
#endif

#endif // D3D8_INTERFACES_H_INCLUDED
