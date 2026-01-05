/**
 * @file Profiler.h
 * @brief Performance Profiling and Analysis System
 *
 * Phase 39: Optimization & Profiling
 *
 * Comprehensive profiling system for CPU timing, GPU profiling (Vulkan),
 * memory tracking, draw call analysis, and performance target validation.
 *
 * Features:
 * - Hierarchical CPU timing markers
 * - GPU query integration (timestamps, occlusion, pipeline stats)
 * - GPU and CPU memory tracking
 * - Performance target definition and validation
 * - Frame analysis and bottleneck detection
 * - Draw call and batching analysis
 * - Shader profiling
 * - Statistics aggregation and trending
 * - Export to JSON/CSV formats
 *
 * Usage:
 * @code
 *  Profiler* profiler = Profiler_Create();
 *  Profiler_Initialize(profiler);
 *  Profiler_SetPerformanceTarget(profiler, "Frame", 16.67);
 *
 *  while (running) {
 *      Profiler_BeginFrame(profiler);
 *      Profiler_BeginCPUMarker(profiler, "Update");
 *      update_game_state();
 *      Profiler_EndCPUMarker(profiler, "Update");
 *      Profiler_EndFrame(profiler);
 *  }
 *
 *  Profiler_PrintStatistics(profiler);
 *  Profiler_ExportStatisticsJSON(profiler, "profile.json");
 *  Profiler_Destroy(profiler);
 * @endcode
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup profiler_types Profiler Types
 * @{
 */

/** Opaque profiler handle */
typedef struct Profiler Profiler;

/** Handle range for profiler instances */
#define PROFILER_HANDLE_MIN 39000
#define PROFILER_HANDLE_MAX 39999

/** Maximum nested markers in a frame */
#define PROFILER_MAX_MARKERS 1024

/** Maximum GPU queries per frame */
#define PROFILER_MAX_GPU_QUERIES 512

/** Frame history buffer size (rolling 60-frame buffer) */
#define PROFILER_FRAME_HISTORY_SIZE 60

/** Marker type enumeration */
typedef enum {
    MARKER_TYPE_FRAME = 0,      /**< Frame boundary marker */
    MARKER_TYPE_CPU = 1,        /**< CPU timing section */
    MARKER_TYPE_GPU = 2,        /**< GPU timing section */
    MARKER_TYPE_MEMORY = 3,     /**< Memory allocation/deallocation */
    MARKER_TYPE_CUSTOM = 4      /**< User-defined marker */
} MarkerType;

/** GPU query type enumeration */
typedef enum {
    GPU_QUERY_TIMESTAMP = 0,    /**< GPU timestamp query */
    GPU_QUERY_OCCLUSION = 1,    /**< Occlusion query (pixels rendered) */
    GPU_QUERY_PIPELINE = 2      /**< Pipeline statistics query */
} GPUQueryType;

/** Frame statistics structure */
typedef struct {
    uint32_t frame_number;           /**< Frame number (0-based) */
    double frame_time_ms;            /**< Total frame time in milliseconds */
    double cpu_time_ms;              /**< CPU processing time */
    double gpu_time_ms;              /**< GPU processing time */
    uint32_t draw_calls;             /**< Number of draw calls this frame */
    uint32_t triangles_rendered;     /**< Triangles rendered */
    uint32_t vertices_processed;     /**< Vertices processed */
    uint64_t gpu_memory_used;        /**< GPU memory currently used */
    uint64_t cpu_memory_used;        /**< CPU memory currently used */
    double fps;                      /**< Frames per second */
    double cpu_utilization;          /**< CPU utilization percentage */
    double gpu_utilization;          /**< GPU utilization percentage */
} FrameStats;

/** Performance target structure */
typedef struct {
    char target_name[64];            /**< Target name (e.g., "Frame", "Render") */
    double target_ms;                /**< Target time in milliseconds */
    int frames_met;                  /**< Number of frames that met target */
    int frames_missed;               /**< Number of frames that missed target */
    double success_rate;             /**< Success rate percentage (0-100) */
} PerformanceTarget;

/** Profiler statistics structure */
typedef struct {
    uint32_t total_frames;           /**< Total frames profiled */
    double avg_frame_time_ms;        /**< Average frame time */
    double min_frame_time_ms;        /**< Minimum frame time */
    double max_frame_time_ms;        /**< Maximum frame time */
    double avg_fps;                  /**< Average FPS */
    uint32_t frames_below_target;    /**< Frames below target threshold */
    uint32_t frames_above_target;    /**< Frames above target threshold */
    double avg_cpu_time_ms;          /**< Average CPU time */
    double avg_gpu_time_ms;          /**< Average GPU time */
    uint64_t peak_memory_usage;      /**< Peak memory usage */
    uint64_t avg_memory_usage;       /**< Average memory usage */
    uint32_t total_draw_calls;       /**< Total draw calls across all frames */
    uint32_t avg_draw_calls;         /**< Average draw calls per frame */
    uint32_t peak_draw_calls;        /**< Peak draw calls in single frame */
} ProfilerStatistics;

/** @} */

/**
 * @defgroup profiler_lifecycle Lifecycle Management
 * @{
 */

/**
 * Create a new profiler instance
 * @return Pointer to new profiler, or NULL on failure
 */
Profiler* Profiler_Create(void);

/**
 * Destroy a profiler instance
 * @param profiler Profiler instance (can be NULL)
 */
void Profiler_Destroy(Profiler* profiler);

/**
 * Initialize profiler subsystems
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_Initialize(Profiler* profiler);

/**
 * Shutdown profiler subsystems gracefully
 * @param profiler Profiler instance
 */
void Profiler_Shutdown(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_frame Frame Management
 * @{
 */

/**
 * Mark the beginning of a frame
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_BeginFrame(Profiler* profiler);

/**
 * Mark the end of a frame and finalize frame statistics
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_EndFrame(Profiler* profiler);

/**
 * Get the current frame number (0-based)
 * @param profiler Profiler instance
 * @return Current frame number, or -1 on error
 */
int Profiler_GetCurrentFrame(Profiler* profiler);

/**
 * Get statistics for a specific frame
 * @param profiler Profiler instance
 * @param frame_number Frame number to query
 * @return FrameStats structure with frame data
 */
FrameStats Profiler_GetFrameStats(Profiler* profiler, uint32_t frame_number);

/** @} */

/**
 * @defgroup profiler_cpu_timing CPU Profiling
 * @{
 */

/**
 * Begin CPU timing marker (hierarchical)
 * @param profiler Profiler instance
 * @param marker_name Name of timing marker
 * @return 0 on success, negative on failure
 */
int Profiler_BeginCPUMarker(Profiler* profiler, const char* marker_name);

/**
 * End CPU timing marker
 * @param profiler Profiler instance
 * @param marker_name Name of timing marker (must match BeginCPUMarker)
 * @return 0 on success, negative on failure
 */
int Profiler_EndCPUMarker(Profiler* profiler, const char* marker_name);

/**
 * Scoped CPU marker (automatically end when function returns)
 * Typically implemented as a macro wrapping a function call
 * @param profiler Profiler instance
 * @param marker_name Name of timing marker
 * @param fn Function to execute within scope
 * @return 0 on success, negative on failure
 */
int Profiler_ScopedCPUMarker(Profiler* profiler, const char* marker_name, 
                             void (*fn)(void));

/**
 * Get accumulated CPU time for a marker
 * @param profiler Profiler instance
 * @param marker_name Name of marker to query
 * @return Time in milliseconds, or -1.0 on error
 */
double Profiler_GetCPUMarkerTime(Profiler* profiler, const char* marker_name);

/**
 * Print CPU profiling results to stdout
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintCPUProfile(Profiler* profiler);

/**
 * Dump CPU timeline to file (CSV format)
 * @param profiler Profiler instance
 * @param filename Output filename
 * @return 0 on success, negative on failure
 */
int Profiler_DumpCPUTimeline(Profiler* profiler, const char* filename);

/** @} */

/**
 * @defgroup profiler_gpu_timing GPU Profiling
 * @{
 */

/**
 * Begin GPU query (requires GPU query support)
 * @param profiler Profiler instance
 * @param query_name Name of GPU query
 * @param type Type of GPU query (timestamp, occlusion, etc)
 * @return 0 on success, negative on failure
 */
int Profiler_BeginGPUQuery(Profiler* profiler, const char* query_name, 
                           GPUQueryType type);

/**
 * End GPU query
 * @param profiler Profiler instance
 * @param query_name Name of GPU query
 * @return 0 on success, negative on failure
 */
int Profiler_EndGPUQuery(Profiler* profiler, const char* query_name);

/**
 * Resolve GPU queries (retrieve results from GPU)
 * May require frame synchronization
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_ResolveGPUQueries(Profiler* profiler);

/**
 * Get GPU time for a query
 * @param profiler Profiler instance
 * @param query_name Name of query
 * @return Time in milliseconds, or -1.0 on error
 */
double Profiler_GetGPUQueryTime(Profiler* profiler, const char* query_name);

/**
 * Print GPU profiling results to stdout
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintGPUProfile(Profiler* profiler);

/**
 * Dump GPU timeline to file (CSV format)
 * @param profiler Profiler instance
 * @param filename Output filename
 * @return 0 on success, negative on failure
 */
int Profiler_DumpGPUTimeline(Profiler* profiler, const char* filename);

/** @} */

/**
 * @defgroup profiler_gpu_memory GPU Memory Tracking
 * @{
 */

/**
 * Track GPU memory allocation
 * @param profiler Profiler instance
 * @param allocation_size Size of allocation in bytes
 * @param allocation_name Name/category of allocation
 * @return 0 on success, negative on failure
 */
int Profiler_TrackGPUMemory(Profiler* profiler, uint64_t allocation_size, 
                            const char* allocation_name);

/**
 * Track GPU memory deallocation
 * @param profiler Profiler instance
 * @param allocation_size Size of deallocation in bytes
 * @return 0 on success, negative on failure
 */
int Profiler_ReleaseGPUMemory(Profiler* profiler, uint64_t allocation_size);

/**
 * Get current GPU memory usage
 * @param profiler Profiler instance
 * @return Memory usage in bytes
 */
uint64_t Profiler_GetGPUMemoryUsage(Profiler* profiler);

/**
 * Get peak GPU memory usage
 * @param profiler Profiler instance
 * @return Peak memory usage in bytes
 */
uint64_t Profiler_GetGPUMemoryPeak(Profiler* profiler);

/**
 * Print GPU memory usage report
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintGPUMemoryReport(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_cpu_memory CPU Memory Tracking
 * @{
 */

/**
 * Track CPU memory allocation
 * @param profiler Profiler instance
 * @param allocation_size Size of allocation in bytes
 * @param allocation_name Name/category of allocation
 * @return 0 on success, negative on failure
 */
int Profiler_TrackCPUMemory(Profiler* profiler, uint64_t allocation_size, 
                            const char* allocation_name);

/**
 * Track CPU memory deallocation
 * @param profiler Profiler instance
 * @param allocation_size Size of deallocation in bytes
 * @return 0 on success, negative on failure
 */
int Profiler_ReleaseCPUMemory(Profiler* profiler, uint64_t allocation_size);

/**
 * Get current CPU memory usage
 * @param profiler Profiler instance
 * @return Memory usage in bytes
 */
uint64_t Profiler_GetCPUMemoryUsage(Profiler* profiler);

/**
 * Get peak CPU memory usage
 * @param profiler Profiler instance
 * @return Peak memory usage in bytes
 */
uint64_t Profiler_GetCPUMemoryPeak(Profiler* profiler);

/**
 * Print CPU memory usage report
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintCPUMemoryReport(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_targets Performance Targets
 * @{
 */

/**
 * Set a named performance target
 * @param profiler Profiler instance
 * @param target_name Name of performance target (e.g., "Frame", "Render")
 * @param target_ms Target time in milliseconds
 * @return 0 on success, negative on failure
 */
int Profiler_SetPerformanceTarget(Profiler* profiler, const char* target_name, 
                                  double target_ms);

/**
 * Validate performance against target
 * @param profiler Profiler instance
 * @param target_name Target name
 * @param current_ms Current measured time in milliseconds
 * @return 1 if target met, 0 if missed, negative on error
 */
int Profiler_ValidatePerformanceTarget(Profiler* profiler, 
                                       const char* target_name, 
                                       double current_ms);

/**
 * Get performance target info
 * @param profiler Profiler instance
 * @param target_name Target name
 * @param target Output structure (can be NULL)
 * @return 0 on success, negative on failure
 */
int Profiler_GetPerformanceTarget(Profiler* profiler, const char* target_name, 
                                  PerformanceTarget* target);

/**
 * Print all performance targets
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintPerformanceTargets(Profiler* profiler);

/**
 * Get number of targets currently met
 * @param profiler Profiler instance
 * @return Number of targets met in current frame
 */
int Profiler_GetTargetsMet(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_analysis Frame Analysis
 * @{
 */

/**
 * Analyze frame for performance bottleneck
 * @param profiler Profiler instance
 * @param frame_number Frame to analyze
 * @return 0 on success, negative on failure
 */
int Profiler_AnalyzeFrameBottleneck(Profiler* profiler, uint32_t frame_number);

/**
 * Compare two frames for performance differences
 * @param profiler Profiler instance
 * @param frame1 First frame number
 * @param frame2 Second frame number
 * @return 0 on success, negative on failure
 */
int Profiler_CompareFrames(Profiler* profiler, uint32_t frame1, uint32_t frame2);

/**
 * Identify performance regression in recent frames
 * @param profiler Profiler instance
 * @return 1 if regression detected, 0 if no regression, negative on error
 */
int Profiler_IdentifyPerformanceRegression(Profiler* profiler);

/**
 * Get the most expensive (hottest) marker
 * @param profiler Profiler instance
 * @param marker_name Output buffer for marker name
 * @param name_size Size of marker_name buffer
 * @return 0 on success, negative on failure
 */
int Profiler_GetHottestMarker(Profiler* profiler, char* marker_name, 
                              int name_size);

/**
 * Print detailed frame analysis
 * @param profiler Profiler instance
 * @param frame_number Frame to analyze
 * @return 0 on success, negative on failure
 */
int Profiler_PrintFrameAnalysis(Profiler* profiler, uint32_t frame_number);

/**
 * Export detailed frame data to file
 * @param profiler Profiler instance
 * @param frame_number Frame to export
 * @param filename Output filename
 * @return 0 on success, negative on failure
 */
int Profiler_ExportFrameData(Profiler* profiler, uint32_t frame_number, 
                             const char* filename);

/** @} */

/**
 * @defgroup profiler_vulkan Vulkan-Specific Profiling
 * @{
 */

/**
 * Enable Vulkan timestamp queries
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_EnableVulkanTimestamps(Profiler* profiler);

/**
 * Disable Vulkan timestamp queries
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_DisableVulkanTimestamps(Profiler* profiler);

/**
 * Get GPU time from Vulkan timestamp
 * @param profiler Profiler instance
 * @param marker_name Marker name
 * @return Time in milliseconds, or -1.0 on error
 */
double Profiler_GetVulkanGPUTime(Profiler* profiler, const char* marker_name);

/**
 * Validate Vulkan query integrity
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_ValidateVulkanQueries(Profiler* profiler);

/**
 * Print Vulkan-specific profiling information
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintVulkanProfile(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_draw_calls Draw Call Profiling
 * @{
 */

/**
 * Begin tracking a draw call
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_BeginDrawCall(Profiler* profiler);

/**
 * End tracking current draw call
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_EndDrawCall(Profiler* profiler);

/**
 * Get draw call count for current frame
 * @param profiler Profiler instance
 * @return Number of draw calls
 */
uint32_t Profiler_GetDrawCallCount(Profiler* profiler);

/**
 * Print draw call statistics
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintDrawCallStats(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_batching Batching Analysis & Optimization
 * @{
 */

/**
 * Analyze batching efficiency
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_AnalyzeBatchingEfficiency(Profiler* profiler);

/**
 * Identify batching opportunities for optimization
 * @param profiler Profiler instance
 * @return Number of opportunities identified, negative on error
 */
int Profiler_IdentifyBatchingOpportunities(Profiler* profiler);

/**
 * Get batch count
 * @param profiler Profiler instance
 * @return Number of batches
 */
int Profiler_GetBatchCount(Profiler* profiler);

/**
 * Print batching efficiency report
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintBatchingReport(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_shaders Shader Profiling
 * @{
 */

/**
 * Profile a specific shader
 * @param profiler Profiler instance
 * @param shader_name Name of shader to profile
 * @return 0 on success, negative on failure
 */
int Profiler_ProfileShader(Profiler* profiler, const char* shader_name);

/**
 * Analyze shader performance
 * @param profiler Profiler instance
 * @param shader_name Name of shader
 * @return 0 on success, negative on failure
 */
int Profiler_AnalyzeShaderPerformance(Profiler* profiler, 
                                      const char* shader_name);

/**
 * Get shader execution time
 * @param profiler Profiler instance
 * @param shader_name Name of shader
 * @return Time in milliseconds, or -1.0 on error
 */
double Profiler_GetShaderTime(Profiler* profiler, const char* shader_name);

/**
 * Print shader profiling results
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintShaderProfile(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_statistics Statistics & Reporting
 * @{
 */

/**
 * Get aggregated profiling statistics
 * @param profiler Profiler instance
 * @return ProfilerStatistics structure
 */
ProfilerStatistics Profiler_GetStatistics(Profiler* profiler);

/**
 * Get average frame time across all profiled frames
 * @param profiler Profiler instance
 * @return Average frame time in milliseconds
 */
double Profiler_GetAverageFrameTime(Profiler* profiler);

/**
 * Get average frames per second
 * @param profiler Profiler instance
 * @return Average FPS
 */
double Profiler_GetAverageFPS(Profiler* profiler);

/**
 * Get count of frames below performance target
 * @param profiler Profiler instance
 * @return Number of frames below target
 */
uint32_t Profiler_GetFramesBelowTarget(Profiler* profiler);

/**
 * Print aggregated statistics to stdout
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintStatistics(Profiler* profiler);

/**
 * Export statistics to JSON file
 * @param profiler Profiler instance
 * @param filename Output JSON filename
 * @return 0 on success, negative on failure
 */
int Profiler_ExportStatisticsJSON(Profiler* profiler, const char* filename);

/**
 * Export statistics to CSV file
 * @param profiler Profiler instance
 * @param filename Output CSV filename
 * @return 0 on success, negative on failure
 */
int Profiler_ExportStatisticsCSV(Profiler* profiler, const char* filename);

/**
 * Export complete profile data
 * @param profiler Profiler instance
 * @param filename Output filename
 * @return 0 on success, negative on failure
 */
int Profiler_ExportProfile(Profiler* profiler, const char* filename);

/** @} */

/**
 * @defgroup profiler_history History & Trending
 * @{
 */

/**
 * Get frame history buffer (rolling history of frames)
 * @param profiler Profiler instance
 * @param history Output array of FrameStats
 * @param history_size Size of history array
 * @return Number of frames in history, negative on error
 */
int Profiler_GetFrameHistory(Profiler* profiler, FrameStats* history, 
                             int history_size);

/**
 * Analyze performance trend over time
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_AnalyzePerformanceTrend(Profiler* profiler);

/**
 * Detect performance anomalies in history
 * @param profiler Profiler instance
 * @return Number of anomalies detected, negative on error
 */
int Profiler_DetectPerformanceAnomalies(Profiler* profiler);

/**
 * Print performance trend analysis
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_PrintPerformanceTrend(Profiler* profiler);

/** @} */

/**
 * @defgroup profiler_config Configuration
 * @{
 */

/**
 * Set sampling frequency (profile every Nth frame)
 * @param profiler Profiler instance
 * @param frame_skip Skip this many frames (0 = profile every frame)
 * @return 0 on success, negative on failure
 */
int Profiler_SetSamplingFrequency(Profiler* profiler, int frame_skip);

/**
 * Enable detailed profiling (slower but more data)
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_EnableDetailedProfiling(Profiler* profiler);

/**
 * Disable detailed profiling
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_DisableDetailedProfiling(Profiler* profiler);

/**
 * Set verbosity level (0 = quiet, 5 = verbose)
 * @param profiler Profiler instance
 * @param level Verbosity level
 * @return 0 on success, negative on failure
 */
int Profiler_SetVerbosity(Profiler* profiler, int level);

/** @} */

/**
 * @defgroup profiler_utility Utility Functions
 * @{
 */

/**
 * Get last error message
 * @param profiler Profiler instance
 * @return Error message string, or empty string if no error
 */
const char* Profiler_GetLastError(Profiler* profiler);

/**
 * Reset all profiling statistics
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_ResetStatistics(Profiler* profiler);

/**
 * Flush profiling data to storage
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_FlushProfile(Profiler* profiler);

/**
 * Clear frame history buffer
 * @param profiler Profiler instance
 * @return 0 on success, negative on failure
 */
int Profiler_ClearHistory(Profiler* profiler);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PROFILER_H */
