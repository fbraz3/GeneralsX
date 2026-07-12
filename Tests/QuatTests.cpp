// GeneralsX @test AI 12/07/2026 WWMath Quat unit tests

#include <gtest/gtest.h>
#include "quat.h"
#include "matrix3d.h"
#include "vector3.h"
#include "wwmath.h"

TEST(QuatTest, ConstructorAndIdentity) {
    Quaternion q;
    q.Make_Identity();
    EXPECT_FLOAT_EQ(q[0], 0.0f);
    EXPECT_FLOAT_EQ(q[1], 0.0f);
    EXPECT_FLOAT_EQ(q[2], 0.0f);
    EXPECT_FLOAT_EQ(q[3], 1.0f); // W is usually index 3
}

// Removed FromMatrix test because there is no matching constructor

TEST(QuatTest, Normalize) {
    Quaternion q(1.0f, 1.0f, 1.0f, 1.0f);
    q.Normalize();
    
    float half = 0.5f; // sqrt(4) = 2, so 1/2 = 0.5
    EXPECT_FLOAT_EQ(q[0], half);
    EXPECT_FLOAT_EQ(q[1], half);
    EXPECT_FLOAT_EQ(q[2], half);
    EXPECT_FLOAT_EQ(q[3], half);
}
