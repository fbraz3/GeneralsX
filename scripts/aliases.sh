#!/usr/bin/env bash
# Source this file in your shell profile: source /path/to/scripts/aliases.sh

export GENERALSX_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export GENERALSX_DEPLOY="$HOME/GeneralsX/GeneralsMD"

alias build_zh="cmake --build \"$GENERALSX_ROOT/build/macos-arm64\" --target GeneralsXZH -j 4"
alias deploy_zh="cp \"$GENERALSX_ROOT/build/macos-arm64/GeneralsMD/GeneralsXZH\" \"$GENERALSX_DEPLOY/\""
alias run_zh="cd \"$GENERALSX_DEPLOY\" && USE_METAL=1 ./GeneralsXZH"

echo "Aliases defined: build_zh, deploy_zh, run_zh"
