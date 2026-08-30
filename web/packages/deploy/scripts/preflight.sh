#!/usr/bin/env bash
# Read-only preflight for the play.generalsx.org production deployment.
#
# Reports what is already provisioned and what is still missing, without
# creating, modifying, or deleting anything, and without printing a single
# token, secret value, account id, or zone id.
#
# Exit codes: 0 = ready to deploy, 1 = at least one hard blocker.
set -euo pipefail
# shellcheck source=./lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

blockers=0
block() { warn "$*"; blockers=$((blockers + 1)); }

log "Toolchain"
for cmd in node npm curl git; do
  require_cmd "${cmd}"
done
ok "node $(node --version), npm $(npm --version)"
wrangler --version >/dev/null 2>&1 || die "wrangler is not installed; run 'npm ci' in web/"
ok "wrangler $(wrangler --version 2>/dev/null | tail -1)"

log "Cloudflare authentication"
if wrangler whoami 2>&1 | grep -qi 'not authenticated'; then
  block "wrangler is not authenticated (run 'wrangler login', or export CLOUDFLARE_API_TOKEN)"
else
  # Deliberately reports only *that* credentials work. Account names, account
  # ids, emails, and the token itself are never printed by this script.
  ok "wrangler has working credentials"
fi
[ -n "${CLOUDFLARE_ACCOUNT_ID:-}" ] && ok "CLOUDFLARE_ACCOUNT_ID is set" || warn "CLOUDFLARE_ACCOUNT_ID is not set (needed for Pages custom domain + rollback)"
[ -n "${CLOUDFLARE_API_TOKEN:-}" ] && ok "CLOUDFLARE_API_TOKEN is set" || warn "CLOUDFLARE_API_TOKEN is not set (needed for Pages custom domain + rollback)"

log "DNS (${GENERALSX_ZONE_NAME})"
if command -v dig >/dev/null 2>&1; then
  for host in "${GENERALSX_LAUNCHER_ORIGIN}" "${GENERALSX_SIGNALING_ORIGIN}" "${GENERALSX_ASSET_ORIGIN}"; do
    name="${host#https://}"
    if [ -n "$(dig +short "${name}" 2>/dev/null)" ]; then
      ok "${name} resolves"
    else
      warn "${name} does not resolve yet (created automatically by the deploy steps below)"
    fi
  done
  ns="$(dig +short NS "${GENERALSX_ZONE_NAME}" 2>/dev/null | head -1)"
  case "${ns}" in
    *cloudflare.com.) ok "${GENERALSX_ZONE_NAME} is served by Cloudflare nameservers" ;;
    "") block "${GENERALSX_ZONE_NAME} has no NS records" ;;
    *) block "${GENERALSX_ZONE_NAME} is not on Cloudflare nameservers (found ${ns})" ;;
  esac
else
  warn "dig not available; skipping DNS checks"
fi

log "Cloudflare resources"
if projects="$(wrangler pages project list 2>/dev/null)"; then
  printf '%s' "${projects}" | grep -q "${GENERALSX_PAGES_PROJECT}" \
    && ok "Pages project '${GENERALSX_PAGES_PROJECT}' exists" \
    || warn "Pages project '${GENERALSX_PAGES_PROJECT}' missing (deploy-pages.sh creates it)"
else
  warn "could not list Pages projects (not authenticated?)"
fi

if buckets="$(wrangler r2 bucket list 2>/dev/null)"; then
  printf '%s' "${buckets}" | grep -q "${GENERALSX_ASSET_BUCKET}" \
    && ok "R2 bucket '${GENERALSX_ASSET_BUCKET}' exists" \
    || warn "R2 bucket '${GENERALSX_ASSET_BUCKET}' missing (provision-r2.sh creates it)"
else
  warn "could not list R2 buckets (not authenticated, or the token lacks R2 scope)"
fi

log "Worker secrets (names only, values are never read)"
if secrets="$(wrangler secret list --name "${GENERALSX_WORKER_NAME}" 2>/dev/null)"; then
  for key in TURN_KEY_ID TURN_KEY_API_TOKEN; do
    printf '%s' "${secrets}" | grep -q "${key}" \
      && ok "${key} is set" \
      || warn "${key} is not set (TURN relay disabled; /readyz reports degraded)"
  done
else
  warn "could not list Worker secrets (Worker not deployed yet, or not authenticated)"
fi

echo
if [ "${blockers}" -gt 0 ]; then
  die "${blockers} blocker(s) must be resolved before deploying"
fi
ok "preflight passed"
