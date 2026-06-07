# Generals Parity Wave 2 Audit: GameEngine Source

Date: 2026-05-11
Scope: `GeneralsMD/Code/GameEngine/Source/**` annotation deltas that do not currently appear in `Generals/Code/GameEngine/Source/**` annotation set.
Status: In Progress (Ready for manual smoke closure)

## Classification Legend

- Applicability:
  - `Direct`: Can be ported with minimal/no semantic adaptation.
  - `Adapted`: Should be ported with Generals base-specific adjustments.
  - `N/A`: Zero Hour-only behavior/content.
- Risk:
  - `Low`, `Medium`, `High`

## ZH -> Generals Missing Candidate Inventory

| File | Subsystem | Applicability | Risk | Decision | Notes |
|---|---|---|---|---|---|
| Code/GameEngine/Source/Common/RTS/PlayerList.cpp | Common/RTS | Direct | Low | Ported (Batch 1) | Added fallback `findPlayerWithName(const AsciiString&)` for qualified skirmish owner resolution. |
| Code/GameEngine/Source/Common/RTS/Team.cpp | Common/RTS | Adapted | Low | Ported (Batch 1) | Added fallback owner resolution by ASCII name only when nameKey lookup fails and owner is non-empty. |
| Code/GameEngine/Source/Common/Thing/ThingTemplate.cpp | Common/Thing | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe pointer cast parity (`uintptr_t` flow only). |
| Code/GameEngine/Source/GameClient/GUI/ControlBar/ControlBar.cpp | GameClient/GUI | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe pointer cast parity (`userData` decode). |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/InGamePopupMessage.cpp | GameClient/GUI | TBD | TBD | Pending | Review for data-driven/asset-root differences only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/LanGameOptionsMenu.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe combo item-data casts only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/LanLobbyMenu.cpp | GameClient/GUI/Menus | TBD | TBD | Pending | Verify LAN browser behavior parity and platform guards. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/PopupHostGame.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe combo item-data casts only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/PopupLadderSelect.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe listbox/window user-data casts only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/ScoreScreen.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe button data decode cast only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishGameOptionsMenu.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3, mechanical only) | Ported only 64-bit-safe casts; control/resource parity still intentionally deferred. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishMapSelectMenu.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe listbox item-data cast only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/WOLBuddyOverlay.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe listbox item-data cast only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/WOLGameSetupMenu.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe combo item-data casts only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/WOLLobbyMenu.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe listbox/combo item-data casts only. |
| Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/WOLQuickMatchMenu.cpp | GameClient/GUI/Menus | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe combo item-data casts only. |
| Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp | GameClient/GUI/Shell | TBD | TBD | Pending | High visibility menu flow; keep deterministic behavior unchanged. |
| Code/GameEngine/Source/GameLogic/AI/AIStates.cpp | GameLogic/AI | TBD | TBD | Pending | Any AI logic change is high-risk unless strictly crash-safety. |
| Code/GameEngine/Source/GameLogic/Object/Collide/CrateCollide/SabotageInternetCenterCrateCollide.cpp | GameLogic/Object | TBD | TBD | Pending | Validate if fix is pure safety and shared between games. |
| Code/GameEngine/Source/GameLogic/Object/PartitionManager.cpp | GameLogic/Object | Adapted | Low | Ported (Batch 3) | Ported mechanical 64-bit-safe player index decode casts only. |
| Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp | GameLogic/System | Adapted | Low | Ported (Batch 3 diagnostics complete) | Added debug-only score-screen diagnostics and kept transition-system parity intentionally deferred. |
| Code/GameEngine/Source/GameNetwork/GUIUtil.cpp | GameNetwork | Adapted | Low | Ported (Batch 2 + Batch 3) | Ported faction/General gating + starting cash combo; Batch 3 added 64-bit-safe item-data decode casts. |

## Generals -> ZH Reverse Deltas (Validation List)

| File | Direction | Decision | Notes |
|---|---|---|---|
| Code/GameEngine/Source/Common/GameLOD.cpp | Generals-only annotation | Pending | Verify if base-only adaptation should mirror to ZH. |
| Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptActions.cpp | Generals-only annotation | Pending | Verify if camera/script defaults should also apply to ZH. |

## Wave 2 Execution Order

1. Common runtime safety and pathing deltas (`Common/*`, `GameNetwork/*`) with low gameplay risk.
2. GameClient GUI/menu files where changes are platform/runtime-only.
3. GameLogic files only for proven crash-safety or deterministic-neutral fixes.
4. Reverse-delta validation and intentional divergence documentation.

## Validation Gates

1. Build Generals (active preset).
2. Deploy Generals.
3. User-run manual smoke (menu + skirmish).
4. Replay smoke validation when touching `GameLogic/*`.

## Batch 1 Progress (This Session)

- Ported low-risk skirmish ownership fallback from Zero Hour into Generals base in `PlayerList` + `TeamFactory::initTeam`.
- Kept scope strictly runtime ownership lookup robustness; no AI targeting or gameplay logic deltas were ported.
- The port required a minimal `Player::getPlayerName()` accessor in the base tree because the raw ASCII player name already existed internally but was not publicly exposed.
- Validation completed with Build + Deploy tasks on macOS without automatic game execution.

## Batch 2 Progress (This Session)

- Ported Zero Hour's lobby-facing faction gating into `GameNetwork/GUIUtil.cpp` for `PopulatePlayerTemplateComboBox`.
- Restored `PopulateStartingCashComboBox` in the Generals base tree using the shared `GUI:StartingMoneyFormat` text path.
- Validation completed with Build + Deploy tasks on macOS without automatic game execution.

## Batch 3 Progress (This Session)

- Ported mechanical 64-bit-safe pointer cast parity in GameEngine Source where Generals still used C-style `(Int)(intptr_t)` / `(Int)(uintptr_t)` decode patterns.
- Scope was strictly mechanical and backend-safe: no gameplay flow or menu behavior changes were introduced.
- Representative files: `PartitionManager.cpp`, `GUIUtil.cpp`, `PopupLadderSelect.cpp`, `WOLQuickMatchMenu.cpp`, `LanGameOptionsMenu.cpp`, `WOLLobbyMenu.cpp`, `ThingTemplate.cpp`.
- Validation completed with Build + Deploy tasks on macOS without automatic game execution.

## Remaining Intentional Deferrals

- `SkirmishGameOptionsMenu.cpp`: non-mechanical control/resource parity (`CheckboxLimitSuperweapons` and related UI support) remains intentionally deferred due missing base controls/resources.
- `GameLogic/ScriptEngine/ScriptActions.cpp`: gameplay/expansion-coupled divergence remains intentionally deferred.

## Notes

- This audit intentionally excludes automatic game execution.
- Every row must end with one of: `Port`, `Port with adaptation`, `Do not port` and a short rationale.
