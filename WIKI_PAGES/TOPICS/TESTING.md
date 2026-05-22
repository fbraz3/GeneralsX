# TOPICS/TESTING.md

## Summary

Testing protocol — verifying builds, replays, and platform compatibility.

---

## Testing Types

### 1. Smoke Tests

Quick verification that the build runs and basic features work.

**Command:**
```bash
./scripts/qa/smoke/docker-smoke-test-zh.sh linux64-deploy
```

**What it checks:**
- Binary loads
- Basic game initialization
- Minimal replay playback

### 2. Replay Tests

Comprehensive replay playback verification.

**Requirements:**
- Retail replays for both games
- Test on both Linux and macOS
- Verify pass/fail reports

**Command:**
```bash
cd ~/GeneralsX/GeneralsMD
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/GeneralsX/logs/manual_run.log
```

### 3. Regression Tests

Verify no regression in functionality.

**Checklist:**
- [ ] Original replays load correctly
- [ ] Gameplay matches original
- [ ] No unexpected behavior changes
- [ ] CRC validation works
- [ ] Map names parse correctly

---

## Testing Checklist

### Pre-Release

- [ ] Smoke test passes
- [ ] Replay tests pass (both games)
- [ ] No new platform issues
- [ ] Determinism verified
- [ ] FPS caps enforced

### Platform-Specific

| Platform | Required Tests |
|----------|---------------|
| Linux | Replay + smoke + determinism |
| macOS | Replay + smoke + transparency check |

---

## CI Integration

**Linux + macOS replay tests** — Per-file pass/fail details

**Explicit CI failure on replay failures** — Build fails immediately on any test failure

---

## Common Issues

### Replay Not Found

**Symptoms:** `Error: Replay file not found`

**Solution:**
```bash
# Verify replay path
ls -la <replay_file>

# Use absolute path
./GeneralsXZH -replay /absolute/path/to/replay.scn
```

### CRC Mismatch

**Symptoms:** `CRC mismatch, replay may be corrupted`

**Solution:**
- Verify file integrity
- Try with different replay
- Check for platform-specific path issues

---

## See Also

- [CONCEPTS/RETAIL.md](../CONCEPTS/RETAIL.md) — Retail compatibility
- [CONCEPTS/DETERMINISM.md](../CONCEPTS/DETERMINISM.md) — Determinism testing

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, testing documentation
