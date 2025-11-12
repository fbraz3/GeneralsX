/**
 * Cross-Platform Validation System
 * 
 * Provides comprehensive validation for multi-platform builds,
 * feature parity verification, performance profiling, and driver compatibility.
 * 
 * Handle range: 38000-38999 (1,000 handles)
 */

#ifndef __CROSS_PLATFORM_VALIDATOR_H__
#define __CROSS_PLATFORM_VALIDATOR_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Handle ranges */
#define VALIDATOR_HANDLE_MIN 38000
#define VALIDATOR_HANDLE_MAX 38999
#define VALIDATOR_MAX_PLATFORMS 4
#define VALIDATOR_MAX_FEATURES 64
#define VALIDATOR_MAX_PERFORMANCE_TESTS 32
#define VALIDATOR_MAX_DRIVERS 16

/* Forward declarations */
typedef struct CrossPlatformValidator CrossPlatformValidator;

/* Platform identifiers */
typedef enum {
    PLATFORM_WINDOWS = 0,
    PLATFORM_MACOS_ARM64 = 1,
    PLATFORM_MACOS_X64 = 2,
    PLATFORM_LINUX_X64 = 3,
    PLATFORM_UNKNOWN = 4
} PlatformId;

/* Build configuration types */
typedef enum {
    BUILD_CONFIG_DEBUG = 0,
    BUILD_CONFIG_RELEASE = 1,
    BUILD_CONFIG_PROFILE = 2,
    BUILD_CONFIG_SHIPPING = 3
} BuildConfiguration;

/* Graphics backend selection */
typedef enum {
    GRAPHICS_BACKEND_VULKAN = 0,
    GRAPHICS_BACKEND_METAL = 1,
    GRAPHICS_BACKEND_OPENGL = 2,
    GRAPHICS_BACKEND_DIRECTX = 3,
    GRAPHICS_BACKEND_UNKNOWN = 4
} GraphicsBackend;

/* Feature validation result codes */
typedef enum {
    FEATURE_OK = 0,
    FEATURE_NOT_AVAILABLE = 1,
    FEATURE_DEGRADED = 2,
    FEATURE_BROKEN = 3,
    FEATURE_NOT_TESTED = 4
} FeatureStatus;

/* Performance test result */
typedef struct {
    char test_name[128];
    double execution_time_ms;
    double target_time_ms;
    int passed;
    char message[256];
} PerformanceTestResult;

/* Feature validation result */
typedef struct {
    char feature_name[128];
    FeatureStatus status;
    int supported_platforms;
    char description[256];
    char details[512];
} FeatureValidationResult;

/* Driver information */
typedef struct {
    char driver_name[256];
    char driver_version[128];
    char device_name[256];
    int is_compatible;
    char notes[512];
} DriverInfo;

/* Platform build validation result */
typedef struct {
    PlatformId platform;
    int build_successful;
    int build_time_seconds;
    int warnings_count;
    int errors_count;
    char build_log[4096];
    double compile_time_ms;
    int64_t executable_size;
} PlatformBuildResult;

/* Comprehensive validation report */
typedef struct {
    uint32_t validation_id;
    time_t timestamp;
    PlatformId primary_platform;
    int total_platforms_tested;
    int platforms_passed;
    int platforms_failed;
    int total_features_validated;
    int features_passed;
    int features_failed;
    int features_degraded;
    int performance_tests_count;
    int performance_tests_passed;
    PerformanceTestResult performance_results[VALIDATOR_MAX_PERFORMANCE_TESTS];
    PlatformBuildResult build_results[VALIDATOR_MAX_PLATFORMS];
    DriverInfo driver_info[VALIDATOR_MAX_DRIVERS];
    char summary[1024];
    char recommendations[2048];
} ValidationReport;

/* Lifecycle management (4 functions) */
CrossPlatformValidator* CrossPlatformValidator_Create(void);
void CrossPlatformValidator_Destroy(CrossPlatformValidator* validator);
int CrossPlatformValidator_Initialize(CrossPlatformValidator* validator);
void CrossPlatformValidator_Shutdown(CrossPlatformValidator* validator);

/* Platform detection (5 functions) */
PlatformId CrossPlatformValidator_GetCurrentPlatform(CrossPlatformValidator* validator);
int CrossPlatformValidator_GetPlatformName(PlatformId platform, char* name, int name_size);
int CrossPlatformValidator_IsPlatformSupported(CrossPlatformValidator* validator, PlatformId platform);
int CrossPlatformValidator_GetSupportedPlatforms(CrossPlatformValidator* validator, PlatformId* platforms);
int CrossPlatformValidator_GetPlatformCount(CrossPlatformValidator* validator);

/* Graphics backend detection (4 functions) */
GraphicsBackend CrossPlatformValidator_GetCurrentGraphicsBackend(CrossPlatformValidator* validator);
int CrossPlatformValidator_IsGraphicsBackendSupported(CrossPlatformValidator* validator, 
                                                      PlatformId platform, GraphicsBackend backend);
int CrossPlatformValidator_GetSupportedBackends(CrossPlatformValidator* validator, 
                                                PlatformId platform, GraphicsBackend* backends);
int CrossPlatformValidator_GetBackendName(GraphicsBackend backend, char* name, int name_size);

/* Build validation (6 functions) */
int CrossPlatformValidator_ValidateBuild(CrossPlatformValidator* validator, 
                                         PlatformId platform, BuildConfiguration config);
int CrossPlatformValidator_ValidateAllBuilds(CrossPlatformValidator* validator);
PlatformBuildResult CrossPlatformValidator_GetBuildResult(CrossPlatformValidator* validator, 
                                                          PlatformId platform);
int CrossPlatformValidator_GetBuildStatus(CrossPlatformValidator* validator, PlatformId platform);
int CrossPlatformValidator_CompareBuildResults(CrossPlatformValidator* validator, 
                                               PlatformId platform1, PlatformId platform2);
const char* CrossPlatformValidator_GetBuildLog(CrossPlatformValidator* validator, PlatformId platform);

/* Feature validation (8 functions) */
int CrossPlatformValidator_RegisterFeature(CrossPlatformValidator* validator, 
                                          const char* feature_name, int supported_platforms);
int CrossPlatformValidator_UnregisterFeature(CrossPlatformValidator* validator, 
                                            const char* feature_name);
int CrossPlatformValidator_ValidateFeature(CrossPlatformValidator* validator, 
                                           const char* feature_name, PlatformId platform);
int CrossPlatformValidator_ValidateAllFeatures(CrossPlatformValidator* validator);
FeatureValidationResult CrossPlatformValidator_GetFeatureResult(CrossPlatformValidator* validator, 
                                                                const char* feature_name);
int CrossPlatformValidator_GetFeatureParity(CrossPlatformValidator* validator, 
                                            const char* feature_name);
int CrossPlatformValidator_GetFeatureCount(CrossPlatformValidator* validator);
int CrossPlatformValidator_PrintFeatureMatrix(CrossPlatformValidator* validator);

/* Performance profiling (7 functions) */
int CrossPlatformValidator_StartPerformanceTest(CrossPlatformValidator* validator, 
                                               const char* test_name);
int CrossPlatformValidator_EndPerformanceTest(CrossPlatformValidator* validator, 
                                             const char* test_name, double target_time_ms);
int CrossPlatformValidator_ProfileFeature(CrossPlatformValidator* validator, 
                                         const char* feature_name, void (*fn)(void));
int CrossPlatformValidator_RunPerformanceBenchmark(CrossPlatformValidator* validator, 
                                                   PlatformId platform);
PerformanceTestResult CrossPlatformValidator_GetPerformanceResult(CrossPlatformValidator* validator, 
                                                                  const char* test_name);
int CrossPlatformValidator_ComparePerformance(CrossPlatformValidator* validator, 
                                             PlatformId platform1, PlatformId platform2);
int CrossPlatformValidator_GetPerformanceTestCount(CrossPlatformValidator* validator);

/* Driver compatibility (5 functions) */
int CrossPlatformValidator_DetectDriver(CrossPlatformValidator* validator, 
                                       PlatformId platform);
int CrossPlatformValidator_ValidateDriver(CrossPlatformValidator* validator, 
                                         PlatformId platform, GraphicsBackend backend);
DriverInfo CrossPlatformValidator_GetDriverInfo(CrossPlatformValidator* validator, 
                                               PlatformId platform);
int CrossPlatformValidator_CheckDriverCompatibility(CrossPlatformValidator* validator, 
                                                    const char* driver_name, 
                                                    GraphicsBackend backend);
int CrossPlatformValidator_ListCompatibleDrivers(CrossPlatformValidator* validator, 
                                                DriverInfo* drivers);

/* Integration validation (4 functions) */
int CrossPlatformValidator_ValidateIntegration(CrossPlatformValidator* validator, 
                                               uint32_t phase_number);
int CrossPlatformValidator_ValidateAllIntegrations(CrossPlatformValidator* validator);
int CrossPlatformValidator_GetIntegrationStatus(CrossPlatformValidator* validator, 
                                               uint32_t phase_number);
int CrossPlatformValidator_PrintIntegrationReport(CrossPlatformValidator* validator);

/* Report generation (5 functions) */
ValidationReport CrossPlatformValidator_GenerateReport(CrossPlatformValidator* validator);
int CrossPlatformValidator_ExportReportJSON(CrossPlatformValidator* validator, 
                                           const char* filename);
int CrossPlatformValidator_ExportReportCSV(CrossPlatformValidator* validator, 
                                          const char* filename);
int CrossPlatformValidator_ExportReportHTML(CrossPlatformValidator* validator, 
                                           const char* filename);
void CrossPlatformValidator_PrintReport(CrossPlatformValidator* validator);

/* Utility & statistics (6 functions) */
int CrossPlatformValidator_GetValidationCount(CrossPlatformValidator* validator);
int CrossPlatformValidator_GetPassedValidations(CrossPlatformValidator* validator);
int CrossPlatformValidator_GetFailedValidations(CrossPlatformValidator* validator);
double CrossPlatformValidator_GetTotalValidationTime(CrossPlatformValidator* validator);
const char* CrossPlatformValidator_GetLastError(CrossPlatformValidator* validator);
int CrossPlatformValidator_ClearResults(CrossPlatformValidator* validator);

/* Configuration (3 functions) */
int CrossPlatformValidator_SetTargetPlatforms(CrossPlatformValidator* validator, 
                                             const PlatformId* platforms, int platform_count);
int CrossPlatformValidator_SetPerformanceTargets(CrossPlatformValidator* validator, 
                                                double* targets, int target_count);
int CrossPlatformValidator_SetVerbosity(CrossPlatformValidator* validator, int level);

#ifdef __cplusplus
}
#endif

#endif /* __CROSS_PLATFORM_VALIDATOR_H__ */
