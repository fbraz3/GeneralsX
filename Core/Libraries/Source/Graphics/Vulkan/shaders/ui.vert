#version 450

// Phase 60: UI Vertex Shader for pre-transformed (RHW) vertices
// D3DFVF_XYZRHW vertices are already in screen-space coordinates
// This shader converts screen coords to Vulkan NDC without MVP transform

// Input vertex format (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
// Position is vec4: (x, y, z, rhw) where x,y are screen pixels
layout(location = 0) in vec4 inPosition;  // Screen-space position (x, y, z, rhw)
layout(location = 1) in vec4 inColor;     // Vertex color (RGBA normalized)
layout(location = 2) in vec2 inTexCoord;  // Texture coordinates

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// Push constants for screen dimensions
layout(push_constant) uniform PushConstants {
    vec2 screenSize;     // Screen width and height in pixels
    int useTexture;      // 0 = vertex color only, 1 = texture * vertex color
    int alphaTest;       // 0 = disabled, 1 = enabled (discard logic)
    float alphaRef;      // Alpha test reference value (0.0 - 1.0)
    float reserved1;     // Padding to preserve std140 alignment
    float reserved2;
    float reserved3;
} pc;

void main() {
    // Convert screen coordinates to Normalized Device Coordinates (NDC)
    // Screen space: (0, 0) top-left, (width, height) bottom-right
    // Vulkan NDC: (-1, -1) top-left, (1, 1) bottom-right
    
    // Transform X: 0 to width -> -1 to 1
    float ndcX = (inPosition.x / pc.screenSize.x) * 2.0 - 1.0;
    
    // Transform Y: 0 to height -> -1 to 1 (Vulkan Y is inverted)
    float ndcY = (inPosition.y / pc.screenSize.y) * 2.0 - 1.0;
    
    // Use Z directly (usually 0 for UI)
    // RHW is the reciprocal of homogeneous W, but in Vulkan we use W=1.0
    gl_Position = vec4(ndcX, ndcY, inPosition.z, 1.0);
    
    // Pass through vertex color and texture coords
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
