# Phase 43.X Series: Linker Symbol Resolution Strategy

**Note**: This document explains how Phases 43.1-43.7 relate to existing Phases 42 and 43.

---

## Phase Timeline Clarification

### Existing Phases (Unchanged)

- **Phase 42**: Final Cleanup & Polish
  - Status: Complete
  - Scope: Remove legacy code, validate compilation consistency
  - Located in: `docs/WORKDIR/phases/4/PHASE42/`

- **Phase 43**: Cross-Platform Compilation Validation
  - Status: Planned (after 42)
  - Scope: Validate compilation on Windows, macOS, Linux
  - Located in: `docs/WORKDIR/phases/4/PHASE43/`

### New Sub-Phases (Symbol Resolution)

Created after comprehensive analysis to resolve 130 linker undefined symbols:

- **Phase 43.1** - Surface & Texture Foundation (25 symbols)
- **Phase 43.2** - Advanced Texture Systems (30 symbols)
- **Phase 43.3** - Rendering Infrastructure (15 symbols)
- **Phase 43.4** - Network/LANAPI Transport (25 symbols)
- **Phase 43.5** - GameSpy Integration (25 symbols)
- **Phase 43.6** - Utilities & Memory Management (10 symbols)
- **Phase 43.7** - Final Symbol Resolution & Validation (all remaining)

Each located in: `docs/WORKDIR/phases/4/PHASE43_X/`

---

## Execution Order

```
Phase 41: Registry Core                          ✅ COMPLETE
    ↓
Phase 42: Final Cleanup & Polish                ✅ COMPLETE
    ↓
Phase 43.1: Surface & Texture Foundation         ✅ COMPLETE
Phase 43.2: Advanced Texture Systems             ✅ COMPLETE
Phase 43.3: Rendering Infrastructure            ✅ COMPLETE
Phase 43.4: Network/LANAPI Transport            ✅ COMPLETE
    ↓
Phase 43.5: GameSpy Integration                 (NEXT)
Phase 43.6: Utilities & Memory Management       (THEN)
Phase 43.7: Final Validation & Testing          (THEN)
    ↓
Phase 43: Cross-Platform Validation             (AFTER)
    ↓
Phase 50+: Advanced Gameplay Systems            (FUTURE)
```

---

## Symbol Resolution Tracking

### Current Status: Phase 43.4 Complete (145 symbols resolved, 35 remaining)

| Phase | Description | Symbols | Target Reduction | Running Total | Status |
|-------|-------------|---------|------------------|----------------|--------|
| 41 | Registry Core | 50 | +50 | 50 of 180 | ✅ COMPLETE |
| 43.1 | Surface & Texture | 25 | +25 | 75 of 180 | ✅ COMPLETE |
| 43.2 | Advanced Texture | 30 | +30 | 105 of 180 | ✅ COMPLETE |
| 43.3 | Rendering | 15 | +15 | 120 of 180 | ✅ COMPLETE |
| 43.4 | Network Transport | 25 | +25 | 145 of 180 | ✅ COMPLETE |
| 43.5 | GameSpy | 25 | +25 | 170 of 180 | 🔄 IN PROGRESS |
| 43.6 | Utilities | 10 | +10 | 180 of 180 | ⏳ PENDING |
| **TOTAL** | **Full Resolution** | **180** | **100%** | **145/180** | **80.6%** |

---

## Key Design Decisions

### Why Sub-Phases Instead of Mega-Phase 43?

1. **Focused Implementation**: Each sub-phase implements one cohesive subsystem
2. **Incremental Testing**: Compile after each phase, verify symbol count
3. **Clear Documentation**: No ambiguity about scope or responsibilities
4. **Reduced Complexity**: 25-30 symbols per phase vs. 130 in one phase
5. **Easier Debugging**: If a phase fails, easy to identify root cause
6. **Better Code Review**: Smaller changesets easier to review and validate

### Why Keep Original Phases 42-43?

1. **Historical Accuracy**: These phases have complete documentation
2. **Avoid Confusion**: Renumbering would break existing references
3. **Logical Separation**: 43.X is about symbol resolution, 43 is about validation
4. **Future-Proof**: Makes room for other sub-phase series if needed

---

## Phase 43.1-43.7 Details

### Phase 43.1: Surface & Texture Foundation (25 symbols)

**Key Classes**:
- `SurfaceClass` (12 symbols) - Pixel data management
- `TextureBaseClass` (4 symbols) - Abstract base class
- `TextureClass` (5 symbols) - 2D texture implementation
- `TextureFilterClass` (2 symbols) - Mipmap management

**Expected Output**: Symbols reduced from 130 → 105

**Details**: See `docs/WORKDIR/phases/4/PHASE43_1/README.md`

### Phase 43.2: Advanced Texture Systems (30 symbols)

**Key Classes**:
- `TexProjectClass` (7 symbols) - Shadow projection
- `PointGroupClass` (19 symbols) - Particle rendering
- `TextureLoader` (3 symbols) - Async loading
- `VolumeTextureClass` (1 symbol) - 3D textures

**Expected Output**: Symbols reduced from 105 → 75

### Phase 43.3: Rendering Infrastructure (15 symbols)

**Key Classes**:
- `SortingRendererClass` (4 symbols)
- Render pass management (11 symbols)

**Expected Output**: Symbols reduced from 75 → 60

### Phase 43.4: Network/LANAPI Transport (25 symbols)

**Key Classes**:
- `Transport` (12 symbols)
- `UDP` (4 symbols)
- `IPEnumeration` (4 symbols)
- Connection management (5 symbols)

**Expected Output**: Symbols reduced from 60 → 35

### Phase 43.5: GameSpy Integration (25 symbols)

**Key Classes**:
- `GameSpyStagingRoom` (3 symbols)
- Global GameSpy objects (12 symbols)
- UI/Message functions (10 symbols)

**Expected Output**: Symbols reduced from 35 → 10

### Phase 43.6: Utilities & Memory (10 symbols)

**Key Components**:
- `Targa` image format (4 symbols)
- `FastAllocatorGeneral` (3 symbols)
- Global data initialization (3 symbols)

**Expected Output**: Symbols reduced from 10 → 0

### Phase 43.7: Final Validation (0 symbols)

**Purpose**: 
- Verify all symbols resolved
- Compile executable
- Basic smoke testing
- Document success

**Expected Output**: Executable created ✅

---

## Implementation Pattern for Each Phase

### File Naming Convention

Each phase creates implementation files:

```
Core/Libraries/Source/Graphics/phase43_X_<component>.cpp
```

Example for Phase 43.1:
- `phase43_1_surface.cpp` (250 lines)
- `phase43_1_texture_base.cpp` (150 lines)
- `phase43_1_texture_class.cpp` (300 lines)

### CMakeLists.txt Updates

For each phase, add to `Core/Libraries/Source/Graphics/CMakeLists.txt`:

```cmake
list(APPEND WW3D2_SOURCE_FILES
    "Source/Graphics/phase43_1_surface.cpp"
    "Source/Graphics/phase43_1_texture_base.cpp"
    "Source/Graphics/phase43_1_texture_class.cpp"
)
```

### Testing & Verification

After each phase implementation:

```bash
# Build
cmake --build build/macos-arm64-vulkan --target GeneralsXZH -j 4 2>&1 | tee logs/phase43_X_build.log

# Verify symbols
grep "\".*\", referenced from:" logs/phase43_X_build.log | sort | uniq | wc -l

# Expected: 130 - (25 * X) where X is phase number
```

---

## Cross-References

### Main Analysis Document

See `docs/WORKDIR/phases/4/PHASE41_COMPREHENSIVE_ANALYSIS.md` for:
- Complete symbol listing (130 symbols)
- Detailed breakdown by class
- Priority classification
- Implementation notes for each symbol

### Individual Phase Documentation

Each Phase 43.X has dedicated README:
- `docs/WORKDIR/phases/4/PHASE43_1/README.md` - Surface & Texture
- `docs/WORKDIR/phases/4/PHASE43_2/README.md` - Advanced Textures
- `docs/WORKDIR/phases/4/PHASE43_3/README.md` - Rendering
- `docs/WORKDIR/phases/4/PHASE43_4/README.md` - Network
- `docs/WORKDIR/phases/4/PHASE43_5/README.md` - GameSpy
- `docs/WORKDIR/phases/4/PHASE43_6/README.md` - Utilities
- `docs/WORKDIR/phases/4/PHASE43_7/README.md` - Validation

---

## Critical Notes

⚠️ **Do NOT create stubs**: Each phase must contain real implementations, not empty placeholders

⚠️ **Do NOT skip phases**: Even if you want to implement Phase 43.5 first, do 43.1-43.4 to establish foundation

⚠️ **Do NOT modify Phase 42-43**: These phases are complete and validated. New work goes in 43.1-43.7

⚠️ **Do compile after each phase**: Verify symbol count reduction before proceeding

✅ **DO follow SDL2/Vulkan patterns**: Use references/ submodules as guides

✅ **DO document each symbol**: Each file should have comments listing which symbols it resolves

✅ **DO commit after each phase**: Each phase gets its own commit with detailed message

---

## Success Criteria

- [ ] Phase 43.1 compiled, 130 → 105 symbols
- [ ] Phase 43.2 compiled, 105 → 75 symbols
- [ ] Phase 43.3 compiled, 75 → 60 symbols
- [ ] Phase 43.4 compiled, 60 → 35 symbols
- [ ] Phase 43.5 compiled, 35 → 10 symbols
- [ ] Phase 43.6 compiled, 10 → 0 symbols
- [ ] Phase 43.7 executable created ✅
- [ ] All documentation updated
- [ ] All commits completed

---

## Session Planning

### Recommended Timeline

- **Day 1-2**: Phase 43.1 implementation & testing
- **Day 2-3**: Phase 43.2 implementation & testing
- **Day 3-4**: Phase 43.3 implementation & testing
- **Day 4-5**: Phase 43.4 implementation & testing (can be parallel)
- **Day 5-6**: Phase 43.5-43.6 implementation & testing
- **Day 6-7**: Phase 43.7 validation & final testing

### Documentation Updates

After each phase:
1. Update `logs/phase43_X_build.log` with symbol count
2. Document any integration issues discovered
3. Update this master document with progress

---

## Rollback Strategy

If any phase introduces issues:

```bash
# Revert to Phase 41 (clean state)
git log --oneline | grep "phase41\|phase43"  # Find commits
git reset --hard <commit>  # Revert to that commit
git clean -fd  # Remove new files

# Then retry with different approach
```

---

## Next Steps

1. ✅ Create this master strategy document
2. ⏳ Create Phase 43.1 detailed README
3. ⏳ Implement Phase 43.1 code
4. ⏳ Build and verify Phase 43.1
5. ⏳ Continue with Phase 43.2-43.7

---

## References

- `docs/WORKDIR/phases/4/PHASE41_COMPREHENSIVE_ANALYSIS.md` - Detailed symbol analysis
- `docs/WORKDIR/phases/4/PHASE42/README.md` - Previous phase context
- `docs/WORKDIR/phases/4/PHASE43/README.md` - Next phase (validation)
- `references/` - Git submodules with proven implementations

---

**Document Version**: 1.0
**Date Created**: November 22, 2025
**Last Updated**: November 22, 2025
