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

// Phase 27.4.8: Point sprite uniforms
uniform bool uPointSpriteEnabled;       // D3DRS_POINTSPRITEENABLE
uniform float uPointSize;               // D3DRS_POINTSIZE
uniform bool uPointScaleEnabled;        // D3DRS_POINTSCALEENABLE
uniform float uPointScaleA;             // D3DRS_POINTSCALE_A (constant)
uniform float uPointScaleB;             // D3DRS_POINTSCALE_B (linear)
uniform float uPointScaleC;             // D3DRS_POINTSCALE_C (quadratic)
uniform float uPointSizeMin;            // D3DRS_POINTSIZE_MIN
uniform float uPointSizeMax;            // D3DRS_POINTSIZE_MAX

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
    
    // Phase 27.4.8: Point sprite size calculation
    // D3D point sprite formula: size = sqrt(1/(A + B*dist + C*dist²))
    // gl_PointSize controls point sprite rendering size
    if (uPointSpriteEnabled) {
        float pointSize = uPointSize;
        
        if (uPointScaleEnabled) {
            // Calculate distance from camera to point in view space
            float dist = length(viewPos.xyz);
            
            // D3D attenuation formula: screenSize = viewportHeight * size * sqrt(1/(A + B*d + C*d²))
            // For simplicity, we calculate: size / sqrt(A + B*d + C*d²)
            float attenuation = uPointScaleA + uPointScaleB * dist + uPointScaleC * dist * dist;
            if (attenuation > 0.0) {
                pointSize = pointSize / sqrt(attenuation);
            }
        }
        
        // Clamp to min/max range
        pointSize = clamp(pointSize, uPointSizeMin, uPointSizeMax);
        gl_PointSize = pointSize;
    } else {
        gl_PointSize = 1.0;  // Default point size
    }
}
