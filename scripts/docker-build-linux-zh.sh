#!/usr/bin/env bash
# Build GeneralsXZH (Zero Hour) for Linux using Docker
# Usage: ./scripts/docker-build-linux-zh.sh [preset]

set -e

PRESET="${1:-linux64-deploy}"
LOG_FILE="logs/build_zh_${PRESET}_docker.log"
DOCKER_IMAGE="generalsx/linux-builder:latest"

echo "🐳 Building GeneralsXZH (Linux, preset: ${PRESET})..."
mkdir -p logs

# Check if Docker image exists, build if not
if ! docker image inspect "$DOCKER_IMAGE" &> /dev/null; then
    echo "⚠️  Docker image not found: $DOCKER_IMAGE"
    echo "📦 Building image (this will take a few minutes)..."
    ./scripts/docker-build-images.sh linux
fi

docker run --rm \
    --platform linux/amd64 \
    -v "$PWD:/work" \
    -v "generalsx-vcpkg:/opt/vcpkg" \
    -w /work \
    "$DOCKER_IMAGE" \
    bash -c "
        set -e
        
        # Bootstrap vcpkg in Docker volume if not exists
        if [ ! -f /opt/vcpkg/vcpkg ]; then
            echo '📦 Bootstrapping vcpkg (first time, will be cached in Docker volume)...'
            # Clean up if directory exists but is incomplete
            if [ -d /opt/vcpkg ]; then
                echo '🧹 Cleaning incomplete vcpkg directory...'
                rm -rf /opt/vcpkg/* /opt/vcpkg/.git 2>/dev/null || true
            fi
            git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg
            /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics
        fi
        
        export VCPKG_ROOT=/opt/vcpkg
        
        echo '⚙️  Configuring CMake with vcpkg...'
        cmake --preset ${PRESET}
        
        echo '🔨 Building GeneralsXZH...'
        cmake --build build/${PRESET} --target z_generals
        
        echo '✅ Build complete!'
        ls -lh build/${PRESET}/GeneralsMD/GeneralsXZH || echo '⚠️  Binary not found'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Build complete. Log: $LOG_FILE"
