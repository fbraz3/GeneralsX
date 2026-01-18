#!/usr/bin/env python3
"""Remove VC6 presets from CMakePresets.json during Phase 10 modernization"""

import json
import sys
from pathlib import Path

def modernize_presets(preset_file):
    """Remove all VC6-specific presets from CMakePresets.json"""
    
    with open(preset_file, "r") as f:
        presets = json.load(f)
    
    def is_vc6_preset(name):
        return name.startswith("vc6")
    
    # Count removed presets for reporting
    config_before = len(presets.get("configurePresets", []))
    build_before = len(presets.get("buildPresets", []))
    workflow_before = len(presets.get("workflowPresets", []))
    
    # Filter configure presets
    presets["configurePresets"] = [p for p in presets["configurePresets"] if not is_vc6_preset(p["name"])]
    
    # Filter build presets
    presets["buildPresets"] = [p for p in presets["buildPresets"] if not is_vc6_preset(p["name"])]
    
    # Filter workflow presets
    presets["workflowPresets"] = [p for p in presets["workflowPresets"] if not is_vc6_preset(p["name"])]
    
    # Remove RTS_BUILD_OPTION_VC6_FULL_DEBUG from all cache variables
    for preset in presets.get("configurePresets", []):
        if "cacheVariables" in preset:
            preset["cacheVariables"].pop("RTS_BUILD_OPTION_VC6_FULL_DEBUG", None)
    
    # Write back with pretty formatting
    with open(preset_file, "w") as f:
        json.dump(presets, f, indent=4)
        f.write("\n")
    
    config_after = len(presets.get("configurePresets", []))
    build_after = len(presets.get("buildPresets", []))
    workflow_after = len(presets.get("workflowPresets", []))
    
    print(f"CMakePresets.json modernization complete:")
    print(f"  Configure presets: {config_before} -> {config_after} (removed {config_before - config_after})")
    print(f"  Build presets: {build_before} -> {build_after} (removed {build_before - build_after})")
    print(f"  Workflow presets: {workflow_before} -> {workflow_after} (removed {workflow_before - workflow_after})")
    print(f"  Variable RTS_BUILD_OPTION_VC6_FULL_DEBUG: removed")
    
    return True

if __name__ == "__main__":
    preset_file = Path(__file__).parent.parent / "CMakePresets.json"
    if not preset_file.exists():
        print(f"Error: {preset_file} not found")
        sys.exit(1)
    
    try:
        modernize_presets(preset_file)
        print(f"\nSuccessfully modernized {preset_file}")
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
