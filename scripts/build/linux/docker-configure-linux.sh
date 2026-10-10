#!/usr/bin/env bash
# Configure Linux build using Docker (Ubuntu 22.04 x86_64)
# Usage: ./scripts/build/linux/docker-configure-linux.sh [preset]

set -euo pipefail

PRESET="${1:-linux64-deploy}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
cd "$PROJECT_ROOT"
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

# GeneralsX @bugfix Mr. Meeseeks 10/10/2026 Mount external realpath if build is a symlink outside PROJECT_ROOT
BUILD_REAL="$(realpath "$PROJECT_ROOT/build" 2>/dev/null || true)"
BUILD_MOUNT=()
if [[ -n "$BUILD_REAL" && "$BUILD_REAL" != "$PROJECT_ROOT/build" && -d "$BUILD_REAL" ]]; then
    BUILD_MOUNT=(-v "$BUILD_REAL:$BUILD_REAL:z")
fi

docker run --rm \
    --name "$CONTAINER_NAME" \
    --platform linux/amd64 \
    --user "${HOST_UID}:${HOST_GID}" \
    -e HOME=/tmp/generalsx-home \
    -e XDG_CACHE_HOME=/tmp/generalsx-cache \
    -v "$PROJECT_ROOT:/work:z" \
    "${BUILD_MOUNT[@]}" \
    -v "$VCPKG_DIR:/opt/vcpkg:z" \
    -w /work \
    "$DOCKER_IMAGE" \
    bash -c "
        set -e
        mkdir -p \"\$HOME\" \"\$XDG_CACHE_HOME\"
        
        # GeneralsX @build Gabriel Petry 30/09/2026 Reuse or bootstrap the host-cached vcpkg executable.
        if [ ! -x /opt/vcpkg/vcpkg ]; then
            echo '📦 Bootstrapping vcpkg (first time, will be cached in Docker volume)...'
            if [ -f /opt/vcpkg/bootstrap-vcpkg.sh ]; then
                /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics
            else
                git clone https://github.com/microsoft/vcpkg.git /tmp/vcpkg-bootstrap
                cp -a /tmp/vcpkg-bootstrap/. /opt/vcpkg/
                /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics
            fi
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
