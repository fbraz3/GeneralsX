# CRITICAL: MinGW vs Native Linux Compilation

**Date**: 2026-02-07  
**Author**: Bender (after embarrassing myself)  
**Status**: Corrected understanding

## The Mistake

Initially assumed fighter19's port used **MinGW** to cross-compile for Linux from other platforms. **This was COMPLETELY WRONG.**

## The Truth

fighter19's port uses **NATIVE Linux compilation** to produce **ELF binaries** that run directly on Linux without Wine.

## Key Differences

### MinGW (What We DON'T Use)

```
Source Code (C++)
      ↓
MinGW Compiler (i686-w64-mingw32-gcc)
      ↓
Windows PE Executable (.exe)
      ↓
Runs on:
  ✅ Windows natively
  ✅ Linux via Wine + DXVK (Windows emulation)
  ❌ Linux natively (wrong binary format)
```

**MinGW = Minimalist GNU for Windows**
- Produces Windows executables
- Can run from Mac/Linux but OUTPUTS Windows binaries
- Used for cross-compiling TO Windows, not FROM Windows

### Native Linux Compilation (What fighter19 ACTUALLY Uses)

```
Source Code (C++)
      ↓
GCC/Clang on Linux (gcc, g++)
      ↓
Linux ELF Executable (no extension or .elf)
      ↓
Runs on:
  ✅ Linux natively (direct execution)
  ❌ Windows (wrong binary format)
  ❌ macOS (wrong binary format)
```

**Native Linux = Real Linux Binaries**
- Produces ELF executables
- No Windows compatibility layer needed
- Direct hardware access via Linux kernel
- Can run on ARM64 (Raspberry Pi 5) because it's truly native

## Evidence from fighter19

### 1. CI Configuration
```yaml
# .github/workflows/ubuntu.yml
runs-on: ubuntu-22.04
VCPKG_DEFAULT_TRIPLET: x64-linux-dynamic  # ← LINUX NATIVE!
```

**NOT** `x64-mingw-dynamic` (which would be Windows)

### 2. CMake Presets
```json
{
  "name": "linux64-deploy",
  "VCPKG_TARGET_TRIPLET": "x64-linux-dynamic",
  "SAGE_USE_SDL3": "ON"
}
```

**NOT** `x64-windows` or `x64-mingw`

### 3. Dependencies
```bash
sudo apt install -y libx11-dev libwayland-dev  # Linux windowing
sudo apt install -y libibus-1.0-dev            # Linux input
```

**NOT** Wine or Windows SDK

### 4. README Evidence
- "libdxvk_d3d8.**so**" (Linux shared library, not .dll)
- "Works on a Raspberry Pi 5" (ARM64 Linux native)
- "Support for Linux x64 and ARM64" (native architectures)
- **Zero mentions of Wine anywhere**

## How DXVK Works in This Context

DXVK is **compiled as a Linux shared library** (.so):

```
Game Code (DirectX 8 API calls)
      ↓
libdxvk_d3d8.so (Linux shared library)
      ↓
Translates DirectX → Vulkan
      ↓
Vulkan API (native Linux)
      ↓
GPU Driver (Linux kernel)
```

**NOT** Windows DXVK running under Wine!

## Implications for Our Project

### From macOS, We CANNOT:
- ❌ Compile Linux binaries natively (macOS can't produce ELF)
- ❌ Use MinGW (produces Windows .exe, not Linux ELF)
- ❌ Use Clang alone (macOS Clang targets Mach-O, not ELF)

### From macOS, We CAN:
- ✅ **Use Docker** to run Ubuntu and compile natively (Linux ELF)
- ✅ **Use Docker with MinGW** to cross-compile Windows .exe (no brew install!)
- ✅ Use Linux VM to compile natively
- ✅ Use Windows VM to compile with VC6/MSVC
- ❌ Cross-compile with specialized toolchain (complex, not recommended)

## Docker Solution (Recommended)

Docker runs a **real Linux kernel** and **real Linux environment**:

### A) Linux Native Builds (ELF)
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && 
  apt install -y build-essential cmake ninja-build git &&
  cmake --preset linux64-deploy &&
  cmake --build build/linux64-deploy --target z_generals
"
```

**Result**: Native Linux ELF binary, identical to building in a Linux VM.

### B) Windows Builds via MinGW (in Docker - keeps Mac clean!)
```bash
docker run --rm -v "$PWD:/work" -w /work ubuntu:22.04 bash -c "
  apt update && 
  apt install -y build-essential cmake ninja-build git mingw-w64 &&
  cmake --preset mingw-w64-i686 &&
  cmake --build build/mingw-w64-i686 --target z_generals
"
```

**Result**: Windows PE executable (.exe) that runs on Windows or Linux (via Wine+DXVK).

**Why Docker rocks**:
- No toolchain pollution on macOS (no `brew install mingw-w64`)
- Reproducible builds across all developers
- Same commands work in CI/CD
- Isolated environments for different build types

## Binary Format Quick Reference

| Format | Platform | Extension | Magic Number |
|--------|----------|-----------|--------------|
| **ELF** | Linux | (none) or .elf | `7F 45 4C 46` |
| **PE** | Windows | .exe, .dll | `4D 5A` (MZ) |
| **Mach-O** | macOS | (none) | `CF FA ED FE` |

Use `file` command to check:
```bash
file build/linux64-deploy/GeneralsMD/GeneralsXZH
# Expected: "ELF 64-bit LSB executable, x86-64"
# NOT: "PE32 executable (console) Intel 80386"
```

## Lesson Learned

**Never assume**. Always verify:
1. Check CI workflows (.github/workflows/)
2. Read CMakePresets.json carefully
3. Look for platform-specific dependencies
4. Search for "wine" or "mingw" in README
5. Check compiler flags and toolchain

**Bite my shiny metal ass** if you make the same mistake! 🤖

---

## Summary

- **MinGW**: Cross-compiles TO Windows (produces .exe) - can run in Docker!
- **fighter19**: Native Linux compilation (produces ELF) - also runs in Docker!
- **Our strategy**: Use Docker for BOTH Linux (native ELF) AND Windows (MinGW .exe)
- **Why Docker**: Real Linux environment, no Mac pollution, reproducible, perfect for CI/CD
