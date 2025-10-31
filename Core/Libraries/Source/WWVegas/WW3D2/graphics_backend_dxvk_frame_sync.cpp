/**
 * @file graphics_backend_dxvk_frame_sync.cpp
 * 
 * Phase 43.2: Frame Synchronization & Timing
 * 
 * Implements GPU-CPU synchronization primitives and frame pacing:
 * - Wait-before-overwrite pattern using fences
 * - Frame time measurement and pacing
 * - 60 FPS target with millisecond precision
 * - Multiple in-flight frames management
 * 
 * Architecture:
 * - Per-frame fences prevent CPU from overwriting GPU-active resources
 * - Per-frame semaphores coordinate GPU pipeline stages
 * - Frame time tracking with high-resolution timing
 * - Adaptive frame pacing (sleep vs spin-wait)
 * 
 * Phase Breakdown:
 * - 43.1: Basic render loop (BeginScene/EndScene/Present)
 * - 43.2: Frame synchronization & timing (this file)
 * - 43.3: Command recording & render pass
 * - 43.4: Error handling & swapchain recreation
 * - 43.5: Performance monitoring & profiling
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <chrono>
#include <thread>

// ============================================================================
// Phase 43.2: Frame Timing
// ============================================================================

// Target frame rate (60 FPS = 16.667ms per frame)
static constexpr float TARGET_FPS = 60.0f;
static constexpr uint64_t TARGET_FRAME_TIME_MS = 1000 / static_cast<uint64_t>(TARGET_FPS);  // ~16ms
static constexpr uint64_t TARGET_FRAME_TIME_US = TARGET_FRAME_TIME_MS * 1000;  // ~16666 microseconds

/**
 * Initialize frame synchronization structures.
 * Called once during device initialization.
 * Prepares fences, semaphores, and timing state for per-frame sync.
 * 
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::InitializeFrameSynchronization() {
    if (!m_device) {
        printf("ERROR: Device not initialized for frame sync\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("Initializing Frame Synchronization...\n");
    printf("  Target FPS: %.1f\n", TARGET_FPS);
    printf("  Frame time budget: %llu ms\n", TARGET_FRAME_TIME_MS);
    
    // Ensure vectors are properly sized
    uint32_t numFrames = m_inFlightFences.size();
    if (numFrames == 0) {
        printf("ERROR: No in-flight fences initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("  Frames in flight: %u\n", numFrames);
    
    // Initialize frame timing state
    m_frameStartTime = std::chrono::high_resolution_clock::now();
    m_frameCount = 0;
    
    printf("Frame synchronization initialized successfully\n");
    return S_OK;
}

/**
 * Shutdown frame synchronization.
 * Called during device cleanup.
 * 
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::ShutdownFrameSynchronization() {
    printf("Shutting down Frame Synchronization\n");
    printf("  Total frames rendered: %u\n", m_frameCount);
    
    // Fences are destroyed in main device shutdown
    return S_OK;
}

// ============================================================================
// Phase 43.2: CPU-GPU Synchronization (Wait-Before-Overwrite Pattern)
// ============================================================================

/**
 * Wait for a specific frame to complete GPU work before reusing resources.
 * This is the core of the "wait-before-overwrite" pattern:
 * 
 * 1. Check if fence is signaled (GPU finished frame)
 * 2. If not, wait for fence with optional timeout
 * 3. Reset fence for next use
 * 
 * Usage:
 * ```cpp
 * // Before reusing frame resources
 * WaitForFrame(frameIndex, UINT64_MAX);  // Wait indefinitely
 * ```
 * 
 * @param frameIndex Which frame to wait for (0-2 typically)
 * @param timeoutNs Timeout in nanoseconds (UINT64_MAX = infinite)
 * @return S_OK if frame complete, D3DERR_DEVICELOST on error
 */
HRESULT DXVKGraphicsBackend::WaitForFrame(uint32_t frameIndex, uint64_t timeoutNs) {
    if (frameIndex >= m_inFlightFences.size()) {
        printf("ERROR: Frame index %u out of range (max %zu)\n", frameIndex, m_inFlightFences.size() - 1);
        return D3DERR_DEVICELOST;
    }
    
    VkFence fence = m_inFlightFences[frameIndex];
    if (fence == VK_NULL_HANDLE) {
        printf("ERROR: Fence for frame %u is null\n", frameIndex);
        return D3DERR_DEVICELOST;
    }
    
    // Wait for GPU to finish rendering this frame
    VkResult vkr = vkWaitForFences(m_device, 1, &fence, VK_TRUE, timeoutNs);
    
    if (vkr == VK_TIMEOUT) {
        printf("WARNING: Timeout waiting for frame %u GPU work\n", frameIndex);
        return D3DERR_DEVICELOST;  // Could wait again or return timeout
    }
    
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkWaitForFences failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    // Reset fence for next frame submission
    vkr = vkResetFences(m_device, 1, &fence);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkResetFences failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    return S_OK;
}

/**
 * Check if a frame has completed without blocking.
 * Useful for non-blocking frame status queries.
 * 
 * @param frameIndex Which frame to check
 * @return true if frame GPU work is complete, false if still in progress
 */
bool DXVKGraphicsBackend::IsFrameComplete(uint32_t frameIndex) const {
    if (frameIndex >= m_inFlightFences.size()) {
        return false;
    }
    
    VkFence fence = m_inFlightFences[frameIndex];
    if (fence == VK_NULL_HANDLE) {
        return false;
    }
    
    // Check if fence is signaled (non-blocking)
    VkResult vkr = vkGetFenceStatus(m_device, fence);
    return (vkr == VK_SUCCESS);  // VK_SUCCESS = fence is signaled
}

// ============================================================================
// Phase 43.2: Frame Pacing & Timing
// ============================================================================

/**
 * Pace the frame to target frame rate (60 FPS).
 * Called at end of frame presentation to maintain consistent timing.
 * 
 * Strategy:
 * 1. Measure actual frame time
 * 2. If too fast, sleep or spin-wait to reach target time
 * 3. Track frame time for performance monitoring
 * 
 * @return Frame time actually spent (milliseconds)
 */
uint64_t DXVKGraphicsBackend::PaceFrameToTargetFPS() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    uint64_t elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        currentTime - m_frameStartTime).count();
    uint64_t elapsedMs = elapsedNs / 1000000;  // Convert to milliseconds
    
    // Check if we're ahead of schedule
    if (elapsedMs < TARGET_FRAME_TIME_MS) {
        uint64_t sleepMs = TARGET_FRAME_TIME_MS - elapsedMs;
        
        // Use hybrid approach: sleep for most of it, spin-wait for last bit
        if (sleepMs > 2) {
            // Sleep for (sleepMs - 1.5ms), then spin-wait the rest
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs - 1));
        }
        
        // Spin-wait for remaining time (more precise)
        auto targetTime = m_frameStartTime + std::chrono::milliseconds(TARGET_FRAME_TIME_MS);
        while (std::chrono::high_resolution_clock::now() < targetTime) {
            // Busy-wait for nanosecond precision
        }
        
        elapsedMs = TARGET_FRAME_TIME_MS;
    }
    
    // Mark start of next frame
    m_frameStartTime = std::chrono::high_resolution_clock::now();
    
    return elapsedMs;
}

/**
 * Get the time budget remaining in current frame (milliseconds).
 * Useful for load balancing physics, AI, etc within frame time budget.
 * 
 * @return Milliseconds remaining until frame time budget exceeded
 */
int64_t DXVKGraphicsBackend::GetFrameTimeBudgetRemaining() const {
    auto currentTime = std::chrono::high_resolution_clock::now();
    uint64_t elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        currentTime - m_frameStartTime).count();
    uint64_t elapsedMs = elapsedNs / 1000000;
    
    int64_t budgetRemaining = static_cast<int64_t>(TARGET_FRAME_TIME_MS) - static_cast<int64_t>(elapsedMs);
    return budgetRemaining;
}

/**
 * Measure actual frame time from start to present.
 * Used for performance profiling and FPS calculation.
 * 
 * @return Frame time in milliseconds
 */
float DXVKGraphicsBackend::MeasureFrameTime() const {
    auto currentTime = std::chrono::high_resolution_clock::now();
    uint64_t elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        currentTime - m_frameStartTime).count();
    
    return static_cast<float>(elapsedNs) / 1000000.0f;  // Convert to ms
}

// ============================================================================
// Phase 43.2: Multi-Frame Resource Management
// ============================================================================

/**
 * Get the current frame index for round-robin resource management.
 * Cycles through 0, 1, 2 (or however many frames are in flight).
 * 
 * Used to index into:
 * - m_inFlightFences[frameIndex]
 * - m_imageAvailableSemaphores[frameIndex]
 * - m_renderFinishedSemaphores[frameIndex]
 * - m_commandBuffers[frameIndex]
 * 
 * @return Current frame index
 */
uint32_t DXVKGraphicsBackend::GetCurrentFrameIndex() const {
    uint32_t numFrames = static_cast<uint32_t>(m_inFlightFences.size());
    if (numFrames == 0) {
        return 0;
    }
    return m_frameCount % numFrames;
}

/**
 * Advance to next frame index after Present().
 * Called automatically by render loop, increments frame counter.
 * 
 * @return New frame index
 */
uint32_t DXVKGraphicsBackend::AdvanceToNextFrame() {
    m_frameCount++;
    return GetCurrentFrameIndex();
}

/**
 * Get total number of frames rendered since initialization.
 * 
 * @return Frame counter
 */
uint32_t DXVKGraphicsBackend::GetFrameCount() const {
    return m_frameCount;
}

// ============================================================================
// Phase 43.2: Synchronization State Queries
// ============================================================================

/**
 * Get number of frames in flight (max concurrent GPU frames).
 * Typically 2 or 3 for good CPU-GPU parallelism.
 * 
 * @return Number of frames
 */
uint32_t DXVKGraphicsBackend::GetMaxFramesInFlight() const {
    return static_cast<uint32_t>(m_inFlightFences.size());
}

/**
 * Report detailed synchronization state for debugging.
 * 
 * Output shows:
 * - Current frame index
 * - Which frames have GPU work pending
 * - Frame time budget usage
 * - Fence status
 */
void DXVKGraphicsBackend::ReportSynchronizationState() const {
    uint32_t currentFrame = GetCurrentFrameIndex();
    printf("\n=== Synchronization State ===\n");
    printf("Current frame: %u\n", currentFrame);
    printf("Total frames: %u\n", m_frameCount);
    printf("Frames in flight: %u\n", GetMaxFramesInFlight());
    printf("Frame time budget: %llu ms\n", TARGET_FRAME_TIME_MS);
    printf("Frame time used: %.2f ms\n", MeasureFrameTime());
    printf("Time remaining: %lld ms\n", GetFrameTimeBudgetRemaining());
    
    printf("\nFrame status:\n");
    for (uint32_t i = 0; i < GetMaxFramesInFlight(); i++) {
        bool complete = IsFrameComplete(i);
        printf("  Frame %u: %s\n", i, complete ? "COMPLETE" : "IN-FLIGHT");
    }
    printf("\n");
}
