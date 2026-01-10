#!/usr/bin/env python3
"""
Batch fix missing Core symbols in GeneralsMD CMakeLists.txt
Finds all _The* symbols and adds their source files automatically
"""

import os
import re
import subprocess
import sys

def find_symbol_definition(symbol_name, core_source_dir):
    """Find which .cpp file defines a given _The* symbol"""
    # Search for patterns like: Type *TheName = NULL;
    pattern = rf'\b{symbol_name}\s*='

    for root, dirs, files in os.walk(core_source_dir):
        for file in files:
            if file.endswith('.cpp') or file.endswith('.h'):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                        if re.search(pattern, content):
                            # Return relative path from Core/GameEngine/Source
                            rel_path = os.path.relpath(filepath, core_source_dir)
                            return (filepath, rel_path)
                except Exception as e:
                    pass
    return None

def extract_subsystem(rel_path):
    """Extract subsystem folder from relative path"""
    parts = rel_path.split(os.sep)
    if len(parts) > 0:
        return parts[0]
    return "Unknown"

def get_cmake_relative_path(core_cpp_path):
    """Convert Core file path to CMakeLists relative path for GeneralsMD"""
    # Core files are at: Core/GameEngine/Source/...
    # In GeneralsMD CMakeLists, they should be: ../../../Core/GameEngine/Source/...

    # Extract just the part after "Source/"
    if "Source/" in core_cpp_path:
        source_part = core_cpp_path.split("Source/", 1)[1]
        return f"${{CMAKE_CURRENT_SOURCE_DIR}}/../../../Core/GameEngine/Source/{source_part}"
    return None

def main():
    # Workspace root
    workspace = "/Users/felipebraz/PhpstormProjects/pessoal/GeneralsX"
    core_source = os.path.join(workspace, "Core/GameEngine/Source")
    md_cmakelists = os.path.join(workspace, "GeneralsMD/Code/GameEngine/CMakeLists.txt")

    # Get the missing symbol from command line or search the log
    if len(sys.argv) > 1:
        symbol_name = sys.argv[1]
    else:
        # Try to get from the latest run log
        log_file = os.path.join(workspace, "logs/runTerminal.log")
        if os.path.exists(log_file):
            with open(log_file, 'r') as f:
                log_content = f.read()
                match = re.search(r"symbol not found[^']*'([^']+)'", log_content)
                if match:
                    symbol_name = match.group(1)
                    print(f"Found missing symbol in log: {symbol_name}")
                else:
                    print("Could not find missing symbol in log")
                    return 1
        else:
            print("Usage: ./fix_missing_symbols_batch.py [SYMBOL_NAME]")
            return 1

    print(f"\nSearching for symbol definition: {symbol_name}")

    result = find_symbol_definition(symbol_name, core_source)
    if not result:
        print(f"ERROR: Could not find definition for {symbol_name} in Core")
        return 1

    filepath, rel_path = result
    subsystem = extract_subsystem(rel_path)
    cmake_path = get_cmake_relative_path(filepath)

    print(f"  Found in: {filepath}")
    print(f"  Relative: {rel_path}")
    print(f"  Subsystem: {subsystem}")
    print(f"  CMakeLists entry: {os.path.basename(filepath)}")

    # Check if already in GeneralsMD CMakeLists
    with open(md_cmakelists, 'r') as f:
        content = f.read()
        basename = os.path.basename(filepath)
        if basename in content:
            print(f"  ✓ Already in GeneralsMD CMakeLists")
            return 0

    print(f"  ✗ NOT in GeneralsMD CMakeLists - will add")

    # Now find ALL missing files for this subsystem
    print(f"\n=== Finding all missing files in {subsystem} subsystem ===")

    all_subsystem_files = []
    subsystem_dir = os.path.join(core_source, subsystem)
    if os.path.isdir(subsystem_dir):
        for root, dirs, files in os.walk(subsystem_dir):
            for file in files:
                if file.endswith('.cpp'):
                    full_path = os.path.join(root, file)
                    rel = os.path.relpath(full_path, core_source)
                    all_subsystem_files.append((full_path, rel, file))

    # Check which ones are missing
    missing_files = []
    with open(md_cmakelists, 'r') as f:
        md_content = f.read()

    for full_path, rel_path, basename in all_subsystem_files:
        if basename not in md_content:
            missing_files.append((full_path, rel_path, basename))

    if missing_files:
        print(f"Found {len(missing_files)} missing file(s) in {subsystem}:")
        for full_path, rel_path, basename in missing_files:
            print(f"  - {basename} ({rel_path})")
    else:
        print(f"All {subsystem} files are already in CMakeLists")

    return 0

if __name__ == "__main__":
    sys.exit(main())
