/**
 * @file graphics_backend_dxvk_perf.cpp
 * 
 * Phase 43.5: Performance Monitoring & Profiling
 * 
 * Implements comprehensive performance tracking:
 * - FPS counter with rolling average
 * - Frame time statistics (min, max, average)
 * - GPU timestamp queries (Phase 45 advanced)
 * - Performance profiling and reporting
 * - Real-time metrics display
 * - Performance history tracking
 * 
 * Architecture:
 * - Circular buffer for frame time history (last 300 frames)
 * - High-resolution timing with chrono
 * - Per-stage timing breakdown (GPU submit, present, etc)
 * - Rolling averages for FPS stability
 * 
 * Phase Breakdown:
 * - 43.1: Basic render loop (BeginScene/EndScene/Present)
 * - 43.2: Frame synchronization & timing
 * - 43.3: Command recording & render pass
 * - 43.4: Error handling & swapchain recreation
 * - 43.5: Performance monitoring & profiling (this file)
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <cmath>
#include <algorithm>

// ============================================================================
// Phase 43.5: Performance Statistics Storage
// ============================================================================

// Performance history: last 300 frames
static constexpr size_t PERF_HISTORY_SIZE = 300;
static float s_frameTimeHistory[PERF_HISTORY_SIZE] = {0.0f};
static size_t s_historyIndex = 0;
static uint32_t s_framesRecorded = 0;

// ============================================================================
// Phase 43.5: FPS Counter
// ============================================================================

/**
 * Get current FPS (frames per second).
 * Calculated as: 1000.0 / average_frame_time_ms
 * Updated once per frame.
 * 
 * @return Current FPS (typically 60.0)
 */
float DXVKGraphicsBackend::GetCurrentFPS() const {
    if (m_frameCount == 0) {
        return 0.0f;
    }
    
    float avgFrameTime = MeasureFrameTime();
    if (avgFrameTime > 0.001f) {  // Avoid division by zero
        return 1000.0f / avgFrameTime;
    }
    
    return 60.0f;  // Default to target FPS
}

/**
 * Get rolling average FPS over recent frames.
 * Smoother metric than instantaneous FPS.
 * Uses last 60 frames (1 second at 60 FPS).
 * 
 * @return Average FPS over recent frames
 */
float DXVKGraphicsBackend::GetAverageFPS() const {
    if (s_framesRecorded == 0) {
        return 0.0f;
    }
    
    // Calculate average over recorded frames
    size_t sampleCount = std::min(s_framesRecorded, (uint32_t)60);
    float totalTime = 0.0f;
    
    for (size_t i = 0; i < sampleCount; i++) {
        size_t idx = (s_historyIndex + PERF_HISTORY_SIZE - sampleCount + i) % PERF_HISTORY_SIZE;
        totalTime += s_frameTimeHistory[idx];
    }
    
    float avgTime = totalTime / sampleCount;
    if (avgTime > 0.001f) {
        return 1000.0f / avgTime;
    }
    
    return 60.0f;
}

// ============================================================================
// Phase 43.5: Frame Time Statistics
// ============================================================================

/**
 * Get minimum frame time from history.
 * Useful for identifying best-case performance.
 * 
 * @return Minimum frame time in milliseconds
 */
float DXVKGraphicsBackend::GetMinFrameTime() const {
    if (s_framesRecorded == 0) {
        return 0.0f;
    }
    
    float minTime = s_frameTimeHistory[0];
    for (size_t i = 1; i < std::min(s_framesRecorded, (uint32_t)PERF_HISTORY_SIZE); i++) {
        minTime = std::min(minTime, s_frameTimeHistory[i]);
    }
    
    return minTime;
}

/**
 * Get maximum frame time from history.
 * Identifies worst-case frames (frame hitches).
 * 
 * @return Maximum frame time in milliseconds
 */
float DXVKGraphicsBackend::GetMaxFrameTime() const {
    if (s_framesRecorded == 0) {
        return 0.0f;
    }
    
    float maxTime = s_frameTimeHistory[0];
    for (size_t i = 1; i < std::min(s_framesRecorded, (uint32_t)PERF_HISTORY_SIZE); i++) {
        maxTime = std::max(maxTime, s_frameTimeHistory[i]);
    }
    
    return maxTime;
}

/**
 * Get average frame time over history.
 * Most important performance metric.
 * 
 * @return Average frame time in milliseconds
 */
float DXVKGraphicsBackend::GetAverageFrameTime() const {
    if (s_framesRecorded == 0) {
        return 0.0f;
    }
    
    float totalTime = 0.0f;
    size_t count = std::min(s_framesRecorded, (uint32_t)PERF_HISTORY_SIZE);
    
    for (size_t i = 0; i < count; i++) {
        totalTime += s_frameTimeHistory[i];
    }
    
    return totalTime / count;
}

/**
 * Record current frame time in history.
 * Called once per frame after Present().
 * Maintains rolling window for statistics.
 * 
 * @param frameTimeMs Frame time in milliseconds
 */
void DXVKGraphicsBackend::RecordFrameTime(float frameTimeMs) {
    s_frameTimeHistory[s_historyIndex] = frameTimeMs;
    s_historyIndex = (s_historyIndex + 1) % PERF_HISTORY_SIZE;
    
    if (s_framesRecorded < PERF_HISTORY_SIZE) {
        s_framesRecorded++;
    }
}

// ============================================================================
// Phase 43.5: Per-Stage Timing Breakdown
// ============================================================================

// Per-stage timing (Phase 43.5 basic tracking)
struct FrameTimingBreakdownData {
    float acquireTime;      // vkAcquireNextImageKHR
    float submitTime;       // vkQueueSubmit
    float presentTime;      // vkQueuePresentKHR
    float totalTime;        // Total frame time
};

static FrameTimingBreakdownData s_lastFrameBreakdown = {0.0f, 0.0f, 0.0f, 0.0f};

/**
 * Record stage timing for frame (acquire time).
 * 
 * @param timeMs Time in milliseconds
 */
void DXVKGraphicsBackend::RecordAcquireTime(float timeMs) {
    s_lastFrameBreakdown.acquireTime = timeMs;
}

/**
 * Record stage timing for frame (submit time).
 * 
 * @param timeMs Time in milliseconds
 */
void DXVKGraphicsBackend::RecordSubmitTime(float timeMs) {
    s_lastFrameBreakdown.submitTime = timeMs;
}

/**
 * Record stage timing for frame (present time).
 * 
 * @param timeMs Time in milliseconds
 */
void DXVKGraphicsBackend::RecordPresentTime(float timeMs) {
    s_lastFrameBreakdown.presentTime = timeMs;
}

/**
 * Get last frame's timing breakdown.
 * Useful for identifying bottlenecks.
 * 
 * @return Struct with timing for each stage
 */
DXVKGraphicsBackend::FrameTimingBreakdown DXVKGraphicsBackend::GetLastFrameBreakdown() const {
    FrameTimingBreakdown breakdown;
    breakdown.acquireTime = s_lastFrameBreakdown.acquireTime;
    breakdown.submitTime = s_lastFrameBreakdown.submitTime;
    breakdown.presentTime = s_lastFrameBreakdown.presentTime;
    breakdown.totalTime = s_lastFrameBreakdown.totalTime;
    return breakdown;
}

// ============================================================================
// Phase 43.5: 99th Percentile Latency
// ============================================================================

/**
 * Get 99th percentile frame time (worst 1% of frames).
 * Critical for identifying occasional hitches.
 * Example: if P99 = 35ms at 60 FPS target (16.67ms),
 * means 1% of frames exceed 35ms.
 * 
 * @return 99th percentile frame time in milliseconds
 */
float DXVKGraphicsBackend::GetPercentileFrameTime(float percentile) const {
    if (s_framesRecorded == 0) {
        return 0.0f;
    }
    
    // Create sorted copy of frame times
    size_t count = std::min(s_framesRecorded, (uint32_t)PERF_HISTORY_SIZE);
    float sortedTimes[PERF_HISTORY_SIZE];
    std::copy(s_frameTimeHistory, s_frameTimeHistory + count, sortedTimes);
    std::sort(sortedTimes, sortedTimes + count);
    
    // Calculate index for percentile (0.0-100.0)
    size_t idx = static_cast<size_t>((percentile / 100.0f) * (count - 1));
    return sortedTimes[idx];
}

// ============================================================================
// Phase 43.5: GPU Timestamp Queries (Stub)
// ============================================================================

/**
 * Get GPU time spent on last frame (stub for Phase 45).
 * Full implementation requires VkQueryPool and timestamp queries.
 * 
 * @return GPU time in milliseconds (0.0 for Phase 43)
 */
float DXVKGraphicsBackend::GetGPUFrameTime() const {
    // Phase 45: Implement with vkCmdWriteTimestamp
    return 0.0f;
}

/**
 * Get GPU utilization percentage (stub).
 * 
 * @return GPU utilization 0-100% (0.0 for Phase 43)
 */
float DXVKGraphicsBackend::GetGPUUtilization() const {
    // Phase 45: Calculate from timestamp deltas
    return 0.0f;
}

// ============================================================================
// Phase 43.5: Performance Reporting
// ============================================================================

/**
 * Generate performance report string for display.
 * Includes FPS, frame time, and statistics.
 * 
 * @param buffer Output buffer
 * @param bufferSize Maximum buffer size
 * @return Pointer to buffer
 */
const char* DXVKGraphicsBackend::GeneratePerformanceReport(char* buffer, size_t bufferSize) const {
    float fps = GetCurrentFPS();
    float avgFps = GetAverageFPS();
    float avgFrameTime = GetAverageFrameTime();
    float minFrameTime = GetMinFrameTime();
    float maxFrameTime = GetMaxFrameTime();
    float p99FrameTime = GetPercentileFrameTime(99.0f);
    
    snprintf(buffer, bufferSize,
        "=== Performance Report (Frame %u) ===\n"
        "Current FPS: %.1f\n"
        "Average FPS: %.1f\n"
        "Avg Frame Time: %.2f ms\n"
        "Min/Max Frame Time: %.2f / %.2f ms\n"
        "99th Percentile: %.2f ms\n"
        "Frames in History: %u\n",
        m_frameCount,
        fps,
        avgFps,
        avgFrameTime,
        minFrameTime,
        maxFrameTime,
        p99FrameTime,
        s_framesRecorded
    );
    
    return buffer;
}

/**
 * Print performance statistics to console.
 * Useful for debugging and profiling.
 */
void DXVKGraphicsBackend::PrintPerformanceStats() const {
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║     Phase 43.5: Performance Monitoring     ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Frame Metrics:\n");
    printf("  Current FPS:         %.1f\n", GetCurrentFPS());
    printf("  Average FPS:         %.1f\n", GetAverageFPS());
    printf("  Current Frame Time:  %.2f ms\n", MeasureFrameTime());
    printf("  Average Frame Time:  %.2f ms\n", GetAverageFrameTime());
    printf("  Min Frame Time:      %.2f ms\n", GetMinFrameTime());
    printf("  Max Frame Time:      %.2f ms\n", GetMaxFrameTime());
    printf("  99th Percentile:     %.2f ms\n", GetPercentileFrameTime(99.0f));
    printf("\n");
    printf("Frame Budget Analysis:\n");
    printf("  Target FPS:          60.0\n");
    printf("  Frame Time Budget:   16.67 ms\n");
    printf("  Budget Headroom:     %.2f ms\n", 16.67f - GetAverageFrameTime());
    printf("  Frames Over Budget:  %u / %u (%.1f%%)\n",
        CountFramesOverBudget(),
        s_framesRecorded,
        (s_framesRecorded > 0) ? (100.0f * CountFramesOverBudget() / s_framesRecorded) : 0.0f
    );
    printf("\n");
    printf("Stage Timing (Last Frame):\n");
    printf("  Acquire:             %.2f ms\n", s_lastFrameBreakdown.acquireTime);
    printf("  Submit:              %.2f ms\n", s_lastFrameBreakdown.submitTime);
    printf("  Present:             %.2f ms\n", s_lastFrameBreakdown.presentTime);
    printf("  Total:               %.2f ms\n", s_lastFrameBreakdown.totalTime);
    printf("\n");
}

// ============================================================================
// Phase 43.5: Performance Analysis
// ============================================================================

/**
 * Count how many frames exceeded the frame time budget (16.67ms for 60 FPS).
 * High count indicates performance issues.
 * 
 * @return Number of frames over budget
 */
uint32_t DXVKGraphicsBackend::CountFramesOverBudget() const {
    const float FRAME_BUDGET = 16.67f;
    uint32_t count = 0;
    
    size_t sampleCount = std::min(s_framesRecorded, (uint32_t)PERF_HISTORY_SIZE);
    for (size_t i = 0; i < sampleCount; i++) {
        if (s_frameTimeHistory[i] > FRAME_BUDGET) {
            count++;
        }
    }
    
    return count;
}

/**
 * Identify performance bottleneck in rendering.
 * Returns which stage took the most time.
 * 
 * @return Bottleneck stage description
 */
const char* DXVKGraphicsBackend::IdentifyBottleneck() const {
    FrameTimingBreakdownData breakdown = s_lastFrameBreakdown;
    
    float maxTime = breakdown.acquireTime;
    const char* bottleneck = "Acquire";
    
    if (breakdown.submitTime > maxTime) {
        maxTime = breakdown.submitTime;
        bottleneck = "Submit";
    }
    
    if (breakdown.presentTime > maxTime) {
        maxTime = breakdown.presentTime;
        bottleneck = "Present";
    }
    
    return bottleneck;
}

/**
 * Generate performance warning if performance degraded.
 * 
 * @return Warning message or empty string if performance good
 */
const char* DXVKGraphicsBackend::GetPerformanceWarning() const {
    float avgFrameTime = GetAverageFrameTime();
    float maxFrameTime = GetMaxFrameTime();
    float p99FrameTime = GetPercentileFrameTime(99.0f);
    
    // Critical: P99 > 25ms (way over 60 FPS budget)
    if (p99FrameTime > 25.0f) {
        return "WARNING: 99th percentile frame time > 25ms (critical)";
    }
    
    // Warning: Average > 17ms (slightly over budget)
    if (avgFrameTime > 17.0f) {
        return "WARNING: Average frame time > 17ms (degraded performance)";
    }
    
    // Info: Max time spike detected
    if (maxFrameTime > 50.0f) {
        return "INFO: Frame time spike detected (check for GC or I/O)";
    }
    
    return "";  // Performance good
}

// ============================================================================
// Phase 43.5: Performance History Reset
// ============================================================================

/**
 * Reset performance statistics.
 * Called on application start or when resetting metrics.
 */
void DXVKGraphicsBackend::ResetPerformanceStats() {
    std::memset(s_frameTimeHistory, 0, sizeof(s_frameTimeHistory));
    s_historyIndex = 0;
    s_framesRecorded = 0;
    std::memset(&s_lastFrameBreakdown, 0, sizeof(s_lastFrameBreakdown));
    
    printf("Performance statistics reset\n");
}

/**
 * Get performance history size.
 * 
 * @return Number of frames tracked in history
 */
uint32_t DXVKGraphicsBackend::GetPerformanceHistorySize() const {
    return s_framesRecorded;
}
