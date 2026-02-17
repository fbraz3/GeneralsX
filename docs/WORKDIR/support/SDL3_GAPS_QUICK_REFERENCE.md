# SDL3 Implementation Gaps - Quick Reference

```
┌─────────────────────────────────────────────────────────────────┐
│ 🔴 CRITICAL GAPS (Fix in Phase 1.7)                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ 1. Buffer Size: 128 → 256                                       │
│    Impact: Event loss on high-frequency input                  │
│    Estimated fix time: 5 min                                   │
│                                                                  │
│ 2. Text Input Not Handled                                       │
│    Impact: Non-Latin keyboard input won't work (IME, accents)  │
│    Estimated fix time: 2-3 hours (Phase 2)                    │
│                                                                  │
│ 3. Double-Click: Manual vs Native SDL3                          │
│    Impact: Unreliable double-click detection                   │
│    Estimated fix time: 30 min                                  │
│                                                                  │
│ 4. Frame Tracking Missing                                       │
│    Impact: Replay determinism broken (timing relies on ms only)│
│    Estimated fix time: 45 min                                  │
│                                                                  │
│ 5. m_lostFocus Not Checked                                      │
│    Impact: Input events processed even when window loses focus │
│    Estimated fix time: 15 min                                  │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ 🟡 MEDIUM PRIORITY GAPS (Phase 2)                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ 6. Keyboard Mapping: ~10 keys → ~30+ keys                       │
│    Missing: Numpad, F1-F12, Insert, Delete, Home, End, etc.   │
│    Estimated fix time: 1 hour                                  │
│                                                                  │
│ 7. Mouse DPI Scaling Not Implemented                            │
│    Impact: Position offset on high-DPI displays               │
│    Estimated fix time: 1.5 hours                              │
│                                                                  │
│ 8. Timestamp Normalization                                      │
│    Impact: Inconsistent timing across event types             │
│    Estimated fix time: 30 min                                  │
│                                                                  │
│ 9. SDL Subsystem Lifecycle                                      │
│    Missing: SDL_InitSubSystem(SDL_INIT_EVENTS) + cleanup      │
│    Estimated fix time: 20 min                                  │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ 🟢 LOW PRIORITY (Phase 3+)                                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ 10. Cursor Animation System                                     │
│     Status: Phase 2 stub - not blocking input                  │
│                                                                  │
│ 11. SDL3GameEngine: argc/argv Support                           │
│     Status: Nice-to-have, not blocking                         │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

📊 CODE COMPARISON

Fighter19 Keyboard:
  ├─ std::vector<SDL_Event> (dynamic)
  ├─ SDL_EVENT_TEXT_INPUT handling
  ├─ IMEManager integration
  ├─ ConvertSDLKey() ~50 lines with full coverage
  └─ SDL_InitSubSystem/SDL_QuitSubSystem

Our Keyboard:
  ├─ SDL_Event[64] (static)
  ├─ No text input (KEY_DOWN/UP only)
  ├─ No IME integration
  ├─ ConvertSDLKey() ~10 lines (minimal)
  └─ No SDL subsystem init/cleanup

Fighter19 Mouse:
  ├─ Buffer: 256 events (Mouse::NUM_MOUSE_EVENTS)
  ├─ Double-click: Uses mouseBtnEvent.clicks (native)
  ├─ Scaling: scaleMouseCoordinates() for DPI awareness
  ├─ Frame tracking: m_inputFrame stored
  ├─ m_lostFocus: Checked in setCursor()
  └─ Cursor: Full animation system with .ani files

Our Mouse:
  ├─ Buffer: 128 events (local constant)
  ├─ Double-click: Manual calculation with timestamps
  ├─ Scaling: None - raw coordinates
  ├─ Frame tracking: None
  ├─ m_lostFocus: Not implemented
  └─ Cursor: Stub only


⏱️ ESTIMATED EFFORT BREAKDOWN

Phase 1.7 Quick Wins (3-4 hours):
  □ Buffer size increase              5 min
  □ m_lostFocus implementation        15 min
  □ Native double-click (clicks)      30 min
  □ Frame tracking integration        45 min
  □ Timestamp normalization           30 min
  □ Testing in Linux VM               1.5 hours
  ────────────────────────────────────
  Total: ~3.5 hours

Phase 2 Text Input & Advanced (6-8 hours):
  □ SDL_EVENT_TEXT_INPUT handler      1.5 hours
  □ IMEManager integration            1.5 hours
  □ Comprehensive key map (fighter19) 1 hour
  □ Mouse DPI scaling                 1.5 hours
  □ SDL subsystem lifecycle           30 min
  □ Testing & validation              1 hour
  ────────────────────────────────────
  Total: ~7 hours

Phase 3 Cursor System (2-3 hours):
  □ AnimatedCursor structure          30 min
  □ Cursor file loading (.ani)        1 hour
  □ Animation frame calculation       45 min
  □ Testing                           30 min
  ────────────────────────────────────
  Total: ~2.5 hours


🎯 RECOMMENDED NEXT STEPS

Session 41 (Quick Wins Phase 1.7):
✅ 1. Increase buffer to 256 (both mouse/keyboard)
✅ 2. Add m_lostFocus - prevents stale input when focused lost
✅ 3. Use mouseBtnEvent.clicks for double-click detection
✅ 4. Add frame number tracking to events (determinism!)
✅ 5. Linux VM smoke test with menu input

Session 42+ (Phase 2 Text Input):
⏳ 1. SDL_EVENT_TEXT_INPUT handler
⏳ 2. IMEManager integration
⏳ 3. Fighter19 comprehensive key mapping
⏳ 4. Complete testing before Phase 2 release

```

---

## 📌 Key Files to Reference

```
Fighter19 Implementation:
  └─ references/fighter19-dxvk-port/
     ├─ GeneralsMD/Code/GameEngineDevice/
     │  ├─ Include/SDL3Device/Common/SDL3GameEngine.h
     │  ├─ Include/SDL3Device/GameClient/SDL3Mouse.h (line 40+)
     │  ├─ Include/SDL3Device/GameClient/SDL3Keyboard.h
     │  └─ Source/SDL3Device/
     │     ├─ GameClient/SDL3Mouse.cpp (line 230: getMouseEvent, line 300+: translateEvent)
     │     └─ GameClient/SDL3Keyboard.cpp (line 77: ConvertSDLKey, line 150: getKey)

Our Implementation:
  └─ GeneralsMD/Code/GameEngineDevice/
     ├─ Include/SDL3GameEngine.h
     ├─ Include/SDL3Device/GameClient/
     │  ├─ SDL3Mouse.h
     │  └─ SDL3Keyboard.h
     └─ Source/
        ├─ SDL3GameEngine.cpp
        └─ SDL3Device/GameClient/
           ├─ SDL3Mouse.cpp
           └─ SDL3Keyboard.cpp
```

---

## 🔍 Verification Checklist

After implementing Phase 1.7 improvements:

- [ ] Mouse buffer has 256 slots (use Mouse::NUM_MOUSE_EVENTS)
- [ ] Keyboard buffer has 256 slots
- [ ] m_lostFocus flag checked in mouse input processing
- [ ] Double-click uses mouseBtnEvent.clicks (not manual calculation)
- [ ] Frame number tracked in MouseIO/KeyboardIO events
- [ ] All timestamps normalized to milliseconds
- [ ] Linux VM test: Menu responds to clicks
- [ ] Replay determinism: Same input produces same output

---

Generated: 2026-02-17 (Session 40.5 - Analysis)  
Source: Deep analysis of fighter19-dxvk-port implementation
