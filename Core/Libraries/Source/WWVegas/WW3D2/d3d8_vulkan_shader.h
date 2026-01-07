/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
**
**  FILE: d3d8_vulkan_shader.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Shader Compilation and Management
**
**  DESCRIPTION:
**    Provides GLSL to SPIR-V shader compilation, VkShaderModule creation,
**    and shader caching for DirectX 8 compatibility. Enables cross-platform
**    shader support with error reporting and reflection capabilities.
**
**    Phase 14: Shader System Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_SHADER_H_
#define D3D8_VULKAN_SHADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>  // For uint32_t, uint64_t, uint8_t

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkShaderModule;

/* ============================================================================
 * Shader Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_SHADER_STAGE
 * @brief Shader stage type
 */
typedef enum {
    D3D8_VULKAN_SHADER_STAGE_VERTEX = 0x1,             /* Vertex shader */
    D3D8_VULKAN_SHADER_STAGE_FRAGMENT = 0x2,           /* Fragment/pixel shader */
    D3D8_VULKAN_SHADER_STAGE_GEOMETRY = 0x4,           /* Geometry shader */
    D3D8_VULKAN_SHADER_STAGE_COMPUTE = 0x8,            /* Compute shader */
    D3D8_VULKAN_SHADER_STAGE_TESSELLATION_CONTROL = 0x10, /* Tessellation control */
    D3D8_VULKAN_SHADER_STAGE_TESSELLATION_EVALUATION = 0x20, /* Tessellation evaluation */
} D3D8_VULKAN_SHADER_STAGE;

/**
 * @enum D3D8_VULKAN_SHADER_SOURCE_FORMAT
 * @brief Shader source code format
 */
typedef enum {
    D3D8_VULKAN_SHADER_SOURCE_GLSL = 0x1,              /* GLSL text source */
    D3D8_VULKAN_SHADER_SOURCE_GLSL_FILE = 0x2,         /* GLSL file path */
    D3D8_VULKAN_SHADER_SOURCE_SPIRV = 0x3,             /* SPIR-V binary */
    D3D8_VULKAN_SHADER_SOURCE_SPIRV_FILE = 0x4,        /* SPIR-V file path */
} D3D8_VULKAN_SHADER_SOURCE_FORMAT;

/**
 * @enum D3D8_VULKAN_SHADER_OPTIMIZATION
 * @brief Shader compilation optimization level
 */
typedef enum {
    D3D8_VULKAN_SHADER_OPTIMIZATION_NONE = 0,          /* No optimization */
    D3D8_VULKAN_SHADER_OPTIMIZATION_SPEED = 1,         /* Optimize for speed */
    D3D8_VULKAN_SHADER_OPTIMIZATION_SIZE = 2,          /* Optimize for size */
} D3D8_VULKAN_SHADER_OPTIMIZATION;

/* ============================================================================
 * Configuration Structures
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_SHADER_CREATE_INFO
 * @brief Shader creation parameters
 */
typedef struct {
    D3D8_VULKAN_SHADER_STAGE stage;                    /* Shader stage */
    D3D8_VULKAN_SHADER_SOURCE_FORMAT source_format;    /* Source format */
    const void* source_data;                           /* Source code/binary */
    uint64_t source_size;                              /* Source size in bytes */
    const char* entry_point;                           /* Shader entry point */
    const char* name;                                  /* Debug name */
    D3D8_VULKAN_SHADER_OPTIMIZATION optimization;      /* Optimization level */
    const char* target_environment;                    /* glslc target (e.g., "vulkan1.3") */
} D3D8_VULKAN_SHADER_CREATE_INFO;

/**
 * @struct D3D8_VULKAN_SHADER_REFLECTION_DATA
 * @brief Shader reflection information
 */
typedef struct {
    uint32_t input_variable_count;                     /* Number of input variables */
    uint32_t output_variable_count;                    /* Number of output variables */
    uint32_t uniform_variable_count;                   /* Number of uniform variables */
    uint32_t sampler_count;                            /* Number of samplers */
    uint32_t image_count;                              /* Number of images */
    uint64_t spirv_code_size;                          /* SPIR-V binary size */
} D3D8_VULKAN_SHADER_REFLECTION_DATA;

/**
 * @struct D3D8_VULKAN_SHADER_COMPILATION_ERROR
 * @brief Shader compilation error information
 */
typedef struct {
    uint32_t line_number;                              /* Line where error occurred */
    uint32_t column_number;                            /* Column where error occurred */
    const char* message;                               /* Error message */
    uint32_t severity;                                 /* Severity (0=info, 1=warning, 2=error) */
} D3D8_VULKAN_SHADER_COMPILATION_ERROR;

/**
 * @struct D3D8_VULKAN_SHADER_HANDLE
 * @brief Opaque shader module handle
 */
typedef struct {
    VkShaderModule module;                             /* Vulkan shader module */
    uint32_t shader_id;                                /* Unique ID */
    D3D8_VULKAN_SHADER_STAGE stage;                    /* Shader stage */
    uint64_t spirv_size;                               /* SPIR-V size */
    uint8_t* spirv_code;                               /* SPIR-V binary (optional cache) */
} D3D8_VULKAN_SHADER_HANDLE;

/* ============================================================================
 * Shader Compilation API
 * ============================================================================ */

/**
 * @brief Compile shader from source to VkShaderModule
 * @param device Vulkan device
 * @param create_info Shader creation parameters
 * @param out_handle Pointer to receive shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CompileShader(VkDevice device, const D3D8_VULKAN_SHADER_CREATE_INFO* create_info,
                              D3D8_VULKAN_SHADER_HANDLE* out_handle);

/**
 * @brief Compile shader from file
 * @param device Vulkan device
 * @param filepath Path to shader source file
 * @param stage Shader stage
 * @param entry_point Shader entry point name
 * @param out_handle Pointer to receive shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CompileShaderFromFile(VkDevice device, const char* filepath,
                                      D3D8_VULKAN_SHADER_STAGE stage, const char* entry_point,
                                      D3D8_VULKAN_SHADER_HANDLE* out_handle);

/**
 * @brief Load pre-compiled SPIR-V shader
 * @param device Vulkan device
 * @param spirv_data SPIR-V binary code
 * @param spirv_size Size in bytes
 * @param stage Shader stage
 * @param name Debug name
 * @param out_handle Pointer to receive shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_LoadShaderSPIRV(VkDevice device, const uint32_t* spirv_data, uint64_t spirv_size,
                                D3D8_VULKAN_SHADER_STAGE stage, const char* name,
                                D3D8_VULKAN_SHADER_HANDLE* out_handle);

/**
 * @brief Load SPIR-V shader from file
 * @param device Vulkan device
 * @param filepath Path to .spv file
 * @param stage Shader stage
 * @param out_handle Pointer to receive shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_LoadShaderSPIRVFile(VkDevice device, const char* filepath,
                                    D3D8_VULKAN_SHADER_STAGE stage,
                                    D3D8_VULKAN_SHADER_HANDLE* out_handle);

/**
 * @brief Destroy shader module
 * @param device Vulkan device
 * @param handle Shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_DestroyShader(VkDevice device, D3D8_VULKAN_SHADER_HANDLE* handle);

/**
 * @brief Get shader reflection data
 * @param handle Shader handle
 * @param out_reflection Pointer to receive reflection data
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_GetShaderReflection(D3D8_VULKAN_SHADER_HANDLE* handle,
                                    D3D8_VULKAN_SHADER_REFLECTION_DATA* out_reflection);

/**
 * @brief Cache compiled shader to file
 * @param handle Shader handle
 * @param filepath Path to save cache file
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CacheShader(D3D8_VULKAN_SHADER_HANDLE* handle, const char* filepath);

/**
 * @brief Load shader from cache file
 * @param device Vulkan device
 * @param filepath Path to cache file
 * @param out_handle Pointer to receive shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_LoadShaderFromCache(VkDevice device, const char* filepath,
                                    D3D8_VULKAN_SHADER_HANDLE* out_handle);

/* ============================================================================
 * Shader Pipeline API
 * ============================================================================ */

/**
 * @brief Create shader pipeline from vertex and fragment shaders
 * @param device Vulkan device
 * @param vertex_shader Compiled vertex shader handle
 * @param fragment_shader Compiled fragment shader handle
 * @param out_pipeline_id Pointer to receive pipeline ID
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateShaderPipeline(VkDevice device, D3D8_VULKAN_SHADER_HANDLE* vertex_shader,
                                     D3D8_VULKAN_SHADER_HANDLE* fragment_shader,
                                     uint32_t* out_pipeline_id);

/**
 * @brief Bind shader module for use
 * @param device Vulkan device
 * @param handle Shader handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_BindShader(VkDevice device, D3D8_VULKAN_SHADER_HANDLE* handle);

/**
 * @brief Get SPIR-V code from compiled shader
 * @param handle Shader handle
 * @param out_code Pointer to receive SPIR-V code pointer
 * @param out_size Pointer to receive SPIR-V code size
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_GetShaderSPIRVCode(D3D8_VULKAN_SHADER_HANDLE* handle,
                                   const uint32_t** out_code, uint64_t* out_size);

/**
 * @brief Clear shader cache (memory cleanup)
 * @return 0 on success
 */
int D3D8_Vulkan_ClearShaderCache(void);

/**
 * @brief Get shader compilation error message
 * @param error_buffer Buffer to receive error message
 * @param buffer_size Size of error buffer
 * @return Length of error message, or 0 if no error
 */
uint32_t D3D8_Vulkan_GetShaderCompilationError(char* error_buffer, uint32_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_SHADER_H_ */
