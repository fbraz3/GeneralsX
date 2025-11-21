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
#include "../WWVegas/WW3D2/d3dx8_vulkan_fvf_compat.h"  // Phase 42: FVF vertex format defines

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
 * 
 * Supports reference counting (Add_Ref/Release_Ref) for memory management.
 */
class DX8VertexBufferClass
{
public:
    enum UsageFlags {
        USAGE_DEFAULT = 0,
        USAGE_DYNAMIC = 1,
        USAGE_SOFTWAREPROCESSING = 2
    };

    // Forward declare WriteLockClass for nested class support
    class WriteLockClass;

    /**
     * Create a vertex buffer with the specified parameters.
     * @param fvf Flexible vertex format (compatible with D3D8 FVF codes)
     * @param numVertices Number of vertices to allocate
     * @param usage Usage flags (dynamic/static)
     */
    DX8VertexBufferClass(uint32_t fvf, uint32_t numVertices, UsageFlags usage = USAGE_DEFAULT);
    
    virtual ~DX8VertexBufferClass();

    /**
     * Reference counting: increment reference count
     */
    virtual void Add_Ref() { m_refCount++; }

    /**
     * Reference counting: decrement and delete if count reaches 0
     */
    virtual void Release_Ref() 
    { 
        if (--m_refCount <= 0) {
            delete this;
        }
    }

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

    /**
     * Get the graphics driver handle for this buffer.
     * @return Handle used by the graphics driver
     */
    Graphics::VertexBufferHandle GetDriverHandle() const { return m_handle; }

protected:
    uint32_t m_fvf;                           // Flexible vertex format
    uint32_t m_numVertices;                   // Number of vertices
    uint32_t m_vertexSize;                    // Size per vertex in bytes
    [[maybe_unused]] UsageFlags m_usage;     // Usage flags
    void* m_lockedPtr;                        // Locked pointer (valid when locked)
    bool m_isLocked;                          // Is currently locked
    Graphics::VertexBufferHandle m_handle;   // Driver handle
    Graphics::IGraphicsDriver* m_driver;     // Cached driver pointer
    int m_refCount = 1;                       // Reference count (starts at 1)

    // Helper to calculate vertex size from FVF
    uint32_t CalculateVertexSize(uint32_t fvf);

public:
    /**
     * Nested WriteLockClass for RAII-style buffer locking.
     * Allows: DX8VertexBufferClass::WriteLockClass lock(buffer, flags);
     */
    class WriteLockClass
    {
    public:
        WriteLockClass(DX8VertexBufferClass* buffer, uint32_t flags = 0)
            : m_buffer(buffer), m_data(nullptr)
        {
            if (m_buffer) {
                m_data = m_buffer->Lock(flags);
            }
        }

        ~WriteLockClass()
        {
            if (m_buffer && m_data) {
                m_buffer->Unlock();
            }
        }

        void* Get_Vertex_Array() { return m_data; }
        void* GetData() { return m_data; }

    private:
        DX8VertexBufferClass* m_buffer;
        void* m_data;
    };

    /**
     * Nested RAII lock class for appending vertices with offset.
     * 
     * Phase 42: Used for batching updates with offsets in legacy W3DVolumetricShadow
     * Locks buffer at offset for count vertices, unlocks on destruction.
     * 
     * Allows: DX8VertexBufferClass::AppendLockClass lock(buffer, offset, count);
     */
    class AppendLockClass
    {
    public:
        AppendLockClass(DX8VertexBufferClass* buffer, uint32_t offset, uint32_t count, uint32_t flags = 0)
            : m_buffer(buffer), m_data(nullptr), m_offset(offset), m_count(count)
        {
            if (m_buffer) {
                // For appending, we still use the main Lock() which locks the entire buffer
                // The offset/count are informational for the caller
                m_data = m_buffer->Lock(flags);
                
                // Calculate pointer offset for the caller
                if (m_data) {
                    // Each vertex size varies, but offset is in vertices not bytes
                    // The caller will handle the pointer arithmetic
                }
            }
        }

        ~AppendLockClass()
        {
            if (m_buffer && m_data) {
                m_buffer->Unlock();
            }
        }

        void* Get_Vertex_Array() { return m_data; }
        void* GetData() { return m_data; }
        uint32_t Get_Offset() const { return m_offset; }
        uint32_t Get_Count() const { return m_count; }

    private:
        DX8VertexBufferClass* m_buffer;
        void* m_data;
        uint32_t m_offset;
        uint32_t m_count;
    };
};


/**
 * Compatibility wrapper for legacy DirectX 8 index buffer operations.
 * 
 * Supports reference counting for memory management.
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

    // Forward declare WriteLockClass for nested class support
    class WriteLockClass;

    /**
     * Create an index buffer with the specified parameters.
     * @param format Index format (16-bit or 32-bit)
     * @param numIndices Number of indices to allocate
     * @param usage Usage flags (dynamic/static)
     */
    DX8IndexBufferClass(IndexFormat format, uint32_t numIndices, UsageFlags usage = USAGE_DEFAULT);
    
    /**
     * Alternative constructor that takes size and usage flags (defaults to 16-bit format)
     * Phase 42: This constructor is used by legacy W3DBufferManager code
     * @param numIndices Number of indices to allocate
     * @param usage Usage flags (USAGE_DEFAULT, USAGE_DYNAMIC, etc.)
     */
    DX8IndexBufferClass(uint32_t numIndices, UsageFlags usage = USAGE_DEFAULT)
        : m_format(INDEX_16BIT), m_numIndices(numIndices), m_indexSize(2),
          m_usage(usage), m_lockedPtr(nullptr), m_isLocked(false),
          m_handle(0), m_driver(nullptr), m_refCount(1)
    {
        // Initialize buffer through graphics driver - actual implementation in cpp file
        InitializeIndexBuffer();
    }

    virtual ~DX8IndexBufferClass();

    /**
     * Reference counting: increment reference count
     */
    virtual void Add_Ref() { m_refCount++; }

    /**
     * Reference counting: decrement and delete if count reaches 0
     */
    virtual void Release_Ref() 
    { 
        if (--m_refCount <= 0) {
            delete this;
        }
    }

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
     * Get the size of each index in bytes (2 for 16-bit, 4 for 32-bit).
     * Phase 42: Used by AppendLockClass for pointer arithmetic
     * @return Index size in bytes
     */
    uint32_t Get_Index_Size() const { return m_indexSize; }

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

protected:
    /**
     * Initialize the index buffer through graphics driver.
     * Phase 42: Called by constructors to set up buffer on GPU
     */
    void InitializeIndexBuffer();

    IndexFormat m_format;                     // Index format (16 or 32 bit)
    uint32_t m_numIndices;                    // Number of indices
    uint32_t m_indexSize;                     // Size per index in bytes
    [[maybe_unused]] UsageFlags m_usage;     // Usage flags
    void* m_lockedPtr;                        // Locked pointer (valid when locked)
    bool m_isLocked;                          // Is currently locked
    Graphics::IndexBufferHandle m_handle;    // Driver handle
    Graphics::IGraphicsDriver* m_driver;     // Cached driver pointer
    int m_refCount = 1;                       // Reference count (starts at 1)

public:
    /**
     * Nested WriteLockClass for RAII-style buffer locking.
     * Allows: DX8IndexBufferClass::WriteLockClass lock(buffer, flags);
     */
    class WriteLockClass
    {
    public:
        WriteLockClass(DX8IndexBufferClass* buffer, uint32_t flags = 0)
            : m_buffer(buffer), m_data(nullptr)
        {
            if (m_buffer) {
                m_data = m_buffer->Lock(flags);
            }
        }

        ~WriteLockClass()
        {
            if (m_buffer && m_data) {
                m_buffer->Unlock();
            }
        }

        void* Get_Index_Array() { return m_data; }
        void* GetData() { return m_data; }

    private:
        DX8IndexBufferClass* m_buffer;
        void* m_data;
    };

    /**
     * Nested RAII lock class for appending indices with offset.
     * 
     * Phase 42: Used for batching updates with offsets in legacy W3DVolumetricShadow
     * Locks buffer at offset for count indices, unlocks on destruction.
     * 
     * Allows: DX8IndexBufferClass::AppendLockClass lock(buffer, offset, count);
     */
    class AppendLockClass
    {
    public:
        AppendLockClass(DX8IndexBufferClass* buffer, uint32_t offset, uint32_t count, uint32_t flags = 0)
            : m_buffer(buffer), m_data(nullptr), m_offset(offset), m_count(count)
        {
            if (m_buffer) {
                // For appending, we still use the main Lock() which locks the entire buffer
                // The offset/count are informational for the caller
                m_data = m_buffer->Lock(flags);
                
                // Calculate pointer offset for the caller
                if (m_data) {
                    // Each index size is m_indexSize (2 or 4 bytes)
                    // The offset is in indices, so we convert to bytes for pointer arithmetic
                    uint8_t* basePtr = static_cast<uint8_t*>(m_data);
                    m_data = basePtr + (offset * m_buffer->Get_Index_Size());
                }
            }
        }

        ~AppendLockClass()
        {
            if (m_buffer) {
                m_buffer->Unlock();
            }
        }

        void* Get_Index_Array() { return m_data; }
        void* GetData() { return m_data; }
        uint32_t Get_Offset() const { return m_offset; }
        uint32_t Get_Count() const { return m_count; }

    private:
        DX8IndexBufferClass* m_buffer;
        void* m_data;
        uint32_t m_offset;
        uint32_t m_count;
    };
};


/**
 * Helper class for RAII-style buffer locking.
 * 
 * Legacy support - WriteLockClass is now nested in DX8VertexBufferClass/DX8IndexBufferClass
 * Use: DX8VertexBufferClass::WriteLockClass lock(buffer, flags);
 * Or:  DX8IndexBufferClass::WriteLockClass lock(buffer, flags);
 */

#endif // DX8BUFFER_COMPAT_H
