#!/usr/bin/env python3
"""
Phase 48 Empty Catch Block Refiner
Replaces generic function names with actual function names
"""

import re
from pathlib import Path

def fix_particle_editor(filepath):
    """Fix ParticleEditor.cpp with proper function names"""
    
    function_map = {
        'CreateParticleSystemDialog': 'CreateParticleSystemDialog',
        'DestroyParticleSystemDialog': 'DestroyParticleSystemDialog',
        'RemoveAllParticleSystems': 'RemoveAllParticleSystems',
        'AppendParticleSystem': 'AppendParticleSystem',
        'RemoveAllThingTemplates': 'RemoveAllThingTemplates',
        'AppendThingTemplate': 'AppendThingTemplate',
        'GetSelectedParticleSystemName': 'GetSelectedParticleSystemName',
        'UpdateCurrentParticleCap': 'UpdateCurrentParticleCap',
        'UpdateCurrentNumParticles': 'UpdateCurrentNumParticles',
        'GetSelectedParticleAsciiStringParm': 'GetSelectedParticleAsciiStringParm',
        'UpdateParticleAsciiStringParm': 'UpdateParticleAsciiStringParm',
        'UpdateCurrentParticleSystem': 'UpdateCurrentParticleSystem',
        'UpdateSystemUseParameters': 'UpdateSystemUseParameters',
        'ShouldWriteINI': 'ShouldWriteINI',
        'HasRequestedReload': 'HasRequestedReload',
    }
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Find DEBUG_LOG lines with "__declspec" and replace with function names
    for i in range(len(lines)):
        line = lines[i]
        # Look for DEBUG_LOG with __declspec function name
        if '__declspec' in line and 'DEBUG_LOG' in line:
            # Find the proper function name by looking backwards
            for j in range(i - 1, max(0, i - 30), -1):
                prev_line = lines[j]
                for func_name, replacement in function_map.items():
                    if func_name in prev_line and '__declspec(dllexport)' in prev_line:
                        # Replace __declspec with the actual function name
                        lines[i] = line.replace('__declspec', replacement)
                        break
                if '__declspec' not in lines[i]:
                    break
    
    with open(filepath, 'w') as f:
        f.writelines(lines)
    
    print(f"Fixed function names in {filepath.name}")

def main():
    """Main entry point"""
    base_dir = Path('/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode')
    
    # Fix ParticleEditor files
    for particle_file in [
        'GeneralsMD/Code/Tools/ParticleEditor/ParticleEditor.cpp',
        'Generals/Code/Tools/ParticleEditor/ParticleEditor.cpp',
    ]:
        filepath = base_dir / particle_file
        if filepath.exists():
            fix_particle_editor(filepath)

if __name__ == '__main__':
    main()
