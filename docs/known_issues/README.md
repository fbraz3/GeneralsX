# Known Issues

This directory contains documentation of known limitations and issues in the GeneralsX project.

## Index

### Graphics & Rendering

#### 1. [Wide Texture Rendering Bug](WIDE_TEXTURE_RENDERING_BUG.md)

**Status**: 🟡 Known Limitation - Accepted  
**Platform**: macOS ARM64 (Metal backend)  
**Severity**: Low (cosmetic only)

**Summary**: Textures with width ≥1024 pixels render with the right half replaced by solid orange color. Affects 4 textures (victory/defeat screens). Root cause unidentified after 30+ hours investigation.

**Impact**:

- Affected textures: 4 of 36 (11%)
- Gameplay: 0% impact (fully playable)
- Context: End-game screens only (non-critical)

**Workarounds**:

- Resize affected textures to 512×256
- Use BC3→RGBA8 CPU decompression (already implemented)

**Related Files**:

- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm`
- `Core/Libraries/Source/WWVegas/WW3D2/bc3decompressor.cpp/h`

---

## Reporting New Issues

When documenting a new known issue, please include:

1. **Clear description** of the problem
2. **Reproduction steps** with minimal test case
3. **Investigation summary** of what was tested
4. **Impact analysis** (gameplay, performance, stability)
5. **Workarounds** if available
6. **Related files** and code references
7. **Platform/environment** specifics

## Issue Status Definitions

- 🔴 **Active Bug**: Under active investigation, blocking progress
- 🟡 **Known Limitation**: Documented and accepted, non-blocking
- 🟢 **Resolved**: Fixed but documented for historical reference
- 🔵 **Platform-Specific**: Limited to specific OS/hardware configurations

