# PLAN-023 Gate 6 completion audit (2026-10-24)

Tip audited: `0a4f84497` (+ pending CI/macOS packaging commits on working tree).
Authoritative sources: PLAN-023, Stage 06 notes, rehearsal notes, smoke logs, CMake options.

Verdict: **NOT COMPLETE** — Linux Gate 6 acceptance is green; macOS validation evidence is missing.

## Requirement matrix

| Requirement | Evidence | Status |
|---|---|---|
| One executable `echelon` with startup `-game generals\|zh` before asset mount | `GameProfile::selectForProcess` in `SDL3Main.cpp` / `WinMain.cpp`; dual-profile smoke exit 0 | **PASS** |
| ZH-canonical engine + Generals compatibility profile | Stage 03–06 + CRC→GameProfile waves 1–11; `RTS_UNIFIED_ENGINE=1` on echelon | **PASS** |
| Campaigns / skirmish path | map-load USA01/Alpine + `-skirmish` record/playback both profiles | **PASS** (headless) |
| LAN | `smoke-echelon-lan.sh` + dual discovery exit 0 | **PASS** |
| Mods | `smoke-echelon-mod-path.sh` dual-profile exit 0 | **PASS** |
| Saves | `smoke-echelon-save-load.sh` ZH+Generals exit 0 | **PASS** |
| Replays | self CRC-clean fixtures + record→playback exit 0; external old fixture soft-pass exit 2 optional | **PASS** (self); WEAK (external) |
| NGMP online target | echelon links NGMP; GameSpy not required | **PASS** (link/policy) |
| Provenance map + checker | `cmake/unified-engine-provenance.json` + check/adapt scripts | **PASS** |
| Upstream rehearsal | Isolated merge @ `0fea59a33`; fresh-tip promote FF `14e21aa83` | **PASS** |
| Dual migration targets removed after acceptance | `RTS_BUILD_MIGRATION_BINARIES` default **OFF**; CI explicitly ON for legacy artifact names | **PASS** (product); CI keep-alive documented |
| Linux builds + headless smoke both profiles | `smoke-echelon-gate6.sh` tip re-verify exit 0 (`logs/smoke-gate6-summary-20261024-052459.txt`) | **PASS** |
| macOS via supported workflow | No Mach-O build/smoke on this tip; branch not on origin; procedure in `smoke-echelon-macos-gate6.md` | **MISSING** |
| Linux/macOS cross-play determinism (Gate 5 wording) | Linux self-CRC clean only; no macOS cross-play CRC on tip | **MISSING** |

## Blocking follow-ups

1. Push `echelon/unified-engine-plan` (or PR) and run **Build macOS** + dual-profile smoke on the artifact / Mac host.
2. Record macOS run URL + SHA in the monthly diary when green.
3. Optional: reconcile older `generalsx-replays` frame-100 CRC soft-pass.

## Non-blocking notes

- Stage 05.7 full header-layout collapse remains deferred; dual rules are via `GameProfile` (accepted for Gate 6).
- Interactive LAN lobby UI not required for Gate 6 headless acceptance.
