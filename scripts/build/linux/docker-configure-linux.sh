#!/usr/bin/env bash
# Configure Linux build using Docker (Ubuntu 22.04 x86_64)
# Usage: ./scripts/build/linux/docker-configure-linux.sh [preset]

set -euo pipefail

PRESET="${1:-linux64-deploy}"
LOG_FILE="logs/configure_${PRESET}_docker.log"
DOCKER_IMAGE="generalsx/linux-builder:latest"
CONTAINER_NAME="generalsx-configure-${PRESET}"

# GeneralsX @build BenderAI 24/03/2026 Preserve host file ownership for bind mounts and vcpkg cache.
HOST_UID="$(id -u)"
HOST_GID="$(id -g)"
VCPKG_DIR="${VCPKG_DIR:-${HOME}/.generalsx/vcpkg}"

echo "🐳 Configuring Linux build (preset: ${PRESET})..."
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

        # GeneralsX @bugfix Copilot 23/04/2026 Drop stale host-generated CMake cache when running inside /work container mount.
        CACHE_FILE='build/${PRESET}/CMakeCache.txt'
        if [ -f \$CACHE_FILE ]; then
            CACHE_HOME_DIR=\$(sed -n 's#^CMAKE_HOME_DIRECTORY:INTERNAL=##p' \$CACHE_FILE | head -n1)
            CACHE_BUILD_DIR=\$(sed -n 's#^CMAKE_CACHEFILE_DIR:INTERNAL=##p' \$CACHE_FILE | head -n1)
            if [ \$CACHE_HOME_DIR != '/work' ] || [ \$CACHE_BUILD_DIR != '/work/build/${PRESET}' ]; then
                echo '🧹 Removing incompatible CMake cache generated outside Docker...'
                rm -rf 'build/${PRESET}'
            fi
        fi
        
        echo '⚙️  Configuring CMake with vcpkg...'
        cmake --preset ${PRESET}
        
        echo '✅ Configuration complete!'
    " 2>&1 | tee "$LOG_FILE"

echo "✅ Configure complete. Log: $LOG_FILE"
