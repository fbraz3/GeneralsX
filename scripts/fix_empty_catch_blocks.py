#!/usr/bin/env python3
"""
Phase 48 Empty Catch Block Fixer
Replaces } catch(...) {} with proper logging for all tool files
"""

import re
import sys
from pathlib import Path

# Files to fix
files_to_fix = [
    "GeneralsMD/Code/Tools/ParticleEditor/ParticleEditor.cpp",
    "Generals/Code/Tools/ParticleEditor/ParticleEditor.cpp",
    "GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilder.cpp",
    "GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilderDoc.cpp",
    "Generals/Code/Tools/WorldBuilder/src/WorldBuilderDoc.cpp",
    "GeneralsMD/Code/Tools/WorldBuilder/src/OpenMap.cpp",
    "GeneralsMD/Code/Tools/WorldBuilder/src/SaveMap.cpp",
]

def extract_function_name(lines, line_num):
    """Extract function name from lines before the catch block"""
    for i in range(line_num - 1, std::max(0, line_num - 20), -1):
        line = lines[i]
        # Look for function definition patterns
        if 'void ' in line or 'bool ' in line or 'int ' in line or 'char ' in line or '__declspec' in line:
            # Extract function name
            match = re.search(r'(\w+)\s*\(', line)
            if match:
                return match.group(1)
            # Check next line if this is the declaration
            if i + 1 < len(lines):
                match = re.search(r'(\w+)\s*\(', lines[i + 1])
                if match:
                    return match.group(1)
    return "UnknownFunction"

def fix_file(filepath):
    """Fix empty catch blocks in a file"""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        
        lines = content.split('\n')
        
        # Count replacements
        count = 0
        
        for i, line in enumerate(lines):
            # Look for } catch(...) {} or } catch(...){} patterns
            if re.search(r'}\s*catch\s*\(\s*\.\.\.\s*\)\s*{\s*}', line):
                # Extract function name
                func_name = extract_function_name(lines, i)
                
                # Replace with logging
                lines[i] = re.sub(
                    r'}\s*catch\s*\(\s*\.\.\.\s*\)\s*{\s*}',
                    f'}} catch (const std::exception& e) {{\n\t\tDEBUG_LOG(("{func_name} - Exception caught: %s", e.what()));\n\t}} catch (...) {{\n\t\tDEBUG_LOG(("{func_name} - Unknown exception caught"));',
                    line
                )
                count += 1
        
        # Write back
        with open(filepath, 'w') as f:
            f.write('\n'.join(lines))
        
        return count
    
    except Exception as e:
        print(f"ERROR processing {filepath}: {e}", file=sys.stderr)
        return 0

def main():
    """Main entry point"""
    base_dir = Path('/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode')
    total_fixed = 0
    
    for filepath_str in files_to_fix:
        filepath = base_dir / filepath_str
        if filepath.exists():
            count = fix_file(filepath)
            if count > 0:
                print(f"Fixed {count} empty catch blocks in {filepath_str}")
                total_fixed += count
        else:
            print(f"WARNING: File not found: {filepath}", file=sys.stderr)
    
    print(f"\nTotal empty catch blocks fixed: {total_fixed}")
    return 0

if __name__ == '__main__':
    sys.exit(main())
