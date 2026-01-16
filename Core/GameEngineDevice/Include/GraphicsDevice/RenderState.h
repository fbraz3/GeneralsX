/*
 * Phase 09: RenderState - Common render state structures and utilities
 * 
 * Defines shared structures used across all graphics backends for render state management.
 */

#ifndef RENDERSTATE_H
#define RENDERSTATE_H

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Blend factor enumeration (DirectX compatible)
 */
enum BlendFactor {
    BLEND_ZERO = 0,
    BLEND_ONE = 1,
    BLEND_SRC_COLOR = 2,
    BLEND_INV_SRC_COLOR = 3,
    BLEND_SRC_ALPHA = 4,
    BLEND_INV_SRC_ALPHA = 5,
    BLEND_DST_COLOR = 6,
    BLEND_INV_DST_COLOR = 7,
    BLEND_DST_ALPHA = 8,
    BLEND_INV_DST_ALPHA = 9,
};

/**
 * @brief Blend operation enumeration
 */
enum BlendOp {
    BLENDOP_ADD = 0,
    BLENDOP_SUBTRACT = 1,
    BLENDOP_REV_SUBTRACT = 2,
    BLENDOP_MIN = 3,
    BLENDOP_MAX = 4,
};

/**
 * @brief Depth test function enumeration
 */
enum DepthFunc {
    DEPTH_ALWAYS = 0,
    DEPTH_NEVER = 1,
    DEPTH_LESS = 2,
    DEPTH_EQUAL = 3,
    DEPTH_LEQUAL = 4,
    DEPTH_GREATER = 5,
    DEPTH_NOTEQUAL = 6,
    DEPTH_GEQUAL = 7,
};

/**
 * @brief Stencil operation enumeration
 */
enum StencilOp {
    STENCIL_KEEP = 0,
    STENCIL_ZERO = 1,
    STENCIL_REPLACE = 2,
    STENCIL_INCR = 3,
    STENCIL_DECR = 4,
    STENCIL_INVERT = 5,
    STENCIL_INCR_WRAP = 6,
    STENCIL_DECR_WRAP = 7,
};

/**
 * @brief Cull mode enumeration
 */
enum CullMode {
    CULL_NONE = 0,
    CULL_FRONT = 1,
    CULL_BACK = 2,
};

/**
 * @brief Fill mode enumeration
 */
enum FillMode {
    FILL_SOLID = 0,
    FILL_WIREFRAME = 1,
    FILL_POINT = 2,
};

/**
 * @brief Blend state configuration
 */
struct BlendState {
    bool enabled;               // Blending enabled?
    BlendFactor srcFactor;      // Source blend factor
    BlendFactor dstFactor;      // Destination blend factor
    BlendOp operation;          // Blend operation
    BlendFactor srcAlphaFactor; // Source alpha blend factor
    BlendFactor dstAlphaFactor; // Destination alpha blend factor
    BlendOp alphaOperation;     // Alpha blend operation
    unsigned int writeMask;     // Color write mask (RGBA)

    // Default blend state
    BlendState() :
        enabled(false),
        srcFactor(BLEND_SRC_ALPHA),
        dstFactor(BLEND_INV_SRC_ALPHA),
        operation(BLENDOP_ADD),
        srcAlphaFactor(BLEND_ONE),
        dstAlphaFactor(BLEND_ZERO),
        alphaOperation(BLENDOP_ADD),
        writeMask(0xF)  // RGBA
    {}
};

/**
 * @brief Depth/Stencil state configuration
 */
struct DepthStencilState {
    bool depthEnabled;          // Depth testing enabled?
    bool depthWriteEnabled;     // Depth writing enabled?
    DepthFunc depthFunc;        // Depth comparison function

    bool stencilEnabled;        // Stencil testing enabled?
    unsigned int stencilRef;    // Stencil reference value
    unsigned int stencilMask;   // Stencil mask
    StencilOp stencilFailOp;    // Stencil fail operation
    StencilOp depthFailOp;      // Depth fail operation
    StencilOp passOp;           // Stencil/depth pass operation

    // Default depth/stencil state
    DepthStencilState() :
        depthEnabled(true),
        depthWriteEnabled(true),
        depthFunc(DEPTH_LESS),
        stencilEnabled(false),
        stencilRef(0),
        stencilMask(0xFF),
        stencilFailOp(STENCIL_KEEP),
        depthFailOp(STENCIL_KEEP),
        passOp(STENCIL_KEEP)
    {}
};

/**
 * @brief Rasterizer state configuration
 */
struct RasterizerState {
    CullMode cullMode;          // Back-face culling mode
    FillMode fillMode;          // Fill mode (solid, wireframe)
    bool depthBiasEnabled;      // Polygon offset enabled?
    float depthBias;            // Polygon offset constant bias
    float depthBiasClamp;       // Max polygon offset change
    float depthSlope;           // Polygon offset slope factor
    bool scissorEnabled;        // Scissor test enabled?
    bool multisampleEnabled;    // Multisample anti-aliasing?
    bool antialiasedLineEnabled; // Line antialiasing?

    // Default rasterizer state
    RasterizerState() :
        cullMode(CULL_BACK),
        fillMode(FILL_SOLID),
        depthBiasEnabled(false),
        depthBias(0.0f),
        depthBiasClamp(0.0f),
        depthSlope(0.0f),
        scissorEnabled(false),
        multisampleEnabled(true),
        antialiasedLineEnabled(true)
    {}
};

/**
 * @brief Sampler state configuration
 */
struct SamplerState {
    enum TextureAddressMode {
        ADDRESS_WRAP = 0,       // Tile/repeat addressing
        ADDRESS_MIRROR = 1,     // Mirror addressing
        ADDRESS_CLAMP = 2,      // Clamp to edge
        ADDRESS_BORDER = 3,     // Border color
    };

    enum TextureFilterMode {
        FILTER_POINT = 0,       // Point sampling
        FILTER_LINEAR = 1,      // Bilinear filtering
        FILTER_ANISOTROPIC = 2, // Anisotropic filtering
    };

    TextureAddressMode addressModeU;
    TextureAddressMode addressModeV;
    TextureAddressMode addressModeW;
    TextureFilterMode minFilter;
    TextureFilterMode magFilter;
    TextureFilterMode mipFilter;
    float maxAnisotropy;
    unsigned int borderColor;   // RGBA32 border color
    float minLod;
    float maxLod;
    float lodBias;

    // Default sampler state
    SamplerState() :
        addressModeU(ADDRESS_WRAP),
        addressModeV(ADDRESS_WRAP),
        addressModeW(ADDRESS_WRAP),
        minFilter(FILTER_LINEAR),
        magFilter(FILTER_LINEAR),
        mipFilter(FILTER_LINEAR),
        maxAnisotropy(1.0f),
        borderColor(0xFF000000),
        minLod(0.0f),
        maxLod(1000.0f),
        lodBias(0.0f)
    {}
};

} // namespace Graphics
} // namespace GeneralsX

#endif // RENDERSTATE_H
