# Network and LAN Status — 2026-03-29

**Status**: Active documentation
**Scope**: GeneralsXZH primary, Generals secondary when shared
**Purpose**: Consolidate the current project position on LAN readiness, accepted legacy behavior, and future online compatibility constraints before dedicated LAN testing begins.

---

## Executive Summary

Current repository evidence indicates that the LAN path is implemented and should be testable once unrelated runtime bugs are resolved.

At this stage, the project should **document and preserve** proven upstream networking behavior rather than treat every legacy limitation as an immediate blocker. The practical near-term goal is:

1. Fix the current non-network bugs that block stable manual testing.
2. Run focused two-machine LAN validation.
3. Reassess only the issues that reproduce in real LAN sessions.

This means some previously flagged items remain **known risks**, but are **not current blockers** for beginning LAN validation.

---

## Current Project Position

### LAN is the immediate target

The repository contains a real LAN implementation, not a stub:

- LAN broadcast discovery via `LANAPI`
- direct connect by IP
- lobby/game announce flow
- join/leave/start/chat handlers
- transport and frame-sync networking layers

Relevant code areas:

- `Core/GameEngine/Include/GameNetwork/LANAPI.h`
- `Core/GameEngine/Source/GameNetwork/LANAPI.cpp`
- `Core/GameEngine/Source/GameNetwork/LANAPIhandlers.cpp`
- `Core/GameEngine/Source/GameNetwork/LANAPICallbacks.cpp`
- `Core/GameEngine/Source/GameNetwork/Network.cpp`
- `Core/GameEngine/Source/GameNetwork/Transport.cpp`

### Preserve proven upstream behavior unless there is concrete LAN failure evidence

Several networking behaviors come directly from upstream/legacy assumptions. If upstream behaves correctly in real LAN and online-derived deployments, these should not be reclassified as blockers without a reproduced failure in GeneralsX.

This applies especially to:

1. CRC join validation behavior in Zero Hour
2. serial duplicate enforcement
3. legacy IPv4 broadcast-based LAN discovery

### Future online compatibility still matters

GeneralsX should avoid taking shortcuts that unnecessarily remove GameSpy-related compatibility if that would make future Generals Online integration harder.

Current practical rule:

- If a legacy online component does not affect LAN validation, it is acceptable to defer it.
- If a change would permanently reduce compatibility with future online integration, it should be avoided unless explicitly approved.

---

## Accepted Non-Blockers for Initial LAN Testing

### 1. CRC join validation is not a current blocker by itself

The LAN join code shows that CRC mismatch denial exists as a concept, but the relevant Zero Hour join gate is not currently treated as a stop-ship issue for this project cycle.

Reasoning:

- The user reports that upstream behavior is already proven in real network use.
- Generals Online derives from the same upstream line.
- Without reproduced LAN failure in GeneralsX, changing this now would be speculative.

Current conclusion:

- Keep documented as a **known inherited behavior**.
- Revisit only if manual LAN testing shows a real compatibility or desync problem tied to version/content mismatch.

### 2. Serial duplicate checking is not needed for current LAN goals

The duplicate serial gate is disabled in the LAN join flow.

For current LAN validation, this is accepted as non-essential and should not consume engineering time.

Current conclusion:

- Documented as intentionally non-blocking.
- No action required for LAN readiness.

### 3. Legacy map validation behavior is currently a risk, not a blocker

Map availability handling exists and map-related CRC logic exists, but there is not yet enough evidence to justify treating current map validation semantics as broken for LAN.

Current conclusion:

- Treat as inherited upstream behavior unless manual LAN testing proves otherwise.
- If it becomes a real issue, handle it as a focused upstream-compatible patch or dedicated PR later.

### 4. Missing dedicated LAN QA automation is acceptable for now

The repository currently has smoke/build/runtime validation, but not dedicated two-machine LAN automation.

Current conclusion:

- Manual LAN validation is the correct next step.
- Automated LAN QA can be planned later if real regressions appear.

### 5. GameSpy runtime dependency is acceptable if it does not interfere with LAN

The build and deploy flows still package GameSpy runtime libraries.

Current conclusion:

- This is acceptable as long as LAN works independently.
- Do not remove or weaken GameSpy compatibility casually, because future Generals Online integration remains desirable.

### 6. WOL/WebBrowser is not a LAN blocker

The embedded WOL browser path is legacy online functionality and is not part of the LAN critical path.

Current conclusion:

- Accept for now as outside immediate LAN scope.
- Revisit when online integration work actually begins.

---

## Open Risks to Revisit After Manual LAN Testing

These items should stay documented, but they do not justify blocking initial LAN validation today.

### A. Content mismatch behavior

Questions to answer later with evidence:

- What happens if players differ in map content?
- What happens if players differ in modded assets or INI data?
- Does the game fail early, desync later, or continue normally?

### B. Cross-platform interoperability

Questions to answer later with evidence:

- Does macOS host/join Linux successfully?
- Does the current transport/timing path behave identically enough in real play?
- Are there any platform-specific disconnects or stalls?

### C. Online compatibility boundaries

Questions to answer later with evidence:

- Which GameSpy-related pieces are still needed for future Generals Online connectivity?
- Which WOL/GameSpy UI paths can remain stubbed without harming that goal?
- Which legacy services are safe to isolate behind feature flags instead of removing?

---

## Immediate Practical Plan

Before LAN testing:

1. Resolve the unrelated bugs currently blocking stable runtime testing.
2. Keep networking code unchanged unless one of those bugs is clearly network-related.
3. Preserve current upstream-aligned networking behavior during this stabilization period.

When ready to test LAN:

1. Validate host discovery on local network.
2. Validate direct connect by IP.
3. Validate lobby synchronization.
4. Validate game start and 10-15 minutes of gameplay.
5. Validate disconnect/leave behavior.
6. Intentionally try one mismatch scenario and record the real behavior.

---

## Documentation Policy Going Forward

Any future discussion about networking should distinguish clearly between these categories:

1. **Immediate LAN blockers**: reproducible issues that prevent host/join/start/play on a local network.
2. **Accepted legacy behavior**: inherited upstream behavior that is not currently proven broken in practice.
3. **Future online work**: GameSpy/WOL/Generals Online compatibility concerns that should not derail LAN stabilization.

This distinction is necessary to avoid turning speculative cleanup into false blockers.

---

## Final Assessment for This Session

The current working assumption is:

- **LAN should be functional enough to justify manual testing soon.**
- **Documentation clarity is more urgent than networking refactors right now.**
- **GameSpy compatibility should be preserved where practical for future online work.**
- **Real LAN test evidence should drive the next round of networking changes.**