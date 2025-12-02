/**
 * DirectX 8 Buffer Compatibility Implementation
 * Phase 41: Wraps Vulkan buffer operations with DirectX 8 interface
 */

#include "dx8buffer_compat.h"
#include "IGraphicsDriver.h"
#include <cstring>
#include <cstdio>

 // Global graphics driver instance
static Graphics::IGraphicsDriver* g_graphics_driver = nullptr;

// Accessor functions
Graphics::IGraphicsDriver* Graphics::GetGraphicsDriver()
{
  if (!g_graphics_driver) {
    printf("WARNING: GetGraphicsDriver() called but no driver initialized\n");
  }
  return g_graphics_driver;
}

void Graphics::SetGraphicsDriver(Graphics::IGraphicsDriver* driver)
{
  if (g_graphics_driver && g_graphics_driver != driver) {
    printf("WARNING: SetGraphicsDriver() - replacing existing driver\n");
  }
  g_graphics_driver = driver;
  if (driver) {
    printf("SetGraphicsDriver() - Driver initialized (%p)\n", driver);
  }
}

// Helper to get vertex size from DirectX 8 FVF codes
static uint32_t GetVertexSizeFromFVF(uint32_t fvf)
{
  uint32_t size = 0;

  // Position (required)
  if (fvf & 0x0010) {  // D3DFVF_XYZ
    size += 12;      // 3 floats
  }
  else if (fvf & 0x0004) {  // D3DFVF_XYZW
    size += 16;      // 4 floats
  }
  else if (fvf & 0x0002) {  // D3DFVF_XYZRHW
    size += 16;      // 4 floats
  }
  else if (fvf & 0x0001) {  // D3DFVF_XYZ
    size += 12;      // 3 floats
  }

  // Normal
  if (fvf & 0x0020) {  // D3DFVF_NORMAL
    size += 12;      // 3 floats
  }

  // Diffuse color
  if (fvf & 0x0040) {  // D3DFVF_DIFFUSE
    size += 4;       // 1 DWORD
  }

  // Specular color
  if (fvf & 0x0080) {  // D3DFVF_SPECULAR
    size += 4;       // 1 DWORD
  }

  // Texture coordinates (up to 8 sets)
  uint32_t texCount = (fvf >> 8) & 0x0F;
  for (uint32_t i = 0; i < texCount; i++) {
    // Determine texture coord size
    uint32_t texFormat = (fvf >> (16 + i * 2)) & 0x03;
    if (texFormat == 0) {        // 2D
      size += 8;               // 2 floats
    }
    else if (texFormat == 1) { // 3D
      size += 12;              // 3 floats
    }
    else if (texFormat == 2) { // 4D
      size += 16;              // 4 floats
    }
    else if (texFormat == 3) { // 1D
      size += 4;               // 1 float
    }
  }

  return size;
}

// Constructor: DX8VertexBufferClass
DX8VertexBufferClass::DX8VertexBufferClass(uint32_t fvf, uint32_t numVertices, UsageFlags usage)
  : m_fvf(fvf)
  , m_numVertices(numVertices)
  , m_usage(usage)
  , m_lockedPtr(nullptr)
  , m_isLocked(false)
  , m_handle(Graphics::INVALID_HANDLE)
  , m_driver(nullptr)
{
  m_vertexSize = CalculateVertexSize(fvf);
  uint32_t totalSize = m_vertexSize * numVertices;

  // Get the graphics driver
  m_driver = Graphics::GetGraphicsDriver();
  if (!m_driver) {
    printf("DX8VertexBufferClass::DX8VertexBufferClass - ERROR: Graphics driver unavailable\n");
    return;
  }

  // Create buffer via driver
  bool is_dynamic = (usage == USAGE_DYNAMIC);
  m_handle = m_driver->CreateVertexBuffer(totalSize, is_dynamic, nullptr);

  if (m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8VertexBufferClass::DX8VertexBufferClass - ERROR: Failed to create vertex buffer (%u bytes, %u vertices)\n",
      totalSize, numVertices);
  }
  else {
    printf("DX8VertexBufferClass::DX8VertexBufferClass - Created vertex buffer (handle=%llu, %u bytes, %u vertices, FVF=0x%08X)\n",
      m_handle, totalSize, numVertices, fvf);
  }
}

// Destructor
DX8VertexBufferClass::~DX8VertexBufferClass()
{
  if (m_isLocked && m_driver && m_handle != Graphics::INVALID_HANDLE) {
    m_driver->UnlockVertexBuffer(m_handle);
    m_isLocked = false;
  }

  if (m_driver && m_handle != Graphics::INVALID_HANDLE) {
    m_driver->DestroyVertexBuffer(m_handle);
    printf("DX8VertexBufferClass::~DX8VertexBufferClass - Destroyed vertex buffer (handle=%llu)\n", m_handle);
  }
}

// Lock the vertex buffer
void* DX8VertexBufferClass::Lock(uint32_t flags)
{
  if (!m_driver || m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8VertexBufferClass::Lock - ERROR: Invalid driver or handle\n");
    return nullptr;
  }

  if (m_isLocked) {
    printf("DX8VertexBufferClass::Lock - WARNING: Buffer already locked\n");
    return m_lockedPtr;
  }

  uint32_t offset = 0;
  uint32_t size = m_vertexSize * m_numVertices;
  bool readonly = (flags & 0x00000001) != 0;  // D3DLOCK_READONLY = 1

  if (!m_driver->LockVertexBuffer(m_handle, offset, size, &m_lockedPtr, readonly)) {
    printf("DX8VertexBufferClass::Lock - ERROR: Failed to lock buffer\n");
    return nullptr;
  }

  m_isLocked = true;
  printf("DX8VertexBufferClass::Lock - Locked buffer (handle=%llu, ptr=%p)\n", m_handle, m_lockedPtr);
  return m_lockedPtr;
}

// Unlock the vertex buffer
bool DX8VertexBufferClass::Unlock()
{
  if (!m_driver || m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8VertexBufferClass::Unlock - ERROR: Invalid driver or handle\n");
    return false;
  }

  if (!m_isLocked) {
    printf("DX8VertexBufferClass::Unlock - WARNING: Buffer not locked\n");
    return true;
  }

  if (!m_driver->UnlockVertexBuffer(m_handle)) {
    printf("DX8VertexBufferClass::Unlock - ERROR: Failed to unlock buffer\n");
    return false;
  }

  m_isLocked = false;
  m_lockedPtr = nullptr;
  printf("DX8VertexBufferClass::Unlock - Unlocked buffer (handle=%llu)\n", m_handle);
  return true;
}

// Get buffer size
uint32_t DX8VertexBufferClass::GetSize() const
{
  if (!m_driver || m_handle == Graphics::INVALID_HANDLE) {
    return 0;
  }
  return m_driver->GetVertexBufferSize(m_handle);
}

// Get vertex count
uint32_t DX8VertexBufferClass::GetVertexCount() const
{
  return m_numVertices;
}

// Calculate vertex size from FVF
uint32_t DX8VertexBufferClass::CalculateVertexSize(uint32_t fvf)
{
  return GetVertexSizeFromFVF(fvf);
}

// ============================================================================
// Index Buffer Implementation
// ============================================================================

// Constructor: DX8IndexBufferClass
DX8IndexBufferClass::DX8IndexBufferClass(IndexFormat format, uint32_t numIndices, UsageFlags usage)
  : m_format(format)
  , m_numIndices(numIndices)
  , m_usage(usage)
  , m_lockedPtr(nullptr)
  , m_isLocked(false)
  , m_handle(Graphics::INVALID_HANDLE)
  , m_driver(nullptr)
{
  m_indexSize = (format == INDEX_16BIT) ? 2 : 4;
  uint32_t totalSize = m_indexSize * numIndices;

  // Get the graphics driver
  m_driver = Graphics::GetGraphicsDriver();
  if (!m_driver) {
    printf("DX8IndexBufferClass::DX8IndexBufferClass - ERROR: Graphics driver unavailable\n");
    return;
  }

  // Create buffer via driver
  bool is_dynamic = (usage == USAGE_DYNAMIC);
  bool is_32bit = (format == INDEX_32BIT);
  m_handle = m_driver->CreateIndexBuffer(totalSize, is_32bit, is_dynamic, nullptr);

  if (m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8IndexBufferClass::DX8IndexBufferClass - ERROR: Failed to create index buffer (%u bytes, %u indices, %s)\n",
      totalSize, numIndices, is_32bit ? "32-bit" : "16-bit");
  }
  else {
    printf("DX8IndexBufferClass::DX8IndexBufferClass - Created index buffer (handle=%llu, %u bytes, %u indices, %s)\n",
      m_handle, totalSize, numIndices, is_32bit ? "32-bit" : "16-bit");
  }
}

// Destructor
DX8IndexBufferClass::~DX8IndexBufferClass()
{
  if (m_isLocked && m_driver && m_handle != Graphics::INVALID_HANDLE) {
    m_driver->UnlockIndexBuffer(m_handle);
    m_isLocked = false;
  }

  if (m_driver && m_handle != Graphics::INVALID_HANDLE) {
    m_driver->DestroyIndexBuffer(m_handle);
    printf("DX8IndexBufferClass::~DX8IndexBufferClass - Destroyed index buffer (handle=%llu)\n", m_handle);
  }
}

// Initialize index buffer (for alternative constructor - Phase 42 legacy support)
void DX8IndexBufferClass::InitializeIndexBuffer()
{
  uint32_t totalSize = m_indexSize * m_numIndices;

  // Get the graphics driver
  m_driver = Graphics::GetGraphicsDriver();
  if (!m_driver) {
    printf("DX8IndexBufferClass::InitializeIndexBuffer - ERROR: Graphics driver unavailable\n");
    return;
  }

  // Create buffer via driver
  bool is_dynamic = (m_usage == USAGE_DYNAMIC);
  bool is_32bit = (m_format == INDEX_32BIT);
  m_handle = m_driver->CreateIndexBuffer(totalSize, is_32bit, is_dynamic, nullptr);

  if (m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8IndexBufferClass::InitializeIndexBuffer - ERROR: Failed to create index buffer (%u bytes, %u indices, %s)\n",
      totalSize, m_numIndices, is_32bit ? "32-bit" : "16-bit");
  }
  else {
    printf("DX8IndexBufferClass::InitializeIndexBuffer - Created index buffer (handle=%llu, %u bytes, %u indices, %s)\n",
      m_handle, totalSize, m_numIndices, is_32bit ? "32-bit" : "16-bit");
  }
}

// Lock the index buffer
void* DX8IndexBufferClass::Lock(uint32_t flags)
{
  if (!m_driver || m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8IndexBufferClass::Lock - ERROR: Invalid driver or handle\n");
    return nullptr;
  }

  if (m_isLocked) {
    printf("DX8IndexBufferClass::Lock - WARNING: Buffer already locked\n");
    return m_lockedPtr;
  }

  uint32_t offset = 0;
  uint32_t size = m_indexSize * m_numIndices;
  bool readonly = (flags & 0x00000001) != 0;  // D3DLOCK_READONLY = 1

  if (!m_driver->LockIndexBuffer(m_handle, offset, size, &m_lockedPtr, readonly)) {
    printf("DX8IndexBufferClass::Lock - ERROR: Failed to lock buffer\n");
    return nullptr;
  }

  m_isLocked = true;
  printf("DX8IndexBufferClass::Lock - Locked buffer (handle=%llu, ptr=%p)\n", m_handle, m_lockedPtr);
  return m_lockedPtr;
}

// Unlock the index buffer
bool DX8IndexBufferClass::Unlock()
{
  if (!m_driver || m_handle == Graphics::INVALID_HANDLE) {
    printf("DX8IndexBufferClass::Unlock - ERROR: Invalid driver or handle\n");
    return false;
  }

  if (!m_isLocked) {
    printf("DX8IndexBufferClass::Unlock - WARNING: Buffer not locked\n");
    return true;
  }

  if (!m_driver->UnlockIndexBuffer(m_handle)) {
    printf("DX8IndexBufferClass::Unlock - ERROR: Failed to unlock buffer\n");
    return false;
  }

  m_isLocked = false;
  m_lockedPtr = nullptr;
  printf("DX8IndexBufferClass::Unlock - Unlocked buffer (handle=%llu)\n", m_handle);
  return true;
}

// Get buffer size
uint32_t DX8IndexBufferClass::GetSize() const
{
  if (!m_driver || m_handle == Graphics::INVALID_HANDLE) {
    return 0;
  }
  return m_driver->GetIndexBufferSize(m_handle);
}

// Get index count
uint32_t DX8IndexBufferClass::GetIndexCount() const
{
  return m_numIndices;
}
