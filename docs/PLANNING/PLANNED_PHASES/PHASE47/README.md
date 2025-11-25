# Phase 47: Release & Distribution

**Phase**: 47  
**Title**: Release Management, Binary Distribution & Version Control  
**Area**: DevOps & Release Engineering  
**Scope**: LARGE  
**Status**: not-started  
**Difficulty**: MEDIUM  
**Estimated Duration**: 2-3 weeks  
**Dependencies**: Phase 39-46 (All optimization & testing phases)

---

## Quick Reminders

- Use `Fail fast` approach when testing new changes
- Focus on GeneralsXZH first, then backport to GeneralsX
- See `.github/instructions/project.instructions.md` for project guidelines
- Check for integration issues from previous phases before proceeding
- Always include rollback capability for applied fixes

---

## Overview

Phase 47 implements a **complete release management system** for building, versioning, packaging, and distributing GeneralsX binaries across all supported platforms (Windows, macOS, Linux).

## Objectives

1. **Semantic Versioning**: Version management system
2. **Multi-Platform Builds**: Automated cross-platform compilation
3. **Binary Distribution**: Release artifact packaging
4. **Changelog Generation**: Automated release notes
5. **Installer Creation**: Platform-specific installers
6. **Code Signing**: Security & authentication
7. **Update Mechanism**: In-game update system

## Key Deliverables

- [ ] VersionManager (semantic versioning)
- [ ] BuildOrchestrator (multi-platform builds)
- [ ] BinaryPackager (release artifact creation)
- [ ] ChangelogGenerator (automated release notes)
- [ ] InstallerBuilder (platform-specific installers)
- [ ] ReleaseValidator (integrity checks)
- [ ] UpdateSystem (in-game updates)
- [ ] ReleaseGitHubAction (CI/CD workflow)

## Key Files

- `scripts/release/build_release.sh`
- `scripts/release/package_release.sh`
- `scripts/release/changelog_generator.py`
- `.github/workflows/release.yml`

## Technical Details

### Versioning Strategy

```
Semantic Versioning: MAJOR.MINOR.PATCH

Examples:
- 1.0.0 - Initial stable release
- 1.1.0 - Minor features added
- 1.1.1 - Bug fixes
- 1.2.0-beta - Pre-release

Version File Structure:
├── version.h (C++ defines)
├── version.txt (current version)
├── version.json (full metadata)
└── CHANGELOG.md (release notes)
```

### Build Artifacts

```
Release Build Structure:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

GeneralsX-v1.2.0/
├── windows/
│   ├── GeneralsX-1.2.0.exe
│   ├── GeneralsX-1.2.0.msi
│   └── dependencies/
├── macos/
│   ├── GeneralsX-1.2.0.dmg
│   ├── GeneralsX-1.2.0-arm64.app/
│   ├── GeneralsX-1.2.0-x86_64.app/
│   └── GeneralsX-1.2.0-arm64-x86_64.app (Universal)
├── linux/
│   ├── generalsX-1.2.0-x86_64.tar.gz
│   ├── generalsX-1.2.0-x86_64.AppImage
│   └── generalsX-1.2.0.x86_64.rpm
├── CHANGELOG.md
└── RELEASE_NOTES.md
```

### Release Workflow

```
Step 1: Version Bump
  - Update version.h
  - Update CHANGELOG.md
  - Tag Git commit
  ↓
Step 2: Multi-Platform Build
  - Build for Windows (VC6)
  - Build for macOS ARM64
  - Build for macOS x86_64
  - Build for Linux x86_64
  ↓
Step 3: Testing
  - Run test suite (all platforms)
  - Verify no regressions
  - Validate binary sizes
  ↓
Step 4: Packaging
  - Create installers
  - Sign binaries (code signing)
  - Generate checksums (SHA256)
  ↓
Step 5: Release
  - Create GitHub release
  - Upload artifacts
  - Update documentation
  - Announce release
```

## Acceptance Criteria

### Build & Compilation

- [ ] All platforms build successfully
- [ ] Release binaries optimized
- [ ] Binary sizes acceptable

### Packaging

- [ ] All artifacts created correctly
- [ ] Checksums match
- [ ] Code signing functional (if applicable)

### Distribution

- [ ] GitHub release created
- [ ] All artifacts uploaded
- [ ] Download links working

### Validation

- [ ] Installer tests pass
- [ ] Binary integrity verified
- [ ] Version info correct in all builds

## Testing Strategy

### Pre-Release Testing

- [ ] Install test on Windows
- [ ] Install test on macOS
- [ ] Install test on Linux
- [ ] Uninstall verification
- [ ] Upgrade path testing

### Post-Release Validation

- [ ] SHA256 checksum verification
- [ ] Binary signature validation
- [ ] Installer execution tests
- [ ] Version display verification

## Success Criteria

- Automated builds for all platforms
- Release process <30 minutes
- All artifacts generated correctly
- Zero failed releases (0% failure rate)
- Complete changelog automation

## Implementation Plan

### Week 1: Versioning & Build Infrastructure

- [ ] VersionManager implementation
- [ ] CMake release target
- [ ] BuildOrchestrator (multi-platform)
- [ ] Build script automation

### Week 2: Packaging & Signing

- [ ] BinaryPackager
- [ ] Installer builders (Windows MSI, macOS DMG, Linux AppImage)
- [ ] Code signing integration
- [ ] Checksum generation

### Week 3: Release Automation

- [ ] ChangelogGenerator
- [ ] GitHub Actions workflow
- [ ] Release validation
- [ ] Documentation automation

## Release Channels

```
Channels:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Stable:
  - Thoroughly tested
  - Recommended for most users
  - Monthly releases
  
Beta:
  - Pre-release builds
  - New features early access
  - Weekly releases
  
Development:
  - Bleeding edge
  - Continuous builds
  - For contributors
```

## Installer Specifications

### Windows (MSI Installer)

```
Features:
- Standard installation directory
- Desktop shortcut creation
- Start menu entries
- Uninstall support
- Registry entries (optional)
- Prerequisite checking (.NET, DirectX)
```

### macOS (DMG Package)

```
Features:
- Drag & drop installation
- Code signing & notarization
- Gatekeeper compatibility
- Universal binary (ARM64 + x86_64)
- Sparkle framework integration
```

### Linux (AppImage & RPM)

```
Features:
- No installation required (AppImage)
- Self-extracting
- Desktop integration
- RPM package for distributions
- Dependency management
```

## Version Metadata

```json
{
  "version": "1.2.0",
  "releaseDate": "2025-11-15",
  "codeName": "Performance Surge",
  "buildNumber": 4521,
  "gitCommit": "abc1234def5678ghi9101112",
  "stability": "stable",
  "minimumOS": {
    "windows": "Windows 7 SP1",
    "macos": "macOS 10.13+",
    "linux": "Ubuntu 18.04+"
  },
  "features": [
    "Multi-platform optimization",
    "Advanced profiling",
    "Performance dashboard",
    "ML regression detection"
  ],
  "knownIssues": [],
  "upgradeNotes": "Optional but recommended"
}
```

## GitHub Actions Workflow

```yaml
name: Release

on:
  push:
    tags:
      - 'v*'

jobs:
  build:
    strategy:
      matrix:
        os: [windows-latest, macos-latest, ubuntu-latest]
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v3
      - name: Build Release
        run: bash scripts/release/build_release.sh
      - name: Package Artifacts
        run: bash scripts/release/package_release.sh
      - name: Upload Artifacts
        uses: actions/upload-artifact@v3

  release:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Create Release
        uses: actions/create-release@v1
        with:
          tag_name: ${{ github.ref }}
          release_name: Release ${{ github.ref }}
          body: ${{ env.CHANGELOG }}
      - name: Upload to Release
        uses: actions/upload-release-asset@v1
```

## Related Phases

- **Phase 39-46**: All optimization and testing phases precede release
- **Phase 47**: Final delivery pipeline

---

**Last Updated**: November 12, 2025  
**Status**: Ready for Implementation
