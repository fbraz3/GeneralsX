# GeneralsX Linux Port - Phase Roadmap (Reorganized)

**Updated**: 2026-02-08  
**Status**: Ready for execution  
**Strategy**: Leverage fighter19 proven code + avoid sunk-cost engineering

🔗 **Related Document**: [Proprietary Tech Roadmap](./PROPRIETARY_TECH_ROADMAP.md) - Strategic elimination of proprietary dependencies across phases

---

## 🎯 Why Reorganize?

**Old sequence**: Graphics → Audio → Video → CompatLib  
**Problem**: Spending Phase 2-3 rebuilding what fighter19 already solved ✗

**New sequence**: Graphics → CompatLib Review → Audio (reuse) → Video → Polish  
**Benefit**: Reduces duplicated effort, validates patterns early ✓

---

## Phase 0: Deep Analysis & Planning ✅ COMPLETE

### Completion Criteria ✅

- [X] Current renderer architecture documented (DX8 interfaces, entry points)
- [X] fighter19 DXVK patterns documented with "what applies to Zero Hour" notes
- [X] jmarshall OpenAL patterns documented with "how to adapt for Zero Hour" notes
- [X] Platform abstraction layer design approved (minimal changes to game logic)
- [X] Docker build workflow configured and tested (native Linux ELF compilation)
- [X] Windows build validation pipeline established (VC6/Win32 must pass)
- [X] Phase 1 implementation plan written with step-by-step checklist
- [X] **NEW**: fighter19 CompatLib architecture fully understood
- [X] **NEW**: Audio implementation (OpenAL) mapped to Phase 2
- [X] **NEW**: Phase sequence reorganized with CompatLib migration

### Deliverable
- ✅ [Phase 0 Analysis Document](./phase0-fighter19-complete-analysis.md)

---

## Phase 1: Linux Graphics - DXVK Integration (ACTIVE)

**Goal**: Port fighter19's DXVK graphics layer to GeneralsXZH, enabling Linux rendering via Vulkan.

**Scope**:
- Implement DXVK DirectX8→Vulkan wrapper integration
- Port SDL3 windowing/input layer (replace Win32 window management)
- Add MinGW build preset for Linux (32-bit i686, 64-bit x86_64)
- Isolate graphics changes to `Core/GameEngineDevice/` and platform headers
- Keep Win32 DX8 path intact behind compile flags
- **NEW**: Protect Windows headers with `#ifdef` guards to reduce Linux build errors

**Non-scope**:
- Audio (will be silent/stubbed for Phase 1)
- Video (defer to Phase 3)
- Full CompatLib integration (defer to Phase 1.5)

**Current Status** (Session 12):
- ✅ Docker build setup complete
- ✅ Intrin compatibility (math/CPU functions) implemented
- ✅ Registry stubs created (valid pattern per fighter19)
- ✅ Graphics layer stubs created
- 🏗️ Build proceeding: 827 tasks recognized, 11 compiling initially
- 🏗️ Core game logic files compiling successfully

**Next Immediate Actions**:
1. Continue protecting Windows-specific headers (`#ifdef _WIN32` guards)
2. Implement D3D8 device stub layer (deferred to Phase 1.5 or minimal for Phase 1)
3. Validate core compilation succeeds (game logic layer)
4. Hit graphics device errors as expected (Phase 1 scope)

**Acceptance Criteria**:
Phase 1 is **COMPLETE** when:
- [ ] GeneralsXZH builds with Linux preset (`linux64-deploy`) via Docker
- [ ] Native Linux ELF binary launches and reaches main menu with graphics rendered
- [ ] Basic gameplay (skirmish map) runs without crashes
- [ ] Windows builds (VC6/Win32) still compile and run correctly
- [ ] Platform-specific code properly isolated (no Linux code in game logic)
- [ ] **NEW**: Statistics collected (compilation time, binary size, memory usage on Linux)

**Estimated Duration**: 2-3 weeks (parallel with CompatLib evaluation)

---

## Phase 1.5: CompatLib Evaluation & Migration 🆕

**Goal**: Evaluate fighter19's CompatLib architecture and decide: adopt, adapt, or parallel-build.

**Rationale**: Avoid duplicating string/thread compatibility work in Phase 2

**Scope**:
- Deep code review of fighter19's CompatLib (thread_compat, string_compat, file_compat, etc.)
- Evaluate: Can we use fighter19's code directly on our codebase?
- Decision point: Merge CompatLib files or keep our incremental intrin_compat approach?
- If adopting fighter19: Refactor into static library target in CMakeLists.txt
- If parallel-building: Document what string/thread compatibility we need for Phase 2
- Validate Windows builds still pass after any CompatLib changes
- **Resolve game registry usage**: Can we use fighter19's approach or need enhanced stubs?

**Key Questions**:
1. Are fighter19's compat implementations correct for our codebase?
   - Different game code paths?
   - Different Windows API usage?
   - Different compiler expectations?

2. Will CMake refactoring break our build structure?
   - Do we need CompatLib as separate target or keep header-only?
   - How does it interact with existing DX8 stubs?

3. What compat areas are actually needed?
   - Thread compatibility? (likely yes, Phase 2+)
   - String compatibility? (maybe, some MSVC intrinsics)
   - File I/O compat? (unknown, may not be needed)
   - Module loading compat? (probably not)

**Acceptance Criteria**:
Phase 1.5 is **COMPLETE** when:
- [ ] CompatLib structure fully documented (what each module does, what we use)
- [ ] Decision made: Adopt fighter19's CompatLib (Y/N)?
- [ ] If YES: CMakeLists.txt refactored, Windows builds validated
- [ ] If NO: Document why + plan for Phase 2 compatibility needs
- [ ] Phase 2 acceptance criteria updated with compat assumptions

**Estimated Duration**: 1-2 weeks (parallel with Phase 1 graphics work)

**Ownership**: Can be parallel work while graphics stabilizes

---

## Phase 2: Linux Audio - OpenAL Integration

**Goal**: Implement audio support using OpenAL, with fighter19's OpenALAudioDevice as proven reference.

**Scope**:
- **Copy fighter19's OpenALAudioDevice implementation** (proven working for effects)
- Create Miles→OpenAL API compatibility layer for music/long voice lines
- Work on audio format conversion (Miles formats→OpenAL formats)
- Isolate audio changes to `Core/GameEngine/Audio/` (new abstraction layer)
- Keep Miles path intact behind compile flags for Windows
- Test audio quality (no pops, clicks, timing issues)

**Known Constraints** (from fighter19):
- ✅ Sound effects work (short, looping)
- ❌ Music/long voices not yet working (Miles format incompatibility)
- May need intermediate solution (silent for music? or fall back to Miles?)

**Acceptance Criteria**:
Phase 2 is **COMPLETE** when:
- [ ] GeneralsXZH Linux build has working sound effects
- [ ] Audio effectiveness comparable to original (no silence except known gaps)
- [ ] Windows builds still use Miles Sound System (no regressions)
- [ ] Audio backend selection via compile flag (`USE_OPENAL` vs `USE_MILES`)
- [ ] Partial support acknowledged: "Effects work, long voices stubbed"

**Estimated Duration**: 2-3 weeks

**Starting Point**: Use fighter19's `GameEngineDevice/Source/OpenALAudioDevice/` directly

---

## Phase 3: Video Playback - FFmpeg or Stub? 🧐

**Goal**: Investigate and prototype video playback or graceful degradation.

**Scope**:
- Research: What video formats does Generals use? (Bink? AVI? something else?)
- Investigate: Can fighter19's VideoDevice handle it?
- Prototype: Either FFmpeg wrapper OR skip videos gracefully
- Document findings and recommend path forward
- **This is a SPIKE**: May result in "defer indefinitely" decision

**Known Unknowns**:
- fighter19 has `VideoDevice/` but status unknown
- README doesn't mention video status
- May be fully implemented, partially stubbed, or not started

**Acceptance Criteria**:
Phase 3 is **COMPLETE** when:
- [ ] Video requirements documented (what file formats, how many, where used)
- [ ] fighter19's VideoDevice status verified (working/broken/stubbed)
- [ ] At least one prototype implemented and tested
- [ ] Recommendation written: "implement FFmpeg wrapper" or "stub/skip videos"
- [ ] If stubbed: Game fails gracefully without videos (skip to menu)

**Estimated Duration**: 1-2 weeks (spike)

**Fallback Strategy**: Videos are nice-to-have. If too complex, ship with: "intro videos skip automatically, load directly to menu"

---

## Phase 4: Polish & Hardening

**Goal**: Address Linux-specific bugs, performance, and compatibility issues beyond Phases 1-3.

**Scope** (TBD based on Phase 1-3 findings):
- Performance tuning (CPU/GPU profiling)
- Crash/crash handler improvements
- Configuration file handling (INI parser robustness)
- Save game compatibility (cross-platform save format)
- Multiplayer networking (if applicable)
- Edge case testing (mods, custom maps, etc.)

**Estimated Duration**: 2-4 weeks

---

## Timeline Summary

```
┌────────┬──────────────────────────────────────────────────────────────┐
│ Phase  │ Duration   │ Start Date   │ Est. End    │ Status             │
├────────┼────────────┼──────────────┼─────────────┼────────────────────┤
│ 0      │ Complete   │ Jan 8        │ Feb 8       │ ✅ DONE            │
│ 1      │ 2-3 weeks  │ Feb 8        │ Feb 22-28   │ 🏗️ ACTIVE         │
│ 1.5    │ 1-2 weeks  │ Feb 15*      │ Feb 22-28   │ 🔵 QUEUED (parallel)│
│ 2      │ 2-3 weeks  │ Feb 28       │ Mar 14-21   │ ⏸️ WAITING        │
│ 3      │ 1-2 weeks  │ Mar 14       │ Mar 21-28   │ ⏸️ WAITING        │
│ 4      │ 2-4 weeks  │ Mar 28       │ Apr 18-May2 │ ⏸️ WAITING        │
└────────┴────────────┴──────────────┴─────────────┴────────────────────┘

* Phase 1.5 can start during Phase 1 (parallel work on CompatLib)
  - Same developer can switch tasks if Phase 1 hits blockers
  - Or: Team member reviews CompatLib while another does graphics
```

---

## Success Criteria - Overall

Linux port successful when:

1. ✅ **Phase 1 SUCCESS**: GeneralsXZH runs on Linux with graphics rendered
   - Native ELF binary (`./GeneralsXZH -win` works on Ubuntu 22.04)
   - Reaches main menu
   - Can load and play skirmish map

2. ✅ **Phase 2 SUCCESS**: Audio working for 80%+ of sounds
   - Sound effects play (weapons, explosions, unit vocalizations)
   - Known gap: Long music/voice lines (acceptable limitation)
   - Windows builds unaffected

3. ✅ **Phase 3 SUCCESS**: Video playback solved (Y/N decision made)
   - If "implement": Intro videos play correctly
   - If "stub": Game launches without videos, no errors
   - Player understands the tradeoff

4. ✅ **Phase 4 SUCCESS**: Linux port production-ready
   - No crashes below P2 severity
   - Consistent 30+ FPS on reference hardware
   - Mods and custom maps work (if applicable)
   - Windows builds remain retail compatible

---

## Known Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Audio format mismatch (Miles→OpenAL) | Phase 2 blocker | Use fighter19's code directly; fallback to silence |
| Video codec complexity | Phase 3 blocker | Decide early to stub; don't spend 4 weeks on this |
| Windows build breaks during compat refactor | Phase 1.5 blocker | Maintain separate intrin_compat.h until validated |
| Graphics performance issues | Phase 1 risk | Profile early; have fallback rendering path |
| Retail compatibility regression | Any phase risk | Run VC6 build + replay tests after each major change |

---

## Review & Approval

- [X] Phase 0 complete (as of Feb 8, 2026)
- [X] Phase 1 starting (Feb 8)
- [ ] Phase 1.5 plan approved (pending tech review)
- [ ] Phase 2 scope approved (pending Phase 1 completion)
- [ ] Phase 3 scope approved (pending Phase 2 completion)
- [ ] Phase 4 scope approved (pending Phase 3 completion)

**Next sync**: After Phase 1 graphics protection complete (est. Feb 15)

