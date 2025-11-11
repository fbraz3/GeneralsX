# PHASE35: Platform-Specific Fixes

## Phase Title

Platform-Specific Fixes - macOS/Linux Compatibility Tweaks

## Objective

Address platform-specific issues for macOS and Linux, ensuring consistent behavior across platforms.

## Dependencies

- All previous phases
- Understanding of Critical Lesson 3: Memory Protections - Defense in Depth

## Key Deliverables

1. File path handling (cross-platform)
2. Input handling per-platform
3. Window management fixes
4. Graphics API compatibility
5. Build configuration per-platform

## Acceptance Criteria

- [ ] macOS version builds without warnings
- [ ] Linux version builds without warnings
- [ ] Behavior consistent across platforms
- [ ] File paths work correctly
- [ ] Graphics rendering identical
- [ ] Input responsive on all platforms

## Technical Details

### Components to Implement

1. **Path Handling**
   - Normalize paths (/ vs \)
   - Case sensitivity handling
   - Environment variables

2. **Platform Abstractions**
   - Window system differences
   - Input device differences
   - Graphics API variations

3. **Build Configuration**
   - CMake presets per-platform
   - Compiler flags
   - Dependency management

### Code Location

```
Core/Libraries/Source/WWVegas/win32_compat.h
Core/GameEngineDevice/Source/Platform/
cmake/config-*.cmake
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Path abstraction: 0.5 day
- Input handling: 0.5 day
- Graphics compatibility: 0.5 day
- Build fixes: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Important References
- **Critical Lesson 3**: `docs/PHASE00/CRITICAL_LESSONS.md` - Memory Protections - Triple validation (Phase 35.6 discovery)
- **Critical Lesson 4**: `docs/PHASE00/CRITICAL_LESSONS.md` - ARC/Global State (macOS-specific)

## Testing Strategy

```bash
# Build and test on macOS and Linux
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Clean builds on all platforms
- Identical behavior across platforms
- No platform-specific bugs
- Performance equivalent

## Reference Documentation

- Cross-platform path handling
- Platform abstraction layers
- Conditional compilation
- See `Core/Libraries/Source/WWVegas/win32_compat.h` for pattern
