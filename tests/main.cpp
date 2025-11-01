#include "core/test_runner.h"
#include "core/test_macros.h"
#include "core/profiler/cpu_profiler.h"
#include "core/profiler/memory_profiler.h"
#include <iostream>
#include <iomanip>

// Simple example tests to demonstrate the framework
namespace {

TEST(ExampleTests, SimpleAssertion) {
    ASSERT_TRUE(1 + 1 == 2);
    ASSERT_EQ(5, 5);
}

TEST(ExampleTests, FloatComparison) {
    float a = 3.14159f;
    float b = 3.14160f;
    ASSERT_FLOAT_EQ(a, b, 0.001f);
}

TEST(ExampleTests, PointerTest) {
    int value = 42;
    int* ptr = &value;
    ASSERT_NOT_NULL(ptr);
    ASSERT_EQ(*ptr, 42);
}

TEST(ExampleTests, StringComparison) {
    const char* str1 = "hello";
    const char* str2 = "hello";
    ASSERT_STREQ(str1, str2);
}

}  // namespace

int main(int argc, char* argv[]) {
    phase47::TestRunner runner;
    runner.ParseArgs(argc, argv);

    // Print header
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "GeneralsX — Phase 47: Testing & Performance Optimization\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "Test Infrastructure Framework\n";
    std::cout << "═══════════════════════════════════════════════════════════\n\n";

    // Run all tests
    int failed_count = runner.RunAllTests();

    // Print profiling data
    std::cout << "\n";
    phase47::CPUProfiler::Instance().PrintReport();
    phase47::MemoryProfiler::Instance().PrintReport();

    // Generate reports
    runner.GenerateReport("test_results.txt");
    phase47::CPUProfiler::Instance().GenerateReport("cpu_profile.txt");
    phase47::MemoryProfiler::Instance().GenerateReport("memory_profile.txt");

    std::cout << "\nReports generated:\n";
    std::cout << "  - test_results.txt\n";
    std::cout << "  - cpu_profile.txt\n";
    std::cout << "  - memory_profile.txt\n";

    return failed_count > 0 ? 1 : 0;
}
