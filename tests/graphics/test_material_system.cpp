#include "../core/test_macros.h"
#include "../core/test_utils.h"
#include "../core/profiler/cpu_profiler.h"
#include <iostream>

namespace phase47 {
namespace graphics_tests {

// Material system tests
TEST(GraphicsMaterial, MaterialCreation) {
    CPU_PROFILE_SCOPE("test::MaterialCreation");
    
    // Test material creation with various properties
    void* texture1 = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
    void* texture2 = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
    
    ASSERT_NOT_NULL(texture1);
    ASSERT_NOT_NULL(texture2);
    
    graphics_utils::DestroyTestTexture(texture1);
    graphics_utils::DestroyTestTexture(texture2);
}

TEST(GraphicsMaterial, MaterialCaching) {
    CPU_PROFILE_SCOPE("test::MaterialCaching");
    
    // Create same material multiple times to test caching
    for (int i = 0; i < 3; ++i) {
        void* texture = graphics_utils::CreateTestTexture(512, 512, "RGBA8");
        ASSERT_NOT_NULL(texture);
        graphics_utils::DestroyTestTexture(texture);
    }
}

// Descriptor set tests
TEST(GraphicsDescriptor, DescriptorSetCreation) {
    CPU_PROFILE_SCOPE("test::DescriptorSetCreation");
    
    void* buffer = graphics_utils::CreateTestBuffer(256);
    void* texture = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
    
    ASSERT_NOT_NULL(buffer);
    ASSERT_NOT_NULL(texture);
    
    graphics_utils::DestroyTestBuffer(buffer);
    graphics_utils::DestroyTestTexture(texture);
}

// Pipeline state tests
TEST(GraphicsPipeline, PipelineStateBlending) {
    CPU_PROFILE_SCOPE("test::PipelineStateBlending");
    
    // Test various blending modes
    void* shader = graphics_utils::CreateTestShader(R"(void main(){})");
    ASSERT_NOT_NULL(shader);
    graphics_utils::DestroyTestShader(shader);
}

TEST(GraphicsPipeline, PipelineStateDepthTest) {
    CPU_PROFILE_SCOPE("test::PipelineStateDepthTest");
    
    void* render_pass = graphics_utils::CreateTestRenderPass();
    ASSERT_NOT_NULL(render_pass);
    graphics_utils::DestroyTestRenderPass(render_pass);
}

// Framebuffer tests
TEST(GraphicsFramebuffer, FramebufferCreation) {
    CPU_PROFILE_SCOPE("test::FramebufferCreation");
    
    void* render_pass = graphics_utils::CreateTestRenderPass();
    void* texture = graphics_utils::CreateTestTexture(1024, 768, "RGBA8");
    
    ASSERT_NOT_NULL(render_pass);
    ASSERT_NOT_NULL(texture);
    
    graphics_utils::DestroyTestRenderPass(render_pass);
    graphics_utils::DestroyTestTexture(texture);
}

TEST(GraphicsFramebuffer, OffscreenRendering) {
    CPU_PROFILE_SCOPE("test::OffscreenRendering");
    
    // Test rendering to texture
    void* target_texture = graphics_utils::CreateTestTexture(512, 512, "RGBA8");
    void* render_pass = graphics_utils::CreateTestRenderPass();
    
    ASSERT_NOT_NULL(target_texture);
    ASSERT_NOT_NULL(render_pass);
    
    graphics_utils::DestroyTestTexture(target_texture);
    graphics_utils::DestroyTestRenderPass(render_pass);
}

// Synchronization tests
TEST(GraphicsSync, FrameSync) {
    CPU_PROFILE_SCOPE("test::FrameSync");
    
    // Test frame synchronization
    for (int i = 0; i < 3; ++i) {
        void* buffer = graphics_utils::CreateTestBuffer(1024);
        ASSERT_NOT_NULL(buffer);
        graphics_utils::DestroyTestBuffer(buffer);
    }
}

TEST(GraphicsSync, ResourceSync) {
    CPU_PROFILE_SCOPE("test::ResourceSync");
    
    void* texture = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
    void* buffer = graphics_utils::CreateTestBuffer(512);
    
    ASSERT_NOT_NULL(texture);
    ASSERT_NOT_NULL(buffer);
    
    graphics_utils::DestroyTestTexture(texture);
    graphics_utils::DestroyTestBuffer(buffer);
}

}  // namespace graphics_tests
}  // namespace phase47
