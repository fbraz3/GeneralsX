# GeneralsX: Minimal Playable Version Roadmap

**Project**: GeneralsX - Cross-platform Command & Conquer Generals Port  
**Target**: Minimal playable game version with core graphics and gameplay  
**Estimated Timeline**: 25-30 development days  
**Current Status**: Phase 40.2 Complete (DXVK Backend Active), Ready for Phase 41

## Phase Overview & Timeline

### Foundation Phases (Already Complete)

| Phase | Name | Status | Duration |
|-------|------|--------|----------|
| 40 | DXVK Backend & Vulkan Integration | ✅ Complete | 2 days |
| 40.1 | Backend Selection Fix | ✅ Complete | 1 day |
| 40.2 | Stub Removal & Real Implementations | ✅ Complete | 1 day |

### Graphics Pipeline Phases (4-6 Days)

| Phase | Name | Depends On | Duration | Key Deliverable |
|-------|------|-----------|----------|-----------------|
| 41 | Drawing Operations & Render States | Phase 40 | 4 days | Colored shapes on screen |
| 42 | Texture System & Format Support | Phase 40, 41 | 4 days | Textured objects |
| 43 | Render Loop & Frame Presentation | Phase 40, 41, 42 | 2-3 days | 60 FPS consistent rendering |

### Geometry & Rendering Phases (5-7 Days)

| Phase | Name | Depends On | Duration | Key Deliverable |
|-------|------|-----------|----------|-----------------|
| 44 | Model Loading & Mesh System | Phase 40-43 | 3-4 days | 3D models rendering |
| 45 | Camera System & View Transforms | Phase 40, 43, 44 | 2-3 days | Viewpoint control |
| 46 | Game Logic Integration | Phase 41-45 | 3-4 days | Game objects in world |

### Integration & Polish Phases (6-8 Days)

| Phase | Name | Depends On | Duration | Key Deliverable |
|-------|------|-----------|----------|-----------------|
| 47 | Testing, Stabilization & Optimization | Phase 41-46 | 3-4 days | Stable, optimized code |
| 48 | Minimal Playable Version | Phase 40-47 | 2-3 days | Complete game v0.1.0 |

## Cumulative Timeline

```
Week 1 (Oct 30 - Nov 5)
├── Oct 30 (Day 1-2): Phase 40 Backend Completion
├── Oct 31 - Nov 1 (Day 3-5): Phase 41-42 Documentation & Planning
└── Planning Phase 43

Week 2 (Nov 6 - Nov 12)
├── Phase 41: Drawing Operations (4 days)
├── Phase 42: Texture System (4 days)
└── Phase 43: Render Loop (2 days)

Week 3 (Nov 13 - Nov 19)
├── Phase 44: Model Loading (3 days)
├── Phase 45: Camera System (3 days)
├── Phase 46: Game Logic (3 days)
└── Partial Phase 47: Early Testing

Week 4 (Nov 20 - Nov 26)
├── Phase 47: Testing & Optimization (3 days)
└── Phase 48: Minimal Playable (3 days)

Total: ~25-30 Days to Minimal Playable Version
```

## Phase Dependencies Graph

```
Phase 40: DXVK Backend (Foundation)
    ↓
┌─────────────────────────────────┐
│ Phase 41-42: Graphics Pipeline  │
│ (Drawing, Textures)             │
└─────────────────────────────────┘
         ↓
    Phase 43: Render Loop
    (60 FPS Frame Rendering)
         ↓
┌─────────────────────────────────┐
│ Phase 44: Model Loading         │
│ + Phase 45: Camera System       │
│ (Parallel work)                 │
└─────────────────────────────────┘
         ↓
    Phase 46: Game Logic
    (Connect Graphics & Game)
         ↓
    Phase 47: Testing & Opt
         ↓
    Phase 48: Playable Version
```

## Success Metrics by Phase

### Phase 41: Drawing Operations
**Acceptance Criteria**:
- Render colored triangles to screen
- Support indexed geometry (16/32-bit indices)
- Apply basic render states (depth, blending)
- Achieve 60 FPS with simple geometry

**Validation**: Triangle test renders in 2ms, FPS stable

### Phase 42: Texture System
**Acceptance Criteria**:
- Load BC3/DXT1/DXT3/DXT5 compressed textures
- Support RGBA8 uncompressed format
- Bind textures to rendering
- Verify pixel-perfect texture display

**Validation**: Textured quad displays correctly, no corruption

### Phase 43: Render Loop
**Acceptance Criteria**:
- BeginScene/EndScene/Present cycle working
- Frame timing stable (60 FPS ±1%)
- No tearing or frame rate drops
- Swapchain recreation on resize

**Validation**: 1000 frames with consistent 16.67ms timing

### Phase 44: Model Loading
**Acceptance Criteria**:
- Parse W3D files successfully
- Create GPU buffers for vertex/index data
- Render models with correct geometry
- Cache models for reuse

**Validation**: Sample building model renders with correct shape

### Phase 45: Camera System
**Acceptance Criteria**:
- View matrix transforms correctly
- Projection matrix applies perspective
- Camera movement responds to input
- Multiple camera modes functional

**Validation**: Geometry gets smaller as distance increases

### Phase 46: Game Logic
**Acceptance Criteria**:
- GameObjects render in world space
- Multiple objects simultaneous
- Input controls camera
- Game loop integrates all systems

**Validation**: 10 units render and respond to movement

### Phase 47: Testing & Optimization
**Acceptance Criteria**:
- 100+ unit tests passing
- Consistent 60 FPS with 100 units
- Memory usage < 512MB
- No crashes in 60-minute run

**Validation**: Full test suite passes, profiling shows no bottlenecks

### Phase 48: Minimal Playable
**Acceptance Criteria**:
- Main menu displays and works
- Map loads successfully
- Basic gameplay functional
- Game can be played end-to-end
- Clean exit without crashes

**Validation**: Complete game session from menu → gameplay → exit

## Key Technical Decisions

### Vulkan Backend
- **Decision**: Use Vulkan via MoltenVK on macOS
- **Rationale**: Modern graphics API, cross-platform, proven on Metal
- **Current Status**: Fully operational (Phase 40)

### DXVK Wrapper
- **Decision**: Direct3D 8 abstraction → Vulkan
- **Rationale**: Reuses existing DX8 API calls, abstracts graphics backend
- **Current Status**: Initialized and working

### Rendering Architecture
- **Decision**: Deferred initialization with real implementations
- **Rationale**: Separate device creation from drawing implementation
- **Current Status**: All 11 device stages verified working

### Test-First Approach
- **Decision**: Create test infrastructure before gameplay code
- **Rationale**: Catch bugs early, verify correctness
- **Target**: 100+ unit tests for Phase 47

## Development Environment Setup

### Build Configuration
```bash
# Configure with DXVK enabled (required for Phase 41+)
cmake --preset macos-arm64 -DUSE_DXVK=ON

# Build with optimization
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Verify build
ls -lh build/macos-arm64/GeneralsMD/GeneralsXZH
```

### Asset Preparation
```bash
# Symlink game assets
mkdir -p $HOME/GeneralsX/GeneralsMD/
ln -s "$(find ~ -path "*Generals*Data*" -type d 2>/dev/null)" $HOME/GeneralsX/GeneralsMD/Data
ln -s "$(find ~ -path "*Generals*Maps*" -type d 2>/dev/null)" $HOME/GeneralsX/GeneralsMD/Maps
```

### Debugging Setup
```bash
# Launch with logging
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=0 ./GeneralsXZH 2>&1 | tee logs/session.log

# Monitor performance
top -pid $(pgrep GeneralsXZH)

# Capture crash info
cat "$HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt"
```

## Estimated Velocity & Burn-Down

### Phase Velocity (typical)
- Graphics pipeline: 1 phase per 3-4 days
- Game logic: 1 phase per 3-4 days
- Integration: 1-2 phases per 2-3 days

### Risk Factors
- Vulkan API learning curve: Mitigated by Phase 39/40 preparation
- Model format complexity: Mitigated by using existing W3D parser
- Game logic coupling: Mitigated by incremental integration

### Buffer for Unforeseen Issues
- 2-3 days allocated for debugging/fixes
- Phase 47 extended testing covers stability

## Minimal Playable Version Feature Set

### Must-Have (MVP)
- ✅ Main menu with map selection
- ✅ Single skirmish map
- ✅ Basic unit spawning/control
- ✅ Unit movement
- ✅ Camera control (WASD + mouse)
- ✅ 60 FPS stable
- ✅ No crashes

### Nice-to-Have (Post-MVP)
- Building production
- Resource system
- Fog of war
- Basic AI
- Combat system
- Sound/music

### Out of Scope (Future Phases)
- Multiplayer networking
- Campaign mode
- Advanced AI
- Full game balance
- Graphics features (reflection, advanced shaders)

## Success Conditions for Phase 48 Completion

### Code Quality
- ✅ All code follows C++20 standards
- ✅ No compilation errors or warnings
- ✅ All tests passing
- ✅ No memory leaks
- ✅ Vulkan validation layer: 0 errors

### Performance
- ✅ Main menu: < 5ms per frame
- ✅ Gameplay: 60 FPS ± 1 FPS
- ✅ Map load: < 5 seconds
- ✅ Memory: < 512MB total

### Functionality
- ✅ Complete game flow working
- ✅ All input responsive
- ✅ Graphics rendering correctly
- ✅ Game logic operational
- ✅ Clean shutdown

### Documentation
- ✅ All phases documented (41-48)
- ✅ Code comments explaining complex sections
- ✅ README with build/run instructions
- ✅ Developer guide for future phases

## Future Roadmap (Beyond Phase 48)

### Phase 49-50: Audio System
- OpenAL backend implementation
- Music and SFX playback
- 3D positional audio

### Phase 51-52: Combat & Balance
- Unit attack mechanics
- Damage calculation
- Building destruction

### Phase 53-54: Campaign Mode
- Story missions
- Progressive difficulty
- Campaign narrative

### Phase 55+: Advanced Features
- Multiplayer networking
- Advanced AI
- Graphics enhancements
- Performance optimization

## References & Resources

### Documentation Structure
- `docs/PHASE41/` through `docs/PHASE48/` - Detailed phase planning
- `docs/MACOS_PORT_DIARY.md` - Development diary (update after each phase)
- `docs/Misc/` - Miscellaneous reference materials

### Build Instructions
- `docs/MACOS_BUILD_INSTRUCTIONS.md` - macOS-specific build guide
- `.github/instructions/project.instructions.md` - Project-level guidelines

### Test Framework
- `tests/` - Unit and integration tests
- `run_tests.sh` - Automated test execution

### Version Control
- All phases tracked in conventional commits
- Each phase tagged at completion: `phase-41-complete`, etc.
- Development diary updated daily

## Commit Convention

All commits follow conventional format:

```
feat(phase41): implement draw primitive operations for basic geometry rendering

- Add DrawPrimitive with triangle strip support
- Implement IndexedPrimitive for indexed geometry
- Add render state management (depth, blending)

Refs: Phase 41 - Drawing Operations
```

Key rules:
- Type: `feat`, `fix`, `docs`, `test`, `refactor`, `perf`
- Scope: Phase number and component (e.g., `phase41`, `graphics`)
- Subject: Clear, concise description
- Body: Detailed explanation with bullet points
- Reference: Issue/phase reference

## Execution Checklist

### Before Each Phase
- [ ] Read phase documentation thoroughly
- [ ] Review all dependencies completed
- [ ] Create feature branch: `phase-XX-implementation`
- [ ] Set up test environment

### During Each Phase
- [ ] Write unit tests first
- [ ] Implement features incrementally
- [ ] Run tests after each significant change
- [ ] Document complex logic with comments
- [ ] Update development diary daily

### After Each Phase
- [ ] All tests passing (100%)
- [ ] Code reviewed for quality
- [ ] Update MACOS_PORT_DIARY.md
- [ ] Create conventional commit
- [ ] Merge to main branch
- [ ] Tag phase completion

## Contact & Support

For questions or blockers during implementation:
1. Check phase documentation for solutions
2. Review reference repositories in `references/`
3. Consult Vulkan documentation
4. Update LESSONS_LEARNED.md with discoveries

---

**Document Version**: 1.0  
**Last Updated**: 2025-10-31  
**Next Review**: After Phase 41 completion
