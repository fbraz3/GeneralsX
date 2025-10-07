#version 330 core

// Phase 27.2.4: Basic vertex shader for DirectX8 → OpenGL translation
// Attributes match D3D FVF (Flexible Vertex Format) layout
layout(location = 0) in vec3 aPosition;   // D3DFVF_XYZ
layout(location = 1) in vec3 aNormal;     // D3DFVF_NORMAL
layout(location = 2) in vec4 aColor;      // D3DFVF_DIFFUSE
layout(location = 3) in vec2 aTexCoord0;  // D3DFVF_TEX1

// Uniforms for transformation matrices (DirectX-style)
uniform mat4 uWorldMatrix;      // D3DTS_WORLD
uniform mat4 uViewMatrix;       // D3DTS_VIEW
uniform mat4 uProjectionMatrix; // D3DTS_PROJECTION

// Outputs to fragment shader
out vec3 vNormal;
out vec4 vColor;
out vec2 vTexCoord0;
out vec3 vWorldPos;

void main()
{
    // Transform position: MVP matrix multiplication
    vec4 worldPos = uWorldMatrix * vec4(aPosition, 1.0);
    vec4 viewPos = uViewMatrix * worldPos;
    gl_Position = uProjectionMatrix * viewPos;
    
    // Transform normal to world space (assuming uniform scaling)
    // For non-uniform scaling, use transpose(inverse(mat3(uWorldMatrix)))
    vNormal = mat3(uWorldMatrix) * aNormal;
    
    // Pass through vertex color and texture coordinates
    vColor = aColor;
    vTexCoord0 = aTexCoord0;
    vWorldPos = worldPos.xyz;
}
