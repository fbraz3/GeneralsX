#!/usr/bin/env python3
"""
dxvk-macos-patches.py - Apply macOS-specific patches to DXVK v2.6 source.

These patches are necessary because DXVK's "native" build targets Linux only.
macOS requires:
  1. util_win32_compat.h: __unix__ not defined on macOS, add __APPLE__ guard
  2. util_env.cpp:        pthread_setname_np macOS takes 1 arg (not 2 like Linux)
  3. util_small_vector.h: lzcnt(n-1) where n is size_t (unsigned long) is
                          ambiguous between uint32_t and uint64_t overloads
  4. util_bit.h:          Add uintptr_t overloads for tzcnt/lzcnt
  5. src/*/meson.build:   --version-script is GNU ld only, not macOS ld

Usage: dxvk-macos-patches.py <dxvk_source_dir>
"""

import sys
import os
import re

def patch_file(path, transform_fn, description):
    """Apply a transformation to a file and report the result."""
    if not os.path.exists(path):
        print(f"SKIP (not found): {os.path.basename(path)}")
        return False
    content = open(path).read()
    patched = transform_fn(content)
    if patched == content:
        print(f"OK (already patched): {os.path.basename(path)} - {description}")
        return True
    open(path, 'w').write(patched)
    print(f"PATCHED: {os.path.basename(path)} - {description}")
    return True


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <dxvk_source_dir>")
        sys.exit(1)

    src = sys.argv[1]
    if not os.path.isdir(src):
        print(f"ERROR: {src} is not a directory")
        sys.exit(1)

    # Patch 1: util_win32_compat.h
    # macOS does NOT define __unix__, so LoadLibraryA/GetProcAddress/FreeLibrary
    # stubs are never compiled. Add __APPLE__ to the guard.
    patch_file(
        os.path.join(src, "src/util/util_win32_compat.h"),
        lambda c: c.replace(
            "#if defined(__unix__)",
            "#if defined(__unix__) || defined(__APPLE__)"
        ),
        "__unix__ guard: add __APPLE__"
    )

    # Patch 2: util_env.cpp
    # macOS pthread_setname_np only takes 1 argument: pthread_setname_np(name)
    # Linux signature:                                 pthread_setname_np(thread, name)
    patch_file(
        os.path.join(src, "src/util/util_env.cpp"),
        lambda c: c.replace(
            "    ::pthread_setname_np(pthread_self(), posixName.data());",
            "#ifdef __APPLE__\n"
            "    ::pthread_setname_np(posixName.data());\n"
            "#else\n"
            "    ::pthread_setname_np(pthread_self(), posixName.data());\n"
            "#endif"
        ),
        "pthread_setname_np 1-arg macOS signature"
    )

    # Patch 3: util_small_vector.h
    # size_t is 'unsigned long' on macOS arm64, distinct from both uint32_t and
    # uint64_t. Cast explicitly to avoid ambiguous overload resolution.
    patch_file(
        os.path.join(src, "src/util/util_small_vector.h"),
        lambda c: c.replace(
            "bit::lzcnt(n - 1)",
            "bit::lzcnt(static_cast<uint64_t>(n - 1))"
        ),
        "lzcnt: cast size_t→uint64_t to resolve overload ambiguity"
    )

    # Patch 4: util_bit.h
    # Add uintptr_t overloads for tzcnt/lzcnt.
    # On macOS arm64, uintptr_t is 'unsigned long' (neither uint32_t=unsigned int
    # nor uint64_t=unsigned long long). Template overload resolution fails.
    def patch_util_bit(c):
        if "uintptr_t n" in c:
            return c  # already patched
        extra = (
            "\n#if defined(__APPLE__)\n"
            "  // macOS arm64: uintptr_t is 'unsigned long', distinct from\n"
            "  // uint32_t (unsigned int) and uint64_t (unsigned long long).\n"
            "  // Explicit overloads to resolve template ambiguity.\n"
            "  inline uint32_t tzcnt(uintptr_t n) { return tzcnt(static_cast<uint64_t>(n)); }\n"
            "  inline uint32_t lzcnt(uintptr_t n) { return lzcnt(static_cast<uint64_t>(n)); }\n"
            "#endif\n"
            "\n"
        )
        # Insert before the last closing brace of the namespace
        last_brace = c.rfind('\n}')
        if last_brace == -1:
            return c
        return c[:last_brace] + extra + c[last_brace:]

    patch_file(
        os.path.join(src, "src/util/util_bit.h"),
        patch_util_bit,
        "uintptr_t overloads for tzcnt/lzcnt (macOS arm64)"
    )

    # Patch 5: meson.build files - --version-script is GNU ld only
    # macOS ld does not support --version-script. Guard the flag with a
    # platform != 'darwin' check.
    meson_files = [
        "src/dxgi/meson.build",
        "src/d3d8/meson.build",
        "src/d3d9/meson.build",
        "src/d3d10/meson.build",
        "src/d3d11/meson.build",
    ]
    for relpath in meson_files:
        patch_file(
            os.path.join(src, relpath),
            lambda c: re.sub(
                r"( +)(.*'-Wl,--version-script'.*\n)",
                r"  if platform != 'darwin'\n\1\2  endif\n",
                c
            ),
            "--version-script: guard with platform != 'darwin'"
        )

    print("\nAll macOS patches applied successfully.")


if __name__ == "__main__":
    main()
