#!/usr/bin/env python3
"""
Fix remaining D3DXMATRIX → Matrix4x4 conversions in W3DWater.cpp
Applies fighter19's GLM conversion pattern with proper casts
"""

import sys
import re

def fix_w3dwater_matrices(filepath):
    """Apply Matrix4x4 fixes to W3DWater.cpp"""
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    changes = []
    
    # Fix 1: Line 242 area - sparkles texture section (unique: waterNoiseTexture)
    # Look for the pattern with waterNoiseTexture before it (unique identifier)
    pattern1 = (
        r'(m_waterNoiseTexture->Peek_D3D_Texture\(\);'
        r'\s+DX8Wrapper::Set_DX8_Texture_Stage_State\(1,\s+D3DTSS_ADDRESSU, D3DTADDRESS_WRAP\);'
        r'\s+DX8Wrapper::Set_DX8_Texture_Stage_State\(1,\s+D3DTSS_ADDRESSV, D3DTADDRESS_WRAP\);'
        r'\s+DX8Wrapper::Set_DX8_Texture_Stage_State\(2,\s+D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION\);'
        r'\s+// Two output coordinates are used\.'
        r'\s+DX8Wrapper::Set_DX8_Texture_Stage_State\(2,\s+D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2\);'
        r'\s+DX8Wrapper::Set_DX8_Texture_Stage_State\(2,\s+D3DTSS_ADDRESSU, D3DTADDRESS_WRAP\);'
        r'\s+DX8Wrapper::Set_DX8_Texture_Stage_State\(2,\s+D3DTSS_ADDRESSV, D3DTADDRESS_WRAP\);)'
        r'\s+'
        r'D3DXMATRIX curView;'
        r'\s+DX8Wrapper::_Get_DX8_Transform\(D3DTS_VIEW, curView\);'
        r'\s+D3DXMATRIX inv;'
        r'\s+float det;'
        r'\s+D3DXMatrixInverse\(&inv, &det, &curView\);'
        r'\s+D3DXMATRIX scale;'
        r'\s+D3DXMatrixScaling\(&scale, NOISE_REPEAT_FACTOR, NOISE_REPEAT_FACTOR,1\);'
        r'\s+D3DXMATRIX destMatrix = inv \* scale;'
        r'\s+D3DXMatrixTranslation\(&scale, m_riverVOrigin, m_riverVOrigin,0\);'
        r'\s+destMatrix = destMatrix\*scale;'
        r'\s+DX8Wrapper::_Set_DX8_Transform\(D3DTS_TEXTURE2, destMatrix\);'
    )
    
    replacement1 = (
        r'\1'
        '\n'
        '\t\t// TheSuperHackers @bugfix BenderAI 13/02/2026 Matrix4x4 for GLM compatibility (fighter19 pattern)\n'
        '\t\tMatrix4x4 curView;\n'
        '\t\tDX8Wrapper::_Get_DX8_Transform(D3DTS_VIEW, curView);\n'
        '\t\tD3DXMATRIX inv;\n'
        '\t\tfloat det;\n'
        '\t\tD3DXMatrixInverse(&inv, &det, (D3DXMATRIX*)&curView);\n'
        '\t\tD3DXMATRIX scale;\n'
        '\t\tD3DXMatrixScaling(&scale, NOISE_REPEAT_FACTOR, NOISE_REPEAT_FACTOR,1);\n'
        '\t\tMatrix4x4 destMatrix;\n'
        '\t\t*((D3DXMATRIX*)&destMatrix) = inv * scale;\n'
        '\t\tD3DXMatrixTranslation(&scale, m_riverVOrigin, m_riverVOrigin,0);\n'
        '\t\t*((D3DXMATRIX*)&destMatrix) = *((D3DXMATRIX*)&destMatrix)*scale;\n'
        '\t\tDX8Wrapper::_Set_DX8_Transform(D3DTS_TEXTURE2, destMatrix);'
    )
    
    if re.search(pattern1, content, re.DOTALL):
        content = re.sub(pattern1, replacement1, content, count=1, flags=re.DOTALL)
        changes.append("✅ Fixed line 242 area (sparkles texture)")
    else:
        changes.append("⚠️  Pattern 1 (line 242) not found or already fixed")
    
    # Fix 2: Line 1788 - matProj, matView, matWW3D declaration
    pattern2 = r'D3DXMATRIX matProj, matView, matWW3D;'
    replacement2 = '// TheSuperHackers @bugfix BenderAI 13/02/2026 Matrix4x4 for GLM compatibility (fighter19 pattern)\nMatrix4x4 matProj, matView, matWW3D;'
    
    if pattern2 in content:
        content = content.replace(pattern2, replacement2, 1)
        changes.append("✅ Fixed line 1788 (matProj, matView, matWW3D)")
    else:
        changes.append("⚠️  Pattern 2 (line 1788) not found or already fixed")
    
    # Fix 3: Line 3006 area - second instance (unique: riverWaterTimeOfDay)
    # This is similar to pattern1 but in a different context
    # Use negative lookahead to avoid the sparkles section
    pattern3 = (
        r'(if \(m_riverWaterTimeOfDay != W3DWater::TIMEOFDAY_DAY\).*?'
        r'DX8Wrapper::Set_DX8_Texture_Stage_State\(2,\s+D3DTSS_ADDRESSV, D3DTADDRESS_WRAP\);)'
        r'\s+'
        r'D3DXMATRIX curView;'
        r'\s+DX8Wrapper::_Get_DX8_Transform\(D3DTS_VIEW, curView\);'
        r'\s+D3DXMATRIX inv;'
        r'\s+float det;'
        r'\s+D3DXMatrixInverse\(&inv, &det, &curView\);'
        r'\s+D3DXMATRIX scale;'
        r'\s+D3DXMatrixScaling\(&scale, NOISE_REPEAT_FACTOR, NOISE_REPEAT_FACTOR,1\);'
        r'\s+D3DXMATRIX destMatrix = inv \* scale;'
        r'\s+D3DXMatrixTranslation\(&scale, m_riverVOrigin, m_riverVOrigin,0\);'
        r'\s+destMatrix = destMatrix\*scale;'
        r'\s+DX8Wrapper::_Set_DX8_Transform\(D3DTS_TEXTURE2, destMatrix\);'
    )
    
    replacement3 = (
        r'\1'
        '\n'
        '\t\t// TheSuperHackers @bugfix BenderAI 13/02/2026 Matrix4x4 for GLM compatibility (fighter19 pattern)\n'
        '\t\tMatrix4x4 curView;\n'
        '\t\tDX8Wrapper::_Get_DX8_Transform(D3DTS_VIEW, curView);\n'
        '\t\tD3DXMATRIX inv;\n'
        '\t\tfloat det;\n'
        '\t\tD3DXMatrixInverse(&inv, &det, (D3DXMATRIX*)&curView);\n'
        '\t\tD3DXMATRIX scale;\n'
        '\t\tD3DXMatrixScaling(&scale, NOISE_REPEAT_FACTOR, NOISE_REPEAT_FACTOR,1);\n'
        '\t\tMatrix4x4 destMatrix;\n'
        '\t\t*((D3DXMATRIX*)&destMatrix) = inv * scale;\n'
        '\t\tD3DXMatrixTranslation(&scale, m_riverVOrigin, m_riverVOrigin,0);\n'
        '\t\t*((D3DXMATRIX*)&destMatrix) = *((D3DXMATRIX*)&destMatrix)*scale;\n'
        '\t\tDX8Wrapper::_Set_DX8_Transform(D3DTS_TEXTURE2, destMatrix);'
    )
    
    if re.search(pattern3, content, re.DOTALL):
        content = re.sub(pattern3, replacement3, content, count=1, flags=re.DOTALL)
        changes.append("✅ Fixed line 3006 area (river section)")
    else:
        changes.append("⚠️  Pattern 3 (line 3006) not found or already fixed")
    
    # Write changes
    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print("\n🤖 Bender's Matrix Conversion Report:")
        print("=" * 60)
        for change in changes:
            print(f"  {change}")
        print("=" * 60)
        print(f"\n✅ Updated {filepath}")
        return 0
    else:
        print("\n⚠️  No changes made (all patterns already fixed or not found)")
        for change in changes:
            print(f"  {change}")
        return 1

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 fix_w3dwater_matrices.py <W3DWater.cpp>")
        sys.exit(1)
    
    sys.exit(fix_w3dwater_matrices(sys.argv[1]))
