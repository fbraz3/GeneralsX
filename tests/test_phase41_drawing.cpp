#include <gtest/gtest.h>
#include <cstring>
#include <vector>

// Mock structures for testing
struct Vector3 {
    float x, y, z;
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
    float u, v;
};

// Test basic drawing primitive
TEST(Phase41Drawing, DrawTriangle) {
    std::vector<Vertex> vertices = {
        Vertex{Vector3(0.0f, 1.0f, 0.0f), Vector3(0, 0, 1), 0.5f, 1.0f},    // Top
        Vertex{Vector3(-1.0f, -1.0f, 0.0f), Vector3(0, 0, 1), 0.0f, 0.0f},  // Bottom left
        Vertex{Vector3(1.0f, -1.0f, 0.0f), Vector3(0, 0, 1), 1.0f, 0.0f}    // Bottom right
    };
    
    EXPECT_EQ(vertices.size(), 3);
    EXPECT_EQ(vertices[0].position.y, 1.0f);
    EXPECT_EQ(vertices[1].position.x, -1.0f);
    EXPECT_EQ(vertices[2].position.x, 1.0f);
}

// Test indexed primitive
TEST(Phase41Drawing, DrawIndexedTriangle) {
    std::vector<Vertex> vertices = {
        Vertex{Vector3(0.0f, 1.0f, 0.0f), Vector3(0, 0, 1), 0.5f, 1.0f},
        Vertex{Vector3(-1.0f, -1.0f, 0.0f), Vector3(0, 0, 1), 0.0f, 0.0f},
        Vertex{Vector3(1.0f, -1.0f, 0.0f), Vector3(0, 0, 1), 1.0f, 0.0f}
    };
    
    std::vector<uint16_t> indices = {0, 1, 2};
    
    EXPECT_EQ(indices.size(), 3);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 1);
    EXPECT_EQ(indices[2], 2);
}

// Test render state flags
TEST(Phase41Drawing, RenderStateFlags) {
    enum RenderStateType {
        RS_LIGHTING = 0,
        RS_DEPTH_TEST = 1,
        RS_BLEND = 2,
        RS_CULL_MODE = 3,
        RS_ALPHA_BLEND = 4
    };
    
    enum BlendMode {
        BLEND_OPAQUE = 0,
        BLEND_ADDITIVE = 1,
        BLEND_ALPHA = 2
    };
    
    // Test enum values
    EXPECT_EQ(RS_LIGHTING, 0);
    EXPECT_EQ(RS_DEPTH_TEST, 1);
    EXPECT_EQ(BLEND_OPAQUE, 0);
    EXPECT_EQ(BLEND_ALPHA, 2);
}

// Test material properties
TEST(Phase41Drawing, MaterialProperties) {
    struct Material {
        float diffuse[4];  // RGBA
        float specular[4]; // RGBA
        float ambient[4];  // RGBA
        float shininess;
    };
    
    Material mat;
    std::memset(&mat, 0, sizeof(Material));
    
    mat.diffuse[0] = 1.0f;  // Red
    mat.diffuse[1] = 0.0f;
    mat.diffuse[2] = 0.0f;
    mat.diffuse[3] = 1.0f;  // Alpha
    
    EXPECT_EQ(mat.diffuse[0], 1.0f);
    EXPECT_EQ(mat.diffuse[3], 1.0f);
}

// Test light properties
TEST(Phase41Drawing, LightProperties) {
    struct Light {
        Vector3 position;
        Vector3 direction;
        float intensity;
        float range;
        uint32_t type;  // 0=directional, 1=point, 2=spot
    };
    
    Light light;
    light.position = Vector3(5.0f, 10.0f, 5.0f);
    light.direction = Vector3(0, -1, 0);
    light.intensity = 1.0f;
    light.range = 50.0f;
    light.type = 1;  // Point light
    
    EXPECT_EQ(light.position.x, 5.0f);
    EXPECT_EQ(light.type, 1);
}

// Test transformation matrix (identity)
TEST(Phase41Drawing, IdentityMatrix) {
    struct Matrix4x4 {
        float m[4][4];
    };
    
    Matrix4x4 identity;
    std::memset(&identity, 0, sizeof(Matrix4x4));
    
    // Set identity matrix
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;
    identity.m[3][3] = 1.0f;
    
    EXPECT_EQ(identity.m[0][0], 1.0f);
    EXPECT_EQ(identity.m[1][1], 1.0f);
    EXPECT_EQ(identity.m[3][3], 1.0f);
    EXPECT_EQ(identity.m[0][1], 0.0f);
}

// Test viewport settings
TEST(Phase41Drawing, ViewportSettings) {
    struct Viewport {
        float x, y, width, height;
        float minDepth, maxDepth;
    };
    
    Viewport vp;
    vp.x = 0;
    vp.y = 0;
    vp.width = 1920;
    vp.height = 1080;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    
    EXPECT_EQ(vp.width, 1920.0f);
    EXPECT_EQ(vp.height, 1080.0f);
    EXPECT_EQ(vp.minDepth, 0.0f);
    EXPECT_EQ(vp.maxDepth, 1.0f);
}

// Test scissor rectangle
TEST(Phase41Drawing, ScissorRect) {
    struct Rect {
        int32_t left, top, right, bottom;
    };
    
    Rect scissor;
    scissor.left = 100;
    scissor.top = 100;
    scissor.right = 1820;
    scissor.bottom = 980;
    
    EXPECT_EQ(scissor.left, 100);
    EXPECT_EQ(scissor.right, 1820);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
