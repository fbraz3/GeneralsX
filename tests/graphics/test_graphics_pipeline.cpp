#include "../core/test_macros.h"
#include "../core/test_utils.h"
#include "../core/profiler/cpu_profiler.h"
#include <iostream>

namespace phase47 {
namespace graphics_tests {

// Graphics pipeline unit tests
TEST(GraphicsTexture, TextureCreation) {
    CPU_PROFILE_SCOPE("test::TextureCreation");
    
    void* texture = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
    ASSERT_NOT_NULL(texture);
    graphics_utils::DestroyTestTexture(texture);
}

TEST(GraphicsTexture, TextureVariousFormats) {
    CPU_PROFILE_SCOPE("test::TextureVariousFormats");
    
    // Test different texture formats
    void* rgba = graphics_utils::CreateTestTexture(512, 512, "RGBA8");
    ASSERT_NOT_NULL(rgba);
    graphics_utils::DestroyTestTexture(rgba);
    
    void* rgb = graphics_utils::CreateTestTexture(512, 512, "RGB8");
    ASSERT_NOT_NULL(rgb);
    graphics_utils::DestroyTestTexture(rgb);
    
    void* bc3 = graphics_utils::CreateTestTexture(512, 512, "BC3");
    ASSERT_NOT_NULL(bc3);
    graphics_utils::DestroyTestTexture(bc3);
}

TEST(GraphicsTexture, TextureSizes) {
    CPU_PROFILE_SCOPE("test::TextureSizes");
    
    // Test various texture sizes
    struct TestSize {
        uint32_t width;
        uint32_t height;
    };
    
    TestSize sizes[] = {
        {64, 64},
        {128, 128},
        {256, 256},
        {512, 512},
        {1024, 1024},
        {2048, 2048}
    };
    
    for (const auto& size : sizes) {
        void* texture = graphics_utils::CreateTestTexture(size.width, size.height, "RGBA8");
        ASSERT_NOT_NULL(texture);
        graphics_utils::DestroyTestTexture(texture);
    }
}

// Buffer tests
TEST(GraphicsBuffer, BufferCreation) {
    CPU_PROFILE_SCOPE("test::BufferCreation");
    
    void* buffer = graphics_utils::CreateTestBuffer(4096);
    ASSERT_NOT_NULL(buffer);
    graphics_utils::DestroyTestBuffer(buffer);
}

TEST(GraphicsBuffer, BufferWithData) {
    CPU_PROFILE_SCOPE("test::BufferWithData");
    
    const uint32_t size = 1024;
    uint8_t test_data[size];
    
    // Fill test data
    for (uint32_t i = 0; i < size; ++i) {
        test_data[i] = (i % 256);
    }
    
    void* buffer = graphics_utils::CreateTestBuffer(size, test_data);
    ASSERT_NOT_NULL(buffer);
    graphics_utils::DestroyTestBuffer(buffer);
}

TEST(GraphicsBuffer, VariousBufferSizes) {
    CPU_PROFILE_SCOPE("test::VariousBufferSizes");
    
    uint32_t sizes[] = {256, 512, 1024, 2048, 4096, 8192, 16384};
    
    for (uint32_t size : sizes) {
        void* buffer = graphics_utils::CreateTestBuffer(size);
        ASSERT_NOT_NULL(buffer);
        graphics_utils::DestroyTestBuffer(buffer);
    }
}

// Shader tests
TEST(GraphicsShader, ShaderCreation) {
    CPU_PROFILE_SCOPE("test::ShaderCreation");
    
    const char* simple_shader = R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        }
    )";
    
    void* shader = graphics_utils::CreateTestShader(simple_shader);
    ASSERT_NOT_NULL(shader);
    graphics_utils::DestroyTestShader(shader);
}

TEST(GraphicsShader, VertexAndFragmentShaders) {
    CPU_PROFILE_SCOPE("test::VertexAndFragmentShaders");
    
    const char* vertex_shader = R"(
        #version 450
        layout(location = 0) in vec3 position;
        void main() {
            gl_Position = vec4(position, 1.0);
        }
    )";
    
    const char* fragment_shader = R"(
        #version 450
        layout(location = 0) out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
    
    void* vs = graphics_utils::CreateTestShader(vertex_shader);
    ASSERT_NOT_NULL(vs);
    
    void* fs = graphics_utils::CreateTestShader(fragment_shader);
    ASSERT_NOT_NULL(fs);
    
    graphics_utils::DestroyTestShader(vs);
    graphics_utils::DestroyTestShader(fs);
}

// Render pass tests
TEST(GraphicsRenderPass, RenderPassCreation) {
    CPU_PROFILE_SCOPE("test::RenderPassCreation");
    
    void* render_pass = graphics_utils::CreateTestRenderPass();
    ASSERT_NOT_NULL(render_pass);
    graphics_utils::DestroyTestRenderPass(render_pass);
}

TEST(GraphicsRenderPass, MultipleRenderPasses) {
    CPU_PROFILE_SCOPE("test::MultipleRenderPasses");
    
    for (int i = 0; i < 5; ++i) {
        void* render_pass = graphics_utils::CreateTestRenderPass();
        ASSERT_NOT_NULL(render_pass);
        graphics_utils::DestroyTestRenderPass(render_pass);
    }
}

// Draw call tests
TEST(GraphicsDrawCall, DrawCallRecording) {
    CPU_PROFILE_SCOPE("test::DrawCallRecording");
    
    // Create resources for draw call
    void* buffer = graphics_utils::CreateTestBuffer(1024);
    void* texture = graphics_utils::CreateTestTexture(256, 256, "RGBA8");
    void* shader = graphics_utils::CreateTestShader(R"(void main(){})");
    void* render_pass = graphics_utils::CreateTestRenderPass();
    
    ASSERT_NOT_NULL(buffer);
    ASSERT_NOT_NULL(texture);
    ASSERT_NOT_NULL(shader);
    ASSERT_NOT_NULL(render_pass);
    
    // Cleanup
    graphics_utils::DestroyTestBuffer(buffer);
    graphics_utils::DestroyTestTexture(texture);
    graphics_utils::DestroyTestShader(shader);
    graphics_utils::DestroyTestRenderPass(render_pass);
}

// Performance tests
PERF_TEST(GraphicsPerformance, TextureCreationSpeed) {
    CPU_PROFILE_SCOPE("test::TextureCreationSpeed");
    
    for (int i = 0; i < 10; ++i) {
        void* texture = graphics_utils::CreateTestTexture(512, 512, "RGBA8");
        ASSERT_NOT_NULL(texture);
        graphics_utils::DestroyTestTexture(texture);
    }
}

PERF_TEST(GraphicsPerformance, BufferCreationSpeed) {
    CPU_PROFILE_SCOPE("test::BufferCreationSpeed");
    
    for (int i = 0; i < 20; ++i) {
        void* buffer = graphics_utils::CreateTestBuffer(4096);
        ASSERT_NOT_NULL(buffer);
        graphics_utils::DestroyTestBuffer(buffer);
    }
}

PERF_TEST(GraphicsPerformance, ShaderCompilationSpeed) {
    CPU_PROFILE_SCOPE("test::ShaderCompilationSpeed");
    
    const char* shader_code = R"(
        #version 450
        void main() { gl_Position = vec4(0.0); }
    )";
    
    for (int i = 0; i < 5; ++i) {
        void* shader = graphics_utils::CreateTestShader(shader_code);
        ASSERT_NOT_NULL(shader);
        graphics_utils::DestroyTestShader(shader);
    }
}

}  // namespace graphics_tests
}  // namespace phase47
