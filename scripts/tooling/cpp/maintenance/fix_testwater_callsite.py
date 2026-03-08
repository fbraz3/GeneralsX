#!/usr/bin/env python3
"""Fix W3DWaterTracks.cpp TestWaterUpdate call site"""
import sys

filepath = sys.argv[1]
with open(filepath, 'r') as f:
    lines = f.readlines()

# Find line 863 (0-indexed as 862)
if len(lines) > 862 and 'TestWaterUpdate' in lines[862]:
    # Replace lines 862-863
    lines[862] = '\t// GeneralsX @bugfix BenderAI 13/02/2026 Windows-only debug function\n'
    lines.insert(863, '#ifdef _WIN32\n')
    lines.insert(864, '\tif (TheGlobalData->m_usingWaterTrackEditor)\n')
    lines.insert(865, '\t\tTestWaterUpdate();\n')
    lines.insert(866, '#endif\n')
    
    with open(filepath, 'w') as f:
        f.writelines(lines)
    print("✅ Fixed TestWaterUpdate call site")
else:
    print("⚠️  Line 863 doesn't match expected pattern")
    sys.exit(1)
