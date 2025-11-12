/**
 * Performance Profiler & Optimization System
 * 
 * Comprehensive profiling for Vulkan graphics, frame timing analysis,
 * GPU memory optimization, and performance monitoring.
 * 
 * Handle range: 39000-39999 (1,000 handles)
 */

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Handle ranges */
#define PROFILER_HANDLE_MIN 39000
#define PROFILER_HANDLE_MAX 39999
#define PROFILER_MAX_MARKERS 256
#define PROFILER_MAX_GPU_QUERIES 128
#define PROFILER_MAX_MEMORY_SAMPLES 512
#define PROFILER_HISTORY_SIZE 60

/* Forward declarations */
typedef struct Profiler Profiler;

/* Profiler marker types */
typedef enum {
    MARKER_TYPE_FRAME = 0,          /* Frame boundary */
    MARKER_TYPE_CPU = 1,            /* CPU timing section */
    MARKER_TYPE_GPU = 2,            /* GPU timing section */
    MARKER_TYPE_MEMORY = 3,         /* Memory allocation */
    MARKER_TYPE_CUSTOM = 4          /* User-defined marker */
} MarkerType;

/* GPU query types */
typedef enum {
    GPU_QUERY_TIMESTAMP = 0,        /* Timestamp query */
    GPU_QUERY_OCCLUSION = 1,        /* Occlusion query */
    GPU_QUERY_PIPELINE = 2          /* Pipeline statistics */
} GPUQueryType;

/* Frame statistics */
typedef struct {
    uint32_t frame_number;
    double frame_time_ms;
    double cpu_time_ms;
    double gpu_time_ms;
    uint32_t draw_calls;
    uint32_t triangles_rendered;
    uint32_t vertices_processed;
    uint64_t gpu_memory_used;
    uint64_t cpu_memory_used;
    double fps;
    double cpu_utilization;
    double gpu_utilization;
} FrameStats;

/* CPU timing marker */
typedef struct {
    char name[128];
    double start_time_ms;
    double end_time_ms;
    double duration_ms;
    int depth;
    int is_active;
} CPUMarker;

/* GPU timing query */
typedef struct {
    char name[128];
    GPUQueryType type;
    uint64_t start_query;
    uint64_t end_query;
    double duration_ms;
    uint32_t sample_count;
    int is_active;
} GPUQuery;

/* Memory sample */
typedef struct {
    uint64_t timestamp;
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t peak_usage;
    uint32_t allocation_count;
    uint32_t free_count;
} MemorySample;

/* Performance target */
typedef struct {
    char name[128];
    double target_ms;
    double current_ms;
    int met;
} PerformanceTarget;

/* Profiler statistics */
typedef struct {
    uint32_t total_frames;
    double avg_frame_time_ms;
    double min_frame_time_ms;
    double max_frame_time_ms;
    double avg_fps;
    uint32_t frames_below_target;
    uint32_t frames_above_target;
    double avg_cpu_time_ms;
    double avg_gpu_time_ms;
    uint64_t peak_memory_usage;
    uint64_t avg_memory_usage;
    uint32_t total_draw_calls;
    uint32_t avg_draw_calls;
    uint32_t peak_draw_calls;
} ProfilerStatistics;

/* Lifecycle management (4 functions) */
Profiler* Profiler_Create(void);
void Profiler_Destroy(Profiler* profiler);
int Profiler_Initialize(Profiler* profiler);
void Profiler_Shutdown(Profiler* profiler);

/* Frame management (4 functions) */
int Profiler_BeginFrame(Profiler* profiler);
int Profiler_EndFrame(Profiler* profiler);
int Profiler_GetCurrentFrame(Profiler* profiler);
FrameStats Profiler_GetFrameStats(Profiler* profiler, uint32_t frame_number);

/* CPU timing (6 functions) */
int Profiler_BeginCPUMarker(Profiler* profiler, const char* marker_name);
int Profiler_EndCPUMarker(Profiler* profiler, const char* marker_name);
int Profiler_ScopedCPUMarker(Profiler* profiler, const char* marker_name, 
                             void (*fn)(void));
double Profiler_GetCPUMarkerTime(Profiler* profiler, const char* marker_name);
int Profiler_PrintCPUProfile(Profiler* profiler);
int Profiler_DumpCPUTimeline(Profiler* profiler, const char* filename);

/* GPU timing (6 functions) */
int Profiler_BeginGPUQuery(Profiler* profiler, const char* query_name, GPUQueryType type);
int Profiler_EndGPUQuery(Profiler* profiler, const char* query_name);
int Profiler_ResolveGPUQueries(Profiler* profiler);
double Profiler_GetGPUQueryTime(Profiler* profiler, const char* query_name);
int Profiler_PrintGPUProfile(Profiler* profiler);
int Profiler_DumpGPUTimeline(Profiler* profiler, const char* filename);

/* GPU memory profiling (5 functions) */
int Profiler_TrackGPUMemory(Profiler* profiler, uint64_t allocation_size, 
                            const char* allocation_name);
int Profiler_ReleaseGPUMemory(Profiler* profiler, uint64_t allocation_size);
uint64_t Profiler_GetGPUMemoryUsage(Profiler* profiler);
uint64_t Profiler_GetGPUMemoryPeak(Profiler* profiler);
int Profiler_PrintGPUMemoryReport(Profiler* profiler);

/* CPU memory profiling (5 functions) */
int Profiler_TrackCPUMemory(Profiler* profiler, uint64_t allocation_size, 
                            const char* allocation_name);
int Profiler_ReleaseCPUMemory(Profiler* profiler, uint64_t allocation_size);
uint64_t Profiler_GetCPUMemoryUsage(Profiler* profiler);
uint64_t Profiler_GetCPUMemoryPeak(Profiler* profiler);
int Profiler_PrintCPUMemoryReport(Profiler* profiler);

/* Performance targets (5 functions) */
int Profiler_SetPerformanceTarget(Profiler* profiler, const char* target_name, double target_ms);
int Profiler_ValidatePerformanceTarget(Profiler* profiler, const char* target_name, double current_ms);
int Profiler_GetPerformanceTarget(Profiler* profiler, const char* target_name, 
                                 PerformanceTarget* target);
int Profiler_PrintPerformanceTargets(Profiler* profiler);
int Profiler_GetTargetsMet(Profiler* profiler);

/* Frame analysis (6 functions) */
int Profiler_AnalyzeFrameBottleneck(Profiler* profiler, uint32_t frame_number);
int Profiler_CompareFrames(Profiler* profiler, uint32_t frame1, uint32_t frame2);
int Profiler_IdentifyPerformanceRegression(Profiler* profiler);
int Profiler_GetHottestMarker(Profiler* profiler, char* marker_name, int name_size);
int Profiler_PrintFrameAnalysis(Profiler* profiler, uint32_t frame_number);
int Profiler_ExportFrameData(Profiler* profiler, uint32_t frame_number, const char* filename);

/* Vulkan-specific profiling (5 functions) */
int Profiler_EnableVulkanTimestamps(Profiler* profiler);
int Profiler_DisableVulkanTimestamps(Profiler* profiler);
int Profiler_GetVulkanGPUTime(Profiler* profiler, const char* marker_name);
int Profiler_ValidateVulkanQueries(Profiler* profiler);
int Profiler_PrintVulkanProfile(Profiler* profiler);

/* Draw call profiling (4 functions) */
int Profiler_BeginDrawCall(Profiler* profiler);
int Profiler_EndDrawCall(Profiler* profiler);
uint32_t Profiler_GetDrawCallCount(Profiler* profiler);
int Profiler_PrintDrawCallStats(Profiler* profiler);

/* Batch optimization (4 functions) */
int Profiler_AnalyzeBatchingEfficiency(Profiler* profiler);
int Profiler_IdentifyBatchingOpportunities(Profiler* profiler);
int Profiler_GetBatchCount(Profiler* profiler);
int Profiler_PrintBatchingReport(Profiler* profiler);

/* Shader optimization (4 functions) */
int Profiler_ProfileShader(Profiler* profiler, const char* shader_name);
int Profiler_AnalyzeShaderPerformance(Profiler* profiler, const char* shader_name);
int Profiler_GetShaderTime(Profiler* profiler, const char* shader_name);
int Profiler_PrintShaderProfile(Profiler* profiler);

/* Statistics & reporting (8 functions) */
ProfilerStatistics Profiler_GetStatistics(Profiler* profiler);
double Profiler_GetAverageFrameTime(Profiler* profiler);
double Profiler_GetAverageFPS(Profiler* profiler);
uint32_t Profiler_GetFramesBelowTarget(Profiler* profiler);
int Profiler_PrintStatistics(Profiler* profiler);
int Profiler_ExportStatisticsJSON(Profiler* profiler, const char* filename);
int Profiler_ExportStatisticsCSV(Profiler* profiler, const char* filename);
int Profiler_ExportProfile(Profiler* profiler, const char* filename);

/* History & trending (4 functions) */
int Profiler_GetFrameHistory(Profiler* profiler, FrameStats* history, int history_size);
int Profiler_AnalyzePerformanceTrend(Profiler* profiler);
int Profiler_DetectPerformanceAnomalies(Profiler* profiler);
int Profiler_PrintPerformanceTrend(Profiler* profiler);

/* Configuration (4 functions) */
int Profiler_SetSamplingFrequency(Profiler* profiler, int frame_skip);
int Profiler_EnableDetailedProfiling(Profiler* profiler);
int Profiler_DisableDetailedProfiling(Profiler* profiler);
int Profiler_SetVerbosity(Profiler* profiler, int level);

/* Utility (4 functions) */
const char* Profiler_GetLastError(Profiler* profiler);
int Profiler_ResetStatistics(Profiler* profiler);
int Profiler_FlushProfile(Profiler* profiler);
int Profiler_ClearHistory(Profiler* profiler);

#ifdef __cplusplus
}
#endif

#endif /* __PROFILER_H__ */
