#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

// Backend-agnostic graphics driver interface
// All handles are opaque uint64_t values - NO backend-specific types

namespace Graphics {

// ============================================================================
// HANDLE DEFINITIONS
// ============================================================================

using TextureHandle = uint64_t;
using VertexBufferHandle = uint64_t;
using IndexBufferHandle = uint64_t;
using VertexFormatHandle = uint64_t;
using RenderTargetHandle = uint64_t;
using DepthStencilHandle = uint64_t;
using ShaderHandle = uint64_t;
using SamplerStateHandle = uint64_t;

constexpr uint64_t INVALID_HANDLE = 0;

// ============================================================================
// ENUMERATIONS - Backend Agnostic
// ============================================================================

enum class RenderState {
    Lighting,
    FogEnable,
    Specular,
    AlphaBlendEnable,
    SrcBlend,
    DstBlend,
    ZEnable,
    ZWriteEnable,
    CullMode,
    FillMode,
    Ambient,
    FogStart,
    FogEnd,
    FogDensity,
    FogColor,
    ZFunc,
    AlphaFunc,
    AlphaRef,
    Stencil,
    StencilFunc,
    StencilRef,
    StencilMask,
    StencilFail,
    StencilZFail,
    StencilPass,
    TextureFactor,
    DitherEnable,
    NormalizeNormals,
    ClipPlaneEnable,
    ColorWriteEnable,
    BlendFactor,
    SeparateAlphaBlendEnable,
    SrcBlendAlpha,
    DstBlendAlpha,
    BlendOpAlpha,
    BlendOp,
    PointSize,
    PointScaleEnable,
    PointSizeMin,
    PointSizeMax,
    PointScaleA,
    PointScaleB,
    PointScaleC,
    MultisampleAntialias,
    MultisampleType,
    DebugMonitorToken,
    IndexedVertexBlendEnable,
    TweenFactor,
    PositionDegree,
    NormalDegree,
    ScissorTestEnable,
    SliceEnable,
    SliceCount
};

enum class BlendMode {
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DstAlpha,
    InvDstAlpha,
    DstColor,
    InvDstColor,
    SrcAlphaSat,
    BothSrcAlpha,
    BothInvSrcAlpha,
    BlendFactor,
    InvBlendFactor,
    SrcColor1,
    InvSrcColor1
};

enum class CullMode {
    None,
    Clockwise,
    CounterClockwise
};

enum class FillMode {
    Point,
    Wireframe,
    Solid
};

enum class TextureFormat {
    Unknown,
    R8G8B8,
    A8R8G8B8,
    X8R8G8B8,
    R5G6B5,
    X1R5G5B5,
    A1R5G5B5,
    A4R4G4B4,
    R3G3B2,
    A8,
    A8R3G3B2,
    X4R4G4B4,
    A2B10G10R10,
    A8B8G8R8,
    X8B8G8R8,
    G16R16,
    A2R10G10B10,
    A16B16G16R16,
    L8,
    A8L8,
    A4L4,
    V8U8,
    L6V5U5,
    X8L8V8U8,
    Q8W8V8U8,
    V16U16,
    A2W10V10U10,
    DXT1,
    DXT2,
    DXT3,
    DXT4,
    DXT5,
    D16Lockable,
    D32,
    D15S1,
    D24S8,
    D24X8,
    D24X4S4,
    D32F,
    D24FS8,
    D32Lockable,
    DS16,
    Index16,
    Index32,
    Q16W16V16U16,
    Multi2ARGB8,
    YUY2,
    UYVY,
    YCRCB,
    INTZ,
    RAWZ,
    NULLREF,
    ATOC,
    SHADOW,
    DF16,
    DF24,
    MONO8,
    INVERT,
    FORCE_DWORD
};

enum class ComparisonFunc {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always
};

enum class StencilOp {
    Keep,
    Zero,
    Replace,
    IncrSat,
    DecrSat,
    Invert,
    Incr,
    Decr
};

enum class PrimitiveType {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    QuadList,
    QuadStrip,
    TrianglePatch,
    RectPatch,
    TriNPatch
};

enum class VertexElementType {
    Float1,
    Float2,
    Float3,
    Float4,
    D3DColor,
    Ubyte4,
    Short2,
    Short4,
    UByte4N,
    Short2N,
    Short4N,
    UShort2N,
    UShort4N,
    UDec3,
    Dec3N,
    Float16_2,
    Float16_4,
    Unused
};

enum class VertexElementUsage {
    Position,
    BlendWeight,
    BlendIndices,
    Normal,
    PointSize,
    TexCoord,
    Tangent,
    Binormal,
    TessFactor,
    PositionTransformed,
    Color,
    Fog,
    Depth,
    Sample
};

enum class BackendType {
    Vulkan,
    OpenGL,
    DirectX12,
    Metal,
    Software,
    Unknown
};

enum class LightType {
    Directional,
    Point,
    Spot
};

// ============================================================================
// STRUCTURES - Backend Agnostic
// ============================================================================

struct Color {
    float r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
};

struct Vector2 {
    float x, y;
    
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
};

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
};

struct Viewport {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    float minZ;
    float maxZ;
    
    Viewport() : x(0), y(0), width(800), height(600), minZ(0), maxZ(1) {}
};

struct Rect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
    
    Rect() : left(0), top(0), right(0), bottom(0) {}
};

struct VertexElement {
    uint16_t stream;
    uint16_t offset;
    VertexElementType type;
    VertexElementUsage usage;
    uint8_t usageIndex;
    
    VertexElement() : stream(0), offset(0), type(VertexElementType::Float3), 
                     usage(VertexElementUsage::Position), usageIndex(0) {}
};

struct TextureDescriptor {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    TextureFormat format;
    uint32_t mipLevels;
    bool cubeMap;
    bool renderTarget;
    bool depthStencil;
    bool dynamic;
    
    TextureDescriptor() : width(512), height(512), depth(1), format(TextureFormat::A8R8G8B8),
                         mipLevels(1), cubeMap(false), renderTarget(false), depthStencil(false), dynamic(false) {}
};

struct Material {
    Color ambient;
    Color diffuse;
    Color specular;
    Color emissive;
    float shininess;
    
    Material() : ambient(0.2f, 0.2f, 0.2f, 1.0f), diffuse(1, 1, 1, 1),
                specular(0.5f, 0.5f, 0.5f, 1), emissive(0, 0, 0, 1), shininess(32) {}
};

struct Light {
    LightType type;
    Color ambient;
    Color diffuse;
    Color specular;
    Vector3 position;
    Vector3 direction;
    float range;
    float falloff;
    float attenuation0;
    float attenuation1;
    float attenuation2;
    float theta;
    float phi;
    
    Light() : type(LightType::Directional), ambient(0.1f, 0.1f, 0.1f, 1),
             diffuse(1, 1, 1, 1), specular(1, 1, 1, 1), position(0, 0, 0),
             direction(0, -1, 0), range(1000), falloff(1), attenuation0(1),
             attenuation1(0), attenuation2(0), theta(0.785f), phi(1.57f) {}
};

struct RenderStateValue {
    RenderState state;
    uint64_t value;
    
    RenderStateValue() : state(RenderState::Lighting), value(0) {}
    RenderStateValue(RenderState s, uint64_t v) : state(s), value(v) {}
};

struct BlendStateDescriptor {
    bool enabled;
    BlendMode srcBlend;
    BlendMode dstBlend;
    BlendMode srcBlendAlpha;
    BlendMode dstBlendAlpha;
    
    BlendStateDescriptor() : enabled(false), srcBlend(BlendMode::One), dstBlend(BlendMode::Zero),
                            srcBlendAlpha(BlendMode::One), dstBlendAlpha(BlendMode::Zero) {}
};

struct DepthStencilStateDescriptor {
    bool depthEnable;
    bool depthWriteEnable;
    ComparisonFunc depthFunc;
    bool stencilEnable;
    uint8_t stencilReadMask;
    uint8_t stencilWriteMask;
    ComparisonFunc frontStencilFunc;
    StencilOp frontStencilFail;
    StencilOp frontStencilZFail;
    StencilOp frontStencilPass;
    ComparisonFunc backStencilFunc;
    StencilOp backStencilFail;
    StencilOp backStencilZFail;
    StencilOp backStencilPass;
    
    DepthStencilStateDescriptor() : depthEnable(true), depthWriteEnable(true),
                                   depthFunc(ComparisonFunc::LessEqual), stencilEnable(false),
                                   stencilReadMask(0xFF), stencilWriteMask(0xFF),
                                   frontStencilFunc(ComparisonFunc::Always),
                                   frontStencilFail(StencilOp::Keep),
                                   frontStencilZFail(StencilOp::Keep),
                                   frontStencilPass(StencilOp::Keep),
                                   backStencilFunc(ComparisonFunc::Always),
                                   backStencilFail(StencilOp::Keep),
                                   backStencilZFail(StencilOp::Keep),
                                   backStencilPass(StencilOp::Keep) {}
};

struct RasterizerStateDescriptor {
    FillMode fillMode;
    CullMode cullMode;
    bool frontCounterClockwise;
    int32_t depthBias;
    float depthBiasClamp;
    float slopeScaledDepthBias;
    bool depthClipEnable;
    bool scissorEnable;
    bool multisampleEnable;
    bool antialiasedLineEnable;
    
    RasterizerStateDescriptor() : fillMode(FillMode::Solid), cullMode(CullMode::CounterClockwise),
                                 frontCounterClockwise(false), depthBias(0), depthBiasClamp(0),
                                 slopeScaledDepthBias(0), depthClipEnable(true),
                                 scissorEnable(false), multisampleEnable(false),
                                 antialiasedLineEnable(false) {}
};

// ============================================================================
// ABSTRACT GRAPHICS DRIVER INTERFACE
// ============================================================================

class IGraphicsDriver {
public:
    virtual ~IGraphicsDriver() = default;

    // ========================================================================
    // INITIALIZATION & CLEANUP
    // ========================================================================
    
    virtual bool Initialize(void* windowHandle, uint32_t width, uint32_t height, bool fullscreen) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;
    virtual BackendType GetBackendType() const = 0;
    virtual const char* GetBackendName() const = 0;
    virtual const char* GetVersionString() const = 0;

    // ========================================================================
    // FRAME MANAGEMENT
    // ========================================================================
    
    virtual bool BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual bool Present() = 0;
    virtual void Clear(float r, float g, float b, float a, bool clearDepth) = 0;
    virtual void SetClearColor(float r, float g, float b, float a) = 0;

    // ========================================================================
    // DRAWING OPERATIONS
    // ========================================================================
    
    virtual void DrawPrimitive(PrimitiveType primType, uint32_t vertexCount) = 0;
    virtual void DrawIndexedPrimitive(PrimitiveType primType, uint32_t indexCount,
                                     IndexBufferHandle ibHandle, uint32_t startIndex) = 0;
    virtual void DrawPrimitiveUP(PrimitiveType primType, uint32_t primCount,
                                const void* vertexData, uint32_t vertexStride) = 0;
    virtual void DrawIndexedPrimitiveUP(PrimitiveType primType, uint32_t minVertexIndex,
                                       uint32_t vertexCount, uint32_t primCount,
                                       const void* indexData, const void* vertexData,
                                       uint32_t vertexStride) = 0;

    // ========================================================================
    // RENDER STATE MANAGEMENT
    // ========================================================================
    
    virtual bool SetRenderState(RenderState state, uint64_t value) = 0;
    virtual uint64_t GetRenderState(RenderState state) const = 0;
    virtual bool SetBlendState(const BlendStateDescriptor& desc) = 0;
    virtual bool SetDepthStencilState(const DepthStencilStateDescriptor& desc) = 0;
    virtual bool SetRasterizerState(const RasterizerStateDescriptor& desc) = 0;
    virtual bool SetScissorRect(const Rect& rect) = 0;
    virtual void SetViewport(const Viewport& vp) = 0;
    virtual Viewport GetViewport() const = 0;

    // ========================================================================
    // VERTEX & INDEX BUFFERS
    // ========================================================================
    
    virtual VertexBufferHandle CreateVertexBuffer(uint32_t sizeInBytes, bool dynamic,
                                                 const void* initialData) = 0;
    virtual void DestroyVertexBuffer(VertexBufferHandle handle) = 0;
    virtual bool LockVertexBuffer(VertexBufferHandle handle, uint32_t offset,
                                 uint32_t size, void** lockedData, bool readOnly) = 0;
    virtual bool UnlockVertexBuffer(VertexBufferHandle handle) = 0;
    virtual uint32_t GetVertexBufferSize(VertexBufferHandle handle) const = 0;

    virtual IndexBufferHandle CreateIndexBuffer(uint32_t sizeInBytes, bool is32Bit,
                                               bool dynamic, const void* initialData) = 0;
    virtual void DestroyIndexBuffer(IndexBufferHandle handle) = 0;
    virtual bool LockIndexBuffer(IndexBufferHandle handle, uint32_t offset,
                                uint32_t size, void** lockedData, bool readOnly) = 0;
    virtual bool UnlockIndexBuffer(IndexBufferHandle handle) = 0;
    virtual uint32_t GetIndexBufferSize(IndexBufferHandle handle) const = 0;

    // ========================================================================
    // VERTEX FORMAT / DECLARATION
    // ========================================================================
    
    virtual VertexFormatHandle CreateVertexFormat(const VertexElement* elements,
                                                 uint32_t elementCount) = 0;
    virtual void DestroyVertexFormat(VertexFormatHandle handle) = 0;
    virtual bool SetVertexFormat(VertexFormatHandle handle) = 0;
    virtual bool SetVertexStreamSource(uint32_t streamIndex, VertexBufferHandle vbHandle,
                                      uint32_t offset, uint32_t stride) = 0;
    
    /**
     * Phase 56: Set the current index buffer for indexed drawing
     * @param ibHandle Index buffer handle (INVALID_HANDLE to unbind)
     * @param startIndex Starting index within the buffer
     * @return true if successful
     */
    virtual bool SetIndexBuffer(IndexBufferHandle ibHandle, uint32_t startIndex) = 0;

    // ========================================================================
    // TEXTURE MANAGEMENT
    // ========================================================================
    
    virtual TextureHandle CreateTexture(const TextureDescriptor& desc, const void* initialData) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;
    virtual bool SetTexture(uint32_t samplerIndex, TextureHandle handle) = 0;
    virtual TextureHandle GetTexture(uint32_t samplerIndex) const = 0;
    virtual bool LockTexture(TextureHandle handle, uint32_t level, void** lockedData,
                            uint32_t* pitch) = 0;
    virtual bool UnlockTexture(TextureHandle handle, uint32_t level) = 0;
    virtual TextureDescriptor GetTextureDescriptor(TextureHandle handle) const = 0;
    virtual bool UpdateTextureSubregion(TextureHandle handle, const void* data,
                                       uint32_t left, uint32_t top, uint32_t right,
                                       uint32_t bottom) = 0;

    // ============================================================================
    // RENDER TARGET & DEPTH STENCIL
    // ============================================================================
    
    virtual RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height,
                                                 TextureFormat format) = 0;
    virtual void DestroyRenderTarget(RenderTargetHandle handle) = 0;
    virtual bool SetRenderTarget(uint32_t targetIndex, RenderTargetHandle handle) = 0;
    virtual RenderTargetHandle GetRenderTarget(uint32_t targetIndex) const = 0;
    virtual bool SetDefaultRenderTarget() = 0;

    virtual DepthStencilHandle CreateDepthStencil(uint32_t width, uint32_t height,
                                                 TextureFormat format) = 0;
    virtual void DestroyDepthStencil(DepthStencilHandle handle) = 0;
    virtual bool SetDepthStencil(DepthStencilHandle handle) = 0;
    virtual DepthStencilHandle GetDepthStencil() const = 0;

    // ========================================================================
    // TRANSFORM MATRICES
    // ========================================================================
    
    virtual void SetWorldMatrix(const Matrix4x4& matrix) = 0;
    virtual void SetViewMatrix(const Matrix4x4& matrix) = 0;
    virtual void SetProjectionMatrix(const Matrix4x4& matrix) = 0;
    virtual Matrix4x4 GetWorldMatrix() const = 0;
    virtual Matrix4x4 GetViewMatrix() const = 0;
    virtual Matrix4x4 GetProjectionMatrix() const = 0;

    // ========================================================================
    // LIGHTING
    // ========================================================================
    
    virtual void SetAmbientLight(float r, float g, float b) = 0;
    virtual void SetLight(uint32_t lightIndex, const Light& light) = 0;
    virtual void DisableLight(uint32_t lightIndex) = 0;
    virtual bool IsLightEnabled(uint32_t lightIndex) const = 0;
    virtual uint32_t GetMaxLights() const = 0;
    virtual void SetMaterial(const Material& material) = 0;

    // ========================================================================
    // SAMPLER & TEXTURE STATES
    // ========================================================================
    
    virtual bool SetSamplerState(uint32_t samplerIndex, uint32_t state, uint32_t value) = 0;
    virtual uint32_t GetSamplerState(uint32_t samplerIndex, uint32_t state) const = 0;
    virtual uint32_t GetMaxTextureSamplers() const = 0;

    // ========================================================================
    // CAPABILITIES & QUERIES
    // ========================================================================
    
    virtual bool SupportsTextureFormat(TextureFormat format) const = 0;
    virtual uint32_t GetMaxTextureWidth() const = 0;
    virtual uint32_t GetMaxTextureHeight() const = 0;
    virtual uint32_t GetMaxVertexBlendMatrices() const = 0;
    virtual uint32_t GetMaxClipPlanes() const = 0;
    virtual bool SupportsHardwareTransformAndLight() const = 0;
    virtual bool SupportsPixelShaders() const = 0;
    virtual bool SupportsVertexShaders() const = 0;
    virtual bool SupportsCompressedTextures() const = 0;
    virtual uint32_t GetMaxPrimitiveCount() const = 0;
    virtual uint32_t GetMaxVertexIndex() const = 0;
    virtual uint32_t GetMaxStreamStride() const = 0;

    // ========================================================================
    // UTILITY
    // ========================================================================
    
    virtual bool ResizeSwapChain(uint32_t width, uint32_t height) = 0;
    virtual void GetDisplaySize(uint32_t& width, uint32_t& height) const = 0;
    virtual bool SetFullscreen(bool fullscreen) = 0;
    virtual bool IsFullscreen() const = 0;
    virtual void Flush() = 0;
    virtual void WaitForGPU() = 0;
    virtual const char* GetLastError() const = 0;
};

} // namespace Graphics

