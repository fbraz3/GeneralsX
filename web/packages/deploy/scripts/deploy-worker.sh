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

# --strict refuses the upload when the live Worker has changes this deploy did
# not produce (someone else's version, or a dashboard edit), instead of
# silently overwriting them. Without it, two operators deploying concurrently
# would each clobber the other and only notice from /readyz afterwards.
log "Deploying ${GENERALSX_WORKER_NAME} at ${SHORT_RELEASE}"
wrangler deploy \
  --strict \
  --var "RELEASE_ID:${RELEASE}" \
  --tag "${SHORT_RELEASE}" \
  --message "deploy ${SHORT_RELEASE}"
ok "deployed"

# A just-deployed custom domain is not instantly resolvable, so poll with
# bounded backoff instead of declaring failure on the first miss. The loop is
# capped in both attempts and wall-clock time so it can never hang a deploy.
log "Verifying the live version"
if command -v curl >/dev/null 2>&1; then
  attempt=1
  delay="${GENERALSX_READY_INITIAL_DELAY:-2}"
  max_attempts="${GENERALSX_READY_ATTEMPTS:-6}"
  waited=0
  budget="${GENERALSX_READY_BUDGET:-120}"
  verified=0
  while [ "${attempt}" -le "${max_attempts}" ]; do
    ready="$(curl -fsS --max-time 10 "${GENERALSX_SIGNALING_ORIGIN}/readyz" || true)"
    if printf '%s' "${ready}" | grep -q "${RELEASE}"; then
      verified=1
      break
    fi
    if [ "${attempt}" -eq "${max_attempts}" ] || [ "${waited}" -ge "${budget}" ]; then
      break
    fi
    log "readyz has not reported ${SHORT_RELEASE} yet (attempt ${attempt}/${max_attempts}); retrying in ${delay}s"
    sleep "${delay}"
    waited=$((waited + delay))
    delay=$((delay * 2))
    if [ "${delay}" -gt 20 ]; then delay=20; fi
    attempt=$((attempt + 1))
  done
  if [ "${verified}" -eq 1 ]; then
    ok "${GENERALSX_SIGNALING_ORIGIN}/readyz reports releaseId ${SHORT_RELEASE}"
  else
    warn "readyz did not report ${SHORT_RELEASE} after ${attempt} attempt(s) over ~${waited}s"
    warn "DNS or the custom-domain certificate may still be propagating — re-check with:"
    warn "  curl -fsS ${GENERALSX_SIGNALING_ORIGIN}/readyz"
  fi
fi

echo
ok "worker deployed — run the full smoke suite next:"
echo "  npm run smoke -w @generalsx-web/deploy -- --release ${RELEASE}"
