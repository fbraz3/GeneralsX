#!/usr/bin/env bash
# Build GeneralsX (base game) natively on macOS (Apple Silicon / arm64)
# GeneralsX @build BenderAI 15/03/2026 Add macOS base-game build script
#
# Usage:
#   ./scripts/build/macos/build-macos-generals.sh               # configure + build
#   ./scripts/build/macos/build-macos-generals.sh --build-only  # skip configure if already done

set -eo pipefail

PRESET="macos-vulkan"
BUILD_DIR="build/${PRESET}"
LOG_FILE="logs/build_generals_${PRESET}.log"
SKIP_CONFIGURE=0

for arg in "$@"; do
    case "$arg" in
        --build-only) SKIP_CONFIGURE=1 ;;
    esac
done

mkdir -p logs

echo "Building GeneralsX (macOS, preset: ${PRESET})..."

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

# GeneralsX @build Copilot 03/05/2026 Auto-detect VCPKG_ROOT for preset toolchain resolution
resolve_vcpkg_root() {
    local candidate=""
    local -a candidates=()
    local brew_vcpkg_root=""

    if [[ -n "${VCPKG_ROOT:-}" ]]; then
        if [[ -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]]; then
            echo "Using VCPKG_ROOT from environment: ${VCPKG_ROOT}"
            return 0
        fi
        echo "WARNING: VCPKG_ROOT is set but invalid: ${VCPKG_ROOT}"
    fi

    if command -v brew &>/dev/null; then
        brew_vcpkg_root="$(brew --prefix vcpkg 2>/dev/null || true)"
    fi

    candidates+=("${PWD}/vcpkg")
    candidates+=("${HOME}/vcpkg")
    candidates+=("/opt/vcpkg")
    candidates+=("/opt/homebrew/opt/vcpkg")
    candidates+=("/usr/local/opt/vcpkg")
    if [[ -n "${brew_vcpkg_root}" ]]; then
        candidates+=("${brew_vcpkg_root}")
    fi

    for candidate in "${candidates[@]}"; do
        if [[ -f "${candidate}/scripts/buildsystems/vcpkg.cmake" ]]; then
            export VCPKG_ROOT="${candidate}"
            echo "Using detected VCPKG_ROOT: ${VCPKG_ROOT}"
            return 0
        fi
    done

    echo "ERROR: VCPKG_ROOT is not configured and no local vcpkg installation was detected."
    echo "Set VCPKG_ROOT to a valid vcpkg root containing scripts/buildsystems/vcpkg.cmake"
    echo "Example: export VCPKG_ROOT=\"/opt/homebrew/opt/vcpkg\""
    exit 1
}

resolve_vcpkg_root

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

if [[ "$SKIP_CONFIGURE" -eq 0 ]]; then
    echo "Configuring CMake (preset: ${PRESET})..."
    cmake --preset "${PRESET}" 2>&1 | tee "${LOG_FILE}"
fi

JOBS=$(( ($(sysctl -n hw.logicalcpu) + 1) / 2 ))
echo "Building GeneralsX (${JOBS} parallel jobs)..."

cmake --build "${BUILD_DIR}" --target g_generals -j"${JOBS}" 2>&1 | tee -a "${LOG_FILE}"

BINARY="${BUILD_DIR}/Generals/GeneralsX"
if [[ -f "${BINARY}" ]]; then
    SIZE=$(du -sh "${BINARY}" | cut -f1)
    echo ""
    echo "Build complete."
    echo "  Binary : ${BINARY} (${SIZE})"
    echo "  Log    : ${LOG_FILE}"
    echo ""
    echo "Next steps:"
    echo "  1. Run: ./scripts/build/macos/deploy-macos-generals.sh"
else
    echo "ERROR: Binary not found at ${BINARY}"
    exit 1
fi
