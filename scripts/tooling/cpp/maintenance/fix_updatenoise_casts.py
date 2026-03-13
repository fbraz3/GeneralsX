#!/usr/bin/env python3
"""
Fix updateNoise function calls to cast Matrix4x4 to D3DXMATRIX*
Handles both basic calls and calls with 'false' parameter with varying whitespace

Pattern: updateNoise1(&curView, &inv) -> updateNoise1((D3DXMATRIX*)&curView, &inv)
Pattern: updateNoise1(&curView, &inv, false) -> updateNoise1((D3DXMATRIX*)&curView, &inv, false)

Uses negative lookbehind to prevent nested replacements.
"""

import re
import sys

def fix_updatenoise_casts(filepath):
    """Fix updateNoise calls by adding D3DXMATRIX casts to &curView parameter"""
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Pattern 1: Basic updateNoise1 calls without false parameter
    # Matches: updateNoise1(&curView,&inv) with optional spaces
    # Negative lookbehind ensures we don't match already-casted calls
    pattern1 = r'(?<!D3DXMATRIX\*\))updateNoise1\(\s*&curView\s*,\s*&inv\s*\)'
    
    # Pattern 2: Basic updateNoise2 calls without false parameter  
    pattern2 = r'(?<!D3DXMATRIX\*\))updateNoise2\(\s*&curView\s*,\s*&inv\s*\)'
    
    # Pattern 3: updateNoise1 calls WITH false parameter
    # Matches: updateNoise1(&curView, &inv, false) with various whitespace
    pattern3 = r'(?<!D3DXMATRIX\*\))updateNoise1\(\s*&curView\s*,\s*&inv\s*,\s*false\s*\)'
    
    # Pattern 4: updateNoise2 calls WITH false parameter
    pattern4 = r'(?<!D3DXMATRIX\*\))updateNoise2\(\s*&curView\s*,\s*&inv\s*,\s*false\s*\)'
    
    # Counts for reporting
    count1 = 0
    count2 = 0
    count3 = 0
    count4 = 0
    
    # Replace pattern 1 (basic updateNoise1)
    def replace1(match):
        nonlocal count1
        count1 += 1
        # Extract the full match and replace &curView with (D3DXMATRIX*)&curView
        original = match.group(0)
        return original.replace('&curView', '(D3DXMATRIX*)&curView', 1)
    
    # Replace pattern 2 (basic updateNoise2)
    def replace2(match):
        nonlocal count2
        count2 += 1
        original = match.group(0)
        return original.replace('&curView', '(D3DXMATRIX*)&curView', 1)
    
    # Replace pattern 3 (updateNoise1 with false)
    def replace3(match):
        nonlocal count3
        count3 += 1
        original = match.group(0)
        return original.replace('&curView', '(D3DXMATRIX*)&curView', 1)
    
    # Replace pattern 4 (updateNoise2 with false)
    def replace4(match):
        nonlocal count4
        count4 += 1
        original = match.group(0)
        return original.replace('&curView', '(D3DXMATRIX*)&curView', 1)
    
    # Apply all replacements
    new_content = re.sub(pattern1, replace1, content)
    new_content = re.sub(pattern2, replace2, new_content)
    new_content = re.sub(pattern3, replace3, new_content)
    new_content = re.sub(pattern4, replace4, new_content)
    
    # Write back
    with open(filepath, 'w') as f:
        f.write(new_content)
    
    # Report results
    if count1 > 0:
        print(f"Fixed {count1} updateNoise1 calls (basic pattern)")
    if count2 > 0:
        print(f"Fixed {count2} updateNoise2 calls (basic pattern)")
    if count3 > 0:
        print(f"Fixed {count3} updateNoise1 calls (with false parameter)")
    if count4 > 0:
        print(f"Fixed {count4} updateNoise2 calls (with false parameter)")
    
    total = count1 + count2 + count3 + count4
    print(f"Total: {total} calls fixed")
    
    return total

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 fix_updatenoise_casts.py <filepath>")
        sys.exit(1)
    
    filepath = sys.argv[1]
    total_fixed = fix_updatenoise_casts(filepath)
    
    if total_fixed > 0:
        print(f"\nSuccessfully updated {filepath}")
    else:
        print(f"\nNo changes needed in {filepath}")
