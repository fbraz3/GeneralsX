#!/usr/bin/env bash
# GeneralsX @build GitHubCopilot 09/04/2026 Flatpak launcher wrapper for Generals with host data path fallback.
set -euo pipefail

# GeneralsX @refactor GitHubCopilot 13/04/2026 Delegate all runtime env and data-path logic to /app/bin/run.sh.
exec /app/bin/run.sh "$@"
