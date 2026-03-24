#!/usr/bin/env bash
# Build GeneralsXZH for Windows using MinGW cross-compiler in Docker
# Usage: ./scripts/build/linux/docker-build-mingw-zh.sh [preset]

set -e

PRESET="${1:-mingw-w64-i686}"
LOG_FILE="logs/build_zh_${PRESET}_docker.log"
DOCKER_IMAGE="generalsx/mingw-builder:latest"
CONTAINER_NAME="generalsx-build-mingw-zh-${PRESET}"

# GeneralsX @build BenderAI 24/03/2026 Preserve host file ownership for bind mounts created by cross-builds.
HOST_UID="$(id -u)"
HOST_GID="$(id -g)"

echo "🐳 Building GeneralsXZH (Windows/MinGW, preset: ${PRESET})..."
mkdir -p logs

# Check if container is already running
if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "⚠️  Container '${CONTAINER_NAME}' is already running!"
    echo "Wait for the current build to finish or stop it with:"
    echo "    docker stop ${CONTAINER_NAME}"
    exit 1
fi

# Check if Docker image exists, build if not
if ! docker image inspect "$DOCKER_IMAGE" &> /dev/null; then
    echo "⚠️  Docker image not found: $DOCKER_IMAGE"
    echo "📦 Building image (this will take a few minutes)..."
    # GeneralsX @bugfix BenderAI 14/03/2026 Follow scripts/env/docker relocation for builder image bootstrap.
    ./scripts/env/docker/docker-build-images.sh mingw
fi

docker run --rm \
    --name "$CONTAINER_NAME" \
    --user "${HOST_UID}:${HOST_GID}" \
    -e HOME=/tmp/generalsx-home \
    -e XDG_CACHE_HOME=/tmp/generalsx-cache \
    -v "$PWD:/work" \
    -w /work \
    "$DOCKER_IMAGE" \
    bash -c "
        set -e
        mkdir -p \"\$HOME\" \"\$XDG_CACHE_HOME\"
        
        echo '⚙️  Configuring CMake (MinGW cross-compile)...'
        cmake --preset ${PRESET}
        
        echo '🔨 Building GeneralsXZH (Windows .exe)...'
        cmake --build build/${PRESET} --target z_generals
        
        echo '✅ Build complete!'
        ls -lh build/${PRESET}/GeneralsMD/GeneralsXZH.exe || echo '⚠️  Binary not found'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Build complete. Log: $LOG_FILE"
echo "ℹ️  To test: Run in Windows VM or Wine"
