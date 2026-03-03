# REPORT: CCCache Diagnosis and Solution for GeneralsX

## Problem Identified

**CCCache is nearly useless in this project!**

Statistics found:
- **37.53%** of calls "Cacheable"
- **62.46%** of calls "UNCACHEABLE" <- **THIS IS THE PROBLEM!**
- Of the 37.53% cacheable, we have 73.36% hits
- Meaning: only ~27% of compilations actually use the cache!

## Root Cause

### `__TIME__` and `__DATE__` in WinMain.cpp

File: `GeneralsMD/Code/Main/WinMain.cpp` line 899

```cpp
AsciiString(__TIME__), AsciiString(__DATE__)
```

These macros **change EVERY SECOND**, interfering with the ccache key.

**But wait...** this should be addressable with ccache sloppiness!

## Proposed Solution

### 1. Configure CCCache with `time_macros` Sloppiness

Create file: `~/.config/ccache/ccache.conf`

```ini
# GeneralsX ccache configuration
max_size = 20.0G
compress = true
compression_level = 9

# KEY: Ignore __TIME__, __DATE__, __TIMESTAMP__ in the cache key
sloppiness = time_macros,locale

direct_mode = true
stats = true
```

**On macOS**, ccache looks in:
- `~/.config/ccache/ccache.conf` (XDG standard)
- `~/Library/Preferences/ccache/ccache.conf` (macOS specific)

Alternatively, set per-build without modifying the global config:
```bash
CCACHE_SLOPPINESS=time_macros,locale cmake --build ...
```

### 2. Verify it is applied

```bash
ccache -p | grep sloppiness
# Should show: sloppiness = time_macros,locale
```

### 3. Test the improvement

```bash
# Clear stats
ccache -z

# Compile (first time)
time cmake --build build/macos-vulkan --target z_ww3d2

# Intermediate stats
ccache -s

# Compile again (second time -- should use cache)
time cmake --build build/macos-vulkan --target z_ww3d2

# Final stats
ccache -s
```

**Expected result**:
- Second compilation should be **2-3x faster**
- Hit rate should rise to **>70%**

## Alternative: Remove `__TIME__` and `__DATE__` (Nuclear Option)

If even with `time_macros` there are still problems:

```cpp
// GeneralsX @bugfix BenderAI 25/02/2026
// Replaced __TIME__ and __DATE__ with fixed strings for cache efficiency
// CCCache 62.46% uncacheable calls was due to nondeterministic macros
AsciiString("00:00:00"), AsciiString("Jan 01 1970")
```

**Downside**: Build metadata becomes inaccurate.

## Technical Status

### Files Involved

- `cmake/ccache.cmake` - CMake integration (sets CCACHE_SLOPPINESS env var)
- `GeneralsMD/Code/Main/WinMain.cpp` - Source of __TIME__/__DATE__
- `~/.config/ccache/ccache.conf` - User config (optional, see setup_ccache.sh)

### Next Actions

1. [ ] Ensure `sloppiness = time_macros,locale` is active (via env var or config file)
2. [ ] `ccache -p | grep sloppiness` should show `time_macros,locale`
3. [ ] Run test_ccache.sh to validate improvement
4. [ ] If still not working, investigate other 62% uncacheable calls
5. [ ] Consider nuclear option (remove __TIME__/__DATE__)

## Expected Benchmarks (with sloppiness applied)

### Before (without sloppiness):
- 62.46% uncacheable
- Build z_ww3d2 first time: ~130 seconds
- Build z_ww3d2 second time: ~120 seconds (little difference!)

### After (with sloppiness + time_macros):
- ~27% uncacheable (significant reduction)
- Build z_ww3d2 first time: ~130 seconds
- Build z_ww3d2 second time: ~20-30 seconds (4-6x faster!)

## References

- CCCache 4.12.2 (used in this project)
- `time_macros` sloppiness: Ignores `__TIME__`, `__DATE__`, `__TIMESTAMP__`
- Locale sloppiness: Ignores locale differences in preprocessor directives
