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
**  FILE: d3d8_vulkan_render_pass.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Render Pass & Graphics Pipeline Abstraction
**
**  DESCRIPTION:
**    Provides Vulkan VkRenderPass and VkGraphicsPipeline creation interfaces
**    for DirectX 8 compatibility layer. Translates DirectX render state to
**    Vulkan pipeline state, enabling cross-platform graphics rendering.
**
**    Phase 09: Render Pass & Pipeline Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_RENDER_PASS_H_
#define D3D8_VULKAN_RENDER_PASS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>  // For uint32_t, uint64_t, uint8_t

/* ============================================================================
 * Forward Declarations (to avoid including Vulkan headers directly)
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkRenderPass;
typedef void* VkPipeline;
typedef void* VkPipelineLayout;
typedef void* VkShaderModule;
typedef unsigned int VkFormat;
typedef unsigned int uint32_t;

/* ============================================================================
 * Render Pass Configuration Types
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_RENDER_STATE
 * @brief DirectX render state enumeration for Vulkan translation
 */
typedef enum {
    D3DRS_BLEND_ENABLED = 0,
    D3DRS_BLEND_OP = 1,
    D3DRS_SRC_BLEND = 2,
    D3DRS_DST_BLEND = 3,
    D3DRS_CULL_MODE = 4,
    D3DRS_FILL_MODE = 5,
    D3DRS_DEPTH_TEST = 6,
    D3DRS_DEPTH_WRITE = 7,
    D3DRS_DEPTH_FUNC = 8,
    D3DRS_STENCIL_TEST = 9,
    D3DRS_STENCIL_FUNC = 10,
    D3DRS_VIEWPORT = 11,
    D3DRS_SCISSOR = 12,
    D3DRS_ALPHA_TEST = 13,
    D3DRS_ALPHA_FUNC = 14,
} D3D8_VULKAN_RENDER_STATE;

/**
 * @struct D3D8_VULKAN_RENDER_STATE_CONFIG
 * @brief Configuration for render state translation
 */
typedef struct {
    int blend_enabled;                  /* Boolean: enable blending */
    unsigned int src_blend;             /* Source blend factor */
    unsigned int dst_blend;             /* Destination blend factor */
    unsigned int blend_op;              /* Blend operation */
    unsigned int cull_mode;             /* Triangle culling (none/front/back) */
    unsigned int fill_mode;             /* Fill mode (solid/wireframe) */
    int depth_test;                     /* Boolean: depth testing enabled */
    int depth_write;                    /* Boolean: depth write enabled */
    unsigned int depth_func;            /* Depth comparison function */
    int stencil_test;                   /* Boolean: stencil testing enabled */
    unsigned int stencil_func;          /* Stencil comparison function */
    float viewport_x, viewport_y;       /* Viewport position */
    float viewport_width, viewport_height;  /* Viewport dimensions */
    float viewport_min_z, viewport_max_z;   /* Depth range */
} D3D8_VULKAN_RENDER_STATE_CONFIG;

/**
 * @struct D3D8_VULKAN_ATTACHMENT_INFO
 * @brief Information about render pass attachments
 */
typedef struct {
    VkFormat color_format;              /* Color attachment format */
    VkFormat depth_format;              /* Depth attachment format (0 if none) */
    int has_stencil;                    /* Boolean: depth attachment has stencil */
    int clear_color_on_load;            /* Boolean: load operation is clear */
    int clear_depth_on_load;            /* Boolean: clear depth on load */
} D3D8_VULKAN_ATTACHMENT_INFO;

/* ============================================================================
 * Public API Functions
 * ============================================================================ */

/**
 * @brief Create a Vulkan render pass from DirectX render state
 *
 * @param device Vulkan device
 * @param attachment_info Information about render pass attachments
 * @param render_pass Output render pass handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateRenderPass(
    VkDevice device,
    const D3D8_VULKAN_ATTACHMENT_INFO* attachment_info,
    VkRenderPass* render_pass);

/**
 * @brief Destroy a Vulkan render pass
 *
 * @param device Vulkan device
 * @param render_pass Render pass to destroy
 */
void D3D8_Vulkan_DestroyRenderPass(
    VkDevice device,
    VkRenderPass render_pass);

/**
 * @brief Create a Vulkan graphics pipeline from DirectX render state
 *
 * @param device Vulkan device
 * @param render_pass Render pass this pipeline will be used with
 * @param render_state Render state configuration
 * @param shader_vertex Vertex shader module (0 for default)
 * @param shader_fragment Fragment shader module (0 for default)
 * @param pipeline Output pipeline handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateGraphicsPipeline(
    VkDevice device,
    VkRenderPass render_pass,
    const D3D8_VULKAN_RENDER_STATE_CONFIG* render_state,
    VkShaderModule shader_vertex,
    VkShaderModule shader_fragment,
    VkPipeline* pipeline);

/**
 * @brief Destroy a Vulkan graphics pipeline
 *
 * @param device Vulkan device
 * @param pipeline Pipeline to destroy
 */
void D3D8_Vulkan_DestroyGraphicsPipeline(
    VkDevice device,
    VkPipeline pipeline);

/**
 * @brief Create a pipeline layout with descriptor sets and push constants
 *
 * @param device Vulkan device
 * @param num_descriptor_sets Number of descriptor sets
 * @param push_constant_size Size of push constants in bytes (0 if none)
 * @param pipeline_layout Output pipeline layout handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreatePipelineLayout(
    VkDevice device,
    uint32_t num_descriptor_sets,
    uint32_t push_constant_size,
    VkPipelineLayout* pipeline_layout);

/**
 * @brief Destroy a pipeline layout
 *
 * @param device Vulkan device
 * @param pipeline_layout Pipeline layout to destroy
 */
void D3D8_Vulkan_DestroyPipelineLayout(
    VkDevice device,
    VkPipelineLayout pipeline_layout);

/**
 * @brief Translate DirectX render state to Vulkan render state config
 *
 * @param d3d_render_state DirectX render state value
 * @param d3d_state_value DirectX state value
 * @param config Output render state config (updated)
 * @return 0 on success, non-zero if state not supported
 */
int D3D8_Vulkan_TranslateRenderState(
    D3D8_VULKAN_RENDER_STATE d3d_render_state,
    unsigned int d3d_state_value,
    D3D8_VULKAN_RENDER_STATE_CONFIG* config);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* D3D8_VULKAN_RENDER_PASS_H_ */
