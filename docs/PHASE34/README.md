# PHASE34: Crash Handling

## Phase Title

Crash Handling - Better Error Reporting

## Objective

Implement robust crash handling with informative error messages and crash dumps for debugging.

## Dependencies

- PHASE23 (Game Loop)
- PHASE32 (Performance Profiling)
- Understanding of Critical Lesson 4: ARC/Global State (macOS specific)

## Key Deliverables

1. Exception handling
2. Crash dump generation
3. Stack trace capture
4. Error logging
5. User-friendly error messages

## Acceptance Criteria

- [ ] Crashes logged with stack traces
- [ ] Crash dumps generated
- [ ] Error messages clear and helpful
- [ ] No crash loops
- [ ] Crash reporter optional (privacy)
- [ ] Recovery attempted when possible

## Technical Details

### Components to Implement

1. **Exception Handling**
   - Try/catch wrapper
   - Signal handlers
   - Platform-specific handlers

2. **Debugging Info**
   - Stack trace capture
   - Module information
   - Symbol resolution

3. **Logging**
   - Crash log file
   - System information
   - Game state snapshot

### Code Location

```
Core/GameEngineDevice/Source/Platform/
GeneralsMD/Code/Debug/CrashHandler/
```

## Estimated Scope

**SMALL** (1-2 days)

- Exception handling: 0.5 day
- Stack trace: 0.5 day
- Logging: 0.5 day
- Testing: 0.5 day

## Status

**not-started**

## Important References
- **Critical Lesson 4**: `docs/PHASE00/CRITICAL_LESSONS.md` - ARC/Global State - macOS specific (Phase 34.3 discovery)
- **Critical Lesson 2**: `docs/PHASE00/CRITICAL_LESSONS.md` - Exception Handling with context

## Testing Strategy

```bash
# Trigger intentional crashes and verify logging
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- Crashes produce useful logs
- Stack traces accurate
- No loss of debugging information
- Users can report issues
- Developer can debug from logs

## Reference Documentation

- Platform-specific crash handling (POSIX signals)
- Stack unwinding
- Symbol debugging information
- Crash analytics patterns
