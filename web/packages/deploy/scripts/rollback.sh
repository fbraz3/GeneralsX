#!/usr/bin/env bash
# Roll back to a previous immutable release.
#
#   rollback.sh worker [version-id]   # wrangler Worker Versions
#   rollback.sh pages  <deployment-id>
#   rollback.sh list                  # show rollback candidates for both
#
# Nothing is rebuilt: both surfaces keep every uploaded version/deployment
# immutably, so a rollback re-points traffic at bytes that were already
# verified in production rather than producing a new artifact.
set -euo pipefail
# shellcheck source=./lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

usage() {
  cat <<'USAGE'
Roll back to a previous immutable release.

  rollback.sh list                  Show rollback candidates for both surfaces
  rollback.sh worker [version-id]   Roll the Worker back (previous version by default)
  rollback.sh pages <deployment-id> Roll the Pages project back to a deployment

Nothing is rebuilt: both surfaces keep every uploaded version/deployment
immutably, so a rollback re-points traffic at bytes that were already
verified in production.
USAGE
  exit "${1:-1}"
}

command="${1:-}"
[ -n "${command}" ] || usage 1

case "${command}" in
  list)
    require_wrangler_auth
    log "Worker versions (${GENERALSX_WORKER_NAME})"
    wrangler versions list
    log "Pages deployments (${GENERALSX_PAGES_PROJECT})"
    (cd "${WEB_DIR}/apps/launcher" && npx --no-install wrangler pages deployment list \
      --project-name "${GENERALSX_PAGES_PROJECT}")
    ;;

  worker)
    require_wrangler_auth
    version_id="${2:-}"
    log "Rolling back ${GENERALSX_WORKER_NAME}"
    if [ -n "${version_id}" ]; then
      wrangler rollback "${version_id}" --message "rollback to ${version_id}" --yes
    else
      # No id: wrangler rolls back to the previously deployed version.
      wrangler rollback --message "rollback to previous version" --yes
    fi
    ok "rolled back"
    if command -v curl >/dev/null 2>&1; then
      log "Live version now reports:"
      curl -fsS "${GENERALSX_SIGNALING_ORIGIN}/readyz" || warn "could not read /readyz"
      echo
    fi
    ;;

  pages)
    deployment_id="${2:-}"
    [ -n "${deployment_id}" ] || die "a Pages deployment id is required (see 'rollback.sh list')"
    log "Rolling back ${GENERALSX_PAGES_PROJECT} to ${deployment_id}"
    response="$(cf_api POST \
      "/accounts/${CLOUDFLARE_ACCOUNT_ID}/pages/projects/${GENERALSX_PAGES_PROJECT}/deployments/${deployment_id}/rollback")"
    cf_api_success "${response}" || die "rollback failed (see the Cloudflare dashboard)"
    ok "rolled back"
    if command -v curl >/dev/null 2>&1; then
      log "Live launcher now reports:"
      curl -fsS "${GENERALSX_LAUNCHER_ORIGIN}/health.json" || warn "could not read /health.json"
      echo
    fi
    ;;

  -h|--help|help)
    usage 0
    ;;

  *)
    die "unknown command: ${command}"
    ;;
esac

echo
ok "verify the rollback with:"
echo "  npm run smoke -w @generalsx-web/deploy"
