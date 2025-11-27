# Phase 49 Quick Start

## 5-Minute Overview

**Problem**: Game hangs during initialization at `TheCDManager`

**Root Cause**: Factory methods returning wrong types or NULL

**Solution**: Fix 2 critical factory methods

---

## What to Fix

### Fix 1: CDManager (Critical - Blocks Everything)

**File**: `Core/Libraries/Source/Graphics/phase41_missing_implementations.cpp`

**Current** (Wrong):
```cpp
class CDManager { };  // No inheritance!
CDManager* CreateCDManager() { ... }
```

**Required** (Correct):
```cpp
class CrossPlatformCDManager : public CDManagerInterface { ... };
CDManagerInterface* CreateCDManager() { return NEW CrossPlatformCDManager; }
```

See [PHASE49_1_CDMANAGER.md](PHASE49_1_CDMANAGER.md) for full implementation.

---

### Fix 2: VideoPlayer (High - Blocks Videos)

**File**: `GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h`

**Current** (Wrong):
```cpp
#ifdef RTS_HAS_FFMPEG
#else
#endif
virtual VideoPlayerInterface *createVideoPlayer() { return NULL; }
```

**Required** (Correct):
```cpp
#ifdef RTS_HAS_FFMPEG
#include "VideoDevice/FFmpeg/FFmpegVideoPlayer.h"
virtual VideoPlayerInterface *createVideoPlayer() { return NEW FFmpegVideoPlayer; }
#else
virtual VideoPlayerInterface *createVideoPlayer() { return NULL; }
#endif
```

See [PHASE49_2_VIDEOPLAYER.md](PHASE49_2_VIDEOPLAYER.md) for details.

---

## Build & Test

```bash
# Build
cmake --build build/macos --target GeneralsXZH -j 4

# Deploy
cp build/macos/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/

# Test
cd $HOME/GeneralsX/GeneralsMD
timeout 60s ./GeneralsXZH 2>&1 | tee logs/phase49_test.log

# Verify
grep -i "CDManager\|TheAudio\|Menu" logs/phase49_test.log
```

---

## Success Criteria

- [x] CDManager initializes without hang
- [x] Audio system initializes
- [ ] Video player available (EA logo plays)
- [ ] Main menu reached

---

## Time Estimate

- Fix 1 (CDManager): 30 minutes
- Fix 2 (VideoPlayer): 15 minutes
- Testing: 30 minutes
- **Total**: ~1-2 hours

---

## Next Phase

After Phase 49, the game should reach the main menu. Next steps:

1. Verify menu rendering
2. Test menu navigation
3. Attempt skirmish game
