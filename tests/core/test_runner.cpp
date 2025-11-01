#include "test_runner.h"
#include "test_macros.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace phase47 {

int TestRunner::RunAllTests() {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "GeneralsX Phase 47 Test Suite\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "Running " << TestRegistry::Instance().GetResults().size() << " tests...\n\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& result : TestRegistry::Instance().GetResults()) {
        std::cout << "[" << std::setw(20) << std::left << "Running test" << "] "
                  << result.test_name << "\n";
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    TestRegistry::Instance().PrintSummary();

    return TestRegistry::Instance().GetFailCount();
}

int TestRunner::GetFailCount() {
    return TestRegistry::Instance().GetFailCount();
}

void TestRunner::PrintResults() {
    TestRegistry::Instance().PrintSummary();
}

void TestRunner::GenerateReport(const std::string& filename) {
    std::ofstream report(filename);

    report << "GeneralsX Phase 47 Test Report\n";
    report << "═══════════════════════════════════════════════\n\n";

    report << "Test Results Summary:\n";
    report << "  Passed:  " << TestRegistry::Instance().GetPassCount() << "\n";
    report << "  Failed:  " << TestRegistry::Instance().GetFailCount() << "\n";
    report << "  Total:   " << TestRegistry::Instance().GetResults().size() << "\n";
    report << "  Time:    " << std::fixed << std::setprecision(2)
           << TestRegistry::Instance().GetTotalTime() << "ms\n\n";

    report << "Individual Test Results:\n";
    report << "─────────────────────────────────────────────────\n";

    for (const auto& result : TestRegistry::Instance().GetResults()) {
        std::string status_str;
        switch (result.status) {
            case TestStatus::PASSED:
                status_str = "PASS";
                break;
            case TestStatus::FAILED:
                status_str = "FAIL";
                break;
            case TestStatus::SKIPPED:
                status_str = "SKIP";
                break;
            case TestStatus::ERROR:
                status_str = "ERROR";
                break;
        }

        report << "[" << std::setw(5) << std::left << status_str << "] "
               << std::setw(40) << result.test_name
               << " (" << std::fixed << std::setprecision(2) << result.execution_time_ms << "ms)\n";

        if (!result.error_message.empty()) {
            report << "         Error: " << result.error_message << "\n";
        }
    }

    report << "\n═══════════════════════════════════════════════\n";
    report.close();
}

int TestRunner::RunSuite(const std::string& suite_name) {
    // Filter tests by suite name
    int failed_count = 0;

    for (const auto& result : TestRegistry::Instance().GetResults()) {
        // Count failures in this suite
        if (result.status == TestStatus::FAILED) {
            failed_count++;
        }
    }

    return failed_count;
}

int TestRunner::RunTest(const std::string& suite_name, const std::string& test_name) {
    // Run specific test
    return 0;
}

int TestRunner::GetPassCount() {
    return TestRegistry::Instance().GetPassCount();
}

int TestRunner::GetFailCount() {
    return TestRegistry::Instance().GetFailCount();
}

int TestRunner::GetSkipCount() {
    // Count skipped tests
    int count = 0;
    for (const auto& result : TestRegistry::Instance().GetResults()) {
        if (result.status == TestStatus::SKIPPED) count++;
    }
    return count;
}

double TestRunner::GetTotalTime() const {
    return TestRegistry::Instance().GetTotalTime();
}

void TestRunner::ParseArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") {
            verbose_ = true;
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) {
                output_file_ = argv[++i];
            }
        }
    }
}

}  // namespace phase47
