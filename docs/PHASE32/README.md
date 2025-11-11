# PHASE32: Performance Profiling

## Phase Title

Performance Profiling - CPU/GPU Analysis Tools

## Objective

Implement profiling tools to measure CPU and GPU performance, identify bottlenecks, and optimize accordingly.

## Dependencies

- PHASE05 (Render Loop)
- PHASE23 (Game Loop)

## Key Deliverables

1. Frame time tracking
2. Per-system profiling
3. GPU timing (query objects)
4. Memory profiling
5. Profiling visualization

## Acceptance Criteria

- [ ] Frame time measurable with < 0.1ms accuracy
- [ ] All major subsystems have timing hooks
- [ ] GPU timing available
- [ ] Memory usage tracked
- [ ] Profiling overlay displays without performance impact
- [ ] Data exportable for analysis

## Technical Details

### Components to Implement

1. **CPU Profiling**
   - High-resolution timer
   - Scope-based timing
   - Frame breakdown

2. **GPU Profiling**
   - Query object timing
   - Draw call counting
   - Shader compilation times

3. **Visualization**
   - On-screen profiler display
   - Graph generation
   - CSV export

### Code Location

```
Core/GameEngineDevice/Source/Profiling/
GeneralsMD/Code/Debug/Profiler/
```

### Profiler Interface

```cpp
class Profiler {
public:
    void BeginFrame();
    void EndFrame();
    void BeginScope(const std::string& name);
    void EndScope();
    
    float GetFrameTime() const;
    float GetGPUTime() const;
    const std::map<std::string, float>& GetScopes() const;
    
    void ToggleDisplay();
    void ExportProfile(const std::string& filename);
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- CPU timing: 0.5 day
- GPU timing: 1 day
- Visualization: 0.5 day
- Testing/validation: 1 day

## Status

**not-started**

## Testing Strategy

```bash
# Profile game and view results
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Profiler"
```

## Success Criteria

- Accurate frame timing
- All systems profiled
- No performance impact from profiler
- Results exportable
- UI displays clearly

## Reference Documentation

- High-resolution timing APIs
- GPU query objects (Vulkan/Metal)
- Performance analysis techniques
- Profiler design patterns
