# Batch Symbol Resolution Strategy - GeneralsXZH

## Analysis

**Total undefined symbols:** 1,065
- **The* Globals:** ~132 symbols
- **Vulkan (vk*):** Should be dynamic
- **SDL symbols:** Should be from SDL2 linking
- **C++ mangled (__Z*):** Expected

## Root Cause Hypothesis

The compilation units **ARE included** in CMakeLists (verified):
- ArchiveFileSystem.cpp ✅
- LocalFileSystem.cpp ✅
- Other core System/*.cpp files ✅

**Problem:** Object files compiled but symbols not exported or visible to linker.

## Batch Fix Strategy (No Trial-and-Error)

### Step 1: Extract All Object Files from Build

```bash
find build/macos -name "*.o" | wc -l
nm build/macos/Core/GameEngine/CMakeFiles/z_gameengine.dir/**/*.cpp.o | grep "ArchiveFileSystem" | head -5
```

### Step 2: Verify Symbols ARE in Object Files

```bash
nm build/macos/Core/GameEngine/CMakeFiles/z_gameengine.dir/Source/Common/System/ArchiveFileSystem.cpp.o | grep TheArchiveFileSystem
```

If it shows: `000... T _TheArchiveFileSystem` → Symbol is defined, exported (T = text/code)
If nothing → Symbol not in this object file

### Step 3: Check What's Actually Being Linked

```bash
nm build/macos/GeneralsMD/CMakeFiles/GeneralsXZH.dir/CMakeFiles.txt  # Check link.txt
```

### Step 4: Batch Apply Fixes

Once we identify **which compilation units are missing from the link command**, we fix:

**Option A:** Update CMakeLists to explicitly include them
**Option B:** Use `target_link_libraries()` to ensure proper linking
**Option C:** Add `-Wl,--whole-archive` to force symbol export

## Key Files to Examine

1. **Core/GameEngine/CMakeLists.txt** - Line 645-667 (System/*.cpp files)
2. **Core/GameEngineDevice/CMakeLists.txt** - Audio/Video compilation units
3. **GeneralsMD/CMakeLists.txt** - Target-specific linking
4. **cmake/config.cmake** - Global linking flags

## Automated Detection Command

```bash
# 1. Export all undefined symbols
nm -u build/macos/GeneralsMD/GeneralsXZH | awk '{print $NF}' | sort > /tmp/undefined.txt

# 2. For each category, check if symbols appear in object files
grep "^_The" /tmp/undefined.txt | while read sym; do
  if ! find build/macos -name "*.o" -exec nm {} + 2>/dev/null | grep -q "^.* T $sym"; then
    echo "MISSING: $sym"
  fi
done | head -20
```

## Next Steps

1. Run symbol extraction script to identify which .cpp files have undefined symbols
2. Check if those .cpp.o files are actually being linked
3. Apply one unified CMakeLists fix
4. Rebuild once
5. Validate all symbols resolved

This avoids the trial-and-error loop by systematically identifying the gap in the build chain.
