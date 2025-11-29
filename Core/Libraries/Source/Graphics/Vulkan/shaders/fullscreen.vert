#version 450

// Phase 54: Fullscreen triangle vertex shader
// Generates a fullscreen triangle without vertex input
// Uses gl_VertexIndex to compute positions (0, 1, 2)

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // Generate fullscreen triangle positions from vertex index
    // Vertex 0: (-1, -1)  -> texcoord (0, 0)
    // Vertex 1: ( 3, -1)  -> texcoord (2, 0)
    // Vertex 2: (-1,  3)  -> texcoord (0, 2)
    // This creates a triangle that covers the entire screen
    
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragTexCoord = positions[gl_VertexIndex] * 0.5 + 0.5;
}
