# Phase 48 Crash Analysis - November 4, 2025

## Summary

**Test Results**: 10 consecutive runs with mixed failures:
- **Success**: Runs 1, 2, 3, 8 (1944-2196 frames rendered)
- **Hung**: Runs 4, 5, 6, 9, 10 (1 frame, process hangs at render encoder creation)
- **Segfault**: 5-6 crashes reported (per user statement)

**Success Rate**: ~40% (4 of 10 ran successfully, 6 hung or crashed)

---

## Problem 1: Intermittent Hang in Metal Render Encoder Creation

### Symptoms
```
METAL DEBUG: About to call renderCommandEncoderWithDescriptor...
[Process hangs indefinitely]
[No exception, no crash - just freeze]
```

**Affected runs**: 4, 5, 6, 9, 10 (5 out of 10 = 50% hang rate)

### Location
File: `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm`
Function: `GX::MetalWrapper::BeginFrame()`
Line: ~522

### Root Cause
The current fix (1ms usleep before encoder creation) is **INSUFFICIENT**.

The hang occurs because:
1. Metal render encoder creation is calling AGX (Apple GPU driver)
2. AGX is compiling shaders in background threads (dispatch_queue: com.apple.root.default-qos)
3. Race condition: foreground thread waits for encoder, background compiler thread accesses shared Metal data structures
4. This causes a deadlock in Metal framework internals

### Evidence from OS Crash Report

**Thread 0 (Main)**: Blocked in Metal framework waiting for render encoder
```
Frame 13: -[AGXG13GFamilyCommandBuffer renderCommandEncoderWithDescriptor:] + 156
Frame 14: GX::MetalWrapper::BeginFrame(float, float, float, float) + 1052
```

**Thread 5 (Background Compiler)**: Crashed in shader compilation while accessing hash table
```
Exception Type:    EXC_BAD_ACCESS (SIGSEGV)
Exception Subtype: KERN_INVALID_ADDRESS (possible pointer authentication failure)
Triggered by Thread: 5, Dispatch Queue: com.apple.root.default-qos

Crash in:
- AGX::Compiler::compileProgram<AGX::BackgroundObjectProgramKey>
- AGCDeserializedReply::AGCDeserializedReply()
- std::__1::__hash_table<std::__1::__hash_value_type<...>>::__emplace_unique_key_args()
```

**Root Cause Chain**:
1. Main thread creates render encoder (AGX/Metal operation)
2. AGX background thread (Thread 5) attempts shader compilation simultaneously
3. Both threads access Metal's hash table for shader cache
4. No synchronization between threads → memory corruption
5. **Either**: Thread 5 crashes with SIGSEGV (observed), **Or**: Thread 0 deadlocks in Metal framework

---

## Problem 2: SIGSEGV from Pointer Authentication Failure

### Crash Details
```
Exception Type:    EXC_BAD_ACCESS (SIGSEGV)
Exception Subtype: KERN_INVALID_ADDRESS at 0xccb7fa9fd2b7299c 
                   -> 0xfffffa9fd2b7299c (possible pointer authentication failure)
VM Region Info: 0xfffffa9fd2b7299c is not in any region
```

### Hardware Context
- Apple Silicon M1 (ARM64e with pointer authentication)
- Pointer authentication codes (PAC) are 5-bit security extension on ARM64e
- Invalid PAC decoding → address becomes gibberish
- CPU rejects access → SIGSEGV

### Why It Happens
The memory corruption in Metal's hash table during concurrent shader compilation causes:
1. Valid pointer address gets corrupted by background compiler
2. Corrupted pointer fails ARM64e PAC validation
3. CPU rejects memory access
4. Process crashes with SIGSEGV

---

## Current State vs Required State

### What We Tried
1. **100ms usleep before file I/O**: Fixed crash during initialization ✅
2. **1ms usleep before render encoder**: Attempted fix - NOT WORKING ❌
3. **@autoreleasepool wrapper**: Tried briefly - Caused Bus error: 10 ❌

### Why 1ms Usleep Doesn't Fix Hang
- File I/O initialization is serialized (100ms delay works)
- **But** render encoder creation is called EVERY FRAME (~2000+ times during test)
- 1ms delay per frame = massive performance hit (60 FPS → 6 FPS impossible)
- Even with delay, race condition still exists (just lower probability)
- Need SYNCHRONIZATION, not just delay

---

## Root Cause: Missing Synchronization

The real problem is that Metal's background compiler threads are running unsynchronized with game render thread.

**Evidence**:
- Runs 1, 2, 3, 8 succeed (lucky timing - compiler isn't active)
- Runs 4, 5, 6, 9, 10 hang/crash (compiler is active, thread collision)
- Pattern is **intermittent** (classic race condition)

**Solution Strategy**:
We need to serialize Metal operations to prevent AGX background compiler from interfering with render encoder creation.

---

## Recommended Fixes (In Priority Order)

### Option 1: Serialize Render Encoder Creation (BEST)
Force Metal to complete background compilation before creating encoder:
```objectivec++
// BEFORE creating render encoder:
[s_cmdBuffer waitUntilScheduled];  // Force wait for pending GPU operations
usleep(10000);  // 10ms - allow shader cache to settle
// NOW safe to create encoder:
s_renderEncoder = [s_cmdBuffer renderCommandEncoderWithDescriptor:pass];
```

**Pros**: 
- Directly addresses root cause (synchronization)
- Minimal code change
- No architectural changes needed

**Cons**:
- Potential small frame rate impact
- Still empirical (10ms might need tuning)

### Option 2: Queue-Based Synchronization (BETTER LONG-TERM)
Wrap all Metal operations in dispatch_sync on Metal queue:
```objectivec++
// Create a serial Metal dispatch queue (once at init):
static dispatch_queue_t metalQueue = dispatch_queue_create(
    "com.generals.metal.render", DISPATCH_QUEUE_SERIAL);

// Before renderer:
dispatch_sync(metalQueue, ^{
    s_renderEncoder = [s_cmdBuffer renderCommandEncoderWithDescriptor:pass];
});
```

**Pros**:
- Explicit synchronization (no race conditions)
- GCD handles scheduling automatically
- Industry-standard pattern for Metal

**Cons**:
- More invasive code change
- Requires verifying no deadlocks with other Metal operations

### Option 3: Disable Background Shader Compilation (NUCLEAR)
Force Metal to compile shaders synchronously:
```objectivec++
// At Metal init:
BOOL useSync = YES;
MTLCompileFunctionRequestData* requestData = ...;
requestData.synchronously = useSync;  // Force sync compilation
```

**Pros**:
- Guaranteed no background thread interference
- No race condition possible

**Cons**:
- Huge frame rate hit during shader compilation
- Only viable for debugging, not production

---

## Testing Plan

### Phase 1: Verify Current Issue (DONE)
- [x] Run 10 tests → observe 50% hang rate
- [x] Analyze logs → hang at renderCommandEncoderWithDescriptor
- [x] Analyze crash report → background compiler conflict confirmed

### Phase 2: Implement Fix
- [ ] Try Option 1: `waitUntilScheduled()` + usleep approach
- [ ] Build and test: 10+ consecutive runs
- [ ] Target: 100% success rate (all runs complete with timeout exit)

### Phase 3: Verify No Regressions
- [ ] Check frame rate (should be ~30 FPS target)
- [ ] Verify no new crashes
- [ ] Test with different GPU drivers if available

### Phase 4: Performance Validation
- [ ] Measure frame times before/after fix
- [ ] Ensure < 1ms overhead per frame
- [ ] Document trade-offs

---

## Key Files to Examine

1. **metalwrapper.mm** - Metal render wrapper
   - Function: `BeginFrame()` (line ~390)
   - Problem area: `renderCommandEncoderWithDescriptor:` call (line ~518-530)

2. **dx8wrapper.cpp** - DirectX 8 mock layer
   - Function: `Begin_Scene()` (line 2052)
   - Calls: `MetalWrapper::BeginFrame()`

3. **ww3d.cpp** - W3D graphics engine
   - Function: `Begin_Render()` (line 862)
   - Called every frame from render loop

---

## Conclusion

**The 1ms usleep fix is not sufficient because:**
1. Render encoder creation is called every frame (~2000+ times)
2. 1ms delay per frame is still too aggressive (breaks frame rate)
3. Race condition still exists - just with lower probability
4. Real solution needs **synchronization**, not timing

**Recommended immediate action:**
Implement Option 1 (`waitUntilScheduled()` + 10ms usleep) and test with 20+ runs to verify stability before moving to Option 2 (queue-based synchronization).

