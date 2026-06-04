# Phase 1 Runtime Validation: Smoke Test Plan

**Status**: 📋 READY TO EXECUTE  
**Binary**: `/build/linux64-deploy/GeneralsMD/generalszh` (177 MB, ELF 64-bit)  
**Goal**: Verify Linux binary launches, renders, and basic gameplay works

> The Bender Way: "We're delilars! Well, sort of. Let's test this magnificent robot!"

---

## Test Environment Setup

### Prerequisites
- [ ] Docker installed (`docker --version`)
- [ ] Linux binary built (Phase 1 complete) ✅
- [ ] Docker image available (`ubuntu-builder` or rebuild)
- [ ] Test logs directory: `logs/smoke-tests/`

### Prepare Test Environment
```bash
# Create test directory
mkdir -p logs/smoke-tests

# Verify binary exists
ls -lh build/linux64-deploy/GeneralsMD/generalszh

# Tag binary size/type
file build/linux64-deploy/GeneralsMD/generalszh
```

---

## Test Suite

### Test 1: Binary Execution - Startup (CRITICAL)
**Objective**: Verify binary executes without immediate crash

**Command** (Docker):
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  timeout 5 /work/build/linux64-deploy/GeneralsMD/generalszh -noshellmap -win 2>&1 | \
  head -100 | tee logs/smoke-tests/test1_startup.log
```

**Expected Output** (first 20 lines):
```
Starting C&C Generals: Zero Hour
Initializing graphics device (DXVK)...
Initialized SDL3 window...
Loading game data...
Connecting to GameSpy...
```

**Failure Points**:
- ❌ `Segmentation fault` → Initialization crash (check stubs)
- ❌ `Cannot load shared library` → Missing dependencies (check docker image)
- ❌ `ELF not found` → Binary corrupted (rebuild)

**Pass Criteria**:
- ✅ Binary executes for 5 seconds
- ✅ No segfault in stderr
- ✅ Some game initialization output visible
- ✅ Process terminates cleanly after timeout

---

### Test 2: SDL3 Window Management (GRAPHICS)
**Objective**: Verify graphics device initializes (windowed mode)

**Command** (Docker with X11 forwarding - macOS):
```bash
# On macOS, you may need:
# brew install XQuartz
# open -a XQuartz
export DISPLAY=:0

docker run --rm \
  -v "$PWD:/work" -w /work \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  ubuntu:22.04 \
  timeout 5 /work/build/linux64-deploy/GeneralsMD/generalszh -win 2>&1 | \
  tee logs/smoke-tests/test2_sdl3_window.log
```

**Expected Behavior**:
- ✅ SDL3 window initializes (may not visible on Docker without X11)
- ✅ DXVK graphics device claimed
- ✅ Video mode set to windowed

**Failure Points**:
- ❌ `SDL3 not found` → Missing SDL3 dependency
- ❌ `DXVK device failed` → Graphics layers not loading
- ❌ `X11 connection refused` → X11 not available (expected on Docker)

**Alternative** (Headless - ignore X11):
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  timeout 5 /work/build/linux64-deploy/GeneralsMD/generalszh -noshellmap 2>&1 | \
  grep -E "(SDL|DXVK|graphics|device|window)" | tee logs/smoke-tests/test2_headless.log
```

---

### Test 3: Asset Loading (DATA INTEGRITY)
**Objective**: Verify game data files load correctly

**Command** (Check .big file handling):
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash << 'EOF'
  cd /work
  
  # Check if .big files exist
  if [ -f "GeneralsMD/Run/Data/genseczh.big" ]; then
    echo "[PASS] genseczh.big found"
  else
    echo "[SKIP] genseczh.big not in Docker - expected"
  fi
  
  # Run with debug flag to dump asset loading
  timeout 3 ./build/linux64-deploy/GeneralsMD/generalszh -noshellmap 2>&1 | \
    grep -i -E "(loading|asset|data|textu|model|image)" | head -20
EOF
```

**Expected Output**:
```
Loading textures from archive...
Loading models from data/...
Loading UI assets...
```

**Failure Points**:
- ⚠️ `File not found: genseczh.big` → Data files needed (download/mount separately)
- ❌ `Archive corrupted` → Data file error
- ⚠️ `Texture loading failed` → Expected if data not present (falls back gracefully)

---

### Test 4: Main Menu Rendering (FULL GRAPHICS)
**Objective**: Reach main menu screen (most comprehensive graphics test)

**Command** (Run with shell map):
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  timeout 10 /work/build/linux64-deploy/GeneralsMD/generalszh -win 2>&1 | \
  tee logs/smoke-tests/test4_main_menu.log
```

**Expected Behavior**:
- ✅ Binary initializes
- ✅ Graphics device ready
- ✅ Shell map (menu background) loads
- ✅ Menu controls respond to input (if input device available)

**Failure Points**:
- ❌ Crashes during menu rendering → DXVK integration issue
- ⚠️ Menu visible but corrupted → Graphics pipeline problem
- ⚠️ Frozen (no update loop) → Event loop problem

**Pass Criteria**:
- ✅ No crash within 10 seconds
- ✅ Menu text/graphics rendered
- ✅ Process terminates cleanly

---

### Test 5: Skirmish Mode Loading (GAMEPLAY)
**Objective**: Load a game (most comprehensive test - requires test map)

**Command** (Requires map file):
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash << 'EOF'
  # Check if test maps exist
  if [ -f "/work/GeneralsMD/Run/Data/Temp/MapCache/multiplayer/gdi01.map" ]; then
    timeout 15 /work/build/linux64-deploy/GeneralsMD/generalszh \
      -win -map gdi01 2>&1 | tee /work/logs/smoke-tests/test5_skirmish.log
  else
    echo "[SKIP] Test map not available in Docker"
  fi
EOF
```

**Expected Behavior** (if map present):
- ✅ Map loads and initializes
- ✅ Game units render
- ✅ Gameplay logic starts (turn-based or real-time)
- ✅ Audio attempts to play (currently silent, OK)

**Failure Points**:
- ❌ `Map not found` → Expected if maps not in Docker
- ❌ `Unit creation failed` → GameLogic error
- ❌ Crash during gameplay → Deterministic logic issue

**Pass Criteria**:
- ✅ Map loads successfully
- ✅ Game runs for 10 seconds without crash
- ✅ Unit AI executes

---

### Test 6: Memory & Resource Cleanup (STABILITY)
**Objective**: Verify game cleans up properly on exit

**Command** (Memory profiling):
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash << 'EOF'
  # Install memory profiling tool
  apt update && apt install -y valgrind > /dev/null 2>&1
  
  # Run with memory checker (light mode)
  timeout 5 valgrind --leak-check=no --track-origins=no \
    /work/build/linux64-deploy/GeneralsMD/generalszh -noshellmap 2>&1 | \
    grep -E "(ERROR SUMMARY|definitely|indirectly)" | tee /work/logs/smoke-tests/test6_memory.log
EOF
```

**Expected Output**:
```
ERROR SUMMARY: 0 errors from 0 contexts
```

**Failure Points**:
- ⚠️ Memory leaks detected → Document but non-critical for Phase 1
- ❌ Invalid memory access → Fix required

**Pass Criteria**:
- ✅ No hard crashes during memory check
- ✅ Game terminates cleanly

---

## Quick Test Script

Run all smoke tests at once:

```bash
#!/bin/bash
# scripts/smoke-test-phase1.sh

echo "🤖 PHASE 1 SMOKE TESTS - GENERALSZH LINUX"
echo "==========================================="

mkdir -p logs/smoke-tests

echo ""
echo "[TEST 1] Binary Execution..."
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  timeout 5 /work/build/linux64-deploy/GeneralsMD/generalszh -noshellmap -win 2>&1 | \
  tee logs/smoke-tests/test1_startup.log | head -30

echo ""
echo "[TEST 2] SDL3 Headless..."
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  timeout 5 /work/build/linux64-deploy/GeneralsMD/generalszh -noshellmap 2>&1 | \
  grep -i -E "(SDL|graphics|device|window|DXVK)" | tee logs/smoke-tests/test2_graphics.log

echo ""
echo "[TEST 3] Main Menu..."
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 \
  timeout 10 /work/build/linux64-deploy/GeneralsMD/generalszh -win 2>&1 | \
  tail -20 | tee logs/smoke-tests/test3_menu.log

echo ""
echo "✅ Smoke tests complete! Logs saved to logs/smoke-tests/"
```

---

## Test Execution Order

**Quick Path** (5 minutes):
1. ✅ Test 1: Binary Execution
2. ✅ Test 2: SDL3 Headless
3. ✅ Test 3: Main Menu
4. → **Proceed to Phase 2 audio if all pass**

**Full Path** (15 minutes):
1. Test 1-6 as above
2. Analyze logs in `logs/smoke-tests/`
3. Document issues in `WORKDIR/support/PHASE1_RUNTIME_ISSUES.md`
4. → **Address specific failures before Phase 2**

---

## Success Criteria (Phase 1 COMPLETE)

### Minimum Viable ✅
- [x] Phase 1 build successful (compilation + linking complete)
- [ ] Test 1 passes (binary executes without immediate crash)
- [ ] Test 2 passes (graphics device initializes)
- [ ] Test 3 passes (no crash for 10 seconds)

### Full Validation ✅
- [ ] All tests 1-6 pass
- [ ] Logs reviewed by developer
- [ ] No unexpected crashes
- [ ] Memory usage reasonable

### Acceptance Criteria for Phase 2 Kickoff
- [x] Phase 1 build complete (done ✅)
- [ ] Smoke test 1-3 passing
- [ ] No segfaults in test logs
- [ ] Binary ready for audio integration
- [ ] Git committed and documented

---

## Next Steps After Smoke Tests

**If Tests Pass** ✅ → **Phase 2 (Audio) Planning Begins**
- Start Phase 2.1: Audio abstraction layer
- Reference jmarshall OpenAL patterns
- Design Miles → OpenAL wrapper

**If Tests Fail** ❌ → **Debug & Document**
- Identify failure point (Test 1-6)
- Grep logs for error patterns
- Check `build/linux64-deploy/_deps/` library status
- Rebuild if needed (`rm -rf build/linux64-deploy && rebuild`)

---

## Reference Materials

- Phase 1 Details: [SESSION_38_BUILD_SUCCESS.md](../WORKDIR/support/SESSION_38_BUILD_SUCCESS.md)
- Dev Blog: [2026-02-DIARY.md](../../DEV_BLOG/2026-02-DIARY.md)
- Phase 2 Plan: [PHASE_2_AUDIO_PLAN.md](./PHASE_2_AUDIO_PLAN.md)
- Docker Scripts: `scripts/docker-smoke-test-zh.sh` (existing test runner)

---

> **Bender's Wisdom**: "I'm interested in expanding my mind. Give me any drug that does that." 
> But in this case, let's expand our testing coverage first! 🤖

