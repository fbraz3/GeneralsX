#version 450

// Phase 54: Basic fragment shader for Vulkan pipeline
// Supports vertex color and optional texture

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    int useTexture;  // 0 = vertex color only, 1 = modulate with texture
} pc;

void main() {
    vec4 color = fragColor;
    
    // Note: push constants have limited size (128 bytes guaranteed)
    // For now, we just output the vertex color
    // Texture support will be added via descriptor sets
    
    outColor = color;
}
