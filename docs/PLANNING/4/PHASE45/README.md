# Phase 45: Advanced Cross-Platform Testing & Long-Duration Stability

**Phase**: 45
**Title**: Extended Runtime Testing, Gameplay Validation & Multi-Platform Optimization
**Duration**: 2 weeks
**Status**: 🕐 Pending (after Phase 44)
**Dependencies**: Phase 44 complete (runtime validation on all platforms)

---

## Overview

Phase 45 takes runtime validation to the next level:

1. **Extended duration testing** (24+ hour stress tests)
2. **Gameplay validation** (campaign/skirmish mode testing)
3. **Multiplayer compatibility** (network/GameSpy systems)
4. **Graphics quality validation** (rendering correctness)
5. **Memory leak detection** (valgrind/ASAN in extended runs)
6. **Platform-specific optimization** (performance tuning per platform)

**Strategic Goal**: Prove the game is stable, feature-complete, and performant across all platforms before Phase 46 (optimization).

**Success Criteria**: Game runs 24+ hours without crashes, all core gameplay features work, cross-platform behavior identical.

---

## Implementation Strategy

### Week 1: Stress Testing & Gameplay Validation

#### Day 1-2: 24-Hour Stability Test (macOS ARM64)

**Compile with debugging enabled**:

```bash
cmake --preset macos-arm64-vulkan -DENABLE_ASAN=ON
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/phase45_asan_build.log
```

**Run 24-hour stress test**:

```bash
cd $HOME/GeneralsX/GeneralsMD

# Background process with logging
nohup bash -c 'USE_METAL=1 ./GeneralsXZH 2>&1' > logs/phase45_stability_24h.log 2>&1 &
GAME_PID=$!

# Monitor for 24 hours
for i in {1..1440}; do
  if ! kill -0 $GAME_PID 2>/dev/null; then
    echo "Game crashed at minute $i"
    break
  fi
  sleep 60
  
  # Log memory usage every 5 minutes
  if [ $((i % 5)) -eq 0 ]; then
    ps aux | grep $GAME_PID | grep -v grep >> logs/phase45_memory_usage.log
  fi
done

# After 24 hours, terminate gracefully
kill -TERM $GAME_PID 2>/dev/null || echo "Game already terminated"
```

**Analysis**:

```bash
# Check for crashes
grep -i "crash\|segfault\|exception" logs/phase45_stability_24h.log | wc -l

# Memory growth analysis
awk '{print $6}' logs/phase45_memory_usage.log | sort -n | tail -5

# Error frequency
grep -i "error" logs/phase45_stability_24h.log | wc -l
```

#### Day 3: Campaign Mode Testing

**Test campaign gameplay loop**:

```bash
cd $HOME/GeneralsX/GeneralsMD

# 1-hour campaign mode test
timeout 3600s bash -c 'USE_METAL=1 ./GeneralsXZH 2>&1' | tee logs/phase45_campaign_1h.log

# Check for gameplay-specific errors
grep -i "campaign\|mission\|objective\|unit\|building" logs/phase45_campaign_1h.log | head -20
```

**Validation checklist**:

```markdown
# Campaign Gameplay Validation

- [ ] Campaign loads without error
- [ ] Mission objectives display
- [ ] Units spawn correctly
- [ ] Buildings construct properly
- [ ] Economy system operational
- [ ] Combat AI responsive
- [ ] Graphics rendering correct
- [ ] No crashes during 1-hour session
- [ ] No memory growth detected
```

#### Day 4: Multiplayer/Network Testing

**Test network functionality** (if applicable):

```bash
cd $HOME/GeneralsX/GeneralsMD

# LAN game startup (connect to other player or local testing)
timeout 600s bash -c 'USE_METAL=1 ./GeneralsXZH 2>&1' | tee logs/phase45_network_test.log

# Check for network errors
grep -i "network\|lanapi\|socket\|connection" logs/phase45_network_test.log
```

**Validation points**:

- [ ] Network initialization succeeds
- [ ] Player can join/host game
- [ ] Commands sync across players
- [ ] No disconnects or timeouts
- [ ] Network performance acceptable

#### Day 5: Cross-Platform Long-Duration Test

**Replicate 24-hour test on Windows and Linux**:

Windows:

```cmd
cd GeneralsX\GeneralsMD
start /high GeneralsXZH.exe 2>&1 | tee logs/phase45_stability_24h_windows.log

REM Monitor for 24 hours (use Task Manager or performance monitoring tool)
```

Linux:

```bash
cd $HOME/GeneralsX/GeneralsMD
timeout 86400s bash -c 'USE_METAL=1 ./GeneralsXZH 2>&1' | tee logs/phase45_stability_24h_linux.log
```

**Compare results**:

```bash
# All platforms should show similar stability metrics
for platform in windows linux macos; do
  echo "=== $platform ==="
  grep -i "crash\|error" logs/phase45_stability_24h_$platform.log | wc -l
done
```

---

### Week 2: Graphics Validation & Performance Profiling

#### Day 1-2: Graphics Quality Validation

**Verify rendering correctness** across all platforms:

```bash
cd $HOME/GeneralsX/GeneralsMD

# Start game and capture screenshots (manual process)
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase45_graphics_test.log

# Visual inspection:
# - Textures render correctly
# - Lighting looks realistic
# - UI elements positioned properly
# - No visual glitches or artifacts
# - Animation smooth and responsive
```

**Create graphics validation report**:

```bash
cat > docs/PLANNING/4/PHASE45/GRAPHICS_VALIDATION.md << 'EOF'
# Graphics Quality Validation Report

## Platform: macOS ARM64

### Terrain Rendering
- [ ] Heightmap renders correctly
- [ ] Terrain textures display properly
- [ ] Lighting applied correctly
- [ ] No missing polygons or gaps

### Unit Rendering
- [ ] Infantry units visible and animated
- [ ] Vehicle units render correctly
- [ ] Aircraft display properly
- [ ] Animations smooth at 30+ FPS

### Building Rendering
- [ ] Structures appear correctly
- [ ] Building animations work
- [ ] Damage states display
- [ ] No Z-fighting or depth issues

### UI Rendering
- [ ] Menu text readable
- [ ] Icons display correctly
- [ ] Buttons responsive to input
- [ ] HUD elements positioned properly

### Special Effects
- [ ] Explosions render
- [ ] Particle effects visible
- [ ] Weather effects functional (if applicable)
- [ ] No performance degradation

## Platform: Windows

(Same validation as above)

## Platform: Linux

(Same validation as above)

## Conclusion
- [ ] Graphics identical across all platforms
- [ ] Rendering performance acceptable
- [ ] No visual glitches detected
EOF
```

#### Day 3: Performance Profiling

**Baseline performance metrics**:

```bash
cd $HOME/GeneralsX/GeneralsMD

# Frame time measurement (5 minutes)
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase45_perf_profile.log

# Extract frame timing data
grep "Frame time\|FPS\|ms/frame" logs/phase45_perf_profile.log | tee logs/phase45_frame_times.csv
```

**Create performance baseline**:

```bash
cat > docs/PLANNING/4/PHASE45/PERFORMANCE_BASELINE.md << 'EOF'
# Performance Baseline Report

## Measurement Configuration
- Duration: 5 minutes continuous gameplay
- Measurement method: Frame time logging
- Graphics quality: Default settings

## Results

### macOS ARM64 (Metal/Vulkan)
| Metric | Value | Target | Status |
|--------|-------|--------|--------|
| Average FPS | ___ | >30 | ✅/❌ |
| Min FPS | ___ | >20 | ✅/❌ |
| Max FPS | ___ | <60 | ✅/❌ |
| Memory Peak | ___ MB | <512 MB | ✅/❌ |
| CPU Usage | ___% | <80% | ✅/❌ |

### Windows
| Metric | Value | Target | Status |
|--------|-------|--------|--------|
| Average FPS | ___ | >30 | ✅/❌ |
| Min FPS | ___ | >20 | ✅/❌ |
| Max FPS | ___ | <60 | ✅/❌ |
| Memory Peak | ___ MB | <512 MB | ✅/❌ |
| CPU Usage | ___% | <80% | ✅/❌ |

### Linux
| Metric | Value | Target | Status |
|--------|-------|--------|--------|
| Average FPS | ___ | >30 | ✅/❌ |
| Min FPS | ___ | >20 | ✅/❌ |
| Max FPS | ___ | <60 | ✅/❌ |
| Memory Peak | ___ MB | <512 MB | ✅/❌ |
| CPU Usage | ___% | <80% | ✅/❌ |

## Platform Comparison
- [ ] Performance consistent across platforms
- [ ] Outliers identified and explained
- [ ] Ready for Phase 46 (optimization)

## Bottleneck Analysis
- Primary bottleneck: [describe]
- Secondary bottleneck: [describe]
- Optimization candidates for Phase 46: [list]
EOF
```

#### Day 4-5: Platform-Specific Optimization

**Identify platform-specific tuning opportunities**:

```bash
# macOS ARM64 optimizations
# - Metal shader compilation caching
# - Memory pool optimization for Apple Silicon
# - Input latency reduction

# Windows optimizations
# - Vulkan driver feature utilization
# - DirectInput optimization
# - CPU/GPU sync points

# Linux optimizations
# - Wayland vs X11 selection
# - Vulkan driver selection (amd/nvidia)
# - Input subsystem tuning
```

---

## Success Criteria

### Must Have (Phase 45 Completion)

- ✅ 24-hour stress test completed without crash
- ✅ Memory stable during extended run
- ✅ Campaign mode fully playable
- ✅ All core gameplay features functional
- ✅ Graphics rendering validated on all platforms
- ✅ Performance baseline established
- ✅ No unhandled exceptions or segfaults

### Should Have

- ✅ Multiplayer testing completed
- ✅ Extended gameplay (different mission types)
- ✅ Platform-specific optimizations applied
- ✅ Detailed performance reports generated

### Known Limitations

- Advanced features (custom maps, replays) may not be fully tested
- Network lag/latency not profiled
- Graphics settings optimization deferred to Phase 46

---

## Debugging Resources

### Memory Leak Detection

```bash
# Run with Valgrind (Linux)
valgrind --leak-check=full ./GeneralsXZH 2>&1 | tee logs/phase45_valgrind.log

# Run with ASAN (all platforms)
export ASAN_OPTIONS=halt_on_error=0
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase45_asan_runtime.log
```

### Crash Debugging

```bash
# Attach debugger to running process
lldb -p $(pgrep GeneralsXZH)

# Or pre-run with debugger
USE_METAL=1 lldb -o run -o bt ./GeneralsXZH
```

### Performance Profiling

```bash
# macOS Instruments (graphical tool)
instruments -t "Counters" $HOME/GeneralsX/GeneralsMD/GeneralsXZH

# Linux perf (command-line)
perf record -o perf.data ./GeneralsXZH
perf report -i perf.data
```

---

## Files to Create/Update

### Create New

```markdown
docs/PLANNING/4/PHASE45/GRAPHICS_VALIDATION.md
docs/PLANNING/4/PHASE45/PERFORMANCE_BASELINE.md
logs/phase45_stability_24h.log
logs/phase45_campaign_1h.log
logs/phase45_perf_profile.log
```

---

## References

- Phase 44 (Runtime validation baseline)
- Phase 42 (Code quality foundation)
- Project performance targets (from Phase 0 planning)

---

**Created**: November 22, 2025  
**Last Updated**: November 22, 2025  
**Status**: 🕐 Pending (awaiting Phase 44 completion)
