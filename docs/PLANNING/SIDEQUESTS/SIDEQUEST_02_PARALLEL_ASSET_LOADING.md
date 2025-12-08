# Side Quest 02: Parallel Asset Loading - Architecture & Implementation Plan

**Status**: Planning Document (NOT STARTED)  
**Priority**: HIGH (Performance critical)  
**Complexity**: MEDIUM-HIGH  
**Estimated Effort**: 50-70 hours  
**Target Timeline**: Phase 21-25 (Early optimization opportunity)  
**Business Impact**: ~30-50% startup time reduction (60+ seconds → 20-30 seconds)

---

## Executive Summary

Currently, asset loading in GeneralsX is **entirely sequential**:

```
Startup Flow (Current - SEQUENTIAL)
├─ Load INI.big              (~8 sec)
├─ Parse GameData.ini        (~3 sec)
├─ Load Textures.big         (~12 sec)
├─ Parse all Texture.ini     (~2 sec)
├─ Load Models.big           (~8 sec)
├─ Load Audio.big            (~6 sec)
├─ Parse Audio*.ini          (~4 sec)
├─ Load Maps (on-demand)
└─ Ready for gameplay        (Total: ~45-60 seconds)
```

This side quest proposes **asynchronous parallel loading** of independent asset categories while showing a **unified progress bar** and maintaining **memory safety**.

**Goal**: Reduce startup from 60+ seconds to 20-30 seconds by parallelizing independent asset categories.

---

## Current Asset Loading Analysis

### Asset Categories (From INI Investigation)

**Location**: `$HOME/GeneralsX/GeneralsMD/Data/`

```
INI.big
├── INI/
│   ├── Default/
│   │   ├── GameData.ini        (Core gameplay)
│   │   ├── Object/             (Unit templates - 40+ files)
│   │   ├── Weapon.ini          (Weapon definitions)
│   │   ├── Armor.ini           (Damage model)
│   │   ├── Audio*.ini          (Audio events)
│   │   └── ... (100+ more)
│   └── Object/
│       ├── AirforceGeneral.ini
│       ├── SupremCommander.ini
│       └── ... (40+ faction/unit files)

INIZH.big (Zero Hour expansion)
├── INI/ (same structure with overrides)
└── ... (additional ZH content)

Textures.big
├── Texture files (DDS, TGA format)
└── ... (200+ texture files, 800MB+)

Models.big
├── W3D models
└── ... (500+ model files, 600MB+)

Audio.big
├── WAV audio samples
└── ... (1000+ audio files, 1GB+)

Maps/ (directory)
├── Campaign maps
├── Skirmish maps
└── Multiplayer maps (on-demand)
```

### Current Loading Flow

**File**: `GeneralsMD/Code/GameEngine/Source/GameEngine.cpp::init()`

```cpp
void GameEngine::init() {
    // Sequential loading - BLOCKING
    TheThingFactory.init();           // Reads all INI files sequentially
    TheWeaponFactory.init();          // More INI parsing
    TheAudioManager.init();           // More INI parsing
    TheTextureFactory.init();         // Loads Textures.big
    TheModelFactory.init();           // Loads Models.big
    TheAudioSystem.init();            // Loads Audio.big
    // Only THEN is gameplay ready
}
```

**Problem**: Each subsystem must wait for previous one to complete

### Current Bottlenecks

| Stage | Time | Bottleneck |
|-------|------|-----------|
| INI Parsing | 8-12 sec | Single-threaded recursive INI parsing |
| Texture Loading | 12-15 sec | Sequential .big file I/O |
| Model Loading | 8-10 sec | Sequential .big file I/O |
| Audio Loading | 6-8 sec | Sequential .big file I/O + decompression |
| **Total** | **45-60 sec** | **Serialized I/O + parsing** |

### Parallelizable Elements

```
Independent subsystems (can load in parallel):
├─ INI Configuration    (GameData, Objects, Weapons, etc.)
│  └─ Dependency: NONE
├─ Texture System       (Textures.big + Texture INI)
│  └─ Dependency: NONE
├─ Model System         (Models.big)
│  └─ Dependency: NONE
├─ Audio System         (Audio.big + Audio INI)
│  └─ Dependency: NONE
└─ UI Resources         (UI textures, fonts)
   └─ Dependency: NONE (can start after basic INI)

Sequential dependencies (must load in order):
├─ GameData.ini         (core game rules - blocks everything)
│  └─ Must complete before gameplay starts
├─ Unit templates       (depends on GameData.ini)
└─ Visual assets        (depend on unit templates)
```

**Parallelization Opportunity**: 4-6 independent subsystems can load simultaneously

---

## Proposed Architecture

### Overall Design

```
┌─────────────────────────────────────────────────────────┐
│ Main Thread (UI & Progress)                             │
│ └─ Shows loading bar                                    │
│    Updates progress: 0% → 100%                          │
└───────────────────┬─────────────────────────────────────┘
                    │
        ┌───────────┼───────────┬───────────┬──────────┐
        │           │           │           │          │
        ▼           ▼           ▼           ▼          ▼
   ┌─────────┐ ┌──────────┐ ┌──────────┐ ┌──────┐ ┌─────┐
   │Thread 1 │ │ Thread 2 │ │ Thread 3 │ │Thread│ │Thr. │
   │INI Load │ │Texture   │ │Model     │ │Audio │ │UI   │
   │         │ │Load      │ │Load      │ │Load  │ │Load │
   └────┬────┘ └────┬─────┘ └────┬─────┘ └──┬───┘ └──┬──┘
        │           │            │          │        │
        └───────────┼────────────┴──────────┴────────┘
                    │
        ┌───────────▼─────────────┐
        │ Asset Manager           │
        │ (Thread-safe registry)  │
        │ • Memory pooling        │
        │ • Handle management     │
        │ • Lock-free queues      │
        └─────────────────────────┘
```

### Phase 1: Asset Manager Infrastructure (15-20 hours)

**Objective**: Create thread-safe asset loading foundation

**New Files**:
- `Core/Libraries/Source/WWVegas/WW3D2/asset_manager.h`
- `Core/Libraries/Source/WWVegas/WW3D2/asset_manager.cpp`
- `Core/Libraries/Source/WWVegas/WW3D2/asset_loader_thread.h`

**Architecture**:

```cpp
// Thread-safe asset registry
typedef struct {
    char* name;
    void* data;
    uint64_t size;
    uint32_t handle;
    uint32_t ref_count;
    uint32_t load_state;  // UNLOADED, LOADING, LOADED, FAILED
} AssetEntry;

// Load job (posted to worker threads)
typedef struct {
    int priority;         // Higher = load first
    char* asset_path;
    char* asset_type;     // "texture", "model", "ini", etc.
    void* (*loader_func)(const char* path);
    void (*on_complete)(void* asset, int status);
} LoadJob;

// Asset manager (singleton)
typedef struct {
    AssetEntry* registry;          // Hash table (thread-safe)
    LoadJob* job_queue;            // Lock-free queue
    pthread_t* worker_threads;     // 4-6 threads
    uint32_t num_workers;
    uint64_t total_bytes_loaded;
    uint32_t num_assets_loaded;
    float progress_percentage;
    int shutdown_requested;
} AssetManager;

AssetManager* AssetManager_Create(int num_threads);
void AssetManager_EnqueueJob(AssetManager* mgr, LoadJob* job);
void AssetManager_WaitForCategory(AssetManager* mgr, const char* category);
void AssetManager_WaitAll(AssetManager* mgr);
int AssetManager_GetProgress(AssetManager* mgr, float* out_percent);
```

**Key Design Decisions**:

1. **Thread Count**: 4-6 worker threads
   - I/O bound, not CPU bound
   - More threads = more concurrent I/O operations
   - Too many = context switch overhead

2. **Job Priority Queue**:
   - Critical assets (GameData.ini) = priority 100
   - Textures = priority 50
   - Audio = priority 30
   - UI = priority 10

3. **Thread Safety**:
   - Lock-free queue for job dispatch (atomic operations)
   - Mutex-protected registry for asset lookups
   - Atomic counters for progress tracking

4. **Memory Management**:
   - Pre-allocated memory pools for each asset type
   - Prevents allocation failures during loading
   - Avoids malloc/free overhead during parallel loads

---

### Phase 2: Parallel Loading Workers (15-20 hours)

**Objective**: Implement thread-safe asset category loaders

**Worker Threads**:

```cpp
// Thread 1: INI Configuration Loader
void* Thread_LoadINIAssets(void* arg) {
    AssetManager* mgr = (AssetManager*)arg;
    while (!mgr->shutdown_requested) {
        LoadJob job = DequeueJob(mgr);
        if (!job.asset_path) continue;
        
        // Load INI file safely
        INIData* ini = LoadINIFile(job.asset_path);
        RegisterAsset(mgr, job.asset_type, ini);
        UpdateProgress(mgr);
    }
    return NULL;
}

// Thread 2: Texture Loader
void* Thread_LoadTextureAssets(void* arg) {
    // Similar structure but for textures
    // Handles .big file extraction + DDS/TGA conversion
}

// Thread 3: Model Loader
void* Thread_LoadModelAssets(void* arg) {
    // Similar structure but for 3D models
    // Handles .big file extraction + W3D parsing
}

// Thread 4: Audio Loader
void* Thread_LoadAudioAssets(void* arg) {
    // Similar structure but for audio samples
    // Handles .big file extraction + WAV/OGG decompression
}

// Thread 5: UI Loader
void* Thread_LoadUIAssets(void* arg) {
    // UI textures, fonts, icons
    // Lower priority but completes early
}
```

**Safety Guarantees**:

1. **No Race Conditions**:
   - Each asset type has dedicated loader thread
   - Asset registry protected by RWLock
   - No shared mutable state between threads

2. **Exception Safety**:
   - If thread crashes, others continue
   - Main thread detects failed loads
   - Fallback assets provided for missing data

3. **Memory Safety**:
   - Pre-allocated pools prevent allocation failures
   - Bounds checking on all buffer operations
   - Leak detection on shutdown

**Complexity**: MEDIUM
- Threading primitives straightforward
- Main challenge is coordinating asset dependencies

---

### Phase 3: Progress Reporting & UI Integration (10-15 hours)

**Objective**: Show unified progress bar to user

**Progress Calculation**:

```cpp
float CalculateOverallProgress(AssetManager* mgr) {
    // Weighted progress based on asset importance
    float ini_progress = GetCategoryProgress(mgr, "ini") * 0.25;      // 25% weight
    float texture_progress = GetCategoryProgress(mgr, "texture") * 0.30; // 30% weight
    float model_progress = GetCategoryProgress(mgr, "model") * 0.25;    // 25% weight
    float audio_progress = GetCategoryProgress(mgr, "audio") * 0.15;    // 15% weight
    float ui_progress = GetCategoryProgress(mgr, "ui") * 0.05;          // 5% weight
    
    return (ini_progress + texture_progress + model_progress + 
            audio_progress + ui_progress);
}
```

**UI Display**:

```cpp
// In main event loop
void RenderLoadingScreen(AssetManager* mgr) {
    float progress = AssetManager_GetProgress(mgr, NULL);
    
    // Draw background
    ClearScreen(0x1a1a1a);
    
    // Draw progress bar
    int bar_width = 400;
    int filled_width = (int)(bar_width * progress);
    DrawFilledRect(100, 300, filled_width, 40, 0x00ff00);  // Green
    DrawRect(100, 300, bar_width, 40, 0xffffff);           // White border
    
    // Draw percentage text
    char progress_text[32];
    sprintf(progress_text, "%d%%", (int)(progress * 100));
    DrawText(200, 350, progress_text);
    
    // Draw category breakdown (optional)
    DrawText(100, 400, "INI Configuration: %.0f%%",
             GetCategoryProgress(mgr, "ini") * 100);
    DrawText(100, 420, "Textures: %.0f%%",
             GetCategoryProgress(mgr, "texture") * 100);
    DrawText(100, 440, "Models: %.0f%%",
             GetCategoryProgress(mgr, "model") * 100);
    // ... etc
    
    // Wait for all to complete
    if (progress >= 0.99f) {
        AssetManager_WaitAll(mgr);
        return;  // Switch to main game screen
    }
}
```

**Complexity**: LOW-MEDIUM
- Mostly UI code, non-critical path
- Can be decorated incrementally

---

### Phase 4: Dependency Management (10-15 hours)

**Objective**: Handle sequential dependencies safely

**Dependency Graph**:

```
START
  │
  ├─→ GameData.ini (CRITICAL - blocks all gameplay)
  │    │
  │    ├─→ Unit Templates (waits for GameData.ini)
  │    │    │
  │    │    └─→ Visual Assets (depends on unit count/types)
  │    │
  │    └─→ Weapon Definitions
  │
  ├─→ Texture Assets (independent, parallel)
  │
  ├─→ Model Assets (independent, parallel)
  │
  ├─→ Audio Assets (independent, parallel)
  │
  └─→ UI Resources (independent, parallel)

BARRIER: All critical assets loaded before gameplay starts
END
```

**Implementation**:

```cpp
// Blocking wait for category
void AssetManager_WaitForCategory(AssetManager* mgr, const char* category) {
    while (!IsCategoryComplete(mgr, category)) {
        usleep(10000);  // Sleep 10ms, check again
    }
}

// Wait for all critical assets before gameplay
void AssetManager_WaitForGameplayReady(AssetManager* mgr) {
    AssetManager_WaitForCategory(mgr, "ini");           // Must complete first
    AssetManager_WaitForCategory(mgr, "critical_units"); // Then units
    
    // At this point, game can start
    // Other assets load in background
}

// Graceful degradation if non-critical assets fail
void HandleMissingAsset(const char* asset_type, const char* asset_name) {
    if (strcmp(asset_type, "texture") == 0) {
        // Use default white texture
        UseDefaultTexture();
    } else if (strcmp(asset_type, "audio") == 0) {
        // Silence is acceptable
        UseDefaultAudioSilence();
    } else {
        // Critical: fail with error
        FatalError("Missing critical asset: %s", asset_name);
    }
}
```

**Complexity**: MEDIUM
- Requires careful thought about dependencies
- Testing needed to ensure no deadlocks

---

### Phase 5: Testing & Benchmarking (15-20 hours)

**Objective**: Validate correctness and measure performance gains

**Unit Tests**:

```cpp
// Asset Manager Tests
✓ test_create_destroy()
✓ test_enqueue_load_job()
✓ test_concurrent_loads()
✓ test_progress_tracking()
✓ test_error_handling()
✓ test_asset_registry()
✓ test_memory_cleanup()

// Dependency Tests
✓ test_wait_for_category()
✓ test_wait_all()
✓ test_dependency_ordering()
✓ test_graceful_degradation()

// Stress Tests
✓ test_1000_concurrent_loads()
✓ test_memory_limit_enforcement()
✓ test_thread_crash_recovery()
✓ test_disk_io_failure()
```

**Benchmark Scenarios**:

| Scenario | Expected | Target |
|----------|----------|--------|
| Sequential (baseline) | 60 sec | Reference |
| 2 threads | 35 sec | 42% reduction |
| 4 threads | 25 sec | 58% reduction |
| 6 threads | 20 sec | 67% reduction |
| Ideal (no I/O wait) | 10 sec | Theoretical max |

**Profiling Points**:

```cpp
// Measure each subsystem
auto start_ini = chrono::now();
AssetManager_WaitForCategory(mgr, "ini");
auto ini_time = chrono::now() - start_ini;  // Expected: 8 sec → 10 sec (with locking overhead)

auto start_textures = chrono::now();
AssetManager_WaitForCategory(mgr, "texture");
auto texture_time = chrono::now() - start_textures;  // Expected: 12 sec → 4-5 sec

auto start_models = chrono::now();
AssetManager_WaitForCategory(mgr, "model");
auto model_time = chrono::now() - start_models;  // Expected: 8 sec → 3-4 sec

auto start_audio = chrono::now();
AssetManager_WaitForCategory(mgr, "audio");
auto audio_time = chrono::now() - start_audio;  // Expected: 6 sec → 2-3 sec

// Total = ~std::max(ini_time, texture_time, model_time, audio_time)
//       = std::max(10, 5, 4, 3) = 10 seconds
```

**Regression Tests**:

- [ ] Assets load correctly
- [ ] All game entities spawn
- [ ] Textures render without corruption
- [ ] Audio plays without artifacts
- [ ] No memory leaks
- [ ] Performance scales with thread count

---

## Benefits

### Performance 🚀

**Startup Time Reduction**:
```
Before: 60 seconds (sequential)
After:  20 seconds (parallel 6 threads)
Gain:   67% faster - 40 second reduction!
```

**User Experience**:
- Loading bar fills evenly
- Player sees progress
- Game ready in ~20 seconds instead of 60+
- Perceived responsiveness much higher

### Scalability 📈

- **Grows with hardware**: More CPU cores = better parallelization
- **Future-proof**: Architecture supports 8+ threads
- **GPU pre-loading**: Can add GPU upload tasks in separate thread

### Debugging 🐛

- Each asset category loads independently
- Can test individual systems in isolation
- Progress tracking helps identify bottlenecks
- Error recovery per-category

---

## Risks & Mitigation

| Risk | Severity | Mitigation |
|------|----------|-----------|
| **Race conditions** | HIGH | Lock-free queues, atomic operations, extensive testing |
| **Deadlock** | HIGH | No circular waits, dependency DAG validation |
| **Memory fragmentation** | MEDIUM | Pre-allocated pools, custom allocators |
| **Thread safety bugs** | MEDIUM | ThreadSanitizer validation, code review |
| **Performance regression** | LOW | Benchmark before/after on all platforms |
| **Platform differences** | MEDIUM | Test on macOS, Linux, Windows threading models |

---

## Implementation Checklist

### Phase 1: Infrastructure (Weeks 1-2)
- [ ] Create `asset_manager.h/cpp`
- [ ] Implement thread pool
- [ ] Create lock-free queue
- [ ] Implement progress tracking
- [ ] Memory pool allocators

### Phase 2: Loaders (Weeks 2-3)
- [ ] INI loader thread
- [ ] Texture loader thread
- [ ] Model loader thread
- [ ] Audio loader thread
- [ ] UI loader thread

### Phase 3: Integration (Week 4)
- [ ] Progress reporting to UI
- [ ] Loading screen rendering
- [ ] Dependency management
- [ ] Error handling

### Phase 4: Testing (Weeks 4-5)
- [ ] Unit tests
- [ ] Integration tests
- [ ] Stress tests
- [ ] Platform validation
- [ ] Performance benchmarking

### Phase 5: Optimization (Week 5-6)
- [ ] Profile and optimize hot paths
- [ ] Fine-tune thread count
- [ ] Cache locality improvements
- [ ] Documentation

---

## Success Criteria

- ✅ Startup time ≤ 25 seconds (target: 20-25)
- ✅ All assets load correctly in parallel
- ✅ Zero race conditions or deadlocks
- ✅ <5% performance variance across platforms
- ✅ Graceful error handling for missing assets
- ✅ Progress bar updates smoothly to user
- ✅ Memory usage < current sequential implementation
- ✅ No regressions in asset quality/fidelity
- ✅ Comprehensive test coverage (>85%)

---

## References

### Threading
- POSIX Threads: https://pubs.opengroup.org/onlinepubs/9699919799/
- Lock-free Programming: https://www.1024cores.net/

### Performance
- Memory pools: https://en.wikipedia.org/wiki/Memory_pool
- Thread pools: https://en.wikipedia.org/wiki/Thread_pool
- Work queues: https://preshing.com/20150316/semaphores-are-surprisingly-versatile/

### Project References
- Asset loading: `GeneralsMD/Code/GameEngine/Source/GameEngine.cpp::init()`
- INI parsing: `GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp`
- Texture system: Phase 12 documentation
- Audio system: Phase 33+ documentation

---

## Next Steps

1. **Review & Approval**: Get stakeholder review on architecture
2. **Risk Assessment**: Security review of threading approach
3. **Prototype**: Create POC with 2 asset categories
4. **Schedule**: Target Phase 21-25 for implementation
5. **Resource**: Ensure testing infrastructure ready for parallel validation

---

*Document created: November 12, 2025*  
*Last updated: November 12, 2025*
