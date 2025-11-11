# PHASE 03: Shader Compilation System (GLSL → SPIR-V)

**Date**: November 11, 2025  
**Phase Number**: 03  
**Objective**: Create shader compilation pipeline converting GLSL source to SPIR-V bytecode for Vulkan  
**Dependencies**: PHASE01-02 (Vulkan device ready)  
**Estimated Scope**: MEDIUM  
**Status**: not-started  

---

## Overview

This phase implements the complete shader compilation system:
1. GLSL source code loading and parsing
2. GLSL compilation to SPIR-V bytecode using glslang
3. VkShaderModule creation from SPIR-V
4. Shader caching for performance
5. Error handling and debug output

---

## Key Deliverables

- ✅ GLSL → SPIR-V compilation working
- ✅ VkShaderModule objects created successfully
- ✅ Vertex and fragment shaders compiling
- ✅ Shader caching implemented
- ✅ Debug info available in error messages
- ✅ Unit tests for shader compilation

---

## Technical Approach

### Shader Compilation Pipeline

```cpp
// GLSL source → SPIR-V bytecode
class ShaderCompiler {
public:
    bool CompileGLSLToSPIRV(
        const std::string& source,
        VkShaderStageFlagBits stage,
        std::vector<uint32_t>& spirvBytecode
    ) {
        // Use glslang to compile GLSL → SPIR-V
        glslang::TShader shader(stage);
        shader.setStrings(&source.c_str(), 1);
        
        if (!shader.parse(...)) {
            printf("VULKAN: Shader compilation failed\n");
            return false;
        }
        
        glslang::TProgram program;
        program.addShader(&shader);
        
        if (!program.link(...)) {
            printf("VULKAN: Program linking failed\n");
            return false;
        }
        
        glslang::GlslangToSpv(program.getIntermediate(stage), spirvBytecode);
        return true;
    }
};
```

### VkShaderModule Creation

```cpp
VkShaderModuleCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = spirvBytecode.size() * sizeof(uint32_t),
    .pCode = spirvBytecode.data()
};

VkShaderModule shaderModule;
vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
```

### Shader Caching

```cpp
class ShaderCache {
    std::unordered_map<std::string, VkShaderModule> cache;
    
    VkShaderModule GetOrCompile(const std::string& filename) {
        auto it = cache.find(filename);
        if (it != cache.end()) {
            printf("VULKAN: Shader cached: %s\n", filename.c_str());
            return it->second;
        }
        
        std::string source = LoadFileAsString(filename);
        std::vector<uint32_t> spirv;
        compiler.CompileGLSLToSPIRV(source, VK_SHADER_STAGE_FRAGMENT_BIT, spirv);
        
        VkShaderModule module = CreateShaderModule(spirv);
        cache[filename] = module;
        return module;
    }
};
```

---

## Acceptance Criteria (Checklist)

- [ ] glslang library linked and working
- [ ] GLSL vertex shader compiles to SPIR-V
- [ ] GLSL fragment shader compiles to SPIR-V
- [ ] Geometry shaders compile (if used)
- [ ] Compute shaders compile (if used)
- [ ] VkShaderModule created from SPIR-V
- [ ] Shader caching reduces recompilation
- [ ] Error messages include line numbers and failure details
- [ ] No SPIR-V validation errors
- [ ] Can clean up with vkDestroyShaderModule

---

## Implementation Files

**Files to Create**:
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_shader_compiler.h` - Shader compiler interface
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_shader_compiler.cpp` - glslang integration
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_shader_cache.h` - Caching system
- `Core/Libraries/Source/WWVegas/WW3D2/shader_sources/` - Directory for .glsl files
  - `shader_sources/basic.vert` - Basic vertex shader
  - `shader_sources/basic.frag` - Basic fragment shader
- `tests/vulkan_shader_test.cpp` - Unit tests

**Files to Modify**:
- `CMakeLists.txt` - Add glslang library linkage
- `cmake/vulkan.cmake` - Add shader compilation support

---

## Testing Strategy

### Unit Test: GLSL → SPIR-V Compilation

```cpp
void test_shader_compilation() {
    ShaderCompiler compiler;
    std::vector<uint32_t> spirv;
    
    std::string glslSource = R"glsl(
        #version 450
        layout(location = 0) out vec4 outColor;
        
        void main() {
            outColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )glsl";
    
    ASSERT_TRUE(compiler.CompileGLSLToSPIRV(glslSource, VK_SHADER_STAGE_FRAGMENT_BIT, spirv));
    ASSERT_GT(spirv.size(), 0);
}

void test_shader_module_creation() {
    // Create VkShaderModule from compiled SPIR-V
    VkShaderModule module = CreateShaderModuleFromSPIRV(device, spirvBytecode);
    ASSERT_TRUE(module != VK_NULL_HANDLE);
}

void test_shader_caching() {
    ShaderCache cache;
    
    VkShaderModule mod1 = cache.GetOrCompile("basic.frag");
    VkShaderModule mod2 = cache.GetOrCompile("basic.frag"); // Should be cached
    
    ASSERT_EQ(mod1, mod2);
}
```

### Debug Output Example

```
VULKAN: Shader Compiler initialized
VULKAN: Compiling shader: basic.frag
VULKAN: GLSL → SPIR-V compilation successful
VULKAN: SPIR-V bytecode size: 2048 bytes
VULKAN: VkShaderModule created: 0x1a2b3c4d
VULKAN: Shader caching enabled
VULKAN: Phase 03 COMPLETE
```

---

## Critical Notes from Lessons Learned

- ✅ **Error messages**: Include source line numbers in compilation errors (helps debugging)
- ✅ **SPIR-V validation**: Always validate SPIR-V output before creating modules
- ✅ **Memory management**: SPIR-V vectors must persist until VkShaderModule created
- ✅ **Cross-platform**: glslang compiles same GLSL on all platforms

---

## Reference Documentation

- **Vulkan Spec**: `docs/Vulkan/getting_started.html` (Shaders section)
- **glslang**: https://github.com/KhronosGroup/glslang (reference compiler)
- **SPIR-V**: Vulkan specification (SPIR-V bytecode format)

---

## Next Phase

PHASE04: Vertex & Index Buffers (use shaders for rendering)

---

**Last Updated**: November 11, 2025  
**Status**: Ready for implementation
