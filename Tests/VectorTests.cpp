// GeneralsX @test AI 12/07/2026 WWMath Vector unit tests

#include <gtest/gtest.h>
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "vector2i.h"
#include "vector3i.h"

// --- Vector2 Tests ---
TEST(Vector2Test, ConstructorAndAssignment) {
    Vector2 v1(1.0f, 2.0f);
    EXPECT_FLOAT_EQ(v1.X, 1.0f);
    EXPECT_FLOAT_EQ(v1.Y, 2.0f);

    Vector2 v2 = v1;
    EXPECT_FLOAT_EQ(v2.X, 1.0f);
    EXPECT_FLOAT_EQ(v2.Y, 2.0f);

    Vector2 v3;
    v3.Set(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v3.X, 3.0f);
    EXPECT_FLOAT_EQ(v3.Y, 4.0f);
}

TEST(Vector2Test, ArithmeticOperators) {
    Vector2 v1(2.0f, 3.0f);
    Vector2 v2(1.0f, 1.5f);

    Vector2 add = v1 + v2;
    EXPECT_FLOAT_EQ(add.X, 3.0f);
    EXPECT_FLOAT_EQ(add.Y, 4.5f);

    Vector2 sub = v1 - v2;
    EXPECT_FLOAT_EQ(sub.X, 1.0f);
    EXPECT_FLOAT_EQ(sub.Y, 1.5f);

    Vector2 mul = v1 * 2.0f;
    EXPECT_FLOAT_EQ(mul.X, 4.0f);
    EXPECT_FLOAT_EQ(mul.Y, 6.0f);

    Vector2 div = v1 / 2.0f;
    EXPECT_FLOAT_EQ(div.X, 1.0f);
    EXPECT_FLOAT_EQ(div.Y, 1.5f);
}

TEST(Vector2Test, DotProductAndLength) {
    Vector2 v1(3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v1.Length(), 5.0f);
    EXPECT_FLOAT_EQ(v1.Length2(), 25.0f);

    Vector2 v2(1.0f, 0.0f);
    Vector2 v3(0.0f, 1.0f);
    EXPECT_FLOAT_EQ(v2 * v3, 0.0f); // Dot product

    Vector2 v4(2.0f, 2.0f);
    EXPECT_FLOAT_EQ(v4 * v4, 8.0f);
}

TEST(Vector2Test, Normalize) {
    Vector2 v1(3.0f, 4.0f);
    v1.Normalize();
    EXPECT_FLOAT_EQ(v1.X, 3.0f / 5.0f);
    EXPECT_FLOAT_EQ(v1.Y, 4.0f / 5.0f);
    EXPECT_FLOAT_EQ(v1.Length(), 1.0f);
}

// --- Vector3 Tests ---
TEST(Vector3Test, ConstructorAndAssignment) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v1.X, 1.0f);
    EXPECT_FLOAT_EQ(v1.Y, 2.0f);
    EXPECT_FLOAT_EQ(v1.Z, 3.0f);

    float arr[3] = {4.0f, 5.0f, 6.0f};
    Vector3 v2(arr);
    EXPECT_FLOAT_EQ(v2.X, 4.0f);
    EXPECT_FLOAT_EQ(v2.Y, 5.0f);
    EXPECT_FLOAT_EQ(v2.Z, 6.0f);

    v2.Set(v1);
    EXPECT_FLOAT_EQ(v2.X, 1.0f);
}

TEST(Vector3Test, ArithmeticOperators) {
    Vector3 v1(2.0f, 4.0f, 6.0f);
    Vector3 v2(1.0f, 2.0f, 3.0f);

    Vector3 add = v1 + v2;
    EXPECT_FLOAT_EQ(add.X, 3.0f);
    EXPECT_FLOAT_EQ(add.Y, 6.0f);
    EXPECT_FLOAT_EQ(add.Z, 9.0f);

    Vector3 sub = v1 - v2;
    EXPECT_FLOAT_EQ(sub.X, 1.0f);
    EXPECT_FLOAT_EQ(sub.Y, 2.0f);
    EXPECT_FLOAT_EQ(sub.Z, 3.0f);

    Vector3 mul = v1 * 2.0f;
    EXPECT_FLOAT_EQ(mul.X, 4.0f);
    EXPECT_FLOAT_EQ(mul.Y, 8.0f);
    EXPECT_FLOAT_EQ(mul.Z, 12.0f);
}

TEST(Vector3Test, DotAndCrossProduct) {
    Vector3 v1(1.0f, 0.0f, 0.0f);
    Vector3 v2(0.0f, 1.0f, 0.0f);

    EXPECT_FLOAT_EQ(v1 * v2, 0.0f); // Dot
    EXPECT_FLOAT_EQ(v1 * v1, 1.0f);

    Vector3 cross;
    Vector3::Cross_Product(v1, v2, &cross);
    EXPECT_FLOAT_EQ(cross.X, 0.0f);
    EXPECT_FLOAT_EQ(cross.Y, 0.0f);
    EXPECT_FLOAT_EQ(cross.Z, 1.0f);
}

TEST(Vector3Test, NormalizeAndLength) {
    Vector3 v1(0.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v1.Length(), 5.0f);
    EXPECT_FLOAT_EQ(v1.Length2(), 25.0f);

    v1.Normalize();
    EXPECT_FLOAT_EQ(v1.X, 0.0f);
    EXPECT_FLOAT_EQ(v1.Y, 3.0f / 5.0f);
    EXPECT_FLOAT_EQ(v1.Z, 4.0f / 5.0f);
}

TEST(Vector3Test, Lerp) {
    Vector3 v1(0.0f, 0.0f, 0.0f);
    Vector3 v2(10.0f, 10.0f, 10.0f);

    Vector3 v3;
    Vector3::Lerp(v1, v2, 0.5f, &v3);
    EXPECT_FLOAT_EQ(v3.X, 5.0f);
    EXPECT_FLOAT_EQ(v3.Y, 5.0f);
    EXPECT_FLOAT_EQ(v3.Z, 5.0f);
}

// --- Vector4 Tests ---
TEST(Vector4Test, ConstructorAndArithmetic) {
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v1.X, 1.0f);
    EXPECT_FLOAT_EQ(v1.W, 4.0f);

    Vector4 v2(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 sum = v1 + v2;
    EXPECT_FLOAT_EQ(sum.X, 2.0f);
    EXPECT_FLOAT_EQ(sum.W, 5.0f);
}
