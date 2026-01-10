#!/usr/bin/env python3
"""
Automatically fix missing source files in GeneralsMD CMakeLists.txt
by detecting files that exist in Core but not in GeneralsMD source directory
"""

import os
import re
from pathlib import Path

workspace_root = "/Users/felipebraz/PhpstormProjects/pessoal/GeneralsX"
md_cmake = os.path.join(workspace_root, "GeneralsMD/Code/GameEngine/CMakeLists.txt")
core_source = os.path.join(workspace_root, "Core/GameEngine/Source")
md_source = os.path.join(workspace_root, "GeneralsMD/Code/GameEngine/Source")

# Files to skip (already corrected or marked as unused)
skip_patterns = [
    "simpleplayer.cpp",  # marked as unused
    "urllaunch.cpp",     # marked as unused
    "WorkerProcess.cpp", # platform-specific
    "GameMemoryNull.cpp" # fallback implementation
]

def file_exists_in_source(filename, source_dir):
    """Check if file exists in source directory"""
    for root, dirs, files in os.walk(source_dir):
        if filename in files:
            return True
    return False

def should_skip(filename):
    """Check if file should be skipped"""
    return any(pattern in filename for pattern in skip_patterns)

# Read current CMakeLists
with open(md_cmake, 'r') as f:
    cmake_content = f.read()

# Find all referenced Source/ files (not Core paths)
# Match patterns like "Source/path/to/file.cpp" but NOT "..../Source"
pattern = r'(?<!/Source/)Source/([A-Za-z0-9/_-]+\.cpp)'
matches = list(re.finditer(pattern, cmake_content))

print(f"Found {len(matches)} Source/* references in CMakeLists.txt")
print("\nChecking which files don't exist in GeneralsMD source...")

replacements = []
for match in matches:
    rel_path = match.group(1)
    filename = os.path.basename(rel_path)

    if should_skip(filename):
        continue

    # Check if file exists in GeneralsMD
    md_full_path = os.path.join(md_source, rel_path)
    if not os.path.exists(md_full_path):
        # Check if it exists in Core
        if file_exists_in_source(filename, core_source):
            print(f"  MISSING: Source/{rel_path}")
            replacements.append((f"Source/{rel_path}", f"../../../Core/GameEngine/Source/{rel_path}"))

if replacements:
    print(f"\nNeed to fix {len(replacements)} file references in CMakeLists.txt")
    print("\nApplying fixes...")

    updated_content = cmake_content
    for old, new in replacements:
        # Count how many times we're replacing
        count = updated_content.count(old)
        updated_content = updated_content.replace(old, new)
        if count != 1:
            print(f"  WARNING: Replaced '{old}' {count} times (expected 1)")

    with open(md_cmake, 'w') as f:
        f.write(updated_content)

    print("✓ CMakeLists.txt updated!")
else:
    print("\n✓ All Source/* files exist in GeneralsMD!")

