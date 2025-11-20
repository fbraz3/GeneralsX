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
**  FILE: d3d8_vulkan_shader.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Shader Implementation
**
**  DESCRIPTION:
**    Stub implementation of Vulkan shader compilation, caching, and
**    management. Provides validation layer and architectural foundation
**    for GLSL to SPIR-V compilation pipeline.
**
**    Phase 14: Shader System Implementation
**
******************************************************************************/

#include "d3d8_vulkan_shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

/* ============================================================================
 * Internal State Management
 * ============================================================================ */

#define MAX_SHADERS 512
#define MAX_SHADER_PIPELINES 64
#define MAX_COMPILATION_ERRORS 128
#define MAX_ERROR_MESSAGE 512

/**
 * @struct ShaderEntry
 * @brief Internal shader tracking
 */
typedef struct {
    void* module;
    uint32_t shader_id;
    int32_t stage;
    uint64_t spirv_size;
    uint8_t* spirv_code;
    int is_allocated;
} ShaderEntry;

/**
 * @struct ShaderPipelineEntry
 * @brief Internal pipeline tracking
 */
typedef struct {
    uint32_t pipeline_id;
    uint32_t vertex_shader_id;
    uint32_t fragment_shader_id;
    int is_allocated;
} ShaderPipelineEntry;

/* Global caches */
static ShaderEntry g_shader_cache[MAX_SHADERS];
static uint32_t g_shader_counter = 7000;

static ShaderPipelineEntry g_pipeline_cache[MAX_SHADER_PIPELINES];
static uint32_t g_pipeline_counter = 8000;

static char g_last_error[MAX_ERROR_MESSAGE] = {0};

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

static int _find_free_shader_slot(void) {
    for (int i = 0; i < MAX_SHADERS; i++) {
        if (!g_shader_cache[i].is_allocated) return i;
    }
    return -1;
}

static int _find_free_pipeline_slot(void) {
    for (int i = 0; i < MAX_SHADER_PIPELINES; i++) {
        if (!g_pipeline_cache[i].is_allocated) return i;
    }
    return -1;
}

static void _set_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_last_error, MAX_ERROR_MESSAGE - 1, format, args);
    va_end(args);
}

static const char* _stage_to_string(D3D8_VULKAN_SHADER_STAGE stage) {
    switch (stage) {
        case D3D8_VULKAN_SHADER_STAGE_VERTEX: return "vertex";
        case D3D8_VULKAN_SHADER_STAGE_FRAGMENT: return "fragment";
        case D3D8_VULKAN_SHADER_STAGE_GEOMETRY: return "geometry";
        case D3D8_VULKAN_SHADER_STAGE_COMPUTE: return "compute";
        case D3D8_VULKAN_SHADER_STAGE_TESSELLATION_CONTROL: return "tessellation_control";
        case D3D8_VULKAN_SHADER_STAGE_TESSELLATION_EVALUATION: return "tessellation_evaluation";
        default: return "unknown";
    }
}

/* ============================================================================
 * Shader Compilation API
 * ============================================================================ */

int D3D8_Vulkan_CompileShader(VkDevice device, const D3D8_VULKAN_SHADER_CREATE_INFO* create_info,
                              D3D8_VULKAN_SHADER_HANDLE* out_handle) {
    printf("[Phase 14] D3D8_Vulkan_CompileShader called: stage=%s source_format=%d\n",
           _stage_to_string(create_info->stage), create_info->source_format);
    
    if (device == NULL || create_info == NULL || out_handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        _set_error("Invalid parameters");
        return -1;
    }
    
    if (create_info->source_data == NULL || create_info->source_size == 0) {
        printf("[Phase 14] ERROR: No source data\n");
        _set_error("No source data provided");
        return -1;
    }
    
    int slot = _find_free_shader_slot();
    if (slot == -1) {
        printf("[Phase 14] ERROR: Shader cache full\n");
        _set_error("Shader cache full");
        return -1;
    }
    
    uint32_t shader_id = g_shader_counter++;
    
    g_shader_cache[slot].is_allocated = 1;
    g_shader_cache[slot].shader_id = shader_id;
    g_shader_cache[slot].stage = (int32_t)create_info->stage;
    g_shader_cache[slot].module = (void*)(uintptr_t)(11000000 + shader_id);
    g_shader_cache[slot].spirv_size = create_info->source_size;
    g_shader_cache[slot].spirv_code = NULL;
    
    out_handle->module = (VkShaderModule)g_shader_cache[slot].module;
    out_handle->shader_id = shader_id;
    out_handle->stage = create_info->stage;
    out_handle->spirv_size = create_info->source_size;
    out_handle->spirv_code = NULL;
    
    printf("[Phase 14] Shader compiled: id=%u stage=%s size=%llu\n",
           shader_id, _stage_to_string(create_info->stage), create_info->source_size);
    
    return 0;
}

int D3D8_Vulkan_CompileShaderFromFile(VkDevice device, const char* filepath,
                                      D3D8_VULKAN_SHADER_STAGE stage, const char* entry_point,
                                      D3D8_VULKAN_SHADER_HANDLE* out_handle) {
    printf("[Phase 14] D3D8_Vulkan_CompileShaderFromFile called: file=%s stage=%s\n",
           filepath, _stage_to_string(stage));
    
    if (device == NULL || filepath == NULL || out_handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        _set_error("Invalid parameters");
        return -1;
    }
    
    if (strlen(filepath) == 0) {
        printf("[Phase 14] ERROR: Empty filepath\n");
        _set_error("Empty filepath");
        return -1;
    }
    
    int slot = _find_free_shader_slot();
    if (slot == -1) {
        printf("[Phase 14] ERROR: Shader cache full\n");
        _set_error("Shader cache full");
        return -1;
    }
    
    uint32_t shader_id = g_shader_counter++;
    
    g_shader_cache[slot].is_allocated = 1;
    g_shader_cache[slot].shader_id = shader_id;
    g_shader_cache[slot].stage = (int32_t)stage;
    g_shader_cache[slot].module = (void*)(uintptr_t)(11000000 + shader_id);
    g_shader_cache[slot].spirv_size = 0;
    g_shader_cache[slot].spirv_code = NULL;
    
    out_handle->module = (VkShaderModule)g_shader_cache[slot].module;
    out_handle->shader_id = shader_id;
    out_handle->stage = stage;
    out_handle->spirv_size = 0;
    out_handle->spirv_code = NULL;
    
    printf("[Phase 14] Shader loaded from file: id=%u file=%s\n", shader_id, filepath);
    return 0;
}

int D3D8_Vulkan_LoadShaderSPIRV(VkDevice device, const uint32_t* spirv_data, uint64_t spirv_size,
                                D3D8_VULKAN_SHADER_STAGE stage, const char* name,
                                D3D8_VULKAN_SHADER_HANDLE* out_handle) {
    printf("[Phase 14] D3D8_Vulkan_LoadShaderSPIRV called: stage=%s size=%llu\n",
           _stage_to_string(stage), spirv_size);
    
    if (device == NULL || spirv_data == NULL || out_handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        _set_error("Invalid parameters");
        return -1;
    }
    
    if (spirv_size == 0 || spirv_size % 4 != 0) {
        printf("[Phase 14] ERROR: Invalid SPIR-V size: %llu\n", spirv_size);
        _set_error("Invalid SPIR-V size");
        return -1;
    }
    
    int slot = _find_free_shader_slot();
    if (slot == -1) {
        printf("[Phase 14] ERROR: Shader cache full\n");
        _set_error("Shader cache full");
        return -1;
    }
    
    uint32_t shader_id = g_shader_counter++;
    
    g_shader_cache[slot].is_allocated = 1;
    g_shader_cache[slot].shader_id = shader_id;
    g_shader_cache[slot].stage = (int32_t)stage;
    g_shader_cache[slot].module = (void*)(uintptr_t)(11000000 + shader_id);
    g_shader_cache[slot].spirv_size = spirv_size;
    g_shader_cache[slot].spirv_code = (uint8_t*)malloc(spirv_size);
    if (g_shader_cache[slot].spirv_code) {
        memcpy(g_shader_cache[slot].spirv_code, spirv_data, spirv_size);
    }
    
    out_handle->module = (VkShaderModule)g_shader_cache[slot].module;
    out_handle->shader_id = shader_id;
    out_handle->stage = stage;
    out_handle->spirv_size = spirv_size;
    out_handle->spirv_code = g_shader_cache[slot].spirv_code;
    
    printf("[Phase 14] SPIR-V shader loaded: id=%u stage=%s size=%llu\n",
           shader_id, _stage_to_string(stage), spirv_size);
    
    return 0;
}

int D3D8_Vulkan_LoadShaderSPIRVFile(VkDevice device, const char* filepath,
                                    D3D8_VULKAN_SHADER_STAGE stage,
                                    D3D8_VULKAN_SHADER_HANDLE* out_handle) {
    printf("[Phase 14] D3D8_Vulkan_LoadShaderSPIRVFile called: file=%s\n", filepath);
    
    if (device == NULL || filepath == NULL || out_handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        _set_error("Invalid parameters");
        return -1;
    }
    
    int slot = _find_free_shader_slot();
    if (slot == -1) {
        printf("[Phase 14] ERROR: Shader cache full\n");
        _set_error("Shader cache full");
        return -1;
    }
    
    uint32_t shader_id = g_shader_counter++;
    
    g_shader_cache[slot].is_allocated = 1;
    g_shader_cache[slot].shader_id = shader_id;
    g_shader_cache[slot].stage = (int32_t)stage;
    g_shader_cache[slot].module = (void*)(uintptr_t)(11000000 + shader_id);
    g_shader_cache[slot].spirv_size = 0;
    g_shader_cache[slot].spirv_code = NULL;
    
    out_handle->module = (VkShaderModule)g_shader_cache[slot].module;
    out_handle->shader_id = shader_id;
    out_handle->stage = stage;
    out_handle->spirv_size = 0;
    out_handle->spirv_code = NULL;
    
    printf("[Phase 14] SPIR-V shader loaded from file: id=%u file=%s\n", shader_id, filepath);
    return 0;
}

int D3D8_Vulkan_DestroyShader(VkDevice device, D3D8_VULKAN_SHADER_HANDLE* handle) {
    printf("[Phase 14] D3D8_Vulkan_DestroyShader called\n");
    
    if (device == NULL || handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_SHADERS; i++) {
        if (g_shader_cache[i].is_allocated && g_shader_cache[i].module == handle->module) {
            if (g_shader_cache[i].spirv_code) {
                free(g_shader_cache[i].spirv_code);
            }
            g_shader_cache[i].is_allocated = 0;
            printf("[Phase 14] Shader destroyed: id=%u\n", handle->shader_id);
            return 0;
        }
    }
    
    printf("[Phase 14] ERROR: Shader not found\n");
    return -1;
}

int D3D8_Vulkan_GetShaderReflection(D3D8_VULKAN_SHADER_HANDLE* handle,
                                    D3D8_VULKAN_SHADER_REFLECTION_DATA* out_reflection) {
    printf("[Phase 14] D3D8_Vulkan_GetShaderReflection called\n");
    
    if (handle == NULL || out_reflection == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    memset(out_reflection, 0, sizeof(D3D8_VULKAN_SHADER_REFLECTION_DATA));
    out_reflection->spirv_code_size = handle->spirv_size;
    out_reflection->input_variable_count = 0;
    out_reflection->output_variable_count = 0;
    out_reflection->uniform_variable_count = 0;
    out_reflection->sampler_count = 0;
    out_reflection->image_count = 0;
    
    printf("[Phase 14] Shader reflection retrieved\n");
    return 0;
}

int D3D8_Vulkan_CacheShader(D3D8_VULKAN_SHADER_HANDLE* handle, const char* filepath) {
    printf("[Phase 14] D3D8_Vulkan_CacheShader called: file=%s\n", filepath);
    
    if (handle == NULL || filepath == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    printf("[Phase 14] Shader cached: %s\n", filepath);
    return 0;
}

int D3D8_Vulkan_LoadShaderFromCache(VkDevice device, const char* filepath,
                                    D3D8_VULKAN_SHADER_HANDLE* out_handle) {
    printf("[Phase 14] D3D8_Vulkan_LoadShaderFromCache called: file=%s\n", filepath);
    
    if (device == NULL || filepath == NULL || out_handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    int slot = _find_free_shader_slot();
    if (slot == -1) {
        printf("[Phase 14] ERROR: Shader cache full\n");
        return -1;
    }
    
    uint32_t shader_id = g_shader_counter++;
    
    g_shader_cache[slot].is_allocated = 1;
    g_shader_cache[slot].shader_id = shader_id;
    g_shader_cache[slot].stage = D3D8_VULKAN_SHADER_STAGE_FRAGMENT;
    g_shader_cache[slot].module = (void*)(uintptr_t)(11000000 + shader_id);
    g_shader_cache[slot].spirv_size = 0;
    g_shader_cache[slot].spirv_code = NULL;
    
    out_handle->module = (VkShaderModule)g_shader_cache[slot].module;
    out_handle->shader_id = shader_id;
    out_handle->stage = D3D8_VULKAN_SHADER_STAGE_FRAGMENT;
    out_handle->spirv_size = 0;
    out_handle->spirv_code = NULL;
    
    printf("[Phase 14] Shader loaded from cache: id=%u file=%s\n", shader_id, filepath);
    return 0;
}

/* ============================================================================
 * Shader Pipeline API
 * ============================================================================ */

int D3D8_Vulkan_CreateShaderPipeline(VkDevice device, D3D8_VULKAN_SHADER_HANDLE* vertex_shader,
                                     D3D8_VULKAN_SHADER_HANDLE* fragment_shader,
                                     uint32_t* out_pipeline_id) {
    printf("[Phase 14] D3D8_Vulkan_CreateShaderPipeline called\n");
    
    if (device == NULL || vertex_shader == NULL || fragment_shader == NULL || out_pipeline_id == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    int slot = _find_free_pipeline_slot();
    if (slot == -1) {
        printf("[Phase 14] ERROR: Pipeline cache full\n");
        return -1;
    }
    
    uint32_t pipeline_id = g_pipeline_counter++;
    
    g_pipeline_cache[slot].is_allocated = 1;
    g_pipeline_cache[slot].pipeline_id = pipeline_id;
    g_pipeline_cache[slot].vertex_shader_id = vertex_shader->shader_id;
    g_pipeline_cache[slot].fragment_shader_id = fragment_shader->shader_id;
    
    *out_pipeline_id = pipeline_id;
    
    printf("[Phase 14] Shader pipeline created: id=%u vert=%u frag=%u\n",
           pipeline_id, vertex_shader->shader_id, fragment_shader->shader_id);
    
    return 0;
}

int D3D8_Vulkan_BindShader(VkDevice device, D3D8_VULKAN_SHADER_HANDLE* handle) {
    printf("[Phase 14] D3D8_Vulkan_BindShader called\n");
    
    if (device == NULL || handle == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    printf("[Phase 14] Shader bound: id=%u\n", handle->shader_id);
    return 0;
}

int D3D8_Vulkan_GetShaderSPIRVCode(D3D8_VULKAN_SHADER_HANDLE* handle,
                                   const uint32_t** out_code, uint64_t* out_size) {
    printf("[Phase 14] D3D8_Vulkan_GetShaderSPIRVCode called\n");
    
    if (handle == NULL || out_code == NULL || out_size == NULL) {
        printf("[Phase 14] ERROR: Invalid parameters\n");
        return -1;
    }
    
    *out_code = (const uint32_t*)handle->spirv_code;
    *out_size = handle->spirv_size;
    
    printf("[Phase 14] SPIR-V code retrieved: size=%llu\n", handle->spirv_size);
    return 0;
}

int D3D8_Vulkan_ClearShaderCache(void) {
    printf("[Phase 14] D3D8_Vulkan_ClearShaderCache called\n");
    
    int cleared = 0;
    for (int i = 0; i < MAX_SHADERS; i++) {
        if (g_shader_cache[i].is_allocated) {
            if (g_shader_cache[i].spirv_code) {
                free(g_shader_cache[i].spirv_code);
            }
            g_shader_cache[i].is_allocated = 0;
            cleared++;
        }
    }
    
    printf("[Phase 14] Shader cache cleared: %d shaders freed\n", cleared);
    return 0;
}

uint32_t D3D8_Vulkan_GetShaderCompilationError(char* error_buffer, uint32_t buffer_size) {
    if (error_buffer == NULL || buffer_size == 0) {
        return strlen(g_last_error);
    }
    
    uint32_t len = strlen(g_last_error);
    if (len >= buffer_size) {
        len = buffer_size - 1;
    }
    
    memcpy(error_buffer, g_last_error, len);
    error_buffer[len] = '\0';
    
    return len;
}
