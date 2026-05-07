# Replay & MapCache Technical Debt

> Temporary reference for opening GitHub issues. Discovered during replay headless testing session (2026-05-06).

---

## Issue 1: Custom Map Replays Fall Back to CRC Resolution Instead of Direct Path

### Symptom

When playing back a replay recorded on a custom user map (e.g. `[rank] arctic arena zh v1`), the engine logs:

```
[GeneralsX] Replay map resolved via CRC fallback: CRC=0x43396DD8 size=162490 -> '...'
```

This means the `M=` field in the replay header is not being resolved directly — the engine is falling back to CRC+size scan across all known maps.

### Root Cause (Suspected)

`GameInfoToAsciiString()` serializes the map path into the `M=` field with percent-encoding applied. However, during replay recording the path stored may still contain characters that are not being round-tripped correctly, OR the path stored is an absolute host path that cannot be matched on decode.

The percent-encode/decode functions (`percentEncodeMapName` / `percentDecodeMapName`) were added in commit `3f9db3133`, but the replays recorded after that fix still fall back to CRC. This suggests the encoded value in `M=` is either:
- Not being decoded back to the right relative path, or
- The path comparison in `ParseAsciiStringToGameInfo()` fails silently and falls through to CRC

### Investigation Hints

- Add a `fprintf(stderr, "[GeneralsX] M= field decoded: '%s'\n", portableMapPath.str())` log just after decoding `M=` in `ParseAsciiStringToGameInfo()` to inspect what path is being looked up
- Compare decoded path against what `TheMapCache` actually knows
- Check if `getReplayDir()` or `getUserMapsDir()` prefix logic is stripping the path differently depending on where the map was installed by the user

### Relevant Files

- `Core/GameEngine/Source/GameNetwork/GameInfo.cpp` — `ParseAsciiStringToGameInfo()`, `GameInfoToAsciiString()`
- `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp` — `readReplayHeader()`

### Impact

Low — CRC fallback works correctly and replays succeed. The fallback is O(n) over all maps in the cache, which is negligible. No gameplay correctness impact.

### Labels (suggested)

`bug`, `replay`, `cross-platform`, `tech-debt`

---

## ~~Issue 2: MapCache.ini Created as `Maps\MapCache.ini` (Literal Backslash in Filename) on POSIX Systems~~

**RESOLVED** — Fix confirmed working. `MapCache::writeCacheINI()` and `MapCache::loadMapsFromMapCacheINI()` were corrected in commit `6b66e528a` to use `/` as path separator. Verified on macOS 2026-05-06.
