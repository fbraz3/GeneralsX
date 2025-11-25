#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build/macos"

echo "Cleaning build directory: $BUILD_DIR"
rm -rf "$BUILD_DIR"

echo "Configuring fresh build with preset 'macos'"
cmake --preset macos

echo "Building GeneralsXZH from scratch..."
cmake --build "$BUILD_DIR" --target GeneralsXZH -j 4

echo "Clean build finished."
