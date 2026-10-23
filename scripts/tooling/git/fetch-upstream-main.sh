#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: fetch TheSuperHackers/main with enough
# history for merge-base. A shallow `git fetch upstream main` leaves unrelated histories
# (merge-base empty) even when the provenance base commit exists locally.
# Usage: ./scripts/tooling/git/fetch-upstream-main.sh [deepen-count]

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
DEEPEN="${1:-5000}"
cd "${ROOT}"

if ! git remote get-url upstream >/dev/null 2>&1; then
	echo "ERROR: remote 'upstream' missing (expected TheSuperHackers/GeneralsGameCode)"
	exit 1
fi

echo "INFO: fetching upstream/main (deepen=${DEEPEN})"
git fetch upstream main --deepen="${DEEPEN}"

base="$(git merge-base HEAD upstream/main 2>/dev/null || true)"
if [[ -z "${base}" ]]; then
	echo "WARN: merge-base still empty; try: git fetch upstream main --unshallow"
	exit 2
fi

echo "OK: merge-base HEAD upstream/main = ${base}"
git log -1 --oneline "${base}"
git log -1 --oneline upstream/main
