#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 /path/to/retail/install" >&2
  exit 2
fi

RETAIL_PATH="$1"

for target in "$HOME/GeneralsX/GeneralsMD" "$HOME/GeneralsX/Generals"; do
  mkdir -p "$target" "$target/logs"
  [ -L "$target/Date" ] && rm -f "$target/Date"
  [ -L "$target/Maps" ] && rm -f "$target/Maps"

  if [ -d "$RETAIL_PATH/Date" ] && [ -d "$RETAIL_PATH/Maps" ]; then
    ln -s "$RETAIL_PATH/Date" "$target/Date"
    ln -s "$RETAIL_PATH/Maps" "$target/Maps"
    echo "Configured assets in $target -> $RETAIL_PATH"
  else
    echo "Retail path missing Date/Maps: $RETAIL_PATH" >&2
    exit 3
  fi
done

echo "Assets setup complete."
