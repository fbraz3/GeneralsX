# Proprietary Tech Roadmap: Liberation Strategy

**Document Type**: Strategic Dependencies Analysis  
**Author**: Community Planning  
**Date**: 2026-02-13  
**Status**: Living Document (Update as libs are replaced)  
**Goal**: Track and eliminate all proprietary/closed-source technical dependencies

---

## Executive Summary

GeneralsX still carries vestiges from its proprietary past. This document maps all remaining proprietary tech, proposes replacements, and schedules elimination across project phases.

**Current Status (Phase 2)**:
- ✅ ~70% proprietary code eliminated (graphics, audio baseline)
- ⚠️ ~30% remaining (mostly stubs and Windows-specific)
- 🎯 Target: 100% open source by Phase 5+

---

## Proprietary Dependencies Matrix

### Legend
- **Currently Used**: In active use (Windows builds)
- **Stubbed**: Disabled (Linux builds or non-critical)
- **Deprecated**: Scheduled for removal
- **Status**: Replacement progress

---

## 1. Audio System

### Miles Sound System (RAD Game Tools)

| Property | Value |
|----------|-------|
| **Name** | Miles Sound System SDK |
| **Vendor** | RAD Game Tools (acquired by Qualcomm) |
| **License** | Proprietary (closed source) |
| **Currently Used** | ✅ Windows builds (MSVC 2022, VC6) |
| **Linux Status** | 🔴 Removed (stub only) |
| **Located In** | `Core/GameEngineDevice/Source/MilesAudioDevice/` |
| **Replacement** | OpenAL (Phase 2) → SDL3 Audio (Phase 4) |

#### Phase Timeline

| Phase | Action | Status | Effort |
|-------|--------|--------|--------|
| **Phase 0** | Analysis complete | ✅ Done | - |
| **Phase 1** | DXVK graphics → OpenAL audio stub | ✅ Done | 40h |
| **Phase 2** | OpenAL full implementation (current) | 🔄 In Progress | 40h |
| **Phase 4** | Migrate OpenAL → SDL3 Audio | 📋 Planned | 24-35h |
| **Phase 5+** | (Optional) Keep both for compatibility | Optional | 0h |

#### Details
- **Files**: `MilesAudioManager.h/cpp`, `MilesAudioDevice/`
- **API**: DirectSound-like (HSTREAM, HSAMPLE, H3DSAMPLE)
- **Game Impact**: Critical (all in-game audio)
- **Can't Remove From Windows?** No—OpenAL works fine on Windows too (just need rebuild)
- **Effort to Replace**: ~40 hours (OpenAL) + 24-35 hours (SDL3 future)

---

## 2. Video Codec

### Bink Video (RAD Game Tools)

| Property | Value |
|----------|-------|
| **Name** | Bink Video Codec SDK |
| **Vendor** | RAD Game Tools (acquired by Qualcomm) |
| **License** | Proprietary (closed source, deprecated?) |
| **Currently Used** | ✅ Windows builds (intro, campaign videos) |
| **Linux Status** | 🔴 Removed (stub/FFmpeg fallback) |
| **Located In** | `Core/GameEngineDevice/Source/VideoDevice/Bink/` |
| **Replacement** | FFmpeg (Phase 3) |

#### Phase Timeline

| Phase | Action | Status | Effort |
|-------|--------|--------|--------|
| **Phase 0** | Analysis complete | ✅ Done | - |
| **Phase 1** | Create video stub | ✅ Done | 8h |
| **Phase 2** | Audio system (video stubs) | ✅ Done | - |
| **Phase 3** | FFmpeg fallback (spike) | 📋 Planned | 20-40h |
| **Phase 4+** | Full FFmpeg replacement for Windows | Optional | 16-24h |

#### Details
- **Files**: `BinkVideoPlayer.h/cpp`, integration with audiotrack
- **API**: `BinkOpen()`, `BinkDoFrame()`, `BinkSetSoundTrack()`
- **Game Impact**: Non-critical (cinematics only, can skip)
- **Can't Remove From Windows?** No—FFmpeg works fine (just re-encode Bink→VP9/H.264)
- **Known Issue**: Bink codec is proprietary, no open-source decoder (would need to reverse-engineer or re-encode)
- **Current Approach**: Both Windows and Linux use FFmpeg fallback (Bink DLL on Windows can be optionally used)
- **Effort to Replace**: ~20-40 hours (FFmpeg spike) + 16-24 hours (full port if needed)

---

## 3. Graphics: DirectX 8 (Microsoft)

### DirectX 8 Headers & Runtime

| Property | Value |
|----------|-------|
| **Name** | DirectX 8 Graphics API |
| **Vendor** | Microsoft |
| **License** | Proprietary (closed source runtime) |
| **Currently Used** | ✅ Windows VC6 builds (native rendering) |
| **Linux Status** | 🟠 Wrapped (DXVK Vulkan translation) |
| **Located In** | `Core/GameEngineDevice/Source/W3DDevice/` |
| **Replacement Option A** | SDL3 Graphics (unified ecosystem) - **RECOMMENDED** |
| **Replacement Option B** | Full Vulkan native (high effort, lower ROI) |

#### Phase Timeline

| Phase | Action | Status | Effort | Strategy |
|-------|--------|--------|--------|----------|
| **Phase 0** | Architecture analysis (fighter19 patterns) | ✅ Done | - | - |
| **Phase 1** | DXVK wrapper (Vulkan layer done) | 🔄 Active | 80-120h | Interim solution |
| **Phase 2** | Audio/platform layer | - | - | - |
| **Phase 3** | Video integration | - | - | - |
| **Phase 4** | SDL3 Audio deployed | 📋 Planned | 24-35h | Audio unified ✅ |
| **Phase 5A** | **SDL3 Graphics migration** (RECOMMENDED) | 📋 Optional | 60-80h | **Full SDL3 ecosystem** |
| **Phase 5B** | Full native Vulkan rewrite | 📋 Alternative | 100-150h | **More effort, less benefit** |

#### Strategic Decision: SDL3 Graphics vs Vulkan Pure

**Option A: SDL3 Graphics (RECOMMENDED)**
```
Phase 4+:
├── SDL3 Audio ✅
├── SDL3 Graphics (renderer abstraction)
└── SDL3 Input/Windowing ✅
= 100% Unified SDL3 ecosystem (graphics + audio + input)
= Multi-backend support (Vulkan, Metal, DirectX, OpenGL)
= Lower code complexity
= Better maintainability
```

**Option B: Vulkan Pure (NOT RECOMMENDED - Higher effort, lower benefit)**
```
Phase 5A+:
├── Custom Vulkan renderer (rewrite W3DDevice)
├── OpenAL Audio (separate)
├── SDL3 Input/Windowing ✅
= Vulkan-only (no DirectX support)
= More complex custom code
= Harder to maintain
= Same end result with more pain
```

#### Why SDL3 Graphics Makes More Sense

| Aspect | SDL3 Graphics | Vulkan Pure |
|--------|---------------|-------------|
| **API Level** | High-level (simple) | Low-level (complex) |
| **Code Written** | ~60-80h | ~100-150h |
| **Backends** | Vulkan + Metal + DX + GL (multi) | Vulkan only (locked) |
| **Portability** | macOS + Windows + Linux | Mostly Linux + Windows |
| **Ecosystem** | Unified (audio + graphics) | Fragmented (separate audio) |
| **Performance** | Excellent (Vulkan backend on Linux) | Excellent (same Vulkan code) |
| **Maintenance** | SDL3 team maintains | Custom code maintenance |
| **Future flexibility** | Add Mac Metal backend easily | Vulkan-limited (no Metal) |
| **Synergy with Phase 4** | Perfect (SDL3 Audio already done) | No (separate systems) |

#### Details
- **Files**: Large (`W3DDevice/`, renderer code)
- **Current Strategy**: DXVK wrapper (phase 1) is interim solution
- **Windows Path**: Direct X8 (closed source) → SDL3 windows with DX backend (if available)
- **Linux Path**: DXVK + Vulkan (open source!) → SDL3 with Vulkan backend
- **Can Replace?** Yes, SDL3 Graphics is best approach (Phase 5A), Vulkan pure is over-engineered (Phase 5B alternative)
- **Strategic Note**: SDL3 Graphics should happen AFTER Phase 4 (SDL3 Audio), for unified ecosystem

#### SDL3 Graphics Implementation Path

1. **Abstract W3DDevice** → SDL3 Renderer interface
2. **Port rendering calls** (Low-level D3D8 → higher-level SDL GPU API)
3. **Test on Linux** (Vulkan backend)
4. **Test on Windows** (SDL3 DirectX backend, or custom D3D8 backend)
5. **Result**: Entire graphics pipeline unified under SDL3

---

## 4. Build Tools & SDKs

### Visual Studio 6 Build System

| Property | Value |
|----------|-------|
| **Name** | Microsoft Visual Studio 6 (VC6) |
| **Vendor** | Microsoft |
| **License** | Proprietary (closed source) |
| **Currently Used** | ✅ Legacy Windows builds (retail compatibility) |
| **Replacement** | MSVC 2022 (modern Windows) OR Clang/GCC (cross-platform) |
| **Status** | ✅ CMake build system created (open source alternative) |

#### Details
- **Strategy**: Not removing VC6 (retail compatibility), just superseding with CMake
- **Current**: CMake generates VC6 project files via VS6 generator
- **Result**: Users can choose VC6 project files OR CMake (both free now)
- **Effort**: Already done (CMake infrastructure)

---

### MaxSDK (Proprietary 3D Asset Tool)

| Property | Value |
|----------|-------|
| **Name** | Autodesk MaxSDK |
| **Vendor** | Autodesk (formerly Discreet) |
| **License** | Proprietary (closed source) |
| **Currently Used** | ⚠️ Optional (VC6 builds only, asset tools) |
| **Located In** | `Dependencies/MaxSDK/` |
| **Replacement** | Blender SDK or skip (use exported assets) |
| **Status** | 📋 Not blocking Linux port |

#### Details
- **Purpose**: In-game asset compilation (3DS Max models → .W3D format)
- **Impact**: Non-critical (artists already exported assets, source `.max` files not needed)
- **Can Remove?** Yes, but need Blender SDK replacement (future work)
- **Effort**: 20-40 hours if decision is made to support Blender workflows

---

## 5. Multiplayer & Networking

### GameSpy (GameSpy Industries → IGN Entertainment)

| Property | Value |
|----------|-------|
| **Name** | GameSpy SDK |
| **Vendor** | GameSpy Industries (defunct, acquired, shut down 2014) |
| **License** | Proprietary (closed source, NOW DEFUNCT!) |
| **Currently Used** | ⚠️ Code exists but disabled (multiplayer) |
| **Located In** | `Core/Libraries/Source/GameSpy/` |
| **Status** | 🔴 DEPRECATED (GameSpy shut down) |
| **Replacement** | Custom solution OR open-source alternatives (phase 5+) |

#### Details
- **Critical Problem**: GameSpy shut down in 2014, servers are offline
- **Impact**: Multiplayer/online features non-functional (was already broken before Linux port)
- **Can Remove?** Yes (already broken)
- **Strategic Note**: This is OUT OF SCOPE for Linux port phases 1-4
- **Future (Phase 5+)**: Custom LAN multiplayer or open-source backend
- **Effort**: 40-80 hours if multiplayer is priority (not for Phase 1-4)

---

## 6. Telemetry & Analytics

### RAD Tools Telemetry/Metrics (Suspected)

| Property | Value |
|----------|-------|
| **Name** | RAD Game Tools Telemetry (if present) |
| **Vendor** | RAD Game Tools |
| **License** | Proprietary (closed source) |
| **Currently Used** | ❓ Unknown (search needed) |
| **Impact** | Low (non-critical) |
| **Status** | 📋 Needs audit |

#### Action
- [ ] Search codebase for RAD telemetry calls
- [ ] If found, remove (privacy + open source)
- [ ] Effort: <2 hours to audit

---

## Proprietary Tech Elimination Roadmap

### Current State

```
Original (2003)
├── DirectX 8 ✗ (Windows-only)
├── Miles Sound System ✗ (Windows-only)
├── Bink Video ✗ (Windows-only)
├── MaxSDK ✗ (tools, optional)
├── GameSpy ✗ (defunct, broken)
└── (Various proprietary toolchains)

Phase 1 (Graphics)
├── DirectX 8: Wrapped in DXVK ⚠️ (Vulkan on Linux, D3D8 on Windows)
└── ✅ DXVK is open source (Wine project)

Phase 2 (Audio)
├── Miles: Replaced with OpenAL ✅ (open source)
└── ✅ Unified cross-platform audio

Phase 3 (Video)
├── Bink: Replaced with FFmpeg ✅ (open source)
└── ✅ Support both Bink (Windows) and FFmpeg (Linux) gracefully

Phase 4 (Polish)
├── BinkVideoPlayer: Full FFmpeg on Windows (optional)
├── Miles: OpenAL fully wired → SDL3 Audio migration (optional)
└── 🎯 Target: Almost 100% open source core

Phase 5 (Strategic Unification - 100% SDL3 Ecosystem)
├── ✅ DirectX 8 → SDL3 Graphics (renderer backend abstraction)
├── ✅ Miles → SDL3 Audio (already Phase 4)
├── ✅ Bink → FFmpeg (already Phase 3)
├── ✅ Win32 Windowing → SDL3 (already Phase 1)
├── ✅ Win32 Input → SDL3 (already Phase 1)
└── 🎯 Result: **100% open source, 100% SDL3 ecosystem core**
    └── Graphics + Audio + Input all unified
    └── Supports Vulkan + Metal + DirectX + OpenGL
    └── Fully community-maintained, zero vendor lock-in
```

---

## Replacement Priority Matrix

| Tech | Vendor | Replacement | Priority | Phase | Effort | Impact | Block? | Notes |
|------|--------|-------------|----------|-------|--------|--------|--------|-------|
| **Miles** | RAD | OpenAL → SDL3 Audio | 🔴 HIGH | 2→4 | Medium | Critical | No | Audio engine |
| **DirectX 8** | MSFT | SDL3 Graphics (RECOMMENDED) | 🔴 HIGH | 1→5 | Large | Critical | No | **Unified SDL3 ecosystem** |
| **Bink** | RAD | FFmpeg | 🟡 MEDIUM | 3 | Medium | Non-critical | No | Videos only |
| **Vulkan Pure** (NOT REC) | - | Full rewrite | 🟢 LOW | 5+ | Massive | None (same result) | No | **Over-engineered, skip** |
| **GameSpy** | defunct | Custom/open-source | 🟢 LOW | 5+ | Large | None (broken) | No | Already defunct |
| **MaxSDK** | Autodesk | Blender SDK | 🟢 LOW | 5+ | Medium | Low (tools) | No | Optional |
| **VC6** | MSFT | CMake/MSVC2022 | 🟢 LOW | 0+ | Low | Low (replaced) | No | Already done |

---

## Strategic Goals & Milestones

### Phase 2 Victory (Current)
```
✅ Audio: Miles → OpenAL (open source!)
✅ Graphics: DirectX → DXVK (open source wrapper!)
✅ Platform: Win32 API → SDL3 (open source!)
= 80% of runtime code is now open source
```

### Phase 4 Victory (Target)
```
✅ Audio: OpenAL → SDL3 Audio (unified!)
✅ Video: Bink → FFmpeg (open source!)
✅ Platform: All Linux rendering chain open source
= 95% of runtime code is open source
= Core game engine fully auditable
```

### Phase 5+ Victory (Long-term)
```
✅ Graphics: DirectX 8 → Full Vulkan (optional)
✅ Multiplayer: GameSpy → Custom solution
✅ Build tools: MaxSDK → Blender SDK (optional)
= 100% open source, no proprietary tech
= Fully community-maintained indefinitely
```

---

## Action Items (Community/Maintainers)

### Immediate (This Month)
- [ ] Audit codebase for any RAD telemetry (search for `telemetry`, `analytics`, etc.)
- [ ] Remove telemetry if found (privacy + open source)
- [ ] Document findings in this roadmap

### Short-term (Phase 2-3)
- [ ] Complete Phase 2 OpenAL implementation
- [ ] Start Phase 3 video playback (FFmpeg spike)

### Medium-term (Phase 4)
- [ ] ✅ Stabilize Linux build (graphics, audio, video)
- [ ] Consider SDL3 Audio migration (if strategic value high)
- [ ] Polish/optimization pass

### Long-term (Phase 5+)
- [ ] Evaluate GameSpy replacement (if multiplayer needed)
- [ ] Evaluate native Vulkan rewrite (if performance matters)
- [ ] Evaluate Blender SDK integration (if asset tools needed)

---

## Legal & Community Impact

### Why This Matters

1. **Code Sovereignty**: Community owns entire codebase, not vendor lock-in
2. **Auditability**: No black-box proprietary code (security, trust, compliance)
3. **Sustainability**: If vendor goes out of business, project lives on
4. **Recruitment**: "100% open source" attracts developers more than "mostly open"
5. **Legal**: GPL-3.0 license requires open-source dependencies (or compatible licensing)

### Current Legal Status

- ✅ Miles: Stubbed (not linked), OK
- ✅ Bink: Stubbed (not linked), OK
- ✅ DirectX: Only headers (allow proprietary), acceptable interim
- ❓ GameSpy: Already defunct/broken (non-issue)
- ⚠️ MaxSDK: Tool-only (not distributed in binary), OK

**Conclusion**: Linux port is legally sound. Windows builds are acceptable interim (Windows allows proprietary SDKs). Future: move to 100% open source.

---

## References

### Current Implementations
- [OpenAL Audio (Phase 2)](../phases/PHASE02_LINUX_AUDIO.md)
- [SDL3 Audio Migration (Phase 4)](../support/future-sdl3-audio-migration.md)
- [Video Playback (Phase 3)](../phases/PHASE03_VIDEO_PLAYBACK.md)
- [DXVK Graphics (Phase 1)](../phases/PHASE01_LINUX_GRAPHICS.md)

### External Projects
- **DXVK**: https://github.com/doitsujin/dxvk (DirectX→Vulkan wrapper)
- **OpenAL**: https://openal.org (3D audio API)
- **SDL3**: https://www.libsdl.org (graphics, audio, input)
- **FFmpeg**: https://ffmpeg.org (video/audio codec library)
- **Vulkan**: https://vulkan.org/native rendering API)

---

## Document Status

**Last Updated**: 2026-02-13  
**Next Review**: After Phase 2 completion (target: 2026-03-XX)  
**Maintained By**: Community

---

**Status Tracking**:
- [ ] Telemetry audit (if present)
- [ ] Phase 2 OpenAL complete
- [ ] Phase 3 FFmpeg spike
- [ ] Phase 4 SDL3 Audio decision
- [ ] Phase 5+ long-term strategy

**Progress**: Awaiting Phase 2 completion

