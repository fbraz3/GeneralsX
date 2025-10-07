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

// Phase 27.4.4: Alpha testing uniforms
uniform bool uAlphaTestEnabled;  // Enable/disable alpha testing
uniform int uAlphaTestFunc;      // Alpha comparison function (D3DCMP_*)
uniform float uAlphaRef;         // Alpha reference value (0.0 - 1.0)

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
    
    // Phase 27.4.4: Alpha testing
    if (uAlphaTestEnabled) {
        float alpha = baseColor.a;
        bool alphaTestPass = false;
        
        // D3DCMP comparison functions:
        // 1 = D3DCMP_NEVER (always fail)
        // 2 = D3DCMP_LESS (alpha < ref)
        // 3 = D3DCMP_EQUAL (alpha == ref)
        // 4 = D3DCMP_LESSEQUAL (alpha <= ref)
        // 5 = D3DCMP_GREATER (alpha > ref)
        // 6 = D3DCMP_NOTEQUAL (alpha != ref)
        // 7 = D3DCMP_GREATEREQUAL (alpha >= ref)
        // 8 = D3DCMP_ALWAYS (always pass)
        
        switch (uAlphaTestFunc) {
            case 1: alphaTestPass = false; break;  // NEVER
            case 2: alphaTestPass = (alpha < uAlphaRef); break;  // LESS
            case 3: alphaTestPass = (abs(alpha - uAlphaRef) < 0.001); break;  // EQUAL (with epsilon)
            case 4: alphaTestPass = (alpha <= uAlphaRef); break;  // LESSEQUAL
            case 5: alphaTestPass = (alpha > uAlphaRef); break;  // GREATER
            case 6: alphaTestPass = (abs(alpha - uAlphaRef) >= 0.001); break;  // NOTEQUAL
            case 7: alphaTestPass = (alpha >= uAlphaRef); break;  // GREATEREQUAL
            case 8: alphaTestPass = true; break;  // ALWAYS
            default: alphaTestPass = true; break;  // Unknown func, default to pass
        }
        
        // Discard fragment if alpha test fails
        if (!alphaTestPass) {
            discard;
        }
    }
    
    // Output final color
    FragColor = baseColor;
}
