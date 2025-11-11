#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build/macos-arm64"
DEPLOY_ZH="$HOME/GeneralsX/GeneralsMD"
DEPLOY_BASE="$HOME/GeneralsX/Generals"

mkdir -p "$DEPLOY_ZH" "$DEPLOY_ZH/logs" "$DEPLOY_BASE" "$DEPLOY_BASE/logs"

echo "Deploying GeneralsXZH..."
if [ -f "$BUILD_DIR/GeneralsMD/GeneralsXZH" ]; then
  cp "$BUILD_DIR/GeneralsMD/GeneralsXZH" "$DEPLOY_ZH/"
  chmod +x "$DEPLOY_ZH/GeneralsXZH"
  echo "Deployed GeneralsXZH to $DEPLOY_ZH"
else
  echo "Build artifact not found: $BUILD_DIR/GeneralsMD/GeneralsXZH" >&2
  exit 1
fi

if [ -f "$BUILD_DIR/Generals/GeneralsX" ]; then
  echo "Deploying GeneralsX..."
  cp "$BUILD_DIR/Generals/GeneralsX" "$DEPLOY_BASE/"
  chmod +x "$DEPLOY_BASE/GeneralsX"
  echo "Deployed GeneralsX to $DEPLOY_BASE"
fi

echo "Deploy finished."
