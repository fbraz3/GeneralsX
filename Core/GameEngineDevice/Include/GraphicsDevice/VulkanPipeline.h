/*
 * Phase 09.3: VulkanPipeline - Graphics Pipeline and Shader Management
 *
 * Encapsulates Vulkan graphics pipelines and compute pipelines.
 * Handles:
 * - Shader compilation and loading
 * - Pipeline state configuration
 * - Descriptor sets for resource binding
 */

#ifndef VULKANPIPELINE_H
#define VULKANPIPELINE_H

#include "GraphicsDevice/GraphicsDevice.h"
#include "GraphicsDevice/vulkan_stubs.h"

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Pipeline type enumeration
 */
enum VulkanPipelineType {
    PIPELINE_TYPE_GRAPHICS = 0,
    PIPELINE_TYPE_COMPUTE = 1,
};

/**
 * @brief Vulkan shader module wrapper
 *
 * Manages a single shader module (vertex, fragment, or compute).
 */
class VulkanShaderModule {
public:
    /**
     * Constructor
     */
    VulkanShaderModule(VkDevice device);

    /**
     * Destructor
     */
    ~VulkanShaderModule();

    /**
     * Load shader from SPIR-V bytecode
     * @param spirvCode SPIR-V bytecode
     * @param spirvSize Size of bytecode in bytes
     * @return true if loading successful
     */
    bool load(const void* spirvCode, size_t spirvSize);

    /**
     * Load shader from source code (compile GLSL to SPIR-V)
     * @param glslSource GLSL source code
     * @param stage Shader stage (vertex, fragment, compute)
     * @return true if compilation and loading successful
     */
    bool loadFromGLSL(const char* glslSource, VkShaderStageFlagBits stage);

    /**
     * Get Vulkan shader module handle
     */
    VkShaderModule getVkShaderModule() const { return module; }

    /**
     * Get shader stage flag
     */
    VkShaderStageFlagBits getStage() const { return stage; }

private:
    VkDevice device;                    ///< Logical device
    VkShaderModule module;              ///< Vulkan shader module
    VkShaderStageFlagBits stage;        ///< Shader stage
};

/**
 * @brief Vulkan graphics/compute pipeline wrapper
 *
 * Manages a complete graphics or compute pipeline including:
 * - Shaders (vertex, fragment, compute)
 * - Pipeline layout
 * - Descriptor set layout
 * - Pipeline state (rasterization, blend, depth, etc.)
 */
class VulkanPipeline {
public:
    /**
     * Constructor
     */
    VulkanPipeline(VkDevice device, VulkanPipelineType type);

    /**
     * Destructor
     */
    ~VulkanPipeline();

    /**
     * Create graphics pipeline from shaders
     * @param vertexShader Compiled vertex shader bytecode
     * @param vertexShaderSize Vertex shader size
     * @param fragmentShader Compiled fragment shader bytecode
     * @param fragmentShaderSize Fragment shader size
     * @param vertexAttributes Array of vertex attribute descriptions
     * @param attributeCount Number of vertex attributes
     * @param renderPass Render pass this pipeline renders to
     * @return true if pipeline creation successful
     */
    bool createGraphics(const void* vertexShader, size_t vertexShaderSize,
                       const void* fragmentShader, size_t fragmentShaderSize,
                       const VertexAttribute* vertexAttributes, int attributeCount,
                       VkRenderPass renderPass);

    /**
     * Create compute pipeline from compute shader
     * @param computeShader Compiled compute shader bytecode
     * @param computeShaderSize Compute shader size
     * @return true if pipeline creation successful
     */
    bool createCompute(const void* computeShader, size_t computeShaderSize);

    /**
     * Get Vulkan pipeline handle
     */
    VkPipeline getVkPipeline() const { return pipeline; }

    /**
     * Get Vulkan pipeline layout
     */
    VkPipelineLayout getVkPipelineLayout() const { return pipelineLayout; }

    /**
     * Get descriptor set layout
     */
    VkDescriptorSetLayout getVkDescriptorSetLayout() const { return descriptorSetLayout; }

    /**
     * Get pipeline type
     */
    VulkanPipelineType getType() const { return type; }

private:
    VkDevice device;                    ///< Logical device
    VkPipeline pipeline;                ///< Vulkan pipeline
    VkPipelineLayout pipelineLayout;    ///< Pipeline layout (uniforms/resources)
    VkDescriptorSetLayout descriptorSetLayout; ///< Descriptor set layout
    VulkanShaderModule* vertexShader;   ///< Vertex shader (graphics only)
    VulkanShaderModule* fragmentShader; ///< Fragment shader (graphics only)
    VulkanShaderModule* computeShader;  ///< Compute shader (compute only)
    VulkanPipelineType type;            ///< Pipeline type

    /**
     * Create graphics pipeline layout
     */
    bool createGraphicsLayout();

    /**
     * Create compute pipeline layout
     */
    bool createComputeLayout();

    /**
     * Create descriptor set layout for resources
     */
    bool createDescriptorSetLayout();

    /**
     * Create graphics pipeline state
     */
    bool createGraphicsPipeline(const VertexAttribute* vertexAttributes,
                               int attributeCount, VkRenderPass renderPass);

    /**
     * Create compute pipeline
     */
    bool createComputePipeline();
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANPIPELINE_H
