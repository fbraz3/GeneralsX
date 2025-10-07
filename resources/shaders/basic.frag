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

// Phase 27.4.5: Fog uniforms
uniform bool uFogEnabled;        // Enable/disable fog
uniform vec3 uFogColor;          // Fog color (RGB)
uniform float uFogStart;         // Fog start distance (linear mode)
uniform float uFogEnd;           // Fog end distance (linear mode)
uniform float uFogDensity;       // Fog density (exp/exp2 modes)
uniform int uFogMode;            // Fog mode: 0=none, 1=exp, 2=exp2, 3=linear

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
    
    // Phase 27.4.5: Fog calculation
    if (uFogEnabled) {
        // Calculate distance from camera (using world position)
        // For now, use simple Z-distance (can be enhanced with camera position uniform)
        float fogDistance = length(vWorldPos);
        float fogFactor = 1.0;
        
        // D3DFOG modes:
        // 0 = D3DFOG_NONE (no fog)
        // 1 = D3DFOG_EXP (exponential fog)
        // 2 = D3DFOG_EXP2 (exponential squared fog)
        // 3 = D3DFOG_LINEAR (linear fog)
        
        if (uFogMode == 1) {
            // Exponential fog: factor = exp(-density * distance)
            fogFactor = exp(-uFogDensity * fogDistance);
        }
        else if (uFogMode == 2) {
            // Exponential squared fog: factor = exp(-(density * distance)^2)
            float fogAmount = uFogDensity * fogDistance;
            fogFactor = exp(-fogAmount * fogAmount);
        }
        else if (uFogMode == 3) {
            // Linear fog: factor = (end - distance) / (end - start)
            fogFactor = (uFogEnd - fogDistance) / (uFogEnd - uFogStart);
        }
        
        // Clamp fog factor to [0, 1]
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        
        // Mix base color with fog color
        // fogFactor = 1.0 means no fog (full object color)
        // fogFactor = 0.0 means full fog (full fog color)
        baseColor.rgb = mix(uFogColor, baseColor.rgb, fogFactor);
    }
    
    // Output final color
    FragColor = baseColor;
}
