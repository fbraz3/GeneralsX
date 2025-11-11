#!/bin/bash

# Script to reorganize phase documentation
# Shifts all phases up by 1 to make room for DirectX compatibility layer

DOCS_DIR="/Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode/docs"

# Phase reorganization mapping (old -> new)
# Phase 01-40 -> Phase 02-41
# Phase 00, 00.5 stay the same

echo "=== GeneralsX Phase Reorganization ==="
echo "Inserting new Phase 01: DirectX 8 Compatibility Layer"
echo "Shifting Phase 01-40 to Phase 02-41"
echo ""

# First, rename in reverse order to avoid conflicts
for i in {40..1}; do
    old_phase="PHASE$(printf "%02d" $i)"
    new_phase="PHASE$(printf "%02d" $((i+1)))"
    
    old_path="$DOCS_DIR/$old_phase"
    new_path="$DOCS_DIR/$new_phase"
    
    if [ -d "$old_path" ]; then
        echo "Moving: $old_phase -> $new_phase"
        mv "$old_path" "$new_path"
    fi
done

echo ""
echo "✅ Phase reorganization complete!"
echo "Next step: Create PHASE01 directory for DirectX 8 Compatibility Layer"
