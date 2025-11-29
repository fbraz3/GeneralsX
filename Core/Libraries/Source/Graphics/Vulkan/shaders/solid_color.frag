#version 450

// Phase 54: Solid color fragment shader
// Used for Clear() operations and debug visualization

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    vec4 color;
} pc;

void main() {
    outColor = pc.color;
}
