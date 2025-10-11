#include <metal_stdlib>
using namespace metal;

// Vertex input structure matching DirectX FVF layout
struct VertexInput {
    float3 position [[attribute(0)]];   // D3DFVF_XYZ
    float3 normal   [[attribute(1)]];   // D3DFVF_NORMAL  
    float4 color    [[attribute(2)]];   // D3DFVF_DIFFUSE (BGRA → RGBA)
    float2 texcoord [[attribute(3)]];   // D3DFVF_TEX1
};

// Vertex output / fragment input
struct VertexOutput {
    float4 position [[position]];
    float4 color;
    float2 texcoord;
    float3 worldPos;
    float3 normal;
};

// Uniform buffer structure matching DX8Wrapper uniforms
struct Uniforms {
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float3 lightDirection;
    float3 lightColor;
    float3 ambientColor;
    float useLighting;
    // Material uniforms
    float4 materialDiffuse;
    float4 materialAmbient;
    // Alpha test uniforms
    float alphaRef;
    float alphaTestEnabled;
    int alphaTestFunc;
    // Fog uniforms
    float3 fogColor;
    float fogStart;
    float fogEnd;
    float fogDensity;
    int fogMode;
    float fogEnabled;
};

// Vertex shader
vertex VertexOutput vertex_main(VertexInput in [[stage_in]],
                               constant Uniforms& uniforms [[buffer(0)]]) {
    VertexOutput out;
    
    // Transform position: Model → World → View → Projection
    float4 worldPos = uniforms.worldMatrix * float4(in.position, 1.0);
    float4 viewPos = uniforms.viewMatrix * worldPos;
    out.position = uniforms.projectionMatrix * viewPos;
    
    // Pass through color and texture coordinates
    out.color = in.color;
    out.texcoord = in.texcoord;
    out.worldPos = worldPos.xyz;
    
    // Transform normal to world space for lighting
    out.normal = normalize((uniforms.worldMatrix * float4(in.normal, 0.0)).xyz);
    
    return out;
}

// Fragment shader
fragment float4 fragment_main(VertexOutput in [[stage_in]],
                             constant Uniforms& uniforms [[buffer(0)]],
                             texture2d<float> diffuseTexture [[texture(0)]],
                             sampler textureSampler [[sampler(0)]]) {
    
    // Sample base texture
    float4 texColor = diffuseTexture.sample(textureSampler, in.texcoord);
    float4 finalColor = texColor * in.color;
    
    // Apply lighting if enabled
    if (uniforms.useLighting > 0.5) {
        // Simple directional lighting
        float3 lightDir = normalize(-uniforms.lightDirection);
        float NdotL = max(dot(in.normal, lightDir), 0.0);
        
        float3 lighting = uniforms.ambientColor + uniforms.lightColor * NdotL;
        finalColor.rgb *= lighting;
        
        // Apply material properties
        finalColor.rgb *= uniforms.materialDiffuse.rgb;
    }
    
    // Alpha testing (if enabled)
    if (uniforms.alphaTestEnabled > 0.5) {
        float alpha = finalColor.a;
        int func = uniforms.alphaTestFunc;
        float ref = uniforms.alphaRef;
        
        bool pass = false;
        if (func == 1) pass = false;                    // NEVER
        else if (func == 2) pass = (alpha < ref);       // LESS
        else if (func == 3) pass = (alpha == ref);      // EQUAL
        else if (func == 4) pass = (alpha <= ref);      // LESSEQUAL
        else if (func == 5) pass = (alpha > ref);       // GREATER
        else if (func == 6) pass = (alpha != ref);      // NOTEQUAL
        else if (func == 7) pass = (alpha >= ref);      // GREATEREQUAL
        else if (func == 8) pass = true;                // ALWAYS
        
        if (!pass) discard_fragment();
    }
    
    // Apply fog (if enabled)
    if (uniforms.fogEnabled > 0.5) {
        float distance = length(in.worldPos);
        float fogFactor = 1.0;
        
        if (uniforms.fogMode == 1) {
            // EXP fog
            fogFactor = exp(-uniforms.fogDensity * distance);
        } else if (uniforms.fogMode == 2) {
            // EXP2 fog
            float d = uniforms.fogDensity * distance;
            fogFactor = exp(-d * d);
        } else if (uniforms.fogMode == 3) {
            // LINEAR fog
            fogFactor = (uniforms.fogEnd - distance) / (uniforms.fogEnd - uniforms.fogStart);
        }
        
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        finalColor.rgb = mix(uniforms.fogColor, finalColor.rgb, fogFactor);
    }
    
    return finalColor;
}