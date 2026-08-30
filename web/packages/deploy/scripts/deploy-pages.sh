#!/usr/bin/env bash
# Idempotent deployment of the static launcher shell to Cloudflare Pages
# (play.generalsx.org).
#
# Creates the Pages project if missing, builds the launcher with the commit
# SHA stamped into dist/health.json, and uploads it as a new deployment.
# Every Pages deployment keeps its own permanent immutable URL; deploying to
# the production branch moves the play.generalsx.org alias to this build and
# leaves the previous one intact for `rollback.sh pages`.
#
# The uploaded directory contains only the launcher shell (HTML/CSS/JS plus
# the generated _headers and health.json). No retail game asset, engine
# binary, or credential is ever part of a Pages deployment.
set -euo pipefail
# shellcheck source=./lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

require_clean_tree
RELEASE="$(release_id)"
SHORT_RELEASE="${RELEASE:0:12}"
LAUNCHER_DIR="${WEB_DIR}/apps/launcher"

log "Building the launcher at ${SHORT_RELEASE}"
(cd "${WEB_DIR}" && GENERALSX_RELEASE_ID="${RELEASE}" npm run build -w @generalsx-web/launcher)
[ -f "${LAUNCHER_DIR}/dist/_headers" ] || die "dist/_headers was not generated"
[ -f "${LAUNCHER_DIR}/dist/health.json" ] || die "dist/health.json was not generated"
grep -q "${RELEASE}" "${LAUNCHER_DIR}/dist/health.json" || die "dist/health.json does not carry ${SHORT_RELEASE}"
ok "dist/ built with _headers and health.json"

if [ "${GENERALSX_DRY_RUN:-0}" = "1" ]; then
  ok "GENERALSX_DRY_RUN=1 — artifacts verified, stopping before upload"
  exit 0
fi

require_wrangler_auth

log "Pages project '${GENERALSX_PAGES_PROJECT}'"
if wrangler pages project list 2>/dev/null | grep -q "${GENERALSX_PAGES_PROJECT}"; then
  ok "already exists"
else
  wrangler pages project create "${GENERALSX_PAGES_PROJECT}" \
    --production-branch "${GENERALSX_PRODUCTION_BRANCH}"
  ok "created"
fi

log "Uploading deployment"
(cd "${LAUNCHER_DIR}" && npx --no-install wrangler pages deploy dist \
  --project-name "${GENERALSX_PAGES_PROJECT}" \
  --branch "${GENERALSX_PRODUCTION_BRANCH}" \
  --commit-hash "${RELEASE}" \
  --commit-message "deploy ${SHORT_RELEASE}")
ok "uploaded"

log "Custom domain ${GENERALSX_LAUNCHER_ORIGIN#https://}"
launcher_host="${GENERALSX_LAUNCHER_ORIGIN#https://}"
# Pages custom domains have no wrangler command; use the REST API, guarded by a
# GET so re-running does not fail on an already-attached domain.
if [ -z "${CLOUDFLARE_API_TOKEN:-}" ] || [ -z "${CLOUDFLARE_ACCOUNT_ID:-}" ]; then
  warn "CLOUDFLARE_API_TOKEN / CLOUDFLARE_ACCOUNT_ID not set; attach the domain once via"
  warn "  Workers & Pages > ${GENERALSX_PAGES_PROJECT} > Custom domains > ${launcher_host}"
else
  existing="$(cf_api GET "/accounts/${CLOUDFLARE_ACCOUNT_ID}/pages/projects/${GENERALSX_PAGES_PROJECT}/domains")"
  if printf '%s' "${existing}" | grep -q "\"name\":\"${launcher_host}\""; then
    ok "already attached"
  else
    response="$(cf_api POST \
      "/accounts/${CLOUDFLARE_ACCOUNT_ID}/pages/projects/${GENERALSX_PAGES_PROJECT}/domains" \
      "{\"name\":\"${launcher_host}\"}")"
    cf_api_success "${response}" || die "failed to attach ${launcher_host} (see the Cloudflare dashboard)"
    ok "attached (Cloudflare creates the proxied DNS record)"
  fi
fi

echo
ok "launcher deployed — run the full smoke suite next:"
echo "  npm run smoke -w @generalsx-web/deploy -- --release ${RELEASE}"
