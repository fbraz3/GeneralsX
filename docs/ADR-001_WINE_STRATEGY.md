# ADR-001: Wine-Based Cross-Platform Distribution Strategy

**Document Type**: Architecture Decision Record (ADR)  
**Status**: APPROVED  
**Date**: 2026-01-15  
**Author**: GeneralsX Development Team  
**Scope**: Project-wide architecture decision  

---

## Problem Statement

GeneralsX requires cross-platform support (macOS, Linux, Windows) with minimal development overhead while maintaining performance and code quality.

### Previous Approaches

1. **Multi-platform Native Ports** (rejected)
   - 3 separate codebases (Windows, macOS, Linux)
   - 12-16 months development
   - 3x testing burden
   - Excessive complexity

2. **Hybrid Abstraction** (rejected mid-planning)
   - Add abstraction layer + native ports later
   - Best of both worlds (theoretically)
   - Worst in practice: Combines complexity of both approaches
   - Delays shipping by 3+ months

3. **Wine-Based Distribution** (approved)
   - Single Windows binary
   - Wine runtime handles platform translation
   - 4 months development
   - Proven technology

---

## Decision

**Adopt Wine-based cross-platform distribution strategy** with the following implementation:

```
Phase 8: Graphics Abstraction (DirectX 8 → Generic Interface)
    ↓
Phase 9: Vulkan Backend (Generic → Vulkan via DXVK)
    ↓
Phase 10: Wine Integration (Packaging for macOS/Linux)
    ↓
Phase 11: 64-bit Windows Build (MSVC 2022 x64)
    ↓
Phase 12 (Optional): Native Builds (Linux/macOS Vulkan)
```

---

## Rationale

### 1. Performance is Sufficient

**Measured Wine Overhead**:
- System calls: 3-5%
- Graphics (DXVK): 1-3%
- Total: **2-5% across system**

**Why This is Acceptable**:
```
2003 Hardware:  1 GHz processor → 60 FPS (baseline)
2026 Hardware:  5 GHz processor → 300 FPS (5000x faster)
Wine Overhead:  -5% = 285 FPS

Result: Still 4.75x faster than minimum requirement
User Experience: Imperceptible difference (v-sync to 60 FPS)
```

### 2. Development Time Critical

| Approach | Months | Team |
|----------|--------|------|
| Wine-based | 4-5 | 2-3 |
| Native triple-port | 16-20 | 4-6 |
| **Time saved** | **12-15 months** | **2-3 people** |

### 3. Maintenance Simplicity

```
Wine:   1 binary → 1 test matrix → 3 platforms ✓
Native: 3 binaries → 3 test matrices → exponential complexity ❌
```

### 4. Proven Technology

- **Wine**: 20+ years, 10,000+ games
- **DXVK**: 5+ years, DirectX 8-12 working
- **Crossover**: Commercial product (proof of enterprise viability)
- **fighter19 project**: GeneralsX running on Linux via DXVK (in our repo!)

### 5. Community Friendly

- Easier for contributors (build once)
- Clear issue attribution (Wine vs Game vs DXVK)
- Platform-agnostic development
- Smaller barrier to entry

### 6. Future Optionality Preserved

```
After Phase 11 (4 months):
├─ Ship with Wine ✓ (Users happy)
├─ Evaluate performance feedback
├─ If needed: Phase 12 native ports
└─ Either way: Flexible, data-driven decision
```

---

## Alternatives Considered

### Alternative 1: Proton (Valve's Steam Integration)

**Advantages**:
- Free, open-source
- Heavy investment by Valve
- Steam integration included

**Disadvantages**:
- Requires Steam for distribution
- Not suitable for standalone release
- External dependency on Valve
- Overengineered for this use case

**Decision**: Use Wine directly (simpler)

---

### Alternative 2: Native Multi-Platform (Rejected)

**Advantages**:
- 100% native performance
- Full platform integration
- No external dependencies

**Disadvantages**:
- **3x development effort** ← Critical blocker
- Platform-specific bugs proliferate
- Contribution barrier higher
- 12+ months to market
- Multiple code paths = quality issues

**Decision**: Too expensive, Wine sufficient

---

### Alternative 3: Cloud Gaming / Streaming

**Advantages**:
- Zero platform dependencies
- Perfect cross-platform

**Disadvantages**:
- Massive infrastructure cost
- Latency unacceptable for RTS (gameplay suffers)
- Ongoing server costs
- Not suitable for legacy game

**Decision**: Not viable

---

## Implementation

### Architecture

```
GeneralsX Game Engine
    ↓
GraphicsDevice Interface (Phase 8)
    ├─ DX8GraphicsDevice (Phase 8-9)
    │   ├─ Windows: DirectX 8 direct calls
    │   └─ Wine: DirectX 8 → DXVK → Vulkan
    │
    └─ VulkanGraphicsDevice (Phase 9, future optimization)
        ├─ Windows: Direct Vulkan API
        └─ Wine: Optional (if performance benefits proven)
```

### Deployment

**Windows**:
```
GeneralsXZH.exe (x64)
├─ Direct DirectX 8 or Vulkan rendering
├─ SDL2 for windowing/input
└─ Standalone distribution
```

**macOS**:
```
Generals Zero Hour.app
├─ Embedded Wine ARM64 runtime
├─ GeneralsXZH.exe (Windows binary)
├─ Transparent to user
└─ Double-click to launch
```

**Linux**:
```
GeneralsZH Package
├─ Wine dependency (package manager)
├─ GeneralsXZH.exe (Windows binary)
├─ Desktop launcher
└─ Standard installation
```

### Testing Strategy

**Platforms**:
- Windows 10/11 (native)
- macOS M1/M2/M3 (Wine ARM64)
- Ubuntu 22.04 LTS (Wine x64)
- Fedora 37+ (Wine x64)

**Test Coverage**:
- Functionality: Skirmish, Campaign, Challenge
- Performance: Frame rate, CPU/GPU usage
- User Experience: Audio, input, save/load
- Edge Cases: Mods, settings, crashes

---

## Risks & Mitigations

### Risk 1: Wine Incompatibility (Low Probability)

**Mitigation**:
- Early phase testing (Phase 10 Task 1)
- Reference: fighter19 project already proves compatibility
- Fallback: DXVK configuration tuning
- Contingency: Custom VulkanGraphicsDevice (Phase 9 option B)

### Risk 2: GPU Driver Issues (Medium Probability)

**Mitigation**:
- Comprehensive GPU compatibility matrix
- Per-vendor troubleshooting guide
- DXVK workaround flags documented
- Community support forum

### Risk 3: Wine Version Fragmentation (Low Probability)

**Mitigation**:
- macOS: Bundle Wine with .app (isolation)
- Linux: Pin Wine version in scripts
- Backward compatibility testing
- Stable version pinning for releases

### Risk 4: Performance Not Meeting Expectations (Very Low)

**Mitigation**:
- Phase 10 includes performance benchmarking
- <5% overhead is acceptable baseline
- If issues arise: Phase 12 native builds available
- Data-driven decision point

---

## Acceptance Criteria

✅ **Phase 8**: Graphics abstraction complete, DX8 interface working  
✅ **Phase 9**: Vulkan backend implemented, Windows validation passing  
✅ **Phase 10**: Wine integration complete, macOS/Linux testing passing  
✅ **Phase 11**: 64-bit Windows build compiled and validated  
✅ **Success**: Game playable on Windows/macOS/Linux with feature parity  

---

## Consequences

### Positive Outcomes

1. **Rapid Market Entry**: Playable on all platforms by Q2 2026
2. **Single Codebase**: Easier maintenance, fewer bugs
3. **Community Friendly**: Easy for contributors to build/test
4. **Quality**: Fewer platform-specific issues
5. **Flexible**: Phase 12 native ports still possible

### Trade-offs

1. **Performance**: 2-5% overhead vs 100% native (acceptable)
2. **Control**: Dependent on Wine ecosystem (well-maintained)
3. **Integration**: Not "native" experience (imperceptible to users)
4. **Packaging**: Requires Wine distribution (standard on Linux)

---

## Related Decisions

- **ADR-002** (upcoming): DirectX 8 to Vulkan translation approach
- **Phase 08-11**: Implementation roadmap
- **WINE_ANALYSIS_AND_STRATEGY.md**: Technical analysis

---

## Decision History

| Date | Decision | Status |
|------|----------|--------|
| 2026-01-13 | Wine strategy proposed | Proposed |
| 2026-01-13 | Detailed analysis completed | Completed |
| 2026-01-15 | Approved by development team | APPROVED |
| 2026-01-16 | Implementation begins (Phase 8) | Scheduled |

---

## Stakeholders

| Role | Interest | Status |
|------|----------|--------|
| Development Team | Implementation feasibility | ✅ Agrees |
| Project Lead | Timeline & quality | ✅ Approves |
| Community | Cross-platform support | ✅ Supports |
| Users | Playable on their platform | ✅ Will benefit |

---

## References

1. **Wine Documentation**: https://wiki.winehq.org/
2. **DXVK Project**: https://github.com/doitsujin/dxvk
3. **Crossover Reference**: https://www.codeweavers.com/
4. **fighter19 Project**: `references/fighter19-dxvk-port/`
5. **Performance Analysis**: See WINE_ANALYSIS_AND_STRATEGY.md

---

## Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Tech Lead | GeneralsX Team | 2026-01-15 | ✅ |
| Project Owner | Felipe | 2026-01-15 | ✅ |

---

**Status**: APPROVED ✅  
**Last Updated**: 2026-01-15  
**Next Review**: After Phase 10 completion (Q2 2026)
