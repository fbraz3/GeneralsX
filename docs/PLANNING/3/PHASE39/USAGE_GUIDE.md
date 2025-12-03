# Phase 39: Profiler API - Usage Guide

## Quick Start

### Basic Initialization
```c
#include "Profiler.h"

/* Create and initialize profiler */
Profiler* profiler = Profiler_Create();
if (Profiler_Initialize(profiler) < 0) {
    printf("Failed to initialize profiler\n");
    return 1;
}

/* Set performance targets */
Profiler_SetPerformanceTarget(profiler, "Frame", 16.67);      /* 60 FPS */
Profiler_SetPerformanceTarget(profiler, "Render", 10.0);      /* Leave headroom */
Profiler_SetPerformanceTarget(profiler, "Physics", 4.0);

/* Main game loop */
while (game_running) {
    Profiler_BeginFrame(profiler);
    
    /* Game code here */
    update_game_state();
    render_frame();
    
    Profiler_EndFrame(profiler);
}

/* Generate reports */
Profiler_PrintStatistics(profiler);
Profiler_ExportStatisticsJSON(profiler, "profile.json");

/* Cleanup */
Profiler_Shutdown(profiler);
Profiler_Destroy(profiler);
```

## Common Patterns

### Pattern 1: CPU Timing Markers

```c
/* Time a code section */
Profiler_BeginCPUMarker(profiler, "Update");

/* ... update logic ... */

Profiler_EndCPUMarker(profiler, "Update");

/* Get timing */
double update_time = Profiler_GetCPUMarkerTime(profiler, "Update");
printf("Update took: %.2f ms\n", update_time);
```

### Pattern 2: Nested Markers (Hierarchical)

```c
Profiler_BeginCPUMarker(profiler, "Render");
{
    Profiler_BeginCPUMarker(profiler, "RenderGeometry");
    render_geometry();
    Profiler_EndCPUMarker(profiler, "RenderGeometry");
    
    Profiler_BeginCPUMarker(profiler, "RenderUI");
    render_ui();
    Profiler_EndCPUMarker(profiler, "RenderUI");
}
Profiler_EndCPUMarker(profiler, "Render");

/* Output hierarchy */
Profiler_PrintCPUProfile(profiler);
/* Output:
   Render: 12.3ms
     RenderGeometry: 8.2ms
     RenderUI: 2.1ms
   Other: 2.0ms
*/
```

### Pattern 3: GPU Timing (Vulkan)

```c
/* Enable Vulkan timestamps */
Profiler_EnableVulkanTimestamps(profiler);

/* Record GPU work */
Profiler_BeginGPUQuery(profiler, "RenderPass", GPU_QUERY_TIMESTAMP);
render_pass();
Profiler_EndGPUQuery(profiler, "RenderPass");

/* Resolve query results */
Profiler_ResolveGPUQueries(profiler);

/* Get GPU time */
double gpu_time = Profiler_GetGPUQueryTime(profiler, "RenderPass");
printf("GPU render pass: %.2f ms\n", gpu_time);
```

### Pattern 4: Draw Call Tracking

```c
Profiler_BeginFrame(profiler);

for (int i = 0; i < num_draw_calls; i++) {
    Profiler_BeginDrawCall(profiler);
    execute_draw_call(i);
    Profiler_EndDrawCall(profiler);
}

Profiler_EndFrame(profiler);

/* Analyze batching */
Profiler_AnalyzeBatchingEfficiency(profiler);
Profiler_PrintDrawCallStats(profiler);

uint32_t count = Profiler_GetDrawCallCount(profiler);
printf("Draw calls per frame: %u\n", count);
```

### Pattern 5: Memory Tracking

```c
/* GPU allocation */
void* texture = allocate_gpu_texture(1024, 1024, D3DFMT_A8R8G8B8);
Profiler_TrackGPUMemory(profiler, 4 * 1024 * 1024, "Texture_MainScene");

/* Later: deallocation */
free_gpu_texture(texture);
Profiler_ReleaseGPUMemory(profiler, 4 * 1024 * 1024);

/* Report */
uint64_t current = Profiler_GetGPUMemoryUsage(profiler);
uint64_t peak = Profiler_GetGPUMemoryPeak(profiler);
printf("GPU Memory: %llu MB / Peak: %llu MB\n", 
       current / (1024*1024), peak / (1024*1024));
```

### Pattern 6: Performance Bottleneck Detection

```c
/* Run profiling */
for (int frame = 0; frame < 300; frame++) {
    Profiler_BeginFrame(profiler);
    update_game_state();
    render_frame();
    Profiler_EndFrame(profiler);
}

/* Find bottleneck */
Profiler_AnalyzeFrameBottleneck(profiler, 0);
Profiler_PrintFrameAnalysis(profiler, 0);

/* Get hottest operation */
char hottest[128];
Profiler_GetHottestMarker(profiler, hottest, sizeof(hottest));
printf("Hottest operation: %s\n", hottest);
```

### Pattern 7: Frame Comparison

```c
/* Capture frame 50 (slow) and frame 100 (fast) */
Profiler_BeginFrame(profiler);
/* ... game code ... */
Profiler_EndFrame(profiler);  /* Frame 50 - 22.3ms */

/* ... more frames ... */

Profiler_BeginFrame(profiler);
/* ... game code ... */
Profiler_EndFrame(profiler);  /* Frame 100 - 15.2ms */

/* Compare */
Profiler_CompareFrames(profiler, 50, 100);
Profiler_PrintFrameAnalysis(profiler, 50);

/* Output:
   Frame 50 vs Frame 100:
     Delta: +7.1ms (+46.7% slower)
     Draw Calls: 1,245 vs 890 (+39.3% more)
     GPU Memory: 456MB vs 420MB (+8.6%)
*/
```

### Pattern 8: Batch Efficiency Analysis

```c
Profiler_AnalyzeBatchingEfficiency(profiler);

/* Get batch count */
int batch_count = Profiler_GetBatchCount(profiler);
printf("Current batches: %d\n", batch_count);

/* Identify opportunities */
Profiler_IdentifyBatchingOpportunities(profiler);
Profiler_PrintBatchingReport(profiler);

/* Output might show:
   Material Batching Opportunities:
   
   1. Terrain_Sand: 2,234 calls → 4 batches
      Savings: 2,230 draw calls (99.8% reduction)
      
   2. Building_Metal: 1,456 calls → 6 batches
      Savings: 1,450 draw calls (99.6% reduction)
      
   Total Savings: 4,600 draw calls (56% reduction)
   Estimated FPS Gain: 8-12ms improvement
*/
```

### Pattern 9: Performance Regression Detection

```c
/* Establish baseline (first 30 frames) */
for (int frame = 0; frame < 30; frame++) {
    Profiler_BeginFrame(profiler);
    /* ... game code ... */
    Profiler_EndFrame(profiler);
}

/* Run longer profiling */
for (int frame = 30; frame < 300; frame++) {
    Profiler_BeginFrame(profiler);
    /* ... game code ... */
    Profiler_EndFrame(profiler);
    
    /* Check for regression every 10 frames */
    if (frame % 10 == 0) {
        if (Profiler_IdentifyPerformanceRegression(profiler) > 0) {
            printf("REGRESSION at frame %d!\n", frame);
            Profiler_PrintFrameAnalysis(profiler, frame);
        }
    }
}
```

### Pattern 10: Scoped Marker with Callback

```c
/* Define a function to profile */
void expensive_operation() {
    /* ... heavy computation ... */
}

/* Profile it using callback */
Profiler_ScopedCPUMarker(profiler, "ExpensiveOp", expensive_operation);

/* Equivalent to:
   Profiler_BeginCPUMarker(profiler, "ExpensiveOp");
   expensive_operation();
   Profiler_EndCPUMarker(profiler, "ExpensiveOp");
*/
```

## Performance Report Examples

### Frame Statistics Output

```
═══════════════════════════════════════════════════════════
                    Frame Statistics
───────────────────────────────────────────────────────────
Total Frames:                                 300
Average Frame Time:                        16.34 ms
Min Frame Time:                            15.23 ms
Max Frame Time:                            18.67 ms
Average FPS:                               61.19 FPS
───────────────────────────────────────────────────────────
Average CPU Time:                           8.45 ms
Average GPU Time:                           6.23 ms
Average Memory Usage:                      456.7 MB
Peak Memory Usage:                         1,234 MB
───────────────────────────────────────────────────────────
Total Draw Calls:                        3,612,420
Average Draw Calls/Frame:                  12,041
Peak Draw Calls:                           15,678
───────────────────────────────────────────────────────────
Frames ≤ Target (16.67ms):                   298 (99.3%)
Frames > Target:                               2 (0.7%)
═══════════════════════════════════════════════════════════
```

### CPU Profile Hierarchy

```
Total Frame Time: 16.34 ms

Profiler_Results_CPU [16.34ms] {

    HandleInput [0.23ms] (1.4%)
    
    UpdateLogic [1.84ms] (11.3%) {
        UpdateUnits [1.12ms] {
            AISystem [0.62ms]
            PhysicsSystem [0.50ms]
        }
        UpdateCamera [0.45ms]
        UpdateEffects [0.27ms]
    }
    
    Render [12.45ms] (76.2%) {
        PrepareRenderLists [1.23ms]
        SubmitDrawCalls [0.89ms]
        RenderGeometry [8.23ms] {
            RenderTerrain [2.50ms]
            RenderBuildings [3.15ms]
            RenderUnits [2.58ms]
        }
        PostProcess [2.10ms]
    }
    
    Other [0.82ms] (5.0%)
}
```

### GPU Timeline (Vulkan)

```
GPU Execution Timeline
─────────────────────────────────────────────
0.00ms  ─────┬──────────────────────────────── 6.23ms
            │
        (Vertex Processing)
       2.34ms ┌────────────────────┐
            │ Vertex Shading     │ 1.23ms
            └────────────────────┘
                                   
       (Fragment Processing)
       3.57ms ┌──────────────────────────────┐
            │ Fragment Shading             │ 2.15ms
            └──────────────────────────────┘
                                           
       (Texture & Blending)
       5.72ms ┌─────────────┐
            │ Texture Ops │ 0.51ms
            └─────────────┘

       (Synchronization)
       6.23ms ──(GPU Ready)──
─────────────────────────────────────────────
```

### Memory Report

```
Memory Profiling Report
═══════════════════════════════════════════════════════════

GPU Memory Allocations (Current: 1,234 MB):

    Texture_MainScene             256 MB (20.8%)
    Texture_ShadowMaps            128 MB (10.4%)
    Texture_UI                     64 MB  (5.2%)
    VertexBuffer_Terrain          256 MB (20.8%)
    VertexBuffer_Geometry         192 MB (15.5%)
    VertexBuffer_Units            128 MB (10.4%)
    ConstantBuffers                64 MB  (5.2%)
    ShaderCache                    48 MB  (3.9%)
    RenderTargets                  48 MB  (3.9%)
    ───────────────────────────────────────
    Total Used:                 1,234 MB
    Peak Usage:                 1,456 MB
    Available:                  2,048 MB
    Memory Pressure:           60.2%

CPU Memory Allocations (Current: 234 MB):

    GameState                      64 MB (27.4%)
    RenderList                     48 MB (20.5%)
    EntityPool                     56 MB (23.9%)
    ParticleSystem                 32 MB (13.7%)
    UI                             16 MB  (6.8%)
    Other                          18 MB  (7.7%)
    ───────────────────────────────────────
    Total Used:                  234 MB
    Peak Usage:                  298 MB
    System Available:            8,192 MB
```

### Batching Report

```
Draw Call Batching Analysis
═══════════════════════════════════════════════════════════

Current State:
    Total Draw Calls:     12,341
    Theoretical Minimum:  1,200
    Batching Efficiency:  9.7%

Top 10 Unbatched Materials:

    1. Terrain_Sand
       Current Calls: 2,234
       Potential Batches: 4
       Per-Batch Size: 558
       Savings: 2,230 calls (99.8%)
       Impact: +3.2ms improvement

    2. Building_Metal
       Current Calls: 1,456
       Potential Batches: 6
       Per-Batch Size: 242
       Savings: 1,450 calls (99.6%)
       Impact: +1.8ms improvement

    3. Unit_Infantry
       Current Calls: 956
       Potential Batches: 9
       Per-Batch Size: 106
       Savings: 947 calls (99.1%)
       Impact: +0.9ms improvement

    [... 7 more ...]

    Total Optimization Potential: 4,600 calls
    Combined Savings: 8-12ms per frame
═══════════════════════════════════════════════════════════
```

## Error Handling

```c
/* Check for errors */
int result = Profiler_BeginCPUMarker(profiler, "BadMarker");
if (result < 0) {
    const char* error = Profiler_GetLastError(profiler);
    printf("Error: %s\n", error);
    /* Error possibilities:
       - Marker stack overflow (max 1024 nested markers)
       - NULL pointer passed
       - Invalid marker name
    */
}
```

## Performance Tips

1. **Disable GPU queries in release builds** - They have overhead
2. **Sample every Nth frame** - Use `Profiler_SetSamplingFrequency()` to reduce overhead
3. **Use scoped markers** - More reliable than manual begin/end
4. **Export data after session** - Don't analyze during runtime
5. **Profile on target hardware** - Desktop results may not match target platform

## Data Export & Analysis

```c
/* Export for external analysis */
Profiler_ExportStatisticsJSON(profiler, "profile.json");
Profiler_ExportStatisticsCSV(profiler, "profile.csv");

/* Then analyze with:
   - Excel/LibreOffice: CSV data
   - Python: JSON parsing for analysis
   - Custom tools: Parse JSON format
*/
```

## Integration with Other Systems

- **Graphics Backend**: GPU query integration points
- **Memory Allocator**: Track GPU/CPU memory
- **Game Loop**: Frame boundaries
- **Debug UI**: Real-time profiling display
- **Log System**: Write profiling data to logs
