#!/usr/bin/env bash
# Build GeneralsXZH (Zero Hour) natively on macOS (Apple Silicon / arm64)
# GeneralsX @build BenderAI 24/02/2026 - Phase 5 macOS port
#
# Prerequisites (install once):
#   brew install cmake ninja meson
#   Install Vulkan SDK from https://vulkan.lunarg.com/sdk/home#mac
#   (installs to ~/VulkanSDK/<version>/macOS)
#
# Usage:
#   ./scripts/build-macos-zh.sh            # configure + build
#   ./scripts/build-macos-zh.sh --build-only  # skip configure if already done
#
# After building:
#   ./scripts/deploy-macos-zh.sh           # copy to runtime dir
#   ./scripts/run-macos-zh.sh -win         # launch windowed

set -e

PRESET="macos-vulkan"
BUILD_DIR="build/${PRESET}"
LOG_FILE="logs/build_zh_${PRESET}.log"
SKIP_CONFIGURE=0

for arg in "$@"; do
    case "$arg" in
        --build-only) SKIP_CONFIGURE=1 ;;
    esac
done

mkdir -p logs

echo "Building GeneralsXZH (macOS, preset: ${PRESET})..."

# ── Prerequisite checks ──────────────────────────────────────────────────────

check_tool() {
    local tool="$1" hint="$2"
    if ! command -v "$tool" &>/dev/null; then
        echo "ERROR: '$tool' not found. ${hint}"
        exit 1
    fi
}

check_tool cmake "brew install cmake"
check_tool ninja "brew install ninja"
check_tool meson "brew install meson"
check_tool python3 "brew install python3"

# Vulkan SDK check
VULKAN_FOUND=0
for sdk_candidate in "${HOME}/VulkanSDK"/*/macOS; do
    if [[ -f "${sdk_candidate}/lib/libvulkan.dylib" ]]; then
        VULKAN_FOUND=1
        echo "Vulkan SDK found: ${sdk_candidate}"
        break
    fi
done
if [[ "$VULKAN_FOUND" -eq 0 ]]; then
    echo "ERROR: Vulkan SDK not found in ~/VulkanSDK/"
    echo "Install from: https://vulkan.lunarg.com/sdk/home#mac"
    echo "Default install path: ~/VulkanSDK/<version>/macOS"
    exit 1
fi

# ── Configure ────────────────────────────────────────────────────────────────

if [[ "$SKIP_CONFIGURE" -eq 0 ]]; then
    echo "Configuring CMake (preset: ${PRESET})..."
    echo "  NOTE: First run fetches DXVK from git and builds it via Meson."
    echo "  This can take 5-10 minutes. Subsequent builds reuse the cache."
    cmake --preset "${PRESET}" 2>&1 | tee "${LOG_FILE}"
fi

# ── Build ────────────────────────────────────────────────────────────────────

JOBS=$(( ($(sysctl -n hw.logicalcpu) + 1) / 2 ))
echo "Building GeneralsXZH (${JOBS} parallel jobs)..."

cmake --build "${BUILD_DIR}" --target z_generals -j"${JOBS}" 2>&1 | tee -a "${LOG_FILE}"

# ── Result ───────────────────────────────────────────────────────────────────

BINARY="${BUILD_DIR}/GeneralsMD/GeneralsXZH"
if [[ -f "${BINARY}" ]]; then
    SIZE=$(du -sh "${BINARY}" | cut -f1)
    echo ""
    echo "Build complete."
    echo "  Binary : ${BINARY} (${SIZE})"
    echo "  Log    : ${LOG_FILE}"
    echo ""
    echo "Next steps:"
    echo "  1. Copy game files to ~/GeneralsX/GeneralsMD/ (retail installation)"
    echo "  2. Run: ./scripts/deploy-macos-zh.sh"
    echo "  3. Run: ./scripts/run-macos-zh.sh -win"
else
    echo "ERROR: Binary not found at ${BINARY}"
    exit 1
fi
