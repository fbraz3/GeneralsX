#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Handle range for Test Infrastructure: 37000-37999 (1,000 handles) */
typedef uint32_t TestHandle;
#define TEST_HANDLE_MIN 37000
#define TEST_HANDLE_MAX 37999

#define TEST_FRAMEWORK_MAX_TESTS 500
#define TEST_FRAMEWORK_MAX_SUITES 50
#define TEST_FRAMEWORK_MAX_ASSERTIONS 10000
#define TEST_FRAMEWORK_MAX_NAME_LEN 256
#define TEST_FRAMEWORK_MAX_MESSAGE_LEN 512

/* Test result codes */
typedef enum {
    TEST_RESULT_PASS = 0,
    TEST_RESULT_FAIL = 1,
    TEST_RESULT_ERROR = 2,
    TEST_RESULT_SKIP = 3,
    TEST_RESULT_TIMEOUT = 4
} TestResult;

/* Assertion result */
typedef enum {
    ASSERT_OK = 0,
    ASSERT_FAILED = 1,
    ASSERT_ERROR = 2
} AssertionResult;

/* Test context */
typedef struct {
    char test_name[TEST_FRAMEWORK_MAX_NAME_LEN];
    char suite_name[TEST_FRAMEWORK_MAX_NAME_LEN];
    TestResult result;
    char message[TEST_FRAMEWORK_MAX_MESSAGE_LEN];
    uint32_t line_number;
    double execution_time;
    uint32_t assertions_total;
    uint32_t assertions_passed;
    uint32_t assertions_failed;
} TestContext;

/* Test statistics */
typedef struct {
    uint32_t total_tests;
    uint32_t passed_tests;
    uint32_t failed_tests;
    uint32_t skipped_tests;
    uint32_t error_tests;
    uint32_t timeout_tests;
    double total_time;
    uint32_t total_assertions;
    uint32_t passed_assertions;
    uint32_t failed_assertions;
} TestStatistics;

/* Test suite */
typedef struct {
    char name[TEST_FRAMEWORK_MAX_NAME_LEN];
    void (*setup_fn)(void);
    void (*teardown_fn)(void);
    uint32_t test_count;
} TestSuite;

/* Test function pointer */
typedef void (*TestFunction)(void);

/* Opaque test runner */
typedef struct TestRunner TestRunner;

/* System Lifecycle */
TestRunner* TestRunner_Create(void);
void TestRunner_Destroy(TestRunner* runner);
void TestRunner_Initialize(TestRunner* runner);
void TestRunner_Shutdown(TestRunner* runner);

/* Test Registration */
int TestRunner_RegisterSuite(TestRunner* runner, const char* suite_name, 
                             void (*setup_fn)(void), void (*teardown_fn)(void));
int TestRunner_RegisterTest(TestRunner* runner, const char* suite_name, 
                            const char* test_name, TestFunction test_fn);
int TestRunner_UnregisterTest(TestRunner* runner, const char* suite_name, const char* test_name);

/* Test Execution */
int TestRunner_RunAllTests(TestRunner* runner);
int TestRunner_RunTestSuite(TestRunner* runner, const char* suite_name);
int TestRunner_RunSingleTest(TestRunner* runner, const char* suite_name, const char* test_name);
int TestRunner_RunTestsByPattern(TestRunner* runner, const char* pattern);

/* Test Context Management */
TestContext* TestRunner_GetCurrentContext(TestRunner* runner);
int TestRunner_SetCurrentContext(TestRunner* runner, const char* suite_name, const char* test_name);

/* Assertion Macros - Core */
int TestRunner_Assert(TestRunner* runner, int condition, const char* assertion, uint32_t line);
int TestRunner_AssertEqual(TestRunner* runner, int expected, int actual, uint32_t line);
int TestRunner_AssertNotEqual(TestRunner* runner, int expected, int actual, uint32_t line);
int TestRunner_AssertNull(TestRunner* runner, void* ptr, uint32_t line);
int TestRunner_AssertNotNull(TestRunner* runner, void* ptr, uint32_t line);

/* Assertion Macros - Floating Point */
int TestRunner_AssertFloatEqual(TestRunner* runner, float expected, float actual, float tolerance, uint32_t line);
int TestRunner_AssertDoubleEqual(TestRunner* runner, double expected, double actual, double tolerance, uint32_t line);

/* Assertion Macros - String */
int TestRunner_AssertStringEqual(TestRunner* runner, const char* expected, const char* actual, uint32_t line);
int TestRunner_AssertStringNotEqual(TestRunner* runner, const char* expected, const char* actual, uint32_t line);
int TestRunner_AssertStringContains(TestRunner* runner, const char* haystack, const char* needle, uint32_t line);

/* Test Control */
int TestRunner_SkipTest(TestRunner* runner, const char* reason);
int TestRunner_SetTestTimeout(TestRunner* runner, double seconds);
int TestRunner_SetupBeforeAll(TestRunner* runner, void (*fn)(void));
int TestRunner_TeardownAfterAll(TestRunner* runner, void (*fn)(void));

/* Statistics & Reporting */
TestStatistics TestRunner_GetStatistics(TestRunner* runner);
uint32_t TestRunner_GetPassedCount(TestRunner* runner);
uint32_t TestRunner_GetFailedCount(TestRunner* runner);
uint32_t TestRunner_GetSkippedCount(TestRunner* runner);
uint32_t TestRunner_GetTotalCount(TestRunner* runner);
double TestRunner_GetExecutionTime(TestRunner* runner);

/* Result Retrieval */
TestResult TestRunner_GetLastTestResult(TestRunner* runner);
const char* TestRunner_GetLastTestMessage(TestRunner* runner);
TestContext* TestRunner_GetTestContext(TestRunner* runner, uint32_t test_index);

/* Reporting */
int TestRunner_PrintReport(TestRunner* runner);
int TestRunner_PrintDetailedReport(TestRunner* runner);
int TestRunner_PrintJunitXML(TestRunner* runner, const char* filename);
int TestRunner_PrintJSON(TestRunner* runner, const char* filename);
int TestRunner_PrintCSV(TestRunner* runner, const char* filename);

/* Filtering & Selection */
int TestRunner_SetVerbosity(TestRunner* runner, int level);
int TestRunner_SetOutputFile(TestRunner* runner, FILE* file);
int TestRunner_SetSilentMode(TestRunner* runner, int enabled);

/* Performance Testing */
double TestRunner_MeasureExecutionTime(TestRunner* runner, void (*fn)(void), uint32_t iterations);
int TestRunner_AssertExecutionTime(TestRunner* runner, void (*fn)(void), double max_seconds);
int TestRunner_ProfileTest(TestRunner* runner, const char* suite_name, const char* test_name);

/* Memory Testing */
uint32_t TestRunner_GetMemoryUsage(TestRunner* runner);
int TestRunner_AssertMemoryLeaks(TestRunner* runner, int should_leak);
int TestRunner_EnableMemoryTracking(TestRunner* runner, int enabled);

/* Parallel Testing */
int TestRunner_SetThreadCount(TestRunner* runner, uint32_t count);
int TestRunner_RunTestsParallel(TestRunner* runner);

/* Fixtures & Mocking */
int TestRunner_RegisterFixture(TestRunner* runner, const char* name, void* data);
void* TestRunner_GetFixture(TestRunner* runner, const char* name);
int TestRunner_RegisterMock(TestRunner* runner, const char* name, void* mock);

/* Error Handling */
const char* TestRunner_GetLastError(TestRunner* runner);
int TestRunner_ClearErrors(TestRunner* runner);
const char* TestRunner_GetResultString(TestResult result);

/* Debug & Utilities */
void TestRunner_PrintTestList(TestRunner* runner);
void TestRunner_PrintSuiteList(TestRunner* runner);
int TestRunner_DumpDebugInfo(TestRunner* runner, const char* filename);
int TestRunner_ValidateTestStructure(TestRunner* runner);

/* Cleanup */
int TestRunner_ClearResults(TestRunner* runner);
int TestRunner_ResetStatistics(TestRunner* runner);

#ifdef __cplusplus
}
#endif

#endif /* TEST_RUNNER_H */
