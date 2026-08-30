# GeneralsX deployment (`@generalsx-web/deploy`)

Idempotent Cloudflare provisioning/deployment scripts and post-deploy smoke
tests for the production browser prototype.

| Surface | Origin | Cloudflare resource |
|---|---|---|
| Launcher shell | `https://play.generalsx.org` | Pages project `generalsx-launcher` (`web/apps/launcher`) |
| Room signaling + TURN issuance | `https://signaling.generalsx.org` | Worker `generalsx-signaling` + Durable Objects (`web/apps/worker`) |
| Authorized game assets | `https://assets.generalsx.org` | R2 bucket `generalsx-web-assets` (operator-published) |

Hostnames and resource names live in [`src/targets.ts`](src/targets.ts);
`scripts/lib.sh` restates them as shell defaults and
`test/targets.test.ts` fails if the two ever drift.

## What is never deployed from here

- **No retail game asset and no engine binary.** `provision-r2.sh` creates and
  configures the bucket; it never uploads an object. Publishing an authorized
  revision is a separate operator step performed from a legally obtained
  install — see [`docs/HOWTO/WEB_ASSET_PIPELINE.md`](../../../docs/HOWTO/WEB_ASSET_PIPELINE.md).
- **No secret.** `TURN_KEY_ID` / `TURN_KEY_API_TOKEN` are set once with
  `wrangler secret put` and are never read, printed, or committed. Deploys do
  not delete existing secrets. `preflight.sh` reports secret *names* only.

## Prerequisites

```bash
cd web && npm ci                       # pins wrangler for every operator
wrangler login                         # or export CLOUDFLARE_API_TOKEN
export CLOUDFLARE_ACCOUNT_ID=…         # Pages custom domain + Pages rollback
export CLOUDFLARE_ZONE_ID=…            # R2 custom domain only
```

The API token needs: `Workers Scripts:Edit`, `Workers R2 Storage:Edit`,
`Cloudflare Pages:Edit`, and `Zone:DNS:Edit` on the `generalsx.org` zone.

TURN secrets (once, before the first deploy):

```bash
cd web/apps/worker
wrangler secret put TURN_KEY_ID
wrangler secret put TURN_KEY_API_TOKEN
```

Without them the deployment still works: `/readyz` reports `degraded`, peers
fall back to direct/STUN ICE, and the launcher shows a visible warning.

## Deploy

```bash
cd web

# Everything except uploads: gates, config validation, artifact verification.
GENERALSX_DRY_RUN=1 packages/deploy/scripts/deploy.sh

# The real thing (safe to re-run; every step converges).
packages/deploy/scripts/deploy.sh
```

`deploy.sh` runs, in order: repository gates (`lint`, `typecheck`, `test`,
`build`) → `preflight.sh` → `provision-r2.sh` → `deploy-worker.sh` →
`deploy-pages.sh` → the smoke suite pinned to the deployed commit SHA.

Individual steps can also be run on their own:

| Script | Does | Idempotent because |
|---|---|---|
| `preflight.sh` | Read-only audit of auth, DNS, resources, secret names | It never writes |
| `provision-r2.sh` | Creates the bucket, applies `config/r2-cors.json`, connects `assets.generalsx.org` | Existence is checked first; `cors set` replaces the whole policy |
| `deploy-worker.sh` | `wrangler deploy --dry-run` then a real deploy tagged with the commit SHA | Wrangler config declares the custom domain, so re-deploying re-converges |
| `deploy-pages.sh` | Builds `dist/`, verifies `_headers` + `health.json`, uploads, attaches the custom domain | Project/domain existence is checked before creating |

## DNS

`generalsx.org` must be on Cloudflare nameservers (`preflight.sh` verifies
this). All three records are then created by the deploy steps themselves —
no manual record editing:

| Record | Created by |
|---|---|
| `play.generalsx.org` | Pages custom domain attach in `deploy-pages.sh` |
| `signaling.generalsx.org` | `[[routes]] custom_domain = true` in `apps/worker/wrangler.toml` |
| `assets.generalsx.org` | `wrangler r2 bucket domain add` in `provision-r2.sh` |

## Health and readiness

| Endpoint | Meaning |
|---|---|
| `https://signaling.generalsx.org/healthz` | Liveness. Dependency-free, always 200 while the isolate runs. |
| `https://signaling.generalsx.org/readyz` | Readiness. 200 when it can serve signaling, 503 otherwise, with a per-check breakdown. |
| `https://play.generalsx.org/health.json` | Static build metadata: service, status, release id, build time. |

All three send `Cache-Control: no-store` and disclose no secret value —
secret-backed checks report `configured` / `missing` only.

## Smoke tests

```bash
npm run smoke -w @generalsx-web/deploy
npm run smoke -w @generalsx-web/deploy -- --release <commit-sha>   # pin the release
npm run smoke -w @generalsx-web/deploy -- --require-turn           # issue a live credential
npm run smoke -w @generalsx-web/deploy -- --launcher https://<hash>.generalsx-launcher.pages.dev
```

Checks (see [`src/smoke.ts`](src/smoke.ts)): launcher reachability, security
headers compared against the *generating* policy in `@generalsx-web/shared`,
launcher `health.json`, immutable caching of hashed assets, Worker
liveness/readiness/security headers, CORS reflecting the launcher origin and
nothing else, optional live TURN issuance (shape only — credential material is
never printed), and the asset origin's resumable-delivery contract (206,
`Content-Range`, strong `ETag`, immutable `Cache-Control`, exposed
validators). The asset probe requests a single byte; it never downloads an
asset. Exit code is non-zero if any check fails.

## Immutable rollback

Every Worker deploy uploads a new immutable Version tagged with its commit
SHA, and every Pages deploy keeps a permanent immutable URL. Both surfaces
echo their release id, so a rollback is verifiable rather than assumed.

```bash
packages/deploy/scripts/rollback.sh list                 # candidates for both surfaces
packages/deploy/scripts/rollback.sh worker               # previous Worker version
packages/deploy/scripts/rollback.sh worker <version-id>  # a specific one
packages/deploy/scripts/rollback.sh pages <deployment-id>
npm run smoke -w @generalsx-web/deploy -- --release <restored-sha>
```

Rollbacks rebuild nothing: they re-point traffic at bytes that already ran in
production. `/index.html` is served `no-cache` (only the content-hashed
`/assets/*` are `immutable`), so a rollback is visible immediately instead of
waiting out a cache TTL.
