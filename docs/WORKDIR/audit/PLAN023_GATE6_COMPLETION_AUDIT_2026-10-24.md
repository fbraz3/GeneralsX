# PLAN-023 Gate 6 completion audit (2026-10-24)

Tip audited: `f893103f8` (plan branch on fork `Cheviiot/Echelon`).
Authoritative sources: PLAN-023, Stage 06 notes, rehearsal notes, smoke logs, CMake options, GitHub Actions.

Verdict: **COMPLETE** for PLAN-023 objective scope (Linux dual-profile headless evidence + macOS supported-workflow build). Optional follow-ups remain.

## Requirement matrix

| Requirement | Evidence | Status |
|---|---|---|
| One executable `echelon` with startup `-game generals\|zh` before asset mount | `GameProfile::selectForProcess` in `SDL3Main.cpp` / `WinMain.cpp`; dual-profile smoke exit 0 | **PASS** |
| ZH-canonical engine + Generals compatibility profile | Stage 03–06 + CRC→GameProfile waves 1–11; `RTS_UNIFIED_ENGINE=1` on echelon | **PASS** |
| Campaigns / skirmish path | map-load USA01/Alpine + `-skirmish` record/playback both profiles | **PASS** (headless) |
| LAN | `smoke-echelon-lan.sh` + dual discovery exit 0 | **PASS** |
| Mods | `smoke-echelon-mod-path.sh` dual-profile exit 0 | **PASS** |
| Saves | `smoke-echelon-save-load.sh` ZH+Generals exit 0 | **PASS** |
| Replays | self CRC-clean fixtures + record→playback exit 0; external old fixture soft-pass exit 2 optional | **PASS** (self); optional external CRC still soft-pass |
| NGMP online target | echelon links NGMP; GameSpy not required | **PASS** (link/policy) |
| Provenance map + checker | `cmake/unified-engine-provenance.json`; checker `valid: true` (170 entries) | **PASS** |
| Upstream rehearsal | Isolated merge @ `0fea59a33`; fresh-tip promote FF `14e21aa83` | **PASS** |
| Dual migration targets removed after acceptance | `RTS_BUILD_MIGRATION_BINARIES` default **OFF**; CI explicitly ON for legacy artifact names | **PASS** |
| Linux builds + headless smoke both profiles | `smoke-echelon-gate6.sh` tip re-verify exit 0 (`logs/smoke-gate6-summary-20261024-052459.txt`) | **PASS** |
| macOS via supported workflow | Build macOS run [40012141402](https://github.com/Cheviiot/Echelon/actions/runs/40012141402) success; step **Build echelon (unified binary)** success; artifacts `macos-arm64-GeneralsXZH.zip` / `GeneralsX.zip` | **PASS** |
| Linux/macOS cross-play CRC (Gate 5 wording) | Linux self-CRC clean; no dual-host cross-play CRC recorded on tip | **OPTIONAL / deferred** |

## PR / CI

- PR: https://github.com/fbraz3/GeneralsX/pull/387
- Fork branch: `Cheviiot/Echelon` `echelon/unified-engine-plan`

## Optional follow-ups (non-blocking)

1. Native Mac dual-profile smoke against deployed echelon (`smoke-echelon-macos-gate6.md`).
2. Reconcile older `generalsx-replays` frame-100 CRC soft-pass.
3. Interactive LAN lobby UI.

## Non-blocking notes

- Stage 05.7 full header-layout collapse remains deferred; dual rules are via `GameProfile` (accepted for Gate 6).
