/**
 * DirectX 8 Buffer Compatibility Layer
 * 
 * Phase 41: Provides legacy DX8VertexBufferClass and DX8IndexBufferClass
 * interfaces that map to the new Vulkan graphics driver backend.
 * 
 * This allows old game code to use buffers without modification while
 * actual rendering is delegated to the pluggable Vulkan backend.
 */

#pragma once

#ifndef DX8BUFFER_COMPAT_H
#define DX8BUFFER_COMPAT_H

#include "IGraphicsDriver.h"

namespace Graphics {

// Forward declare interface
class IGraphicsDriver;

/**
 * Global accessor for the current graphics driver instance.
 * Initialized by GraphicsDriverFactory during game startup.
 * 
 * Returns nullptr if no driver has been created yet.
 */
IGraphicsDriver* GetGraphicsDriver();

/**
 * Internal function to set the current graphics driver instance.
 * Called by GraphicsDriverFactory::CreateDriver() - NOT for game code.
 */
void SetGraphicsDriver(IGraphicsDriver* driver);

} // namespace Graphics

/**
 * Compatibility wrapper for legacy DirectX 8 vertex buffer operations.
 * 
 * This class provides the interface the old game code expects while
 * delegating to the new Vulkan-based graphics driver.
 */
class DX8VertexBufferClass
{
public:
    enum UsageFlags {
        USAGE_DEFAULT = 0,
        USAGE_DYNAMIC = 1,
        USAGE_SOFTWAREPROCESSING = 2
    };

    /**
     * Create a vertex buffer with the specified parameters.
     * @param fvf Flexible vertex format (compatible with D3D8 FVF codes)
     * @param numVertices Number of vertices to allocate
     * @param usage Usage flags (dynamic/static)
     */
    DX8VertexBufferClass(uint32_t fvf, uint32_t numVertices, UsageFlags usage = USAGE_DEFAULT);
    
    ~DX8VertexBufferClass();

    /**
     * Lock the buffer for CPU access.
     * @param flags Lock flags (D3DLOCK_DISCARD, D3DLOCK_READONLY, etc.)
     * @return Pointer to locked buffer memory, or nullptr on failure
     */
    void* Lock(uint32_t flags = 0);
    
    /**
     * Unlock the buffer after CPU modifications.
     * @return true if successful
     */
    bool Unlock();

    /**
     * Get the size of the buffer in bytes.
     * @return Size in bytes
     */
    uint32_t GetSize() const;

    /**
     * Get the number of vertices in the buffer.
     * @return Vertex count
     */
    uint32_t GetVertexCount() const;

    /**
     * Get the vertex format (FVF) for this buffer.
     * @return D3D8 FVF code
     */
    uint32_t GetFormat() const { return m_fvf; }

    /**\n     * Get the graphics driver handle for this buffer.
     * @return Handle used by the graphics driver
     */
    Graphics::VertexBufferHandle GetDriverHandle() const { return m_handle; }

private:
    uint32_t m_fvf;                           // Flexible vertex format
    uint32_t m_numVertices;                   // Number of vertices
    uint32_t m_vertexSize;                    // Size per vertex in bytes
    [[maybe_unused]] UsageFlags m_usage;     // Usage flags
    void* m_lockedPtr;                        // Locked pointer (valid when locked)
    bool m_isLocked;                          // Is currently locked
    Graphics::VertexBufferHandle m_handle;   // Driver handle
    Graphics::IGraphicsDriver* m_driver;     // Cached driver pointer

    // Helper to calculate vertex size from FVF
    uint32_t CalculateVertexSize(uint32_t fvf);
};


/**
 * Compatibility wrapper for legacy DirectX 8 index buffer operations.
 */
class DX8IndexBufferClass
{
public:
    enum IndexFormat {
        INDEX_16BIT = 0,
        INDEX_32BIT = 1
    };

    enum UsageFlags {
        USAGE_DEFAULT = 0,
        USAGE_DYNAMIC = 1,
        USAGE_SOFTWAREPROCESSING = 2
    };

    /**
     * Create an index buffer with the specified parameters.
     * @param format Index format (16-bit or 32-bit)
     * @param numIndices Number of indices to allocate
     * @param usage Usage flags (dynamic/static)
     */
    DX8IndexBufferClass(IndexFormat format, uint32_t numIndices, UsageFlags usage = USAGE_DEFAULT);
    
    ~DX8IndexBufferClass();

    /**
     * Lock the buffer for CPU access.
     * @param flags Lock flags (D3DLOCK_DISCARD, D3DLOCK_READONLY, etc.)
     * @return Pointer to locked buffer memory, or nullptr on failure
     */
    void* Lock(uint32_t flags = 0);
    
    /**
     * Unlock the buffer after CPU modifications.
     * @return true if successful
     */
    bool Unlock();

    /**
     * Get the size of the buffer in bytes.
     * @return Size in bytes
     */
    uint32_t GetSize() const;

    /**
     * Get the number of indices in the buffer.
     * @return Index count
     */
    uint32_t GetIndexCount() const;

    /**
     * Get the index format for this buffer.
     * @return Format (16-bit or 32-bit)
     */
    IndexFormat GetFormat() const { return m_format; }

    /**
     * Get the graphics driver handle for this buffer.
     * @return Handle used by the graphics driver
     */
    Graphics::IndexBufferHandle GetDriverHandle() const { return m_handle; }

private:
    IndexFormat m_format;                     // Index format (16 or 32 bit)
    uint32_t m_numIndices;                    // Number of indices
    uint32_t m_indexSize;                     // Size per index in bytes
    [[maybe_unused]] UsageFlags m_usage;     // Usage flags
    void* m_lockedPtr;                        // Locked pointer (valid when locked)
    bool m_isLocked;                          // Is currently locked
    Graphics::IndexBufferHandle m_handle;    // Driver handle
    Graphics::IGraphicsDriver* m_driver;     // Cached driver pointer
};


/**
 * Helper class for RAII-style buffer locking.
 * 
 * Used in game code like:
 *   DX8VertexBufferClass::WriteLockClass lock(vertexBuffer, D3DLOCK_DISCARD);
 *   void* data = lock.GetData();
 *   // Modify data...
 *   // Unlock happens automatically in destructor
 */
class WriteLockHelper
{
public:
    WriteLockHelper() = default;
    virtual ~WriteLockHelper() = default;
    virtual void* GetData() = 0;
};

// Vertex buffer lock helper
class VertexBufferWriteLock : public WriteLockHelper
{
public:
    VertexBufferWriteLock(DX8VertexBufferClass* buffer, uint32_t flags = 0)
        : m_buffer(buffer), m_data(nullptr)
    {
        if (m_buffer) {
            m_data = m_buffer->Lock(flags);
        }
    }

    ~VertexBufferWriteLock()
    {
        if (m_buffer && m_data) {
            m_buffer->Unlock();
        }
    }

    void* GetData() override { return m_data; }

private:
    DX8VertexBufferClass* m_buffer;
    void* m_data;
};

// Index buffer lock helper
class IndexBufferWriteLock : public WriteLockHelper
{
public:
    IndexBufferWriteLock(DX8IndexBufferClass* buffer, uint32_t flags = 0)
        : m_buffer(buffer), m_data(nullptr)
    {
        if (m_buffer) {
            m_data = m_buffer->Lock(flags);
        }
    }

    ~IndexBufferWriteLock()
    {
        if (m_buffer && m_data) {
            m_buffer->Unlock();
        }
    }

    void* GetData() override { return m_data; }

private:
    DX8IndexBufferClass* m_buffer;
    void* m_data;
};

// Macro to support old code patterns:
// DX8VertexBufferClass::WriteLockClass lock(buffer, flags);
template<typename BufferType>
class WriteLockClass;

template<>
class WriteLockClass<DX8VertexBufferClass> : public VertexBufferWriteLock
{
public:
    using VertexBufferWriteLock::VertexBufferWriteLock;
};

template<>
class WriteLockClass<DX8IndexBufferClass> : public IndexBufferWriteLock
{
public:
    using IndexBufferWriteLock::IndexBufferWriteLock;
};

// Static accessor pattern support for old code
namespace DX8VertexBufferClassHelper {
    template<typename T>
    class WriteLockClass {
    public:
        // This allows: DX8VertexBufferClass::WriteLockClass lock(buffer, flags);
    };
}

#endif // DX8BUFFER_COMPAT_H
