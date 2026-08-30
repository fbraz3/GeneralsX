#!/usr/bin/env bash
# Idempotent deployment of the signaling Worker (Durable Objects + TURN
# credential issuance) to signaling.generalsx.org.
#
# Every deploy uploads a new immutable Worker Version tagged with the commit
# SHA and stamps that SHA into RELEASE_ID, so /healthz and /readyz prove which
# version is live and `rollback.sh worker` can return to a known-good one.
#
# Secrets (TURN_KEY_ID / TURN_KEY_API_TOKEN) are NOT set here: they are
# provisioned once with `wrangler secret put` and are never read, printed, or
# committed. Deploys never delete existing secrets.
set -euo pipefail
# shellcheck source=./lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

require_clean_tree
RELEASE="$(release_id)"
SHORT_RELEASE="${RELEASE:0:12}"

log "Verifying configuration (dry run, nothing is uploaded)"
wrangler deploy --dry-run --outdir dist >/dev/null
ok "wrangler config and bundle are valid"

if [ "${GENERALSX_DRY_RUN:-0}" = "1" ]; then
  ok "GENERALSX_DRY_RUN=1 — stopping before upload"
  exit 0
fi

require_wrangler_auth

log "Deploying ${GENERALSX_WORKER_NAME} at ${SHORT_RELEASE}"
wrangler deploy \
  --var "RELEASE_ID:${RELEASE}" \
  --tag "${SHORT_RELEASE}" \
  --message "deploy ${SHORT_RELEASE}"
ok "deployed"

log "Verifying the live version"
if command -v curl >/dev/null 2>&1; then
  ready="$(curl -fsS "${GENERALSX_SIGNALING_ORIGIN}/readyz" || true)"
  if printf '%s' "${ready}" | grep -q "${RELEASE}"; then
    ok "${GENERALSX_SIGNALING_ORIGIN}/readyz reports releaseId ${SHORT_RELEASE}"
  else
    warn "readyz did not report ${SHORT_RELEASE} yet; the custom domain may still be propagating"
  fi
fi

echo
ok "worker deployed — run the full smoke suite next:"
echo "  npm run smoke -w @generalsx-web/deploy -- --release ${RELEASE}"
