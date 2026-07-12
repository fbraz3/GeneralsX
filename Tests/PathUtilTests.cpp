// GeneralsX @test AI 11/07/2026 PathUtil unit tests

#include <gtest/gtest.h>
#include "Lib/PathUtil.h"

TEST(PathUtilTest, GetExtension_StandardPath) {
    const char* path = "models/tank.w3d";
    const char* ext = getExtension(path);
    ASSERT_NE(ext, nullptr);
    EXPECT_STREQ(ext, ".w3d");
}

TEST(PathUtilTest, GetExtension_NoExtension) {
    const char* path = "models/tank";
    const char* ext = getExtension(path);
    EXPECT_EQ(ext, nullptr);
}

TEST(PathUtilTest, GetExtension_DotInDirectory) {
    const char* path = "my.models/tank";
    const char* ext = getExtension(path);
    EXPECT_EQ(ext, nullptr);
}

TEST(PathUtilTest, GetExtension_MultipleDots) {
    const char* path = "models/tank.v2.w3d";
    const char* ext = getExtension(path);
    ASSERT_NE(ext, nullptr);
    EXPECT_STREQ(ext, ".w3d");
}

TEST(PathUtilTest, GetExtension_WideChar) {
    const wchar_t* path = L"models/tank.w3d";
    const wchar_t* ext = getExtension(path);
    ASSERT_NE(ext, nullptr);
    EXPECT_STREQ(ext, L".w3d");
}
