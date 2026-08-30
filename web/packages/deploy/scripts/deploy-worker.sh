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

# --- Concurrency precondition ---------------------------------------------
# Wrangler has no compare-and-swap on the deployed script version: `wrangler
# deploy` is last-write-wins, and `--strict` does not change that (it refuses
# an upload when the live Worker's *settings* have drifted from wrangler.toml,
# e.g. a dashboard edit — a different problem).
#
# So the closest available guard is an explicit precondition on what is live
# right now. Set GENERALSX_EXPECTED_RELEASE_ID to the SHA you believe you are
# replacing and the deploy aborts if the live version is anything else, which
# is what catches "somebody deployed while I was building". It narrows the
# race to the seconds between this check and the upload; it does not close it.
# CI serializes the workflow instead (see .github/workflows/deploy-web.yml),
# which is the only real mutual exclusion available here.
EXPECTED_RELEASE="${GENERALSX_EXPECTED_RELEASE_ID:-}"
if [ -n "${EXPECTED_RELEASE}" ]; then
  log "Checking the live version matches the expected precondition"
  if ! command -v curl >/dev/null 2>&1; then
    die "GENERALSX_EXPECTED_RELEASE_ID requires curl to read ${GENERALSX_SIGNALING_ORIGIN}/readyz"
  fi
  live_body="$(curl -fsS --max-time 10 "${GENERALSX_SIGNALING_ORIGIN}/readyz" || true)"
  if [ -z "${live_body}" ]; then
    die "cannot read ${GENERALSX_SIGNALING_ORIGIN}/readyz to verify GENERALSX_EXPECTED_RELEASE_ID; refusing to deploy blind"
  fi
  if ! printf '%s' "${live_body}" | grep -q "${EXPECTED_RELEASE}"; then
    warn "the live Worker is not the version you expected to replace."
    warn "expected releaseId: ${EXPECTED_RELEASE}"
    warn "someone else has deployed since you started. Re-check and retry with:"
    warn "  curl -fsS ${GENERALSX_SIGNALING_ORIGIN}/readyz"
    die "deploy aborted by GENERALSX_EXPECTED_RELEASE_ID precondition"
  fi
  ok "live version matches ${EXPECTED_RELEASE:0:12}"
fi

# --strict refuses the upload when the live Worker's settings have drifted
# from this repository's wrangler.toml — a dashboard edit that a deploy would
# otherwise silently discard. It is configuration drift protection, not
# deploy-concurrency protection; see the precondition above for that.
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
