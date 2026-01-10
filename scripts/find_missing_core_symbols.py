#!/usr/bin/env python3
"""
Find missing Core symbols in GeneralsMD CMakeLists.txt
Maps all _The* symbols defined in Core and identifies which source files
are missing from GeneralsMD compilation.
"""

import os
import re
import subprocess
from pathlib import Path
from collections import defaultdict

# Paths
PROJECT_ROOT = Path("/Users/felipebraz/PhpstormProjects/pessoal/GeneralsX")
CORE_SOURCE = PROJECT_ROOT / "Core/GameEngine/Source"
CORE_DEVICE_SOURCE = PROJECT_ROOT / "Core/GameEngineDevice/Source"
GENERALMD_CMAKELISTS = PROJECT_ROOT / "GeneralsMD/Code/GameEngine/CMakeLists.txt"
GENERALMD_DEVICE_CMAKELISTS = PROJECT_ROOT / "GeneralsMD/Code/GameEngineDevice/CMakeLists.txt"

def find_symbol_definitions(directory):
    """Find all _The* symbol definitions in a directory."""
    symbols = defaultdict(list)

    # Pattern to find "Type *TheSymbol = NULL;" or similar
    pattern = re.compile(r'(\w+\s*\*?)\s+(_The\w+)\s*=')

    for cpp_file in directory.rglob("*.cpp"):
        try:
            with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                for match in pattern.finditer(content):
                    symbol_name = match.group(2)
                    rel_path = cpp_file.relative_to(PROJECT_ROOT)
                    symbols[symbol_name].append(str(rel_path))
        except Exception as e:
            pass

    return symbols

def find_cpp_files_in_core(directory):
    """Find all .cpp files in a Core directory."""
    cpp_files = {}
    for cpp_file in directory.rglob("*.cpp"):
        filename = cpp_file.name
        rel_path = str(cpp_file.relative_to(PROJECT_ROOT))
        if filename not in cpp_files:
            cpp_files[filename] = rel_path
    return cpp_files

def read_generalmd_cmakelists(filepath):
    """Read GeneralsMD CMakeLists and extract all included files."""
    included_files = set()
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            # Find all paths ending with .cpp
            cpp_pattern = re.compile(r'["\']?([^"\']+\.cpp)["\']?')
            for match in cpp_pattern.finditer(content):
                filepath_str = match.group(1)
                # Extract just the filename
                filename = os.path.basename(filepath_str)
                included_files.add(filename)
    except Exception as e:
        print(f"Error reading {filepath}: {e}")

    return included_files

def main():
    print("=" * 80)
    print("MAPPING MISSING CORE SYMBOLS IN GENERALMD")
    print("=" * 80)

    # Find all symbols in Core
    print("\n[1] Scanning Core for _The* symbol definitions...")
    core_symbols = find_symbol_definitions(CORE_SOURCE)
    core_device_symbols = find_symbol_definitions(CORE_DEVICE_SOURCE)

    all_symbols = {**core_symbols, **core_device_symbols}

    print(f"    Found {len(all_symbols)} unique _The* symbols")

    # Find all .cpp files in Core
    print("\n[2] Mapping Core .cpp files...")
    core_cpp_files = find_cpp_files_in_core(CORE_SOURCE)
    core_device_cpp_files = find_cpp_files_in_core(CORE_DEVICE_SOURCE)

    all_cpp_files = {**core_cpp_files, **core_device_cpp_files}
    print(f"    Found {len(all_cpp_files)} .cpp files in Core")

    # Read GeneralsMD CMakeLists
    print("\n[3] Reading GeneralsMD CMakeLists...")
    generalmd_files = read_generalmd_cmakelists(GENERALMD_CMAKELISTS)
    generalmd_device_files = read_generalmd_cmakelists(GENERALMD_DEVICE_CMAKELISTS)

    all_included = generalmd_files | generalmd_device_files
    print(f"    Found {len(all_included)} .cpp files already in GeneralsMD")

    # Find missing files
    print("\n[4] Identifying missing Core files...")
    missing_files = {}

    for symbol, files in sorted(all_symbols.items()):
        for file_path in files:
            filename = os.path.basename(file_path)
            if filename not in all_included:
                if filename not in missing_files:
                    missing_files[filename] = {
                        'path': file_path,
                        'symbols': []
                    }
                missing_files[filename]['symbols'].append(symbol)

    # Print results
    print(f"\n    Found {len(missing_files)} missing files")

    if missing_files:
        print("\n" + "=" * 80)
        print("MISSING CORE FILES (Need to add to GeneralsMD CMakeLists)")
        print("=" * 80)

        for filename in sorted(missing_files.keys()):
            info = missing_files[filename]
            print(f"\n📄 {filename}")
            print(f"   Path: {info['path']}")
            print(f"   Symbols: {', '.join(info['symbols'])}")

            # Generate CMakeLists entry
            if 'GameEngine' in info['path']:
                # Convert to relative path from GeneralsMD/Code/GameEngine
                rel_path = info['path'].replace(
                    'Core/GameEngine/Source/',
                    '../../../Core/GameEngine/Source/'
                )
            elif 'GameEngineDevice' in info['path']:
                # Convert to relative path from GeneralsMD/Code/GameEngineDevice
                rel_path = info['path'].replace(
                    'Core/GameEngineDevice/Source/',
                    '../../../Core/GameEngineDevice/Source/'
                )
            else:
                rel_path = info['path']

            print(f"   CMakeLists entry: {rel_path}")

    # Summary
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print(f"Total _The* symbols in Core: {len(all_symbols)}")
    print(f"Total .cpp files in Core: {len(all_cpp_files)}")
    print(f"Total .cpp files in GeneralsMD: {len(all_included)}")
    print(f"Missing .cpp files: {len(missing_files)}")

    # Generate CMakeLists additions
    if missing_files:
        print("\n" + "=" * 80)
        print("SUGGESTED CMakeLists ADDITIONS")
        print("=" * 80)

        gameengine_adds = []
        gamedevice_adds = []

        for filename in sorted(missing_files.keys()):
            info = missing_files[filename]
            if 'GameEngine' in info['path'] and 'GameEngineDevice' not in info['path']:
                rel_path = info['path'].replace(
                    'Core/GameEngine/Source/',
                    '../../../Core/GameEngine/Source/'
                )
                gameengine_adds.append(f"    {rel_path}")
            elif 'GameEngineDevice' in info['path']:
                rel_path = info['path'].replace(
                    'Core/GameEngineDevice/Source/',
                    '../../../Core/GameEngineDevice/Source/'
                )
                gamedevice_adds.append(f"    {rel_path}")

        if gameengine_adds:
            print("\nFor GeneralsMD/Code/GameEngine/CMakeLists.txt:")
            for add in sorted(gameengine_adds):
                print(add)

        if gamedevice_adds:
            print("\nFor GeneralsMD/Code/GameEngineDevice/CMakeLists.txt:")
            for add in sorted(gamedevice_adds):
                print(add)

    print("\n✅ Analysis complete!")
    return missing_files

if __name__ == "__main__":
    missing = main()
    exit(0 if not missing else 0)  # Always exit 0
