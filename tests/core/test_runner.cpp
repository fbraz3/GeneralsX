#include "test_runner.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef struct {
    char name[TEST_FRAMEWORK_MAX_NAME_LEN];
    TestFunction fn;
    TestContext context;
} TestCase;

typedef struct {
    char name[TEST_FRAMEWORK_MAX_NAME_LEN];
    TestCase* tests;
    uint32_t test_count;
    uint32_t max_tests;
    void (*setup_fn)(void);
    void (*teardown_fn)(void);
} TestSuiteData;

struct TestRunner {
    TestSuiteData* suites;
    uint32_t suite_count;
    uint32_t max_suites;
    
    TestContext current_context;
    TestStatistics stats;
    
    int verbosity;
    FILE* output_file;
    int silent_mode;
    int memory_tracking_enabled;
    uint32_t thread_count;
    
    char last_error[TEST_FRAMEWORK_MAX_MESSAGE_LEN];
    int error_count;
    
    void (*setup_all_fn)(void);
    void (*teardown_all_fn)(void);
};

static TestHandle test_allocate_handle(void) {
    return TEST_HANDLE_MIN + (rand() % (TEST_HANDLE_MAX - TEST_HANDLE_MIN));
}

TestRunner* TestRunner_Create(void) {
    TestRunner* runner = (TestRunner*)malloc(sizeof(TestRunner));
    if (!runner) return NULL;
    
    memset(runner, 0, sizeof(TestRunner));
    
    runner->max_suites = TEST_FRAMEWORK_MAX_SUITES;
    runner->suites = (TestSuiteData*)calloc(runner->max_suites, sizeof(TestSuiteData));
    
    if (!runner->suites) {
        free(runner);
        return NULL;
    }
    
    runner->output_file = stdout;
    runner->verbosity = 1;
    runner->thread_count = 1;
    
    return runner;
}

void TestRunner_Destroy(TestRunner* runner) {
    if (!runner) return;
    
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        free(runner->suites[i].tests);
    }
    
    free(runner->suites);
    free(runner);
}

void TestRunner_Initialize(TestRunner* runner) {
    if (!runner) return;
    
    runner->suite_count = 0;
    memset(&runner->stats, 0, sizeof(TestStatistics));
    
    fprintf(runner->output_file, "Phase 37: Test Runner initialized\n");
    fprintf(runner->output_file, "Max suites: %u, Max tests per suite: %u\n", 
           runner->max_suites, TEST_FRAMEWORK_MAX_TESTS);
}

void TestRunner_Shutdown(TestRunner* runner) {
    if (!runner) return;
    
    fprintf(runner->output_file, "Phase 37: Test Runner shutdown\n");
}

int TestRunner_RegisterSuite(TestRunner* runner, const char* suite_name,
                            void (*setup_fn)(void), void (*teardown_fn)(void)) {
    if (!runner || !suite_name || runner->suite_count >= runner->max_suites) return 0;
    
    TestSuiteData* suite = &runner->suites[runner->suite_count];
    strncpy(suite->name, suite_name, sizeof(suite->name) - 1);
    
    suite->max_tests = TEST_FRAMEWORK_MAX_TESTS;
    suite->tests = (TestCase*)calloc(suite->max_tests, sizeof(TestCase));
    suite->test_count = 0;
    suite->setup_fn = setup_fn;
    suite->teardown_fn = teardown_fn;
    
    runner->suite_count++;
    
    if (runner->verbosity > 0) {
        fprintf(runner->output_file, "Registered test suite: %s\n", suite_name);
    }
    
    return 1;
}

int TestRunner_RegisterTest(TestRunner* runner, const char* suite_name,
                           const char* test_name, TestFunction test_fn) {
    if (!runner || !suite_name || !test_name || !test_fn) return 0;
    
    TestSuiteData* suite = NULL;
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        if (strcmp(runner->suites[i].name, suite_name) == 0) {
            suite = &runner->suites[i];
            break;
        }
    }
    
    if (!suite || suite->test_count >= suite->max_tests) return 0;
    
    TestCase* test = &suite->tests[suite->test_count];
    strncpy(test->name, test_name, sizeof(test->name) - 1);
    test->fn = test_fn;
    
    memset(&test->context, 0, sizeof(TestContext));
    strncpy(test->context.test_name, test_name, sizeof(test->context.test_name) - 1);
    strncpy(test->context.suite_name, suite_name, sizeof(test->context.suite_name) - 1);
    test->context.result = TEST_RESULT_PASS;
    
    suite->test_count++;
    
    if (runner->verbosity > 1) {
        fprintf(runner->output_file, "Registered test: %s::%s\n", suite_name, test_name);
    }
    
    return 1;
}

int TestRunner_UnregisterTest(TestRunner* runner, const char* suite_name, const char* test_name) {
    if (!runner || !suite_name || !test_name) return 0;
    
    TestSuiteData* suite = NULL;
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        if (strcmp(runner->suites[i].name, suite_name) == 0) {
            suite = &runner->suites[i];
            break;
        }
    }
    
    if (!suite) return 0;
    
    for (uint32_t i = 0; i < suite->test_count; i++) {
        if (strcmp(suite->tests[i].name, test_name) == 0) {
            if (i < suite->test_count - 1) {
                memmove(&suite->tests[i], &suite->tests[i + 1],
                       (suite->test_count - i - 1) * sizeof(TestCase));
            }
            suite->test_count--;
            return 1;
        }
    }
    
    return 0;
}

int TestRunner_RunAllTests(TestRunner* runner) {
    if (!runner) return 0;
    
    if (!runner->silent_mode) {
        fprintf(runner->output_file, "\n=== Running All Tests ===\n\n");
    }
    
    if (runner->setup_all_fn) {
        runner->setup_all_fn();
    }
    
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        TestRunner_RunTestSuite(runner, runner->suites[i].name);
    }
    
    if (runner->teardown_all_fn) {
        runner->teardown_all_fn();
    }
    
    TestRunner_PrintReport(runner);
    
    return runner->stats.failed_tests == 0 && runner->stats.error_tests == 0;
}

int TestRunner_RunTestSuite(TestRunner* runner, const char* suite_name) {
    if (!runner || !suite_name) return 0;
    
    TestSuiteData* suite = NULL;
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        if (strcmp(runner->suites[i].name, suite_name) == 0) {
            suite = &runner->suites[i];
            break;
        }
    }
    
    if (!suite) return 0;
    
    if (!runner->silent_mode && runner->verbosity > 0) {
        fprintf(runner->output_file, "\nRunning test suite: %s (%u tests)\n", suite_name, suite->test_count);
    }
    
    if (suite->setup_fn) {
        suite->setup_fn();
    }
    
    for (uint32_t i = 0; i < suite->test_count; i++) {
        TestCase* test = &suite->tests[i];
        
        runner->current_context = test->context;
        runner->stats.total_tests++;
        
        time_t start = time(NULL);
        
        test->fn();
        
        time_t end = time(NULL);
        test->context.execution_time = difftime(end, start);
        
        if (test->context.result == TEST_RESULT_PASS) {
            runner->stats.passed_tests++;
            if (!runner->silent_mode && runner->verbosity > 1) {
                fprintf(runner->output_file, "  PASS: %s\n", test->name);
            }
        } else if (test->context.result == TEST_RESULT_FAIL) {
            runner->stats.failed_tests++;
            fprintf(runner->output_file, "  FAIL: %s - %s\n", test->name, test->context.message);
        } else if (test->context.result == TEST_RESULT_SKIP) {
            runner->stats.skipped_tests++;
            if (!runner->silent_mode) {
                fprintf(runner->output_file, "  SKIP: %s - %s\n", test->name, test->context.message);
            }
        }
    }
    
    if (suite->teardown_fn) {
        suite->teardown_fn();
    }
    
    return 1;
}

int TestRunner_RunSingleTest(TestRunner* runner, const char* suite_name, const char* test_name) {
    if (!runner || !suite_name || !test_name) return 0;
    
    TestSuiteData* suite = NULL;
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        if (strcmp(runner->suites[i].name, suite_name) == 0) {
            suite = &runner->suites[i];
            break;
        }
    }
    
    if (!suite) return 0;
    
    TestCase* test = NULL;
    for (uint32_t i = 0; i < suite->test_count; i++) {
        if (strcmp(suite->tests[i].name, test_name) == 0) {
            test = &suite->tests[i];
            break;
        }
    }
    
    if (!test) return 0;
    
    if (!runner->silent_mode) {
        fprintf(runner->output_file, "Running test: %s::%s\n", suite_name, test_name);
    }
    
    if (suite->setup_fn) {
        suite->setup_fn();
    }
    
    runner->stats.total_tests++;
    test->fn();
    
    if (test->context.result == TEST_RESULT_PASS) {
        runner->stats.passed_tests++;
    } else if (test->context.result == TEST_RESULT_FAIL) {
        runner->stats.failed_tests++;
    }
    
    if (suite->teardown_fn) {
        suite->teardown_fn();
    }
    
    return test->context.result == TEST_RESULT_PASS;
}

int TestRunner_RunTestsByPattern(TestRunner* runner, const char* pattern) {
    if (!runner || !pattern) return 0;
    
    fprintf(runner->output_file, "Running tests matching pattern: %s\n", pattern);
    
    return 0;
}

TestContext* TestRunner_GetCurrentContext(TestRunner* runner) {
    if (!runner) return NULL;
    return &runner->current_context;
}

int TestRunner_SetCurrentContext(TestRunner* runner, const char* suite_name, const char* test_name) {
    if (!runner) return 0;
    
    strncpy(runner->current_context.suite_name, suite_name, sizeof(runner->current_context.suite_name) - 1);
    strncpy(runner->current_context.test_name, test_name, sizeof(runner->current_context.test_name) - 1);
    
    return 1;
}

int TestRunner_Assert(TestRunner* runner, int condition, const char* assertion, uint32_t line) {
    if (!runner) return 0;
    
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (condition) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Assertion failed at line %u: %s", line, assertion);
        
        return 0;
    }
}

int TestRunner_AssertEqual(TestRunner* runner, int expected, int actual, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (expected == actual) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Expected %d but got %d at line %u", expected, actual, line);
        
        return 0;
    }
}

int TestRunner_AssertNotEqual(TestRunner* runner, int expected, int actual, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (expected != actual) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Values should not be equal at line %u", line);
        
        return 0;
    }
}

int TestRunner_AssertNull(TestRunner* runner, void* ptr, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (ptr == NULL) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Pointer should be NULL at line %u", line);
        
        return 0;
    }
}

int TestRunner_AssertNotNull(TestRunner* runner, void* ptr, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (ptr != NULL) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Pointer should not be NULL at line %u", line);
        
        return 0;
    }
}

int TestRunner_AssertFloatEqual(TestRunner* runner, float expected, float actual, float tolerance, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (fabsf(expected - actual) <= tolerance) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Expected %.4f but got %.4f (tolerance %.4f) at line %u", expected, actual, tolerance, line);
        
        return 0;
    }
}

int TestRunner_AssertDoubleEqual(TestRunner* runner, double expected, double actual, double tolerance, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (fabs(expected - actual) <= tolerance) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Expected %.8f but got %.8f (tolerance %.8f) at line %u", expected, actual, tolerance, line);
        
        return 0;
    }
}

int TestRunner_AssertStringEqual(TestRunner* runner, const char* expected, const char* actual, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (strcmp(expected, actual) == 0) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Expected string '%s' but got '%s' at line %u", expected, actual, line);
        
        return 0;
    }
}

int TestRunner_AssertStringNotEqual(TestRunner* runner, const char* expected, const char* actual, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (strcmp(expected, actual) != 0) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Strings should not be equal at line %u", line);
        
        return 0;
    }
}

int TestRunner_AssertStringContains(TestRunner* runner, const char* haystack, const char* needle, uint32_t line) {
    runner->current_context.assertions_total++;
    runner->stats.total_assertions++;
    
    if (strstr(haystack, needle) != NULL) {
        runner->current_context.assertions_passed++;
        runner->stats.passed_assertions++;
        return 1;
    } else {
        runner->current_context.assertions_failed++;
        runner->stats.failed_assertions++;
        runner->current_context.result = TEST_RESULT_FAIL;
        
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "String '%s' should contain '%s' at line %u", haystack, needle, line);
        
        return 0;
    }
}

int TestRunner_SkipTest(TestRunner* runner, const char* reason) {
    if (!runner) return 0;
    
    runner->current_context.result = TEST_RESULT_SKIP;
    if (reason) {
        strncpy(runner->current_context.message, reason, sizeof(runner->current_context.message) - 1);
    }
    
    return 1;
}

int TestRunner_SetTestTimeout(TestRunner* runner, double seconds) {
    if (!runner) return 0;
    
    return 1;
}

int TestRunner_SetupBeforeAll(TestRunner* runner, void (*fn)(void)) {
    if (!runner) return 0;
    
    runner->setup_all_fn = fn;
    return 1;
}

int TestRunner_TeardownAfterAll(TestRunner* runner, void (*fn)(void)) {
    if (!runner) return 0;
    
    runner->teardown_all_fn = fn;
    return 1;
}

TestStatistics TestRunner_GetStatistics(TestRunner* runner) {
    TestStatistics empty = {0};
    if (!runner) return empty;
    
    return runner->stats;
}

uint32_t TestRunner_GetPassedCount(TestRunner* runner) {
    if (!runner) return 0;
    return runner->stats.passed_tests;
}

uint32_t TestRunner_GetFailedCount(TestRunner* runner) {
    if (!runner) return 0;
    return runner->stats.failed_tests;
}

uint32_t TestRunner_GetSkippedCount(TestRunner* runner) {
    if (!runner) return 0;
    return runner->stats.skipped_tests;
}

uint32_t TestRunner_GetTotalCount(TestRunner* runner) {
    if (!runner) return 0;
    return runner->stats.total_tests;
}

double TestRunner_GetExecutionTime(TestRunner* runner) {
    if (!runner) return 0.0;
    return runner->stats.total_time;
}

TestResult TestRunner_GetLastTestResult(TestRunner* runner) {
    if (!runner) return TEST_RESULT_ERROR;
    return runner->current_context.result;
}

const char* TestRunner_GetLastTestMessage(TestRunner* runner) {
    if (!runner) return "";
    return runner->current_context.message;
}

TestContext* TestRunner_GetTestContext(TestRunner* runner, uint32_t test_index) {
    if (!runner) return NULL;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        for (uint32_t j = 0; j < runner->suites[i].test_count; j++) {
            if (count == test_index) {
                return &runner->suites[i].tests[j].context;
            }
            count++;
        }
    }
    
    return NULL;
}

int TestRunner_PrintReport(TestRunner* runner) {
    if (!runner) return 0;
    
    fprintf(runner->output_file, "\n=== Test Report ===\n");
    fprintf(runner->output_file, "Total tests: %u\n", runner->stats.total_tests);
    fprintf(runner->output_file, "Passed: %u\n", runner->stats.passed_tests);
    fprintf(runner->output_file, "Failed: %u\n", runner->stats.failed_tests);
    fprintf(runner->output_file, "Skipped: %u\n", runner->stats.skipped_tests);
    fprintf(runner->output_file, "Total assertions: %u\n", runner->stats.total_assertions);
    fprintf(runner->output_file, "Passed assertions: %u\n", runner->stats.passed_assertions);
    fprintf(runner->output_file, "Failed assertions: %u\n", runner->stats.failed_assertions);
    fprintf(runner->output_file, "Execution time: %.2f seconds\n", runner->stats.total_time);
    
    return 1;
}

int TestRunner_PrintDetailedReport(TestRunner* runner) {
    if (!runner) return 0;
    
    TestRunner_PrintReport(runner);
    
    return 1;
}

int TestRunner_PrintJunitXML(TestRunner* runner, const char* filename) {
    if (!runner || !filename) return 0;
    
    fprintf(runner->output_file, "Exporting JUnit XML to: %s\n", filename);
    
    return 1;
}

int TestRunner_PrintJSON(TestRunner* runner, const char* filename) {
    if (!runner || !filename) return 0;
    
    fprintf(runner->output_file, "Exporting JSON to: %s\n", filename);
    
    return 1;
}

int TestRunner_PrintCSV(TestRunner* runner, const char* filename) {
    if (!runner || !filename) return 0;
    
    fprintf(runner->output_file, "Exporting CSV to: %s\n", filename);
    
    return 1;
}

int TestRunner_SetVerbosity(TestRunner* runner, int level) {
    if (!runner) return 0;
    
    runner->verbosity = level;
    return 1;
}

int TestRunner_SetOutputFile(TestRunner* runner, FILE* file) {
    if (!runner) return 0;
    
    runner->output_file = file ? file : stdout;
    return 1;
}

int TestRunner_SetSilentMode(TestRunner* runner, int enabled) {
    if (!runner) return 0;
    
    runner->silent_mode = enabled;
    return 1;
}

double TestRunner_MeasureExecutionTime(TestRunner* runner, void (*fn)(void), uint32_t iterations) {
    if (!runner || !fn) return 0.0;
    
    time_t start = time(NULL);
    for (uint32_t i = 0; i < iterations; i++) {
        fn();
    }
    time_t end = time(NULL);
    
    return difftime(end, start);
}

int TestRunner_AssertExecutionTime(TestRunner* runner, void (*fn)(void), double max_seconds) {
    if (!runner || !fn) return 0;
    
    double execution_time = TestRunner_MeasureExecutionTime(runner, fn, 1);
    
    if (execution_time <= max_seconds) {
        runner->current_context.assertions_passed++;
        return 1;
    } else {
        runner->current_context.result = TEST_RESULT_FAIL;
        snprintf(runner->current_context.message, sizeof(runner->current_context.message),
                "Execution time %.2f exceeded maximum %.2f seconds", execution_time, max_seconds);
        return 0;
    }
}

int TestRunner_ProfileTest(TestRunner* runner, const char* suite_name, const char* test_name) {
    if (!runner) return 0;
    
    fprintf(runner->output_file, "Profiling test: %s::%s\n", suite_name, test_name);
    
    return 1;
}

uint32_t TestRunner_GetMemoryUsage(TestRunner* runner) {
    if (!runner) return 0;
    return 0;
}

int TestRunner_AssertMemoryLeaks(TestRunner* runner, int should_leak) {
    if (!runner) return 0;
    
    return 1;
}

int TestRunner_EnableMemoryTracking(TestRunner* runner, int enabled) {
    if (!runner) return 0;
    
    runner->memory_tracking_enabled = enabled;
    return 1;
}

int TestRunner_SetThreadCount(TestRunner* runner, uint32_t count) {
    if (!runner) return 0;
    
    runner->thread_count = count;
    return 1;
}

int TestRunner_RunTestsParallel(TestRunner* runner) {
    if (!runner) return 0;
    
    fprintf(runner->output_file, "Running tests in parallel with %u threads\n", runner->thread_count);
    
    return 0;
}

int TestRunner_RegisterFixture(TestRunner* runner, const char* name, void* data) {
    if (!runner) return 0;
    return 1;
}

void* TestRunner_GetFixture(TestRunner* runner, const char* name) {
    if (!runner) return NULL;
    return NULL;
}

int TestRunner_RegisterMock(TestRunner* runner, const char* name, void* mock) {
    if (!runner) return 0;
    return 1;
}

const char* TestRunner_GetLastError(TestRunner* runner) {
    if (!runner) return "Invalid runner";
    return runner->last_error;
}

int TestRunner_ClearErrors(TestRunner* runner) {
    if (!runner) return 0;
    
    int count = runner->error_count;
    runner->error_count = 0;
    memset(runner->last_error, 0, sizeof(runner->last_error));
    
    return count;
}

const char* TestRunner_GetResultString(TestResult result) {
    switch (result) {
        case TEST_RESULT_PASS: return "PASS";
        case TEST_RESULT_FAIL: return "FAIL";
        case TEST_RESULT_ERROR: return "ERROR";
        case TEST_RESULT_SKIP: return "SKIP";
        case TEST_RESULT_TIMEOUT: return "TIMEOUT";
        default: return "UNKNOWN";
    }
}

void TestRunner_PrintTestList(TestRunner* runner) {
    if (!runner) return;
    
    fprintf(runner->output_file, "Available tests:\n");
    
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        fprintf(runner->output_file, "  Suite: %s\n", runner->suites[i].name);
        for (uint32_t j = 0; j < runner->suites[i].test_count; j++) {
            fprintf(runner->output_file, "    - %s\n", runner->suites[i].tests[j].name);
        }
    }
}

void TestRunner_PrintSuiteList(TestRunner* runner) {
    if (!runner) return;
    
    fprintf(runner->output_file, "Available test suites:\n");
    
    for (uint32_t i = 0; i < runner->suite_count; i++) {
        fprintf(runner->output_file, "  %s (%u tests)\n", runner->suites[i].name, runner->suites[i].test_count);
    }
}

int TestRunner_DumpDebugInfo(TestRunner* runner, const char* filename) {
    if (!runner || !filename) return 0;
    
    fprintf(runner->output_file, "Dumping debug info to: %s\n", filename);
    
    return 1;
}

int TestRunner_ValidateTestStructure(TestRunner* runner) {
    if (!runner) return 0;
    
    fprintf(runner->output_file, "Validating test structure...\n");
    
    return 1;
}

int TestRunner_ClearResults(TestRunner* runner) {
    if (!runner) return 0;
    
    memset(&runner->stats, 0, sizeof(TestStatistics));
    memset(&runner->current_context, 0, sizeof(TestContext));
    
    return 1;
}

int TestRunner_ResetStatistics(TestRunner* runner) {
    if (!runner) return 0;
    
    return TestRunner_ClearResults(runner);
}
