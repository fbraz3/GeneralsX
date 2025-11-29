#version 450

// Phase 60: UI Fragment Shader with vertex color modulation
// Combines texture color with vertex color for UI rendering
// Handles both textured and untextured UI elements

layout(location = 0) in vec4 fragColor;     // Interpolated vertex color
layout(location = 1) in vec2 fragTexCoord;  // Texture coordinates

layout(location = 0) out vec4 outColor;

// Texture sampler (binding 0 in descriptor set)
layout(binding = 0) uniform sampler2D texSampler;

// Push constants for rendering options
// Note: These must match the vertex shader push constants layout
layout(push_constant) uniform PushConstants {
    vec2 screenSize;     // Screen width and height
    int useTexture;      // 0 = vertex color only, 1 = texture * vertex color
    int alphaTest;       // 0 = disabled, 1 = enabled (discard if alpha < threshold)
    float alphaRef;      // Alpha test reference value (0.0 - 1.0)
    float reserved1;     // Padding for alignment
    float reserved2;     // Padding for alignment
    float reserved3;     // Padding for alignment
} pc;

void main() {
    vec4 color;
    
    if (pc.useTexture != 0) {
        // Sample texture and modulate by vertex color
        vec4 texColor = texture(texSampler, fragTexCoord);
        color = texColor * fragColor;
    } else {
        // Use vertex color only
        color = fragColor;
    }
    
    // Alpha test (commonly used for UI transparency)
    if (pc.alphaTest != 0 && color.a < pc.alphaRef) {
        discard;
    }
    
    outColor = color;
}
