# PHASE40: Minimal Playable Release (v0.1.0)

## Phase Title

Minimal Playable Release - Version 0.1.0 Launch

## Objective

Prepare and release first playable version with core functionality, marking project MVP milestone.

## Dependencies

- All phases PHASE01-39 completed
- PHASE39 (Cross-Platform Validation)
- Understanding of Critical Lesson 5: Build System Sensitivity - Clean Reconfigure Protocol

## Key Deliverables

1. Release build configuration
2. Asset packaging
3. Release notes documentation
4. Version tagging (v0.1.0)
5. Distribution packages (macOS, Linux)

## Acceptance Criteria

- [ ] Game compiles in Release mode
- [ ] All features working
- [ ] Performance meets targets (60 FPS)
- [ ] No known critical bugs
- [ ] Release notes complete
- [ ] Installation instructions clear
- [ ] Supported platforms documented

## Technical Details

### Release Tasks

1. **Build Configuration**
   - Release optimization flags
   - Symbol stripping
   - Binary signing (macOS)

2. **Packaging**
   - Asset optimization
   - Installer creation
   - README and licenses

3. **Documentation**
   - Release notes
   - Changelog
   - Known issues
   - System requirements

4. **Distribution**
   - GitHub Releases
   - Installation instructions
   - Feedback mechanism

### Code Location

```
CMakeLists.txt (Release configuration)
RELEASE_NOTES.md
CHANGELOG.md
README.md
```

## Estimated Scope

**MEDIUM** (2-3 days)

- Build optimization: 0.5 day
- Packaging: 0.5 day
- Documentation: 1 day
- Release process: 1 day

## Status

**not-started**

## Important References
- **Critical Lesson 5**: `docs/PHASE00/CRITICAL_LESSONS.md` - Build System Sensitivity - Clean reconfigure protocol (Phase 48 discovery)
- **Build Targets**: `docs/PHASE00/BUILD_TARGETS.md` - Executable naming and deployment strategies

## Release Checklist

- [ ] All tests pass
- [ ] No performance regressions
- [ ] Graphics working on all platforms
- [ ] Audio working
- [ ] Controls responsive
- [ ] Menu functional
- [ ] Game loop stable
- [ ] No memory leaks
- [ ] Release notes written
- [ ] Changelog complete
- [ ] Assets finalized
- [ ] Build verified on all platforms

## Success Criteria

- Release builds successfully
- Game runs on all supported platforms
- Performance acceptable
- Users can install and play
- Bug reports actionable
- Community feedback positive

## Reference Documentation

- Semantic versioning (semver.org)
- Release notes best practices
- GitHub release workflows
- See all prior PHASEs for completion verification
