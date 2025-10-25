# Phase 36 - Conflict Resolution Tracker

**Total Conflicts**: 31
**Strategy**: Three-way merge (A+B) - preserve BOTH our changes AND upstream improvements

## Conflict Categories

### Priority 1: Platform-Specific (KEEP OURS)
None identified yet - Metal backend files auto-merged successfully

### Priority 2: Shared Game Logic (MANUAL MERGE - A+B)
These require analyzing BOTH our changes AND upstream changes, then combining:

1. [ ] `Core/GameEngine/Source/Common/FrameRateLimit.cpp`
2. [ ] `Core/GameEngine/Source/Common/System/ArchiveFileSystem.cpp`
3. [ ] `Core/GameEngineDevice/Source/Win32Device/Common/Win32BIGFileSystem.cpp`
4. [ ] `Core/Libraries/Source/WWVegas/WWDownload/Download.cpp`
5. [ ] `Core/Libraries/Source/debug/debug_io_flat.cpp` ⚠️ RESTART TRIGGER
6. [ ] `Core/Libraries/Source/debug/debug_stack.cpp`
7. [ ] `Generals/Code/GameEngine/Include/GameNetwork/WOLBrowser/WebBrowser.h`
8. [ ] `Generals/Code/GameEngine/Source/Common/GameEngine.cpp` ⚠️ CRITICAL (Phase 35.2 init order)
9. [ ] `Generals/Code/GameEngine/Source/Common/GameMain.cpp`
10. [ ] `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h`
11. [ ] `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DProjectedShadow.h`
12. [ ] `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DShadow.h`
13. [ ] `Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DVolumetricShadow.h`
14. [ ] `Generals/Code/GameEngineDevice/Include/Win32Device/Common/Win32CDManager.h`
15. [ ] `Generals/Code/GameEngineDevice/Include/Win32Device/GameClient/Win32DIKeyboard.h`
16. [ ] `Generals/Code/GameEngineDevice/Include/Win32Device/GameClient/Win32DIMouse.h`
17. [ ] `Generals/Code/GameEngineDevice/Include/Win32Device/GameClient/Win32Mouse.h`
18. [ ] `Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`
19. [ ] `Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DFileSystem.cpp`
20. [ ] `Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp`
21. [ ] `Generals/Code/Libraries/Source/WWVegas/WW3D2/texture.h`
22. [ ] `Generals/Code/Libraries/Source/WWVegas/WW3D2/textureloader.h`
23. [ ] `Generals/Code/Libraries/Source/WWVegas/WW3D2/texturethumbnail.h`
24. [ ] `Generals/Code/Main/WinMain.h`
25. [ ] `GeneralsMD/Code/GameEngine/Include/Common/ScopedMutex.h`
26. [ ] `GeneralsMD/Code/GameEngine/Include/GameNetwork/WOLBrowser/WebBrowser.h`
27. [ ] `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp` ⚠️ CRITICAL (Phase 35.2 init order)
28. [ ] `GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp`
29. [ ] `GeneralsMD/Code/GameEngineDevice/Include/Win32Device/Common/Win32GameEngine.h`
30. [ ] `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/Common/System/W3DRadar.cpp`
31. [ ] `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`

## Resolution Workflow (Per File)

For EACH conflict:

```bash
# 1. Analyze our changes (backup vs original base)
git diff original/main:FILE phase35-backup:FILE > logs/conflict_ours_FILE.diff

# 2. Analyze upstream changes (original base vs current)
git diff original/main~71:FILE original/main:FILE > logs/conflict_upstream_FILE.diff

# 3. Understand BOTH change sets
# 4. Manually merge - combine improvements from BOTH
# 5. git add FILE
```

## Incremental Compilation (Every 10 Files)

After resolving 10 conflicts:
```bash
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee logs/phase36_incremental_N.log
```

## Progress

- **Resolved**: 0/31
- **Current Focus**: Starting with debug files (#5-6)
- **Next Batch**: GameEngine init order (#8, #27)
