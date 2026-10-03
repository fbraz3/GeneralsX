# TOPICS/DEBUGGING.md

## Summary

Debugging techniques — tools, procedures, and best practices for troubleshooting.

---

## Tools

### GDB (Linux/macOS)

**Command:**
```bash
mkdir -p logs
gdb -batch -ex "run -win" -ex "bt full" -ex "thread apply all bt" \
  ./build/linux64-deploy/GeneralsMD/GeneralsXZH 2>&1 | tee logs/gdb.log
```

### Log Viewer

**Command:**
```bash
./run.sh -win -logToCon 2>&1 | grep -v "D3DRS_PATCHSEGMENTS" | tee ~/GeneralsX/logs/manual_run.log
```

---

## Procedures

### 1. Isolate Platform Issue

1. **Verify build** — Ensure clean build
2. **Test on both platforms** — Linux vs macOS
3. **Check platform-specific logs** — Look for platform errors
4. **Test minimal scenario** — Simplify to isolate

### 2. Isolate Game Issue

1. **Test with retail replay** — Known-good baseline
2. **Check CRC validation** — Ensure replay integrity
3. **Verify FPS caps** — Ensure frame pacing
4. **Test without custom content** — Rule out mod conflicts

---

## Common Issues

### Platform Issues

| Issue | Symptoms | Solution |
|-------|----------|----------|
| FPS uncapped | Runs fast on high-refresh monitors | Verify FPS cap enabled, check `-ffp-contract=off` |
| Asset loading fails | Missing textures/sounds | Check case-insensitive VFS, verify paths |
| Transparency issues | Visual artifacts | Verify DX8Wrapper pillarbox switching |

### Game Issues

| Issue | Symptoms | Solution |
|-------|----------|----------|
| Replay doesn't match | Different results | Check determinism, FPS caps, math consistency |
| AI behavior varies | Different decisions | Verify FPS cap, frame pacing |
| Map not found | "Map not found" error | Check VFS path handling, case sensitivity |

---

## Debug Flags

### `-logToCon`

**Note:** Available only with `RTS_BUILD_OPTION_DEBUG=ON`

**Purpose:** Output console logging

**Command:**
```bash
./GeneralsXZH -win -logToCon 2>&1 | tee log.txt
```

### `-replay`

**Purpose:** Load replay file

**Command:**
```bash
./GeneralsXZH -replay /path/to/replay.scn
```

---

## Best Practices

1. **Test on both platforms** — Catch platform-specific issues
2. **Use retail replays** — Known-good baseline
3. **Check CRC validation** — Ensure replay integrity
4. **Verify FPS caps** — Ensure determinism
5. **Log platform errors** — Capture platform-specific issues

---

## See Also

- [CONCEPTS/RETAIL.md](../CONCEPTS/RETAIL.md) — Retail compatibility
- [CONCEPTS/DETERMINISM.md](../CONCEPTS/DETERMINISM.md) — Determinism rules

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, debugging documentation
