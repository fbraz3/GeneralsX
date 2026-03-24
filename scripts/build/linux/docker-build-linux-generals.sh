#!/usr/bin/env bash
# Build GeneralsX (base game) for Linux using Docker
# Usage: ./scripts/build/linux/docker-build-linux-generals.sh [preset]

set -e

PRESET="${1:-linux64-deploy}"
LOG_FILE="logs/build_generals_${PRESET}_docker.log"
DOCKER_IMAGE="generalsx/linux-builder:latest"
CONTAINER_NAME="generalsx-build-generals-${PRESET}"

# GeneralsX @build BenderAI 24/03/2026 Preserve host file ownership for bind mounts and vcpkg cache.
HOST_UID="$(id -u)"
HOST_GID="$(id -g)"
VCPKG_DIR="${VCPKG_DIR:-${HOME}/.generalsx/vcpkg}"

echo "🐳 Building GeneralsX (Linux, preset: ${PRESET})..."
mkdir -p logs
mkdir -p "$VCPKG_DIR"

if [[ ! -w "$VCPKG_DIR" ]]; then
    echo "ERROR: vcpkg directory is not writable: $VCPKG_DIR" >&2
    echo "Fix ownership or set VCPKG_DIR to a writable path." >&2
    exit 1
fi

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
    ./scripts/env/docker/docker-build-images.sh linux
fi

docker run --rm \
    --name "$CONTAINER_NAME" \
    --platform linux/amd64 \
    --user "${HOST_UID}:${HOST_GID}" \
    -e HOME=/tmp/generalsx-home \
    -e XDG_CACHE_HOME=/tmp/generalsx-cache \
    -v "$PWD:/work" \
    -v "$VCPKG_DIR:/opt/vcpkg" \
    -w /work \
    "$DOCKER_IMAGE" \
    bash -c "
        set -e
        mkdir -p \"\$HOME\" \"\$XDG_CACHE_HOME\"

        PROC=1
        if command -v nproc &> /dev/null; then
            PROC=\$(( (\$(nproc) + 1) / 2 ))
        fi
        echo \"🛠  Using \$PROC parallel jobs for building...\"
        
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
        
        echo '⚙️  Configuring CMake (if needed)...'
        cmake --preset ${PRESET}
        
        echo '🔨 Building GeneralsX...'
        cmake --build build/${PRESET} --target g_generals -j\$PROC
        
        echo '✅ Build complete!'
        ls -lh build/${PRESET}/Generals/GeneralsX || echo '⚠️  Binary not found'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Build complete. Log: $LOG_FILE"
