#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build/macos"
DEPLOY_DIR="$HOME/GeneralsX/GeneralsMD"
LOGS_DIR="$PROJECT_DIR/logs"

mkdir -p "$DEPLOY_DIR" "$LOGS_DIR"

echo "Building GeneralsXZH..."
cmake --build "$BUILD_DIR" --target GeneralsXZH -j 4 2>&1 | tee "$LOGS_DIR/phase00_5_build_$(date +%Y%m%d_%H%M%S).log"

echo "Deploying GeneralsXZH to $DEPLOY_DIR..."
if [ -f "$BUILD_DIR/GeneralsMD/GeneralsXZH" ]; then
  cp "$BUILD_DIR/GeneralsMD/GeneralsXZH" "$DEPLOY_DIR/"
  chmod +x "$DEPLOY_DIR/GeneralsXZH"
  echo "Deployed to $DEPLOY_DIR/GeneralsXZH"
else
  echo "Error: executable not found at $BUILD_DIR/GeneralsMD/GeneralsXZH" >&2
  exit 1
fi

echo "Build+deploy finished. Log: $LOGS_DIR"
