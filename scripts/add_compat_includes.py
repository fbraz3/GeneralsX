#!/usr/bin/env python3
"""
Add #include <Utility/compat.h> to all files that use GET_PATH_SEPARATOR()
This script dynamically finds all files using the macro and adds the include.
"""

import os
import re
import sys
import subprocess

def find_files_using_macro(workspace_root):
    """
    Dynamically find all files that use GET_PATH_SEPARATOR() macro.
    """
    files = set()
    search_dirs = ['Core', 'Generals', 'GeneralsMD']
    
    for search_dir in search_dirs:
        dir_path = os.path.join(workspace_root, search_dir)
        if not os.path.isdir(dir_path):
            continue
        
        try:
            # Use grep to find files containing GET_PATH_SEPARATOR()
            result = subprocess.run(
                ['grep', '-r', 'GET_PATH_SEPARATOR()', dir_path,
                 '--include=*.cpp', '--include=*.h', '--include=*.hpp',
                 '-l'],
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.stdout:
                for filepath in result.stdout.strip().split('\n'):
                    if filepath:  # Skip empty lines
                        files.add(filepath)
        except Exception as e:
            print(f"⚠️  Error searching {search_dir}: {e}")
    
    return sorted(files)

def add_compat_include(filepath):
    """Add #include <Utility/compat.h> to a file if it's not already present."""
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"⚠️  Error reading {filepath}: {e}")
        return False
    
    # Check if include already exists
    if re.search(r'#include\s*[<"]Utility/compat\.h[>"]', content):
        # Get relative path for display
        rel_path = filepath.replace(os.path.dirname(os.path.dirname(filepath)) + '/', '')
        print(f"⏭️  {rel_path} (already has include)")
        return None
    
    # Find the position after the last #include statement
    lines = content.split('\n')
    last_include_idx = -1
    
    for i, line in enumerate(lines):
        if re.match(r'\s*#include\s', line):
            last_include_idx = i
    
    if last_include_idx == -1:
        rel_path = filepath.replace(os.path.dirname(os.path.dirname(filepath)) + '/', '')
        print(f"⚠️  {rel_path} (no includes found)")
        return False
    
    # Insert the new include after the last include
    lines.insert(last_include_idx + 1, '#include <Utility/compat.h>')
    
    # Write back
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        rel_path = filepath.replace(os.path.dirname(os.path.dirname(filepath)) + '/', '')
        print(f"✅ {rel_path} (include added)")
        return True
    except Exception as e:
        rel_path = filepath.replace(os.path.dirname(os.path.dirname(filepath)) + '/', '')
        print(f"❌ Error writing {rel_path}: {e}")
        return False

def main():
    workspace_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    print("Dynamically finding files that use GET_PATH_SEPARATOR()...")
    files_to_fix = find_files_using_macro(workspace_root)
    
    if not files_to_fix:
        print("No files found using GET_PATH_SEPARATOR()")
        return 0
    
    print(f"Found {len(files_to_fix)} files\n")
    print("Adding #include <Utility/compat.h> to files that use GET_PATH_SEPARATOR()")
    print("=" * 75)
    print()
    
    updated_count = 0
    skipped_count = 0
    failed_count = 0
    
    for file in files_to_fix:
        if not os.path.isfile(file):
            continue
        
        result = add_compat_include(file)
        if result is True:
            updated_count += 1
        elif result is False:
            failed_count += 1
        else:  # None means skipped
            skipped_count += 1
    
    print()
    print("=" * 75)
    print(f"Summary: Updated: {updated_count} | Skipped: {skipped_count} | Failed: {failed_count}")
    print("=" * 75)
    
    return 0 if failed_count == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
