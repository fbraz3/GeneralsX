#!/usr/bin/env bash
# Configure Linux build using Docker (Ubuntu 22.04)
# Usage: ./scripts/docker-configure-linux.sh [preset]

set -e

PRESET="${1:-linux64-deploy}"
LOG_FILE="logs/configure_${PRESET}_docker.log"

echo "🐳 Configuring Linux build (preset: ${PRESET})..."
mkdir -p logs

docker run --rm \
    -v "$PWD:/work" \
    -w /work \
    ubuntu:22.04 \
    bash -c "
        set -e
        echo '📦 Installing dependencies...'
        apt-get update -qq
        apt-get install -y -qq build-essential ninja-build git curl
        
        echo '📦 Installing CMake 3.25+ (required for CMakePresets.json v6)...'
        curl -sL https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0-linux-aarch64.tar.gz | tar -xz -C /usr/local --strip-components=1
        
        echo '⚙️  Configuring CMake...'
        cmake --preset ${PRESET}
        
        echo '✅ Configuration complete!'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Configure complete. Log: $LOG_FILE"
