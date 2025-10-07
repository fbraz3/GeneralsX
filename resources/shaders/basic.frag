#version 330 core

// Phase 27.2.4: Basic fragment shader for DirectX8 → OpenGL translation
// Inputs from vertex shader
in vec3 vNormal;
in vec4 vColor;
in vec2 vTexCoord0;
in vec3 vWorldPos;

// Uniforms for texturing
uniform sampler2D uTexture0;  // D3D texture stage 0
uniform bool uUseTexture;     // Enable/disable texturing

// Uniforms for lighting (simple directional light for now)
uniform vec3 uLightDirection;  // Directional light direction (normalized)
uniform vec3 uLightColor;      // Light color
uniform vec3 uAmbientColor;    // Ambient light color
uniform bool uUseLighting;     // Enable/disable lighting

// Output color
out vec4 FragColor;

void main()
{
    // Start with vertex color
    vec4 baseColor = vColor;
    
    // Apply texture if enabled
    if (uUseTexture) {
        vec4 texColor = texture(uTexture0, vTexCoord0);
        baseColor *= texColor;
    }
    
    // Apply lighting if enabled
    if (uUseLighting) {
        // Normalize interpolated normal
        vec3 normal = normalize(vNormal);
        
        // Calculate diffuse lighting (N·L)
        float diff = max(dot(normal, -uLightDirection), 0.0);
        vec3 diffuse = diff * uLightColor;
        
        // Combine ambient + diffuse
        vec3 lighting = uAmbientColor + diffuse;
        
        // Apply lighting to base color
        baseColor.rgb *= lighting;
    }
    
    // Output final color
    FragColor = baseColor;
}
