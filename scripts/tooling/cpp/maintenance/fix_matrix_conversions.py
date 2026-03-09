#!/usr/bin/env python3
"""
Fix all Matrix4x4/D3DXMATRIX conversions in W3DShaderManager.cpp
Applies fighter19's GLM pattern:
1. D3DXMATRIX curView → Matrix4x4 curView
2. D3DXMatrixInverse(&inv, &det, &curView) → D3DXMatrixInverse(&inv, &det, (D3DXMATRIX*)&curView)
3. curView = (expr) → *((D3DXMATRIX*)&curView) = (expr)
"""

import re
import sys

def fix_matrix_conversions(filepath):
    """Apply all Matrix4x4 conversion patterns"""
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Counters
    count_decl = 0
    count_inverse = 0
    count_assign = 0
    
    # Pattern 1: Change declarations
    # D3DXMATRIX curView; → Matrix4x4 curView;
    pattern_decl = r'\bD3DXMATRIX\s+curView\s*;'
    def replace_decl(match):
        nonlocal count_decl
        count_decl += 1
        return 'Matrix4x4 curView;'
    
    new_content = re.sub(pattern_decl, replace_decl, content)
    
    # Pattern 2: Add casts to D3DXMatrixInverse calls
    # D3DXMatrixInverse(&inv, &det, &curView) → D3DXMatrixInverse(&inv, &det, (D3DXMATRIX*)&curView)
    # Use negative lookbehind to avoid double-casting
    pattern_inverse = r'(?<!D3DXMATRIX\*\))D3DXMatrixInverse\(([^,]+),\s*([^,]+),\s*&curView\s*\)'
    def replace_inverse(match):
        nonlocal count_inverse
        count_inverse += 1
        arg1 = match.group(1)
        arg2 = match.group(2)
        return f'D3DXMatrixInverse({arg1}, {arg2}, (D3DXMATRIX*)&curView)'
    
    new_content = re.sub(pattern_inverse, replace_inverse, new_content)
    
    # Pattern 3: Fix curView assignments with matrix multiplication
    # curView = (expr) → *((D3DXMATRIX*)&curView) = (expr)
    # Use negative lookbehind to avoid double-casting
    pattern_assign = r'(?<!\*\(\(D3DXMATRIX\*\)&)curView\s*=\s*\('
    def replace_assign(match):
        nonlocal count_assign
        count_assign += 1
        return '*((D3DXMATRIX*)&curView) = ('
    
    new_content = re.sub(pattern_assign, replace_assign, new_content)
    
    # Write back
    with open(filepath, 'w') as f:
        f.write(new_content)
    
    # Report
    if count_decl > 0:
        print(f"Fixed {count_decl} curView declarations (D3DXMATRIX → Matrix4x4)")
    if count_inverse > 0:
        print(f"Fixed {count_inverse} D3DXMatrixInverse calls (added casts)")
    if count_assign > 0:
        print(f"Fixed {count_assign} curView assignments (added pointer casts)")
    
    total = count_decl + count_inverse + count_assign
    print(f"Total: {total} fixes applied")
    
    return total

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 fix_matrix_conversions.py <filepath>")
        sys.exit(1)
    
    filepath = sys.argv[1]
    total_fixed = fix_matrix_conversions(filepath)
    
    if total_fixed > 0:
        print(f"\nSuccessfully updated {filepath}")
    else:
        print(f"\nNo changes needed in {filepath}")
