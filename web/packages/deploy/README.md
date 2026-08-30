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
| `deploy-worker.sh` | `wrangler deploy --dry-run`, then `wrangler deploy --strict` tagged with the commit SHA | Wrangler config declares the custom domain, so re-deploying re-converges |
| `deploy-pages.sh` | Builds `dist/`, verifies `_headers` + `health.json`, uploads, attaches the custom domain | Project/domain existence is checked before creating |

### Worker deploy safety

`--strict` makes `wrangler deploy` refuse the upload when the live Worker
carries changes this deploy did not produce — another operator's version, or a
dashboard edit. Without it two concurrent deploys silently clobber each other
and the loser only finds out from `/readyz`.

`keep_vars = false` in `apps/worker/wrangler.toml` is a separate, orthogonal
decision: that file is the single source of truth for the Worker's non-secret
vars, so a deploy replaces the live var set wholesale and any var added by
hand in the dashboard is intentionally removed. It does **not** guard against
concurrent deploys. Secrets live in a different store and are never deleted by
a deploy, so `TURN_KEY_ID` / `TURN_KEY_API_TOKEN` survive.

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
npm run smoke -w @generalsx-web/deploy -- --launcher https://<hash>.generalsx-launcher.pages.dev
npm run smoke -w @generalsx-web/deploy -- --no-retry               # already-live deployment
```

Checks (see [`src/smoke.ts`](src/smoke.ts)): launcher reachability, security
headers compared against the *generating* policy in `@generalsx-web/shared`,
launcher `health.json`, immutable caching of hashed assets, Worker
liveness/readiness/security headers, CORS reflecting the launcher origin and
nothing else, TURN credentials being refused without a room admission token,
and the asset origin's resumable-delivery contract (206, `Content-Range`,
strong `ETag`, immutable `Cache-Control`, exposed validators). The asset probe
requests a single byte; it never downloads an asset. Exit code is non-zero if
any check fails.

The suite holds no credential and issues none: the TURN check asserts that an
anonymous request *and* a forged bearer token are both rejected with 401, with
an allowed `Origin` presented in each case — because CORS is a browser-side
read restriction that a non-browser caller simply ignores, and so can never be
the thing protecting the endpoint.

### Propagation retry

A deployment is not reachable the instant it uploads: a new custom domain has
to propagate and its certificate has to be issued. Probes therefore retry
propagation-shaped failures — transport errors (DNS/TCP/TLS) and the statuses
`408, 425, 429, 5xx, 520-527` — with exponential backoff.

| Flag | Default | Meaning |
| --- | --- | --- |
| `--retry-attempts <n>` | `6` | Attempts per origin, including the first |
| `--retry-budget <sec>` | `120` | Total seconds the run may spend waiting |
| `--no-retry` | off | Fail on the first propagation-shaped error |

Retrying happens inside the fetch, never around a check's assertions, so a
wrong header, a stale release id, or a 404 still fails on the *first* attempt
— a real regression is never hidden behind a two-minute wait. The budget is
shared across all origins, an origin that has answered once is never waited on
again, and an origin that exhausts its attempts fails every later check
immediately. Giving up reports the origin, attempt count, elapsed time, and
the underlying failure, so "still propagating" is distinguishable from
"misconfigured" without re-running anything.

`deploy-worker.sh` polls `/readyz` for the deployed release id with the same
bounded strategy (`GENERALSX_READY_ATTEMPTS`, `GENERALSX_READY_BUDGET`,
`GENERALSX_READY_INITIAL_DELAY`).

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
