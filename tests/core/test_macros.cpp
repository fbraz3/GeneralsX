#include "test_macros.h"
#include <chrono>
#include <iostream>
#include <iomanip>

namespace phase47 {

void TestRegistry::RegisterTest(
    const std::string& suite_name,
    const std::string& test_name,
    std::function<void()> test_fn,
    const char* file,
    int line
) {
    // Store test information in a map for later execution
    // This is a simplified version - full implementation would be in TestRunner
    auto& instance = Instance();
    // Implementation would store test details
}

void TestRegistry::AddResult(const TestResult& result) {
    results_.push_back(result);
}

void TestRegistry::Clear() {
    results_.clear();
}

void TestRegistry::PrintSummary() const {
    int passed = 0, failed = 0, skipped = 0;
    double total_time = 0.0;

    for (const auto& result : results_) {
        switch (result.status) {
            case TestStatus::PASSED:
                passed++;
                break;
            case TestStatus::FAILED:
                failed++;
                break;
            case TestStatus::SKIPPED:
                skipped++;
                break;
            default:
                break;
        }
        total_time += result.execution_time_ms;
    }

    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "Test Results Summary\n";
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "Passed:  " << std::setw(3) << passed << "\n";
    std::cout << "Failed:  " << std::setw(3) << failed << "\n";
    std::cout << "Skipped: " << std::setw(3) << skipped << "\n";
    std::cout << "Total:   " << std::setw(3) << results_.size() << "\n";
    std::cout << "Time:    " << std::fixed << std::setprecision(2) << total_time << "ms\n";
    std::cout << "═══════════════════════════════════════════════\n";

    if (failed > 0) {
        std::cout << "\nFailed Tests:\n";
        for (const auto& result : results_) {
            if (result.status == TestStatus::FAILED) {
                std::cout << "  - " << result.test_name << " (" << result.file_path
                          << ":" << result.line_number << ")\n";
                if (!result.error_message.empty()) {
                    std::cout << "    Error: " << result.error_message << "\n";
                }
            }
        }
    }
}

int TestRegistry::GetPassCount() const {
    int count = 0;
    for (const auto& result : results_) {
        if (result.status == TestStatus::PASSED) count++;
    }
    return count;
}

int TestRegistry::GetFailCount() const {
    int count = 0;
    for (const auto& result : results_) {
        if (result.status == TestStatus::FAILED) count++;
    }
    return count;
}

double TestRegistry::GetTotalTime() const {
    double total = 0.0;
    for (const auto& result : results_) {
        total += result.execution_time_ms;
    }
    return total;
}

}  // namespace phase47
