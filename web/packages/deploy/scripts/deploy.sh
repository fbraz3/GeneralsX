#!/usr/bin/env bash
# One idempotent command for the whole production deployment.
#
#   deploy.sh              # preflight -> gates -> R2 -> worker -> pages -> smoke
#   GENERALSX_DRY_RUN=1 deploy.sh
#                          # everything except the uploads: config validation,
#                          # artifact generation, and artifact verification
#
# Safe to re-run: every step converges on the same state.
set -euo pipefail
# shellcheck source=./lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

SCRIPTS_DIR="$(dirname "${BASH_SOURCE[0]}")"
DRY_RUN="${GENERALSX_DRY_RUN:-0}"

log "1/6 Repository gates (lint, typecheck, test, build)"
(cd "${WEB_DIR}" && npm run lint && npm run typecheck && npm run test && npm run build)
ok "gates passed"

if [ "${DRY_RUN}" = "1" ]; then
  warn "GENERALSX_DRY_RUN=1 — skipping preflight and every upload"
else
  log "2/6 Preflight"
  "${SCRIPTS_DIR}/preflight.sh"

  log "3/6 R2 asset origin"
  "${SCRIPTS_DIR}/provision-r2.sh"
fi

log "4/6 Signaling Worker"
"${SCRIPTS_DIR}/deploy-worker.sh"

log "5/6 Launcher (Cloudflare Pages)"
"${SCRIPTS_DIR}/deploy-pages.sh"

if [ "${DRY_RUN}" = "1" ]; then
  echo
  ok "dry run complete — artifacts generated and verified, nothing uploaded"
  exit 0
fi

log "6/6 Post-deploy smoke tests"
RELEASE="$(release_id)"
(cd "${WEB_DIR}" && npm run smoke -w @generalsx-web/deploy -- --release "${RELEASE}")

echo
ok "production deployment complete at ${RELEASE:0:12}"
echo "  launcher   ${GENERALSX_LAUNCHER_ORIGIN}"
echo "  signaling  ${GENERALSX_SIGNALING_ORIGIN}"
echo "  assets     ${GENERALSX_ASSET_ORIGIN}"
