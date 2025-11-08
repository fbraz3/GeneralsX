#!/bin/bash
echo "Waiting for build to complete..."
while jobs %1 >/dev/null 2>&1; do
    size=$(wc -c < logs/generalsxzh_build_final_attempt.log 2>/dev/null || echo 0)
    printf "\r$(date '+%H:%M:%S') Build running... log size: $size bytes"
    sleep 20
done

echo ""
echo "Build completed at $(date)"
echo ""
echo "=== Build Result ==="
if grep -q "Built target z_generals" logs/generalsxzh_build_final_attempt.log; then
    echo "✅ BUILD SUCCESSFUL"
    ls -lh build/macos-arm64-vulkan/GeneralsMD/z_generals 2>/dev/null || echo "Executable not found"
else
    echo "❌ BUILD FAILED"
    echo "=== Errors ==="
    grep -i "error" logs/generalsxzh_build_final_attempt.log | head -5
fi
