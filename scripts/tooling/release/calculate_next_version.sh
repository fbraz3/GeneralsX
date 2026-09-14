#!/usr/bin/env bash
# Calculate the next Semantic Version (SemVer) based on base-version.txt and git tags.
#
# Usage:
#   ./scripts/tooling/release/calculate_next_version.sh [PATH_TO_BASE_VERSION_FILE]
#
# Environment:
#   BASE_VERSION_FILE   Optional custom path to base-version.txt
#   GITHUB_OUTPUT       If set, writes 'version=<calculated_version>' to GITHUB_OUTPUT file
#
# Logic:
#   - Reads MAJOR.MINOR from base-version.txt (e.g. "1.0")
#   - Queries git tags matching the specified MAJOR.MINOR prefix
#   - If no tags exist in that series, returns <MAJOR>.<MINOR>.0 (e.g. "1.0.0")
#   - If tags exist, increments the highest patch number by 1 (e.g. "1.0.1")

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

if [ -n "${1:-}" ]; then
  VERSION_FILE="$1"
elif [ -n "${BASE_VERSION_FILE:-}" ]; then
  VERSION_FILE="$BASE_VERSION_FILE"
elif [ -f "base-version.txt" ]; then
  VERSION_FILE="base-version.txt"
else
  VERSION_FILE="${PROJECT_ROOT}/base-version.txt"
fi

if [ ! -f "$VERSION_FILE" ]; then
  echo "ERROR: Version base file not found at '$VERSION_FILE'" >&2
  exit 1
fi

RAW_BASE="$(tr -d '[:space:]' < "$VERSION_FILE")"

if [ -z "$RAW_BASE" ]; then
  echo "ERROR: Version base file '$VERSION_FILE' is empty" >&2
  exit 1
fi

if ! [[ "$RAW_BASE" =~ ^[0-9]+\.[0-9]+$ ]]; then
  echo "ERROR: Invalid base version format in '$VERSION_FILE': '$RAW_BASE'. Expected format 'MAJOR.MINOR' (e.g. 1.0)" >&2
  exit 1
fi

# Extract MAJOR and MINOR components
MAJOR="$(echo "$RAW_BASE" | cut -d'.' -f1)"
MINOR="$(echo "$RAW_BASE" | cut -d'.' -f2)"

# Query existing git tags (matching 1.0.X, v1.0.X, or GeneralsX-1.0.X)
EXISTING_PATCHES="$(git tag -l 2>/dev/null | sed -nE "s/^(GeneralsX-|v)?${MAJOR}\.${MINOR}\.([0-9]+)$/\2/p" | sort -n || true)"

if [ -z "$EXISTING_PATCHES" ]; then
  NEXT_PATCH="0"
else
  LATEST_PATCH="$(echo "$EXISTING_PATCHES" | tail -n 1)"
  NEXT_PATCH="$((LATEST_PATCH + 1))"
fi

NEXT_VERSION="${MAJOR}.${MINOR}.${NEXT_PATCH}"

# Standard output
echo "$NEXT_VERSION"

# GitHub Actions output support
if [ -n "${GITHUB_OUTPUT:-}" ] && [ -w "$GITHUB_OUTPUT" ]; then
  echo "version=${NEXT_VERSION}" >> "$GITHUB_OUTPUT"
fi
