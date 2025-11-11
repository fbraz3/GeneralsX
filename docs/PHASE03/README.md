# PHASE03: Material System

## Phase Title

Material System - Shaders and Uniforms

## Objective

Implement a material system that handles shader programs, uniform buffers, and material properties. This enables rendering with dynamic lighting, color, and texture parameters.

## Dependencies

- PHASE01 (Geometry Rendering)
- PHASE02 (Texture System)
- Uniform buffer creation and updates

## Key Deliverables

1. Material definition format
2. Shader program management
3. Uniform buffer objects (UBOs) or push constants
4. Material property setters (color, texture indices, etc.)
5. Material binding during render

## Acceptance Criteria

- [ ] Materials load from definition files
- [ ] Uniforms update correctly without stalls
- [ ] Multiple materials can be active simultaneously
- [ ] Shader compilation errors reported clearly
- [ ] No performance degradation from shader switches

## Technical Details

### Components to Implement

1. **Material Definition**
   - Shader program references
   - Texture slot assignments
   - Default uniform values

2. **Shader Manager**
   - Program compilation with error logging
   - Uniform reflection (if needed)
   - Pipeline state caching

3. **Uniform Updates**
   - Per-frame updates (camera, lights)
   - Per-object updates (world matrix, color)

### Code Location

```
Core/GameEngineDevice/Source/Shader/
GeneralsMD/Code/Graphics/Material/
```

### Material Class Design

```cpp
class Material {
public:
    void SetUniformMatrix4f(const std::string& name, const glm::mat4& value);
    void SetUniformVec3(const std::string& name, const glm::vec3& value);
    void SetUniformFloat(const std::string& name, float value);
    void BindTexture(uint32_t slot, Texture* texture);
    void Apply();  // Bind to current pipeline
    
private:
    ShaderProgram* shader;
    std::map<std::string, UniformValue> uniforms;
    std::vector<Texture*> textures;
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Material definition: 0.5 day
- Uniform management: 1 day
- Shader compilation: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Create test materials with various uniforms
# Render objects with different materials
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Materials apply correctly
- No shader compilation warnings
- Uniform values update in-frame without flicker
- Performance acceptable

## Reference Documentation

- Vulkan pipeline layout documentation
- Metal argument buffers
- DirectX constant buffer patterns (for porting from D3D)
