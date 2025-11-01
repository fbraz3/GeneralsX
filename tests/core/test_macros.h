#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <cstring>
#include <cassert>

namespace phase47 {

// Test result tracking
enum class TestStatus {
    PASSED,
    FAILED,
    SKIPPED,
    ERROR
};

struct TestResult {
    std::string test_name;
    TestStatus status;
    std::string error_message;
    double execution_time_ms;
    int line_number;
    const char* file_path;
};

// Global test registry
class TestRegistry {
public:
    static TestRegistry& Instance() {
        static TestRegistry instance;
        return instance;
    }

    void RegisterTest(
        const std::string& suite_name,
        const std::string& test_name,
        std::function<void()> test_fn,
        const char* file,
        int line
    );

    void AddResult(const TestResult& result);
    const std::vector<TestResult>& GetResults() const { return results_; }
    void Clear();
    void PrintSummary() const;
    int GetPassCount() const;
    int GetFailCount() const;
    double GetTotalTime() const;

private:
    TestRegistry() = default;
    std::vector<TestResult> results_;
};

// Internal test execution
class TestExecutor {
public:
    static void RunAllTests();
    static int GetFailCount();
    static void PrintResults();
};

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
        } \
    } while (0)

#define ASSERT_FALSE(condition) \
    do { \
        if ((condition)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #condition + " should be false"); \
        } \
    } while (0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + \
                #expected + " != " + #actual); \
        } \
    } while (0)

#define ASSERT_NE(value1, value2) \
    do { \
        if ((value1) == (value2)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + \
                #value1 + " == " + #value2); \
        } \
    } while (0)

#define ASSERT_LT(value1, value2) \
    do { \
        if (!((value1) < (value2))) { \
            throw std::runtime_error(std::string("Assertion failed: ") + \
                #value1 + " >= " + #value2); \
        } \
    } while (0)

#define ASSERT_LE(value1, value2) \
    do { \
        if (!((value1) <= (value2))) { \
            throw std::runtime_error(std::string("Assertion failed: ") + \
                #value1 + " > " + #value2); \
        } \
    } while (0)

#define ASSERT_GT(value1, value2) \
    do { \
        if (!((value1) > (value2))) { \
            throw std::runtime_error(std::string("Assertion failed: ") + \
                #value1 + " <= " + #value2); \
        } \
    } while (0)

#define ASSERT_GE(value1, value2) \
    do { \
        if (!((value1) >= (value2))) { \
            throw std::runtime_error(std::string("Assertion failed: ") + \
                #value1 + " < " + #value2); \
        } \
    } while (0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            throw std::runtime_error(std::string("Assertion failed: pointer is not null")); \
        } \
    } while (0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            throw std::runtime_error(std::string("Assertion failed: pointer is null")); \
        } \
    } while (0)

#define ASSERT_STREQ(str1, str2) \
    do { \
        if (std::strcmp((str1), (str2)) != 0) { \
            throw std::runtime_error(std::string("Assertion failed: strings not equal")); \
        } \
    } while (0)

#define ASSERT_FLOAT_EQ(expected, actual, tolerance) \
    do { \
        float diff = std::abs((expected) - (actual)); \
        if (diff > (tolerance)) { \
            throw std::runtime_error(std::string("Assertion failed: float values differ by ") + \
                std::to_string(diff)); \
        } \
    } while (0)

// Test registration macro
#define TEST(SuiteName, TestName) \
    class SuiteName##TestName##Registrar { \
    public: \
        SuiteName##TestName##Registrar() { \
            phase47::TestRegistry::Instance().RegisterTest( \
                #SuiteName, #TestName, \
                [](){ SuiteName##TestName##_impl(); }, \
                __FILE__, __LINE__ \
            ); \
        } \
    }; \
    static SuiteName##TestName##Registrar SuiteName##TestName##_registrar; \
    static void SuiteName##TestName##_impl()

// Fixture-based tests
#define TEST_F(FixtureName, TestName) \
    class FixtureName##TestName##Test : public FixtureName { \
    public: \
        void Run(); \
    }; \
    class FixtureName##TestName##Registrar { \
    public: \
        FixtureName##TestName##Registrar() { \
            phase47::TestRegistry::Instance().RegisterTest( \
                #FixtureName, #TestName, \
                [](){ FixtureName##TestName##Test test; test.Run(); }, \
                __FILE__, __LINE__ \
            ); \
        } \
    }; \
    static FixtureName##TestName##Registrar FixtureName##TestName##_registrar; \
    void FixtureName##TestName##Test::Run()

// Performance test macro
#define PERF_TEST(SuiteName, TestName) TEST(SuiteName, TestName)

// Skip macro
#define SKIP_TEST() throw std::runtime_error("Test skipped")

}  // namespace phase47
