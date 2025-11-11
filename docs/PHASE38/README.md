# PHASE38: Build System Optimization

## Phase Title

Build System Optimization - Faster Compilation and Link Times

## Objective

Optimize CMake build system for faster incremental builds and reduced compile times.

## Dependencies

- Build infrastructure (CMake, compilers)

## Key Deliverables

1. Precompiled headers (PCH)
2. Link-time optimization (LTO) tuning
3. Parallel compilation optimization
4. Incremental build support
5. Build time profiling

## Acceptance Criteria

- [ ] Full build time < 30 minutes (from clean)
- [ ] Incremental build < 2 minutes
- [ ] No link errors from PCH
- [ ] LTO enabled without excessive time
- [ ] Parallel builds utilize all cores
- [ ] ccache integration working

## Technical Details

### Components to Implement

1. **Precompiled Headers**
   - Common includes in PCH
   - CMake PCH configuration
   - Per-target PCH

2. **Optimization**
   - LTO settings per configuration
   - Unity builds (optional)
   - ccache integration

3. **Profiling**
   - Build time measurement
   - Bottleneck identification

### Code Location

```
CMakeLists.txt
cmake/config-build.cmake
cmake/config-*.cmake
```

## Estimated Scope

**SMALL** (1-2 days)

- PCH setup: 0.5 day
- LTO tuning: 0.5 day
- Profiling: 0.5 day
- Testing: 0.5 day

## Status

**not-started**

## Testing Strategy

```bash
# Measure build times
time cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Significant build time reduction (50%+)
- No build stability issues
- Link succeeds consistently
- Incremental builds fast
- Clean builds acceptable

## Reference Documentation

- CMake PCH documentation
- Link-time optimization
- ccache usage
- Build performance profiling
