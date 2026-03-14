#!/usr/bin/env bash
# Configure Linux build using Docker (Ubuntu 22.04 x86_64)
# Usage: ./scripts/build/linux/docker-configure-linux.sh [preset]

set -e

PRESET="${1:-linux64-deploy}"
LOG_FILE="logs/configure_${PRESET}_docker.log"
DOCKER_IMAGE="generalsx/linux-builder:latest"
CONTAINER_NAME="generalsx-configure-${PRESET}"

echo "🐳 Configuring Linux build (preset: ${PRESET})..."
mkdir -p logs

# Check if container is already running
if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "⚠️  Container '${CONTAINER_NAME}' is already running!"
    echo "Wait for the current configuration to finish or stop it with:"
    echo "    docker stop ${CONTAINER_NAME}"
    exit 1
fi

# Check if Docker image exists, build if not
if ! docker image inspect "$DOCKER_IMAGE" &> /dev/null; then
    echo "⚠️  Docker image not found: $DOCKER_IMAGE"
    echo "📦 Building image (this will take a few minutes)..."
    # GeneralsX @bugfix BenderAI 14/03/2026 Follow scripts/env/docker relocation for builder image bootstrap.
    ./scripts/env/docker/docker-build-images.sh linux
fi

docker run --rm \
    --name "$CONTAINER_NAME" \
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
        
        echo '✅ Configuration complete!'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Configure complete. Log: $LOG_FILE"
