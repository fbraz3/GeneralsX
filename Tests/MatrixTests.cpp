// GeneralsX @test AI 12/07/2026 WWMath Matrix unit tests

#include <gtest/gtest.h>
#include "matrix3.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "vector3.h"

// --- Matrix3x3 Tests ---
TEST(Matrix3Test, ConstructorAndIdentity) {
    Matrix3x3 m(1); // Identity constructor
    EXPECT_FLOAT_EQ(m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m[0][1], 0.0f);
    EXPECT_FLOAT_EQ(m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m[2][2], 1.0f);
}

TEST(Matrix3Test, VectorMultiplication) {
    Matrix3x3 m(1); // Identity
    Vector3 v(1.0f, 2.0f, 3.0f);
    
    Vector3 res;
    Matrix3x3::Rotate_Vector(m, v, &res);
    EXPECT_FLOAT_EQ(res.X, 1.0f);
    EXPECT_FLOAT_EQ(res.Y, 2.0f);
    EXPECT_FLOAT_EQ(res.Z, 3.0f);
}

TEST(Matrix3Test, MatrixMultiplication) {
    Matrix3x3 m1(1);
    Matrix3x3 m2(1);
    
    Matrix3x3 res;
    Matrix3x3::Multiply(m1, m2, &res);
    EXPECT_FLOAT_EQ(res[0][0], 1.0f);
    EXPECT_FLOAT_EQ(res[1][1], 1.0f);
    EXPECT_FLOAT_EQ(res[2][2], 1.0f);
}

// --- Matrix3D Tests ---
TEST(Matrix3DTest, ConstructorAndIdentity) {
    Matrix3D m(1); // 1 = Identity
    EXPECT_FLOAT_EQ(m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m[2][2], 1.0f);
    EXPECT_FLOAT_EQ(m[0][3], 0.0f); // Translation part
}

TEST(Matrix3DTest, Translation) {
    Matrix3D m(1);
    Vector3 trans(5.0f, 10.0f, 15.0f);
    m.Set_Translation(trans);
    
    EXPECT_FLOAT_EQ(m[0][3], 5.0f);
    EXPECT_FLOAT_EQ(m[1][3], 10.0f);
    EXPECT_FLOAT_EQ(m[2][3], 15.0f);
    
    Vector3 v(0.0f, 0.0f, 0.0f);
    Vector3 res;
    Matrix3D::Transform_Vector(m, v, &res);
    EXPECT_FLOAT_EQ(res.X, 5.0f);
    EXPECT_FLOAT_EQ(res.Y, 10.0f);
    EXPECT_FLOAT_EQ(res.Z, 15.0f);
}

TEST(Matrix3DTest, MatrixMultiplication) {
    Matrix3D m1(1);
    m1.Set_Translation(Vector3(1.0f, 0.0f, 0.0f));
    
    Matrix3D m2(1);
    m2.Set_Translation(Vector3(0.0f, 1.0f, 0.0f));
    
    Matrix3D res;
    Matrix3D::Multiply(m1, m2, &res);
    // (Translation 1,0,0) * (Translation 0,1,0) = (1,1,0)
    EXPECT_FLOAT_EQ(res[0][3], 1.0f);
    EXPECT_FLOAT_EQ(res[1][3], 1.0f);
    EXPECT_FLOAT_EQ(res[2][3], 0.0f);
}

// --- Matrix4x4 Tests ---
TEST(Matrix4Test, ConstructorAndIdentity) {
    Matrix4x4 m(1);
    EXPECT_FLOAT_EQ(m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(m[1][1], 1.0f);
    EXPECT_FLOAT_EQ(m[2][2], 1.0f);
    EXPECT_FLOAT_EQ(m[3][3], 1.0f);
}
