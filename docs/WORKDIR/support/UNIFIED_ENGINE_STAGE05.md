# Unified Engine Stage 05: Rules and Serialization (Player/Object Xfer)

Date: 2026-10-23
Status: 05.1–05.6 done (all smokes exit 0); 05.7 partial — Player/Object vision dual-layout + GameProfile routing; full CRC=1 still deferred
Source brief: explore agent Player/Object xfer design (PLAN-023 Gate 5)

## Verdict

Player `currentVersion = 8` is **not** a cross-game format. Generals xfers
`m_visionSpiedBy` / `m_visionSpiedMask` on **Player**; Zero Hour moved those to
**Object** and added Object **v9** reveal-all. Core upgrade-mask CRC width is
still compile-macro branched (8 vs 16). Prefer ZH-canonical classes with
`GameProfile` stream branches — do not treat matching version numbers as portable.

## Implementation order

| Step | Change | Status |
|---|---|---|
| **05.1** | `Xfer::xferUpgradeMask` CRC size via `GameProfile` (8 Generals / 16 ZH) | done (smoke 0) |
| **05.2** | `Object::xfer` version select 7/8/9 via profile + `RETAIL_COMPATIBLE_XFER_SAVE` | done (smoke 0) |
| **05.3** | Gate ZH-only Object stream fields (`reveal-all`, vision) with `isZeroHour()` | done (smoke 0) |
| **05.4** | Middle disabled-frame trio gated by profile | done (smoke 0) |
| **05.5** | `Player::xfer` vision pair only when `isGenerals()` | done (smoke 0) |
| **05.6** | ZH-only Object sleep helpers create gated (Status/Subdual/TempWeapon) | done in tree |
| **05.7** | Header layout consolidation after call sites proven | advanced — vision dual-layout; TunnelTracker/`BodyModule`/StickyBomb; **OpenContain** all CRC compile gates → `GameProfile`; Generals `getGeneralFeatures` stub; Horde DEFAULT still compile-macro (`PRESERVE_PERPETUAL_HORDE_BONUS=1`); `RETAIL_COMPATIBLE_CRC` flag still 0 |

## Anti-patterns

- Do not merge game Include trees to “fix” serializers (NGMP bleed).
- Do not drop Generals Player vision APIs before gameplay + xfer both proven.
- Wrong-profile load must fail loudly, not silently mis-parse.

## References

- `docs/WORKDIR/audit/UNIFIED_ENGINE_SOURCE_COMPARISON_2026-10-23.md` (Player v8 note)
- `Core/GameEngine/Include/Common/GameProfile.h`
