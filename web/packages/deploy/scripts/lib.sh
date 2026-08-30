#!/usr/bin/env bash
# Shared helpers for the GeneralsX Cloudflare deployment scripts.
#
# Source this file; do not execute it.
#
# Secrets policy: this file never prints a token, a secret value, or an
# account/zone identifier. Credentials are read from the environment
# (CLOUDFLARE_API_TOKEN / CLOUDFLARE_ACCOUNT_ID) and passed straight to
# wrangler or curl; they are never echoed, logged, or written to a file.

# --- Deployment targets ----------------------------------------------------
# Keep in sync with ../src/targets.ts (test/targets.test.ts asserts they match).
GENERALSX_PAGES_PROJECT="${GENERALSX_PAGES_PROJECT:-generalsx-launcher}"
GENERALSX_WORKER_NAME="${GENERALSX_WORKER_NAME:-generalsx-signaling}"
GENERALSX_ASSET_BUCKET="${GENERALSX_ASSET_BUCKET:-generalsx-web-assets}"
GENERALSX_LAUNCHER_ORIGIN="${GENERALSX_LAUNCHER_ORIGIN:-https://play.generalsx.org}"
GENERALSX_SIGNALING_ORIGIN="${GENERALSX_SIGNALING_ORIGIN:-https://signaling.generalsx.org}"
GENERALSX_ASSET_ORIGIN="${GENERALSX_ASSET_ORIGIN:-https://assets.generalsx.org}"
GENERALSX_PRODUCTION_BRANCH="${GENERALSX_PRODUCTION_BRANCH:-main}"
GENERALSX_ZONE_NAME="${GENERALSX_ZONE_NAME:-generalsx.org}"

DEPLOY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WEB_DIR="$(cd "${DEPLOY_DIR}/../.." && pwd)"

log()  { printf '\033[1;34m==>\033[0m %s\n' "$*"; }
ok()   { printf '\033[1;32m  ok\033[0m %s\n' "$*"; }
warn() { printf '\033[1;33m  !!\033[0m %s\n' "$*" >&2; }
die()  { printf '\033[1;31merror:\033[0m %s\n' "$*" >&2; exit 1; }

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "required command not found: $1"
}

# Runs wrangler from the web workspace's pinned devDependency (never a
# floating global install), so every operator uses the same CLI version.
wrangler() {
  (cd "${WEB_DIR}/apps/worker" && npx --no-install wrangler "$@")
}

# The immutable release identifier stamped into both surfaces. Defaults to the
# current commit; a dirty tree is rejected so a deployed release id always
# points at code that exists in git.
release_id() {
  if [ -n "${GENERALSX_RELEASE_ID:-}" ]; then
    printf '%s' "${GENERALSX_RELEASE_ID}"
    return
  fi
  git -C "${WEB_DIR}" rev-parse HEAD
}

require_clean_tree() {
  if [ -n "${GENERALSX_RELEASE_ID:-}" ]; then return; fi
  if [ -n "$(git -C "${WEB_DIR}" status --porcelain)" ]; then
    die "working tree is dirty; commit first or set GENERALSX_RELEASE_ID explicitly"
  fi
}

# Verifies wrangler has credentials without printing any of them.
require_wrangler_auth() {
  local output
  if ! output="$(wrangler whoami 2>&1)"; then
    die "wrangler whoami failed; run 'wrangler login' or export CLOUDFLARE_API_TOKEN"
  fi
  if printf '%s' "${output}" | grep -qi 'not authenticated'; then
    die "wrangler is not authenticated; run 'wrangler login' or export CLOUDFLARE_API_TOKEN"
  fi
  ok "wrangler is authenticated"
}

# Minimal Cloudflare REST helper for the few operations wrangler does not
# expose (Pages custom domains and Pages deployment rollback). Prints the
# response body only; the token stays in the header.
cf_api() {
  local method="$1" path="$2" body="${3:-}"
  [ -n "${CLOUDFLARE_API_TOKEN:-}" ] || die "CLOUDFLARE_API_TOKEN is required for ${method} ${path}"
  [ -n "${CLOUDFLARE_ACCOUNT_ID:-}" ] || die "CLOUDFLARE_ACCOUNT_ID is required for ${method} ${path}"
  local args=(-sS -X "${method}"
    -H "Authorization: Bearer ${CLOUDFLARE_API_TOKEN}"
    -H "Content-Type: application/json")
  [ -n "${body}" ] && args+=(-d "${body}")
  curl "${args[@]}" "https://api.cloudflare.com/client/v4${path}"
}

cf_api_success() {
  printf '%s' "$1" | grep -q '"success":[[:space:]]*true'
}
