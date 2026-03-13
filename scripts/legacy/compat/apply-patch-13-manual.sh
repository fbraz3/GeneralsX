#!/bin/bash
# Manual Patch 13 application script - for emergency use if cmake patches fail
# GeneralsX Session 67 — Terrain Shader Fix

set -e

DXVK_CPP="build/macos-vulkan/_deps/dxvk-src/src/dxso/dxso_compiler.cpp"

if [ ! -f "$DXVK_CPP" ]; then
    echo "ERROR: Cannot find $DXVK_CPP"
    echo "Wait for cmake configure to complete first."
    exit 1
fi

echo "Applying PATCH 13 to dxso_compiler.cpp..."

# Check if already patched
if grep -q "GeneralsX Patch 13" "$DXVK_CPP"; then
    echo "✓ Patch 13 already applied"
    exit 0
fi

# Backup
cp "$DXVK_CPP" "$DXVK_CPP.backup"
echo "Created backup: $DXVK_CPP.backup"

# Part A: Fix emitDclSampler (line ~754)
old_decl='    const bool implicit = m_programInfo.majorVersion() < 2 || m_moduleInfo.options.forceSamplerTypeSpecConstants;'
new_decl='    // GeneralsX Patch 13: MoltenVK/SPIRV-Cross does not declare dummy variables for inactive
    // sampler type variants in the MSL entry-point wrapper. Removing the majorVersion() < 2
    // auto-trigger prevents DXVK from emitting all-type SPIR-V for PS1.x shaders. Only
    // forceSamplerTypeSpecConstants=True (explicit opt-in) enables multi-type mode.
    const bool implicit = m_moduleInfo.options.forceSamplerTypeSpecConstants;'

sed -i.bak1 "s|$old_decl|$new_decl|" "$DXVK_CPP" || echo "Part A: sed pattern might need manual review"

# Part B: Fix emitTextureSample (line ~3015)
old_sample='    if (m_programInfo.majorVersion() >= 2 && !m_moduleInfo.options.forceSamplerTypeSpecConstants) {'
new_sample='    // GeneralsX Patch 13b: Remove majorVersion() < 2 auto-trigger for spec-constant sampler
    // type switching. Same rationale as Patch 13: for PS1.x + forceSamplerTypeSpecConstants=False,
    // emit a direct 2D sample call instead of an OpSwitch over {2D, 3D, Cube} spec constants.
    // SPIRV-Cross generates ps_main() params for every OpSwitch case label even when those
    // samplers were not declared as SPIR-V bindings → "undeclared identifier" in Metal MSL.
    if (!m_moduleInfo.options.forceSamplerTypeSpecConstants) {'

sed -i.bak2 "s|$old_sample|$new_sample|" "$DXVK_CPP" || echo "Part B: sed pattern might need manual review"

# Verify
if grep -q "GeneralsX Patch 13" "$DXVK_CPP"; then
    echo "✓ Patch 13 applied successfully"
    rm -f "$DXVK_CPP.backup" "$DXVK_CPP.bak1" "$DXVK_CPP.bak2"
    exit 0
else
    echo "✗ Patch 13 may not have applied - manual verification needed"
    exit 1
fi
