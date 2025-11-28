/**
 * @file d3d8_memory_texture.h
 * @brief Phase 51: Real DirectX 8 Texture and Surface Implementations
 * 
 * Provides memory-backed implementations of IDirect3DTexture8 and IDirect3DSurface8
 * that store actual pixel data, enabling the game's texture system to work on
 * non-Windows platforms without real DirectX.
 * 
 * These implementations:
 * - Allocate pixel buffers based on format and dimensions
 * - Support Lock/Unlock for CPU access
 * - Track mip levels and surfaces
 * - Enable pixel operations (DrawPixel, Clear, Copy)
 * 
 * Phase 51: GameClient Subsystem Initialization
 * Created: Phase 51 - Texture/Surface foundation for W3DDisplay
 */

#ifndef D3D8_MEMORY_TEXTURE_H_INCLUDED
#define D3D8_MEMORY_TEXTURE_H_INCLUDED

#include "d3d8_vulkan_graphics_compat.h"
#include <cstdint>
#include <cstring>
#include <vector>

/**
 * Calculate bytes per pixel for a given D3D format
 */
inline unsigned int GetBytesPerPixel(D3DFORMAT format) {
    switch (format) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_A8B8G8R8:
        case D3DFMT_X8B8G8R8:
        case D3DFMT_D32:
        case D3DFMT_D24S8:
        case D3DFMT_D24X8:
            return 4;
        case D3DFMT_R8G8B8:
            return 3;
        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4:
        case D3DFMT_X4R4G4B4:
        case D3DFMT_A8L8:
        case D3DFMT_V8U8:
        case D3DFMT_D16:
            return 2;
        case D3DFMT_A8:
        case D3DFMT_L8:
        case D3DFMT_P8:
        case D3DFMT_R3G3B2:
            return 1;
        case D3DFMT_DXT1:
            return 0;  // Block compressed - special handling needed
        case D3DFMT_DXT3:
        case D3DFMT_DXT5:
            return 0;  // Block compressed - special handling needed
        default:
            return 4;  // Default to 32-bit
    }
}

/**
 * Calculate row pitch for a surface/texture level
 */
inline unsigned int CalculatePitch(unsigned int width, D3DFORMAT format) {
    unsigned int bpp = GetBytesPerPixel(format);
    if (bpp == 0) {
        // DXT compressed formats: 4x4 blocks
        // DXT1: 8 bytes per block, DXT3/5: 16 bytes per block
        unsigned int blockWidth = (width + 3) / 4;
        if (format == D3DFMT_DXT1) {
            return blockWidth * 8;
        } else {
            return blockWidth * 16;
        }
    }
    return width * bpp;
}

/**
 * Calculate total size for a surface/texture level
 */
inline unsigned int CalculateSurfaceSize(unsigned int width, unsigned int height, D3DFORMAT format) {
    unsigned int bpp = GetBytesPerPixel(format);
    if (bpp == 0) {
        // DXT compressed formats: 4x4 blocks
        unsigned int blockWidth = (width + 3) / 4;
        unsigned int blockHeight = (height + 3) / 4;
        if (format == D3DFMT_DXT1) {
            return blockWidth * blockHeight * 8;
        } else {
            return blockWidth * blockHeight * 16;
        }
    }
    return width * height * bpp;
}

/**
 * @class MemoryDirect3DSurface8
 * @brief Memory-backed implementation of IDirect3DSurface8
 * 
 * Stores actual pixel data that can be locked and modified.
 */
class MemoryDirect3DSurface8 : public IDirect3DSurface8 {
public:
    MemoryDirect3DSurface8(unsigned int width, unsigned int height, D3DFORMAT format, D3DPOOL pool = D3DPOOL_MANAGED)
        : m_width(width)
        , m_height(height)
        , m_format(format)
        , m_pool(pool)
        , m_refCount(1)
        , m_locked(false)
    {
        m_pitch = CalculatePitch(width, format);
        m_size = CalculateSurfaceSize(width, height, format);
        m_data.resize(m_size, 0);
        
        fprintf(stderr, "[MemoryDirect3DSurface8] Created %ux%u surface, format=%d, size=%u bytes\n",
                width, height, (int)format, m_size);
    }
    
    virtual ~MemoryDirect3DSurface8() {
        fprintf(stderr, "[MemoryDirect3DSurface8] Destroyed %ux%u surface\n", m_width, m_height);
    }
    
    // COM methods
    HRESULT QueryInterface(const IID &riid, void **ppvObj) override { return S_OK; }
    
    ULONG AddRef() override { 
        return ++m_refCount; 
    }
    
    ULONG Release() override { 
        ULONG count = --m_refCount;
        if (count == 0) {
            delete this;
        }
        return count;
    }
    
    // Surface methods
    HRESULT GetDesc(D3DSURFACE_DESC *pDesc) override {
        if (!pDesc) return E_INVALIDARG;
        
        pDesc->Format = m_format;
        pDesc->Type = D3DRTYPE_SURFACE;
        pDesc->Usage = 0;
        pDesc->Pool = m_pool;
        pDesc->Width = m_width;
        pDesc->Height = m_height;
        pDesc->Size = m_size;
        
        return S_OK;
    }
    
    HRESULT LockRect(D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) override {
        if (!pLockedRect) return E_INVALIDARG;
        if (m_locked) return E_FAIL;  // Already locked
        
        m_locked = true;
        
        // If pRect is NULL, lock entire surface
        if (pRect == nullptr) {
            pLockedRect->pBits = m_data.data();
            pLockedRect->Pitch = m_pitch;
        } else {
            // Lock sub-region
            unsigned int bpp = GetBytesPerPixel(m_format);
            if (bpp == 0) {
                // Compressed formats - lock whole surface
                pLockedRect->pBits = m_data.data();
                pLockedRect->Pitch = m_pitch;
            } else {
                unsigned int offset = pRect->top * m_pitch + pRect->left * bpp;
                pLockedRect->pBits = m_data.data() + offset;
                pLockedRect->Pitch = m_pitch;
            }
        }
        
        return S_OK;
    }
    
    HRESULT UnlockRect() override {
        if (!m_locked) return E_FAIL;
        m_locked = false;
        return S_OK;
    }
    
    HRESULT GetDC(HDC *phdc) override { return E_NOTIMPL; }
    HRESULT ReleaseDC(HDC hdc) override { return E_NOTIMPL; }
    
    // Additional accessors for internal use
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    D3DFORMAT GetFormat() const { return m_format; }
    unsigned int GetPitch() const { return m_pitch; }
    void* GetData() { return m_data.data(); }
    const void* GetData() const { return m_data.data(); }
    
private:
    unsigned int m_width;
    unsigned int m_height;
    D3DFORMAT m_format;
    D3DPOOL m_pool;
    unsigned int m_pitch;
    unsigned int m_size;
    std::vector<uint8_t> m_data;
    ULONG m_refCount;
    bool m_locked;
};

/**
 * @class MemoryDirect3DTexture8
 * @brief Memory-backed implementation of IDirect3DTexture8
 * 
 * Stores actual texture data with mip levels that can be locked and modified.
 */
class MemoryDirect3DTexture8 : public IDirect3DTexture8 {
public:
    MemoryDirect3DTexture8(unsigned int width, unsigned int height, unsigned int levels,
                           DWORD usage, D3DFORMAT format, D3DPOOL pool)
        : m_width(width)
        , m_height(height)
        , m_format(format)
        , m_pool(pool)
        , m_usage(usage)
        , m_refCount(1)
    {
        // Calculate number of mip levels
        if (levels == 0) {
            // Auto-generate mip count
            m_levelCount = 1;
            unsigned int w = width, h = height;
            while (w > 1 || h > 1) {
                w = (w > 1) ? w / 2 : 1;
                h = (h > 1) ? h / 2 : 1;
                m_levelCount++;
            }
        } else {
            m_levelCount = levels;
        }
        
        // Create surfaces for each mip level
        unsigned int w = width, h = height;
        for (unsigned int i = 0; i < m_levelCount; i++) {
            m_surfaces.push_back(new MemoryDirect3DSurface8(w, h, format, pool));
            w = (w > 1) ? w / 2 : 1;
            h = (h > 1) ? h / 2 : 1;
        }
        
        fprintf(stderr, "[MemoryDirect3DTexture8] Created %ux%u texture, format=%d, %u mip levels\n",
                width, height, (int)format, m_levelCount);
    }
    
    virtual ~MemoryDirect3DTexture8() {
        for (auto surface : m_surfaces) {
            surface->Release();
        }
        m_surfaces.clear();
        fprintf(stderr, "[MemoryDirect3DTexture8] Destroyed %ux%u texture\n", m_width, m_height);
    }
    
    // COM methods
    HRESULT QueryInterface(const IID &riid, void **ppvObj) override { return S_OK; }
    
    ULONG AddRef() override { 
        return ++m_refCount; 
    }
    
    ULONG Release() override { 
        ULONG count = --m_refCount;
        if (count == 0) {
            delete this;
        }
        return count;
    }
    
    // Texture state methods (from IDirect3DBaseTexture8)
    DWORD SetLOD(DWORD LODNew) override { return 0; }
    DWORD GetLOD() override { return 0; }
    DWORD GetLevelCount() override { return m_levelCount; }
    
    // Texture methods
    HRESULT GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) override {
        if (Level >= m_levelCount) return E_INVALIDARG;
        if (!pDesc) return E_INVALIDARG;
        
        return m_surfaces[Level]->GetDesc(pDesc);
    }
    
    HRESULT GetSurfaceLevel(UINT Level, IDirect3DSurface8 **ppSurfaceLevel) override {
        if (Level >= m_levelCount) return E_INVALIDARG;
        if (!ppSurfaceLevel) return E_INVALIDARG;
        
        m_surfaces[Level]->AddRef();
        *ppSurfaceLevel = m_surfaces[Level];
        
        return S_OK;
    }
    
    HRESULT LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags) override {
        if (Level >= m_levelCount) return E_INVALIDARG;
        
        return m_surfaces[Level]->LockRect(pLockedRect, pRect, Flags);
    }
    
    HRESULT UnlockRect(UINT Level) override {
        if (Level >= m_levelCount) return E_INVALIDARG;
        
        return m_surfaces[Level]->UnlockRect();
    }
    
    HRESULT AddDirtyRect(const RECT *pDirtyRect) override { return S_OK; }
    
    // Additional methods for texture management
    DWORD SetPriority(DWORD priority) { m_priority = priority; return m_priority; }
    DWORD GetPriority() const { return m_priority; }
    
    // Additional accessors for internal use
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    D3DFORMAT GetFormat() const { return m_format; }
    
private:
    unsigned int m_width;
    unsigned int m_height;
    D3DFORMAT m_format;
    D3DPOOL m_pool;
    DWORD m_usage;
    unsigned int m_levelCount;
    std::vector<MemoryDirect3DSurface8*> m_surfaces;
    ULONG m_refCount;
    DWORD m_priority = 0;
};

/**
 * Factory function to create a memory-backed texture
 */
inline IDirect3DTexture8* CreateMemoryTexture(
    unsigned int width, 
    unsigned int height, 
    unsigned int levels,
    DWORD usage, 
    D3DFORMAT format, 
    D3DPOOL pool)
{
    return new MemoryDirect3DTexture8(width, height, levels, usage, format, pool);
}

/**
 * Factory function to create a memory-backed surface
 */
inline IDirect3DSurface8* CreateMemorySurface(
    unsigned int width, 
    unsigned int height, 
    D3DFORMAT format,
    D3DPOOL pool = D3DPOOL_MANAGED)
{
    return new MemoryDirect3DSurface8(width, height, format, pool);
}

#endif // D3D8_MEMORY_TEXTURE_H_INCLUDED
