#pragma once

#include "test_macros.h"
#include <chrono>
#include <iomanip>

namespace phase47 {

class TestRunner {
public:
    static TestRunner& Instance() {
        static TestRunner instance;
        return instance;
    }

    // Run all registered tests
    int RunAllTests();

    // Run tests from a specific category/suite
    int RunSuite(const std::string& suite_name);

    // Run a single test by name
    int RunTest(const std::string& suite_name, const std::string& test_name);

    // Generate report to file
    void GenerateReport(const std::string& filename);

    // Print summary to console
    void PrintSummary() const;

    // Get statistics
    int GetPassCount() const;
    int GetFailCount() const;
    int GetSkipCount() const;
    double GetTotalTime() const;

    // Command line parsing
    void ParseArgs(int argc, char* argv[]);

private:
    TestRunner() = default;

    struct TestInfo {
        std::string suite_name;
        std::string test_name;
        std::function<void()> test_fn;
        std::string file;
        int line;
    };

    std::vector<TestInfo> tests_;
    std::vector<TestResult> results_;
    std::string output_file_;
    bool verbose_ = false;

    void ExecuteTest(const TestInfo& test);
    TestStatus RunTestFunction(std::function<void()> fn, std::string& error);
};

}  // namespace phase47
