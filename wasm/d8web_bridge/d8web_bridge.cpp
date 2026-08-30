// Igroteka wasm — COM bridge: DXVK/mingw d3d8.h interfaces over the d8web core.
//
// The engine compiles against DXVK's d3d8 headers and obtains the API through a
// Direct3DCreate8 function pointer (normally dlopen'd). On wasm we hand it
// Igroteka_Direct3DCreate8 from this file instead. Every COM interface here is
// a thin adapter forwarding to d8web's namespaced implementation; enums are
// numerically identical to real D3D8 and shared structs are layout-identical,
// so conversions are casts. Methods d8web doesn't model return sane defaults —
// loudly once, silently after.

#include <d3d8.h>  // DXVK/mingw headers (global namespace)

#define D8WEB_NO_GLOBAL_FACTORY
#include <d8web/d3d8.h>

#include <cstdio>
#include <cstring>

namespace {

namespace dw = d8web;

// The d8web header cleared the real d3d8.h macros for these names (see its
// coexistence block); re-import the namespaced equivalents. Values are
// numerically identical to the D3D8 originals.
using dw::D3D_OK;
using dw::D3DERR_INVALIDCALL;
using dw::D3DERR_NOTAVAILABLE;
using dw::D3DVS_VERSION;
using dw::D3DPS_VERSION;
using dw::D3DCREATE_HARDWARE_VERTEXPROCESSING;

#define BRIDGE_STUB_ONCE(name)                                            \
    do {                                                                  \
        static bool warned = false;                                       \
        if (!warned) {                                                    \
            std::fprintf(stderr, "[d8web-bridge] stub: %s\n", name);      \
            warned = true;                                                \
        }                                                                 \
    } while (0)

// Layout-identical struct casts (verified against both headers)
inline const dw::D3DMATRIX* cvt(const D3DMATRIX* m) { return reinterpret_cast<const dw::D3DMATRIX*>(m); }
inline dw::D3DMATRIX* cvt(D3DMATRIX* m) { return reinterpret_cast<dw::D3DMATRIX*>(m); }
inline const dw::D3DVIEWPORT8* cvt(const D3DVIEWPORT8* v) { return reinterpret_cast<const dw::D3DVIEWPORT8*>(v); }
inline dw::D3DVIEWPORT8* cvt(D3DVIEWPORT8* v) { return reinterpret_cast<dw::D3DVIEWPORT8*>(v); }
inline const dw::D3DMATERIAL8* cvt(const D3DMATERIAL8* v) { return reinterpret_cast<const dw::D3DMATERIAL8*>(v); }
inline dw::D3DMATERIAL8* cvt(D3DMATERIAL8* v) { return reinterpret_cast<dw::D3DMATERIAL8*>(v); }
inline const dw::D3DLIGHT8* cvt(const D3DLIGHT8* v) { return reinterpret_cast<const dw::D3DLIGHT8*>(v); }
inline dw::D3DLIGHT8* cvt(D3DLIGHT8* v) { return reinterpret_cast<dw::D3DLIGHT8*>(v); }
inline dw::D3DPRESENT_PARAMETERS* cvt(D3DPRESENT_PARAMETERS* v) { return reinterpret_cast<dw::D3DPRESENT_PARAMETERS*>(v); }
inline dw::D3DLOCKED_RECT* cvt(D3DLOCKED_RECT* v) { return reinterpret_cast<dw::D3DLOCKED_RECT*>(v); }
inline const dw::RECT* cvt(const RECT* v) { return reinterpret_cast<const dw::RECT*>(v); }
inline dw::D3DSURFACE_DESC* cvt(D3DSURFACE_DESC* v) { return reinterpret_cast<dw::D3DSURFACE_DESC*>(v); }

class BridgeDevice;

// ---------------------------------------------------------------------------
// Common refcount plumbing
// ---------------------------------------------------------------------------
template <typename Base>
class BridgeUnknown : public Base {
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void** out) override {
        if (out) *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG r = --m_ref;
        if (r == 0) delete this;
        return r;
    }

protected:
    virtual ~BridgeUnknown() = default;

private:
    ULONG m_ref = 1;
};

// Resource-flavoured no-ops shared by textures/buffers/surfaces
#define BRIDGE_RESOURCE_BOILERPLATE                                                     \
    HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** dev) override;               \
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, const void*, DWORD, DWORD) override { return D3D_OK; } \
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void*, DWORD*) override { return D3DERR_NOTFOUND; }    \
    HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3D_OK; }      \
    DWORD STDMETHODCALLTYPE SetPriority(DWORD) override { return 0; }                   \
    DWORD STDMETHODCALLTYPE GetPriority() override { return 0; }                        \
    void STDMETHODCALLTYPE PreLoad() override {}

// ---------------------------------------------------------------------------
class BridgeVB final : public BridgeUnknown<IDirect3DVertexBuffer8> {
public:
    BridgeVB(dw::IDirect3DVertexBuffer8* inner, BridgeDevice* dev, UINT length, DWORD usage, DWORD fvf)
        : m_inner(inner), m_dev(dev), m_length(length), m_usage(usage), m_fvf(fvf) {}
    ~BridgeVB() override { m_inner->Release(); }

    BRIDGE_RESOURCE_BOILERPLATE
    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_VERTEXBUFFER; }

    HRESULT STDMETHODCALLTYPE Lock(UINT offset, UINT size, BYTE** data, DWORD flags) override {
        return m_inner->Lock(offset, size, data, flags);
    }
    HRESULT STDMETHODCALLTYPE Unlock() override { return m_inner->Unlock(); }
    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC* desc) override {
        if (!desc) return D3DERR_INVALIDCALL;
        desc->Format = D3DFMT_VERTEXDATA;
        desc->Type = D3DRTYPE_VERTEXBUFFER;
        desc->Usage = m_usage;
        desc->Pool = D3DPOOL_MANAGED;
        desc->Size = m_length;
        desc->FVF = m_fvf;
        return D3D_OK;
    }

    dw::IDirect3DVertexBuffer8* inner() const { return m_inner; }

private:
    dw::IDirect3DVertexBuffer8* m_inner;
    BridgeDevice* m_dev;
    UINT m_length;
    DWORD m_usage, m_fvf;
};

class BridgeIB final : public BridgeUnknown<IDirect3DIndexBuffer8> {
public:
    BridgeIB(dw::IDirect3DIndexBuffer8* inner, BridgeDevice* dev, UINT length, DWORD usage, D3DFORMAT fmt)
        : m_inner(inner), m_dev(dev), m_length(length), m_usage(usage), m_format(fmt) {}
    ~BridgeIB() override { m_inner->Release(); }

    BRIDGE_RESOURCE_BOILERPLATE
    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_INDEXBUFFER; }

    HRESULT STDMETHODCALLTYPE Lock(UINT offset, UINT size, BYTE** data, DWORD flags) override {
        return m_inner->Lock(offset, size, data, flags);
    }
    HRESULT STDMETHODCALLTYPE Unlock() override { return m_inner->Unlock(); }
    HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC* desc) override {
        if (!desc) return D3DERR_INVALIDCALL;
        desc->Format = m_format;
        desc->Type = D3DRTYPE_INDEXBUFFER;
        desc->Usage = m_usage;
        desc->Pool = D3DPOOL_MANAGED;
        desc->Size = m_length;
        return D3D_OK;
    }

    dw::IDirect3DIndexBuffer8* inner() const { return m_inner; }

private:
    dw::IDirect3DIndexBuffer8* m_inner;
    BridgeDevice* m_dev;
    UINT m_length;
    DWORD m_usage;
    D3DFORMAT m_format;
};

class BridgeSurface final : public BridgeUnknown<IDirect3DSurface8> {
public:
    BridgeSurface(dw::IDirect3DSurface8* inner, BridgeDevice* dev) : m_inner(inner), m_dev(dev) {}
    ~BridgeSurface() override { m_inner->Release(); }

    HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** dev) override;
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, const void*, DWORD, DWORD) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void*, DWORD*) override { return D3DERR_NOTFOUND; }
    HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE GetContainer(REFIID, void** out) override {
        if (out) *out = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC* desc) override {
        return m_inner->GetDesc(cvt(desc));
    }
    HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT* lr, const RECT* rect, DWORD flags) override {
        return m_inner->LockRect(cvt(lr), cvt(rect), flags);
    }
    HRESULT STDMETHODCALLTYPE UnlockRect() override { return m_inner->UnlockRect(); }

    dw::IDirect3DSurface8* inner() const { return m_inner; }

private:
    dw::IDirect3DSurface8* m_inner;
    BridgeDevice* m_dev;
};

class BridgeTexture final : public BridgeUnknown<IDirect3DTexture8> {
public:
    BridgeTexture(dw::IDirect3DTexture8* inner, BridgeDevice* dev) : m_inner(inner), m_dev(dev) {}
    ~BridgeTexture() override { m_inner->Release(); }

    BRIDGE_RESOURCE_BOILERPLATE
    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_TEXTURE; }
    DWORD STDMETHODCALLTYPE SetLOD(DWORD) override { return 0; }
    DWORD STDMETHODCALLTYPE GetLOD() override { return 0; }
    DWORD STDMETHODCALLTYPE GetLevelCount() override { return m_inner->GetLevelCount(); }

    HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT level, D3DSURFACE_DESC* desc) override {
        return m_inner->GetLevelDesc(level, cvt(desc));
    }
    HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT level, IDirect3DSurface8** out) override;
    HRESULT STDMETHODCALLTYPE LockRect(UINT level, D3DLOCKED_RECT* lr, const RECT* rect, DWORD flags) override {
        return m_inner->LockRect(level, cvt(lr), cvt(rect), flags);
    }
    HRESULT STDMETHODCALLTYPE UnlockRect(UINT level) override { return m_inner->UnlockRect(level); }
    HRESULT STDMETHODCALLTYPE AddDirtyRect(const RECT*) override { return D3D_OK; }

    dw::IDirect3DTexture8* inner() const { return m_inner; }

private:
    dw::IDirect3DTexture8* m_inner;
    BridgeDevice* m_dev;
};

// ---------------------------------------------------------------------------
class BridgeDevice final : public BridgeUnknown<IDirect3DDevice8> {
public:
    BridgeDevice(dw::IDirect3DDevice8* inner, IDirect3D8* parent, UINT bbWidth, UINT bbHeight)
        : m_inner(inner), m_parent(parent), m_bbWidth(bbWidth ? bbWidth : 1024),
          m_bbHeight(bbHeight ? bbHeight : 768) {}
    // GeneralsX @bugfix Copilot 30/08/2026 Release every reference owned by
    // the bridge device before destroying the underlying WebGL device.
    ~BridgeDevice() override {
        for (IDirect3DBaseTexture8*& texture : m_boundTextures) {
            if (texture) texture->Release();
        }
        if (m_backBuffer) m_backBuffer->Release();
        if (m_depthSurface) m_depthSurface->Release();
        m_inner->Release();
        m_parent->Release();
    }

    // --- cooperative/status ---
    HRESULT STDMETHODCALLTYPE TestCooperativeLevel() override { return D3D_OK; }
    UINT STDMETHODCALLTYPE GetAvailableTextureMem() override { return m_inner->GetAvailableTextureMem(); }
    HRESULT STDMETHODCALLTYPE ResourceManagerDiscardBytes(DWORD b) override { return m_inner->ResourceManagerDiscardBytes(b); }
    HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        *out = m_parent;
        m_parent->AddRef();
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS8* caps) override;
    HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE* mode) override {
        return m_inner->GetDisplayMode(reinterpret_cast<dw::D3DDISPLAYMODE*>(mode));
    }
    HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* p) override {
        if (!p) return D3DERR_INVALIDCALL;
        std::memset(p, 0, sizeof(*p));
        p->DeviceType = D3DDEVTYPE_HAL;
        p->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        return D3D_OK;
    }

    // --- cursor (browser cursor handled by SDL/DOM) ---
    HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT, UINT, IDirect3DSurface8*) override { return D3D_OK; }
    void STDMETHODCALLTYPE SetCursorPosition(UINT, UINT, DWORD) override {}
    WINBOOL STDMETHODCALLTYPE ShowCursor(WINBOOL show) override { return show; }

    // --- swap chain ---
    HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS*, IDirect3DSwapChain8** out) override {
        if (out) *out = nullptr;
        BRIDGE_STUB_ONCE("CreateAdditionalSwapChain");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS* pp) override { return m_inner->Reset(cvt(pp)); }
    HRESULT STDMETHODCALLTYPE Present(const RECT* src, const RECT* dst, HWND wnd, const RGNDATA*) override {
        return m_inner->Present(cvt(src), cvt(dst), wnd, nullptr);
    }
    HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        *out = fakeSurface(m_backBuffer, D3DFMT_X8R8G8B8);
        return *out ? D3D_OK : D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS* rs) override {
        if (!rs) return D3DERR_INVALIDCALL;
        rs->InVBlank = FALSE;
        rs->ScanLine = 0;
        return D3D_OK;
    }
    void STDMETHODCALLTYPE SetGammaRamp(DWORD, const D3DGAMMARAMP*) override {}
    void STDMETHODCALLTYPE GetGammaRamp(D3DGAMMARAMP* ramp) override {
        if (ramp)
            for (int i = 0; i < 256; ++i)
                ramp->red[i] = ramp->green[i] = ramp->blue[i] = WORD(i << 8);
    }

    // --- resource creation ---
    HRESULT STDMETHODCALLTYPE CreateTexture(UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt,
                                            D3DPOOL pool, IDirect3DTexture8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        dw::IDirect3DTexture8* inner = nullptr;
        HRESULT hr = m_inner->CreateTexture(w, h, levels, usage, dw::D3DFORMAT(fmt),
                                            dw::D3DPOOL(pool), &inner);
        *out = SUCCEEDED(hr) ? new BridgeTexture(inner, this) : nullptr;
        return hr;
    }
    HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL,
                                                  IDirect3DVolumeTexture8** out) override {
        if (out) *out = nullptr;
        BRIDGE_STUB_ONCE("CreateVolumeTexture");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL,
                                                IDirect3DCubeTexture8** out) override {
        if (out) *out = nullptr;
        BRIDGE_STUB_ONCE("CreateCubeTexture");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL,
                                                 IDirect3DVertexBuffer8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        dw::IDirect3DVertexBuffer8* inner = nullptr;
        HRESULT hr = m_inner->CreateVertexBuffer(length, usage, fvf, dw::D3DPOOL_MANAGED, &inner);
        *out = SUCCEEDED(hr) ? new BridgeVB(inner, this, length, usage, fvf) : nullptr;
        return hr;
    }
    HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT fmt, D3DPOOL,
                                                IDirect3DIndexBuffer8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        dw::IDirect3DIndexBuffer8* inner = nullptr;
        HRESULT hr = m_inner->CreateIndexBuffer(length, usage, dw::D3DFORMAT(fmt),
                                                dw::D3DPOOL_MANAGED, &inner);
        *out = SUCCEEDED(hr) ? new BridgeIB(inner, this, length, usage, fmt) : nullptr;
        return hr;
    }
    HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, WINBOOL,
                                                 IDirect3DSurface8** out) override {
        if (out) *out = nullptr;
        BRIDGE_STUB_ONCE("CreateRenderTarget");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE,
                                                        IDirect3DSurface8** out) override {
        if (out) *out = nullptr;
        BRIDGE_STUB_ONCE("CreateDepthStencilSurface");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CreateImageSurface(UINT w, UINT h, D3DFORMAT fmt,
                                                 IDirect3DSurface8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        dw::IDirect3DSurface8* inner = nullptr;
        HRESULT hr = m_inner->CreateImageSurface(w, h, dw::D3DFORMAT(fmt), &inner);
        *out = SUCCEEDED(hr) ? new BridgeSurface(inner, this) : nullptr;
        return hr;
    }
    HRESULT STDMETHODCALLTYPE CopyRects(IDirect3DSurface8* src, const RECT* rects, UINT count,
                                        IDirect3DSurface8* dst, const POINT* points) override {
        if (!src || !dst) return D3DERR_INVALIDCALL;
        return m_inner->CopyRects(static_cast<BridgeSurface*>(src)->inner(), cvt(rects), count,
                                  static_cast<BridgeSurface*>(dst)->inner(),
                                  reinterpret_cast<const dw::POINT*>(points));
    }
    HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture8* src, IDirect3DBaseTexture8* dst) override {
        if (!src || !dst) return D3DERR_INVALIDCALL;
        if (src->GetType() != D3DRTYPE_TEXTURE || dst->GetType() != D3DRTYPE_TEXTURE)
            return D3DERR_NOTAVAILABLE;
        return m_inner->UpdateTexture(static_cast<BridgeTexture*>(src)->inner(),
                                      static_cast<BridgeTexture*>(dst)->inner());
    }
    HRESULT STDMETHODCALLTYPE GetFrontBuffer(IDirect3DSurface8*) override {
        BRIDGE_STUB_ONCE("GetFrontBuffer");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE SetRenderTarget(IDirect3DSurface8*, IDirect3DSurface8*) override {
        BRIDGE_STUB_ONCE("SetRenderTarget");
        return D3D_OK;  // pretend success: engine restores the default RT at startup
    }
    HRESULT STDMETHODCALLTYPE GetRenderTarget(IDirect3DSurface8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        *out = fakeSurface(m_backBuffer, D3DFMT_X8R8G8B8);
        return *out ? D3D_OK : D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        *out = fakeSurface(m_depthSurface, D3DFMT_D24S8);
        return *out ? D3D_OK : D3DERR_NOTAVAILABLE;
    }

    // --- frame ---
    HRESULT STDMETHODCALLTYPE BeginScene() override { return m_inner->BeginScene(); }
    HRESULT STDMETHODCALLTYPE EndScene() override { return m_inner->EndScene(); }
    HRESULT STDMETHODCALLTYPE Clear(DWORD count, const D3DRECT*, DWORD flags, D3DCOLOR color,
                                    float z, DWORD stencil) override {
        return m_inner->Clear(count, nullptr, flags, color, z, stencil);
    }

    // --- transforms/state ---
    HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE s, const D3DMATRIX* m) override {
        return m_inner->SetTransform(dw::D3DTRANSFORMSTATETYPE(s), cvt(m));
    }
    HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE s, D3DMATRIX* m) override {
        return m_inner->GetTransform(dw::D3DTRANSFORMSTATETYPE(s), cvt(m));
    }
    HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE s, const D3DMATRIX* m) override {
        if (!m) return D3DERR_INVALIDCALL;
        D3DMATRIX cur{};
        GetTransform(s, &cur);
        D3DMATRIX r{};
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j) {
                float acc = 0;
                for (int k = 0; k < 4; ++k) acc += cur.m[i][k] * m->m[k][j];
                r.m[i][j] = acc;
            }
        return SetTransform(s, &r);
    }
    HRESULT STDMETHODCALLTYPE SetViewport(const D3DVIEWPORT8* v) override { return m_inner->SetViewport(cvt(v)); }
    HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT8* v) override { return m_inner->GetViewport(cvt(v)); }
    HRESULT STDMETHODCALLTYPE SetMaterial(const D3DMATERIAL8* m) override { return m_inner->SetMaterial(cvt(m)); }
    HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL8* m) override { return m_inner->GetMaterial(cvt(m)); }
    HRESULT STDMETHODCALLTYPE SetLight(DWORD i, const D3DLIGHT8* l) override { return m_inner->SetLight(i, cvt(l)); }
    HRESULT STDMETHODCALLTYPE GetLight(DWORD i, D3DLIGHT8* l) override { return m_inner->GetLight(i, cvt(l)); }
    HRESULT STDMETHODCALLTYPE LightEnable(DWORD i, WINBOOL e) override { return m_inner->LightEnable(i, e); }
    HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD i, WINBOOL* e) override { return m_inner->GetLightEnable(i, e); }
    HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD i, const float* p) override { return m_inner->SetClipPlane(i, p); }
    HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD, float* p) override {
        if (p) std::memset(p, 0, sizeof(float) * 4);
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE s, DWORD v) override {
        return m_inner->SetRenderState(dw::D3DRENDERSTATETYPE(s), v);
    }
    HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE s, DWORD* v) override {
        return m_inner->GetRenderState(dw::D3DRENDERSTATETYPE(s), v);
    }

    // --- state blocks (accepted, not replayed) ---
    HRESULT STDMETHODCALLTYPE BeginStateBlock() override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE EndStateBlock(DWORD* token) override {
        if (token) *token = ++m_stateBlockToken;
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE ApplyStateBlock(DWORD) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE CaptureStateBlock(DWORD) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE DeleteStateBlock(DWORD) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE, DWORD* token) override {
        if (token) *token = ++m_stateBlockToken;
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE SetClipStatus(const D3DCLIPSTATUS8*) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS8* cs) override {
        if (cs) std::memset(cs, 0, sizeof(*cs));
        return D3D_OK;
    }

    // --- textures ---
    HRESULT STDMETHODCALLTYPE GetTexture(DWORD stage, IDirect3DBaseTexture8** out) override {
        if (!out) return D3DERR_INVALIDCALL;
        *out = (stage < 8) ? m_boundTextures[stage] : nullptr;
        if (*out) (*out)->AddRef();
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE SetTexture(DWORD stage, IDirect3DBaseTexture8* tex) override {
        dw::IDirect3DBaseTexture8* inner = nullptr;
        if (tex && tex->GetType() == D3DRTYPE_TEXTURE)
            inner = static_cast<BridgeTexture*>(tex)->inner();
        const HRESULT result = m_inner->SetTexture(stage, inner);
        if (SUCCEEDED(result) && stage < 8) {
            if (tex) tex->AddRef();
            IDirect3DBaseTexture8* previous = m_boundTextures[stage];
            m_boundTextures[stage] = tex;
            if (previous) previous->Release();
        }
        return result;
    }
    HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE t, DWORD* v) override {
        return m_inner->GetTextureStageState(stage, dw::D3DTEXTURESTAGESTATETYPE(t), v);
    }
    HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE t, DWORD v) override {
        return m_inner->SetTextureStageState(stage, dw::D3DTEXTURESTAGESTATETYPE(t), v);
    }
    HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* passes) override { return m_inner->ValidateDevice(passes); }
    HRESULT STDMETHODCALLTYPE GetInfo(DWORD, void*, DWORD) override { return E_FAIL; }

    // --- palettes (8-bit textures unused) ---
    HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT, const PALETTEENTRY*) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT, PALETTEENTRY*) override { return D3DERR_NOTAVAILABLE; }
    HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT* n) override {
        if (n) *n = 0;
        return D3D_OK;
    }

    // --- draws ---
    HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE t, UINT start, UINT count) override {
        return m_inner->DrawPrimitive(dw::D3DPRIMITIVETYPE(t), start, count);
    }
    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE t, UINT minIdx, UINT numV,
                                                   UINT startIdx, UINT primCount) override {
        return m_inner->DrawIndexedPrimitive(dw::D3DPRIMITIVETYPE(t), minIdx, numV, startIdx, primCount);
    }
    HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE t, UINT count, const void* data,
                                              UINT stride) override {
        return m_inner->DrawPrimitiveUP(dw::D3DPRIMITIVETYPE(t), count, data, stride);
    }
    HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE t, UINT minIdx, UINT numV,
                                                     UINT primCount, const void* idxData,
                                                     D3DFORMAT idxFmt, const void* vData,
                                                     UINT stride) override {
        return m_inner->DrawIndexedPrimitiveUP(dw::D3DPRIMITIVETYPE(t), minIdx, numV, primCount,
                                               idxData, dw::D3DFORMAT(idxFmt), vData, stride);
    }
    HRESULT STDMETHODCALLTYPE ProcessVertices(UINT, UINT, UINT, IDirect3DVertexBuffer8*, DWORD) override {
        BRIDGE_STUB_ONCE("ProcessVertices");
        return D3DERR_NOTAVAILABLE;
    }

    // --- shaders: FVF only (caps report no programmable shaders) ---
    HRESULT STDMETHODCALLTYPE CreateVertexShader(const DWORD*, const DWORD*, DWORD* shader, DWORD) override {
        if (shader) *shader = 0;
        BRIDGE_STUB_ONCE("CreateVertexShader");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE SetVertexShader(DWORD handle) override { return m_inner->SetVertexShader(handle); }
    HRESULT STDMETHODCALLTYPE GetVertexShader(DWORD* h) override {
        if (h) *h = 0;
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE DeleteVertexShader(DWORD) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE SetVertexShaderConstant(DWORD r, const void* d, DWORD c) override {
        return m_inner->SetVertexShaderConstant(r, d, c);
    }
    HRESULT STDMETHODCALLTYPE GetVertexShaderConstant(DWORD, void*, DWORD) override { return D3DERR_NOTAVAILABLE; }
    HRESULT STDMETHODCALLTYPE GetVertexShaderDeclaration(DWORD, void*, DWORD*) override { return D3DERR_NOTAVAILABLE; }
    HRESULT STDMETHODCALLTYPE GetVertexShaderFunction(DWORD, void*, DWORD*) override { return D3DERR_NOTAVAILABLE; }
    HRESULT STDMETHODCALLTYPE SetStreamSource(UINT n, IDirect3DVertexBuffer8* vb, UINT stride) override {
        return m_inner->SetStreamSource(n, vb ? static_cast<BridgeVB*>(vb)->inner() : nullptr, stride);
    }
    HRESULT STDMETHODCALLTYPE GetStreamSource(UINT, IDirect3DVertexBuffer8** vb, UINT* stride) override {
        if (vb) *vb = nullptr;
        if (stride) *stride = 0;
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer8* ib, UINT base) override {
        return m_inner->SetIndices(ib ? static_cast<BridgeIB*>(ib)->inner() : nullptr, base);
    }
    HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer8** ib, UINT* base) override {
        if (ib) *ib = nullptr;
        if (base) *base = 0;
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CreatePixelShader(const DWORD*, DWORD* shader) override {
        if (shader) *shader = 0;
        BRIDGE_STUB_ONCE("CreatePixelShader");
        return D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE SetPixelShader(DWORD h) override { return m_inner->SetPixelShader(h); }
    HRESULT STDMETHODCALLTYPE GetPixelShader(DWORD* h) override {
        if (h) *h = 0;
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE DeletePixelShader(DWORD) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE SetPixelShaderConstant(DWORD r, const void* d, DWORD c) override {
        return m_inner->SetPixelShaderConstant(r, d, c);
    }
    HRESULT STDMETHODCALLTYPE GetPixelShaderConstant(DWORD, void*, DWORD) override { return D3DERR_NOTAVAILABLE; }
    HRESULT STDMETHODCALLTYPE GetPixelShaderFunction(DWORD, void*, DWORD*) override { return D3DERR_NOTAVAILABLE; }

    // --- patches (unused by SAGE) ---
    HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT, const float*, const D3DRECTPATCH_INFO*) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT, const float*, const D3DTRIPATCH_INFO*) override { return D3D_OK; }
    HRESULT STDMETHODCALLTYPE DeletePatch(UINT) override { return D3D_OK; }

private:
    // Lazily created CPU-side stand-ins; the engine mostly reads their desc.
    IDirect3DSurface8* fakeSurface(IDirect3DSurface8*& slot, D3DFORMAT fmt) {
        if (!slot) {
            dw::IDirect3DSurface8* inner = nullptr;
            if (SUCCEEDED(m_inner->CreateImageSurface(m_bbWidth, m_bbHeight,
                                                      dw::D3DFORMAT(fmt), &inner)))
                slot = new BridgeSurface(inner, this);
        }
        if (slot) slot->AddRef();
        return slot;
    }

    dw::IDirect3DDevice8* m_inner;
    IDirect3D8* m_parent;
    IDirect3DBaseTexture8* m_boundTextures[8] = {};
    IDirect3DSurface8* m_backBuffer = nullptr;
    IDirect3DSurface8* m_depthSurface = nullptr;
    DWORD m_stateBlockToken = 0;
    UINT m_bbWidth, m_bbHeight;
};

HRESULT STDMETHODCALLTYPE BridgeVB::GetDevice(IDirect3DDevice8** dev) {
    if (!dev) return D3DERR_INVALIDCALL;
    *dev = m_dev;
    m_dev->AddRef();
    return D3D_OK;
}
HRESULT STDMETHODCALLTYPE BridgeIB::GetDevice(IDirect3DDevice8** dev) {
    if (!dev) return D3DERR_INVALIDCALL;
    *dev = m_dev;
    m_dev->AddRef();
    return D3D_OK;
}
HRESULT STDMETHODCALLTYPE BridgeSurface::GetDevice(IDirect3DDevice8** dev) {
    if (!dev) return D3DERR_INVALIDCALL;
    *dev = m_dev;
    m_dev->AddRef();
    return D3D_OK;
}
HRESULT STDMETHODCALLTYPE BridgeTexture::GetDevice(IDirect3DDevice8** dev) {
    if (!dev) return D3DERR_INVALIDCALL;
    *dev = m_dev;
    m_dev->AddRef();
    return D3D_OK;
}
HRESULT STDMETHODCALLTYPE BridgeTexture::GetSurfaceLevel(UINT level, IDirect3DSurface8** out) {
    if (!out) return D3DERR_INVALIDCALL;
    dw::IDirect3DSurface8* inner = nullptr;
    HRESULT hr = m_inner->GetSurfaceLevel(level, &inner);
    *out = SUCCEEDED(hr) ? new BridgeSurface(inner, m_dev) : nullptr;
    return hr;
}

// ---------------------------------------------------------------------------
// Caps: generous fixed-function claims, no programmable shaders.
// The engine's dx8caps layer reads these to pick rendering paths.
// ---------------------------------------------------------------------------
void fillCaps(D3DCAPS8* caps) {
    std::memset(caps, 0, sizeof(*caps));
    caps->DeviceType = D3DDEVTYPE_HAL;
    caps->AdapterOrdinal = 0;
    caps->Caps2 = D3DCAPS2_FULLSCREENGAMMA | D3DCAPS2_DYNAMICTEXTURES;
    caps->PresentationIntervals = D3DPRESENT_INTERVAL_ONE | D3DPRESENT_INTERVAL_IMMEDIATE;
    caps->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_HWRASTERIZATION |
                    D3DDEVCAPS_DRAWPRIMTLVERTEX | D3DDEVCAPS_TEXTUREVIDEOMEMORY |
                    D3DDEVCAPS_TLVERTEXVIDEOMEMORY;
    caps->PrimitiveMiscCaps = D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW |
                              D3DPMISCCAPS_MASKZ | D3DPMISCCAPS_BLENDOP;
    caps->RasterCaps = D3DPRASTERCAPS_FOGVERTEX | D3DPRASTERCAPS_FOGTABLE | D3DPRASTERCAPS_ZBIAS |
                       D3DPRASTERCAPS_MIPMAPLODBIAS | D3DPRASTERCAPS_ANISOTROPY;
    caps->ZCmpCaps = 0xFF;      // all D3DPCMPCAPS_*
    caps->AlphaCmpCaps = 0xFF;
    caps->SrcBlendCaps = 0x1FFF;   // all D3DPBLENDCAPS_*
    caps->DestBlendCaps = 0x1FFF;
    caps->ShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB | D3DPSHADECAPS_SPECULARGOURAUDRGB |
                      D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_FOGGOURAUD;
    caps->TextureCaps = D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_ALPHA |
                        D3DPTEXTURECAPS_MIPMAP | D3DPTEXTURECAPS_PROJECTED;
    caps->TextureFilterCaps = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR |
                              D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MIPFLINEAR |
                              D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR;
    caps->TextureAddressCaps = D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR |
                               D3DPTADDRESSCAPS_CLAMP | D3DPTADDRESSCAPS_INDEPENDENTUV;
    caps->LineCaps = D3DLINECAPS_TEXTURE | D3DLINECAPS_ZTEST | D3DLINECAPS_BLEND;
    caps->MaxTextureWidth = 4096;
    caps->MaxTextureHeight = 4096;
    caps->MaxTextureRepeat = 8192;
    caps->MaxTextureAspectRatio = 4096;
    caps->MaxAnisotropy = 1;
    caps->MaxVertexW = 1e10f;
    caps->GuardBandLeft = -32768.f;
    caps->GuardBandTop = -32768.f;
    caps->GuardBandRight = 32768.f;
    caps->GuardBandBottom = 32768.f;
    caps->StencilCaps = D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE |
                        D3DSTENCILCAPS_INCR | D3DSTENCILCAPS_DECR | D3DSTENCILCAPS_INVERT;
    caps->FVFCaps = 8;  // up to 8 texcoord sets
    caps->TextureOpCaps = D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
                          D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_MODULATE4X |
                          D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_ADDSIGNED | D3DTEXOPCAPS_SUBTRACT |
                          D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA |
                          D3DTEXOPCAPS_BLENDCURRENTALPHA | D3DTEXOPCAPS_DOTPRODUCT3;
    caps->MaxTextureBlendStages = 2;
    caps->MaxSimultaneousTextures = 2;
    caps->VertexProcessingCaps = D3DVTXPCAPS_DIRECTIONALLIGHTS | D3DVTXPCAPS_POSITIONALLIGHTS |
                                 D3DVTXPCAPS_MATERIALSOURCE7 | D3DVTXPCAPS_TEXGEN;
    caps->MaxActiveLights = 8;
    caps->MaxUserClipPlanes = 0;
    caps->MaxVertexBlendMatrices = 0;
    caps->MaxPointSize = 64.f;
    caps->MaxPrimitiveCount = 1u << 20;
    caps->MaxVertexIndex = 1u << 20;
    caps->MaxStreams = 1;
    caps->MaxStreamStride = 256;
    caps->VertexShaderVersion = D3DVS_VERSION(0, 0);  // FFP only
    caps->PixelShaderVersion = D3DPS_VERSION(0, 0);
}

HRESULT STDMETHODCALLTYPE BridgeDevice::GetDeviceCaps(D3DCAPS8* caps) {
    if (!caps) return D3DERR_INVALIDCALL;
    fillCaps(caps);
    return D3D_OK;
}

// ---------------------------------------------------------------------------
class BridgeD3D8 final : public BridgeUnknown<IDirect3D8> {
public:
    BridgeD3D8() : m_inner(dw::CreateDirect3D8()) {}
    ~BridgeD3D8() override { m_inner->Release(); }

    HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void*) override { return D3D_OK; }
    UINT STDMETHODCALLTYPE GetAdapterCount() override { return 1; }
    HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER8* id) override {
        if (!id) return D3DERR_INVALIDCALL;
        std::memset(id, 0, sizeof(*id));
        std::snprintf(id->Driver, sizeof(id->Driver), "d8web");
        std::snprintf(id->Description, sizeof(id->Description), "d8web WebGL2 (Igroteka)");
        id->VendorId = 0x10DE;  // report a mainstream vendor so GPU heuristics take a known path
        id->DeviceId = 0x0201;
        return D3D_OK;
    }
    UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT) override { return kModeCount; }
    HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT, UINT mode, D3DDISPLAYMODE* out) override {
        if (!out || mode >= kModeCount) return D3DERR_INVALIDCALL;
        *out = kModes[mode];
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT, D3DDISPLAYMODE* out) override {
        if (!out) return D3DERR_INVALIDCALL;
        *out = kModes[1];  // 1024x768 default, matches the SDL window
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, WINBOOL) override {
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE,
                                                D3DFORMAT checkFormat) override {
        switch (checkFormat) {
            case D3DFMT_A8R8G8B8:
            case D3DFMT_X8R8G8B8:
            case D3DFMT_R5G6B5:
            case D3DFMT_A1R5G5B5:
            case D3DFMT_A4R4G4B4:
            case D3DFMT_L8:
            case D3DFMT_A8L8:
            case D3DFMT_DXT1:
            case D3DFMT_DXT2:
            case D3DFMT_DXT3:
            case D3DFMT_DXT4:
            case D3DFMT_DXT5:
            case D3DFMT_D16:
            case D3DFMT_D24S8:
            case D3DFMT_INDEX16:
            case D3DFMT_INDEX32:
                return D3D_OK;
            default:
                return D3DERR_NOTAVAILABLE;
        }
    }
    HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT, D3DDEVTYPE, D3DFORMAT, WINBOOL,
                                                         D3DMULTISAMPLE_TYPE ms) override {
        return ms == D3DMULTISAMPLE_NONE ? D3D_OK : D3DERR_NOTAVAILABLE;
    }
    HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT) override {
        return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS8* caps) override {
        if (!caps) return D3DERR_INVALIDCALL;
        fillCaps(caps);
        return D3D_OK;
    }
    HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT) override { return nullptr; }
    HRESULT STDMETHODCALLTYPE CreateDevice(UINT, D3DDEVTYPE, HWND wnd, DWORD,
                                           D3DPRESENT_PARAMETERS* pp,
                                           IDirect3DDevice8** out) override {
        if (!out || !pp) return D3DERR_INVALIDCALL;
        dw::IDirect3DDevice8* inner = nullptr;
        HRESULT hr = m_inner->CreateDevice(0, dw::D3DDEVTYPE_HAL, wnd,
                                           dw::D3DCREATE_HARDWARE_VERTEXPROCESSING, cvt(pp), &inner);
        *out = SUCCEEDED(hr)
                   ? new BridgeDevice(inner, this, pp->BackBufferWidth, pp->BackBufferHeight)
                   : nullptr;
        if (SUCCEEDED(hr)) AddRef();  // device holds a parent reference
        return hr;
    }

private:
    static constexpr UINT kModeCount = 5;
    static constexpr D3DDISPLAYMODE kModes[kModeCount] = {
        {800, 600, 60, D3DFMT_X8R8G8B8},
        {1024, 768, 60, D3DFMT_X8R8G8B8},
        {1280, 720, 60, D3DFMT_X8R8G8B8},
        {1600, 900, 60, D3DFMT_X8R8G8B8},
        {1920, 1080, 60, D3DFMT_X8R8G8B8},
    };

    dw::IDirect3D8* m_inner;
};

constexpr D3DDISPLAYMODE BridgeD3D8::kModes[];

}  // namespace

// Entry point handed to DX8Wrapper on wasm in place of the dlopen'd symbol.
extern "C" IDirect3D8* WINAPI Igroteka_Direct3DCreate8(UINT) {
    std::fprintf(stderr, "[d8web-bridge] Igroteka_Direct3DCreate8: serving d8web WebGL2 backend\n");
    return new BridgeD3D8();
}
