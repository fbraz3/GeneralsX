#!/usr/bin/env bash
# Idempotent provisioning of the authorized asset origin (Cloudflare R2).
#
# Creates the bucket if missing, applies the CORS policy the launcher's
# resumable downloader requires, and connects assets.generalsx.org.
# Re-running is a no-op.
#
# This script NEVER uploads game data. Publishing an authorized asset revision
# is a separate, operator-run step performed from a legally obtained install
# (see docs/HOWTO/WEB_ASSET_PIPELINE.md); no asset byte, digest list, or
# manifest belongs in this repository.
set -euo pipefail
# shellcheck source=./lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

require_cmd curl
require_wrangler_auth

log "R2 bucket '${GENERALSX_ASSET_BUCKET}'"
if wrangler r2 bucket list 2>/dev/null | grep -q "${GENERALSX_ASSET_BUCKET}"; then
  ok "already exists"
else
  wrangler r2 bucket create "${GENERALSX_ASSET_BUCKET}"
  ok "created"
fi

log "CORS policy"
# `cors set` replaces the whole configuration, so applying the same file twice
# converges on the same state.
wrangler r2 bucket cors set "${GENERALSX_ASSET_BUCKET}" \
  --file "${DEPLOY_DIR}/config/r2-cors.json" --force
ok "applied ${DEPLOY_DIR}/config/r2-cors.json"

log "Custom domain ${GENERALSX_ASSET_ORIGIN#https://}"
asset_host="${GENERALSX_ASSET_ORIGIN#https://}"
if wrangler r2 bucket domain list "${GENERALSX_ASSET_BUCKET}" 2>/dev/null | grep -q "${asset_host}"; then
  ok "already connected"
elif [ -z "${CLOUDFLARE_ZONE_ID:-}" ]; then
  warn "CLOUDFLARE_ZONE_ID is not set; connect the domain manually:"
  warn "  wrangler r2 bucket domain add ${GENERALSX_ASSET_BUCKET} --domain ${asset_host} --zone-id <zone-id> --min-tls 1.2"
else
  wrangler r2 bucket domain add "${GENERALSX_ASSET_BUCKET}" \
    --domain "${asset_host}" --zone-id "${CLOUDFLARE_ZONE_ID}" --min-tls 1.2 --force
  ok "connected (Cloudflare creates the proxied DNS record)"
fi

echo
ok "R2 asset origin provisioned"
warn "objects are still published by the operator, out of band, per revision prefix"
