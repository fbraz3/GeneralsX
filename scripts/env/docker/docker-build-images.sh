#!/usr/bin/env bash
# Build Docker images for GeneralsX development
# Usage: ./scripts/env/docker/docker-build-images.sh [linux|mingw|all]

set -e

BUILD_TARGET="${1:-all}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
DOCKERFILES_DIR="$PROJECT_ROOT/resources/dockerbuild"

# Image names
LINUX_IMAGE="generalsx/linux-builder:latest"
MINGW_IMAGE="generalsx/mingw-builder:latest"

echo "🐳 GeneralsX Docker Image Builder"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

build_linux_image() {
    echo ""
    echo "📦 Building Linux native builder image..."
    echo "   Image: $LINUX_IMAGE"
    echo "   Dockerfile: $DOCKERFILES_DIR/Dockerfile.linux"
    
    docker build \
        --platform linux/amd64 \
        -t "$LINUX_IMAGE" \
        -f "$DOCKERFILES_DIR/Dockerfile.linux" \
        "$DOCKERFILES_DIR"
    
    echo "✅ Linux builder image ready: $LINUX_IMAGE"
}

build_mingw_image() {
    echo ""
    echo "📦 Building MinGW cross-compiler image..."
    echo "   Image: $MINGW_IMAGE"
    echo "   Dockerfile: $DOCKERFILES_DIR/Dockerfile.mingw"
    
    docker build \
        --platform linux/amd64 \
        -t "$MINGW_IMAGE" \
        -f "$DOCKERFILES_DIR/Dockerfile.mingw" \
        "$DOCKERFILES_DIR"
    
    echo "✅ MinGW builder image ready: $MINGW_IMAGE"
}

show_summary() {
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "📋 Available Docker images:"
    docker images | grep -E "generalsx|REPOSITORY"
    echo ""
    echo "💡 Usage:"
    echo "   Linux builds:  Use image '$LINUX_IMAGE'"
    echo "   MinGW builds:  Use image '$MINGW_IMAGE'"
    echo ""
    echo "🔧 Rebuild images:"
    echo "   ./scripts/env/docker/docker-build-images.sh all"
}

case "$BUILD_TARGET" in
    linux)
        build_linux_image
        ;;
    mingw)
        build_mingw_image
        ;;
    all)
        build_linux_image
        build_mingw_image
        ;;
    *)
        echo "❌ Invalid target: $BUILD_TARGET"
        echo "Usage: $0 [linux|mingw|all]"
        exit 1
        ;;
esac

show_summary
