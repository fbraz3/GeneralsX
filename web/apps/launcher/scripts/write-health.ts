/**
 * Post-build step: writes `dist/health.json` into the launcher output so the
 * static Cloudflare Pages deployment has a machine-readable liveness/version
 * endpoint (`https://play.generalsx.org/health.json`).
 *
 * Cloudflare Pages keeps every deployment at its own immutable URL; the only
 * way to prove *which* build the production alias currently serves — after a
 * promote or a rollback — is for the build itself to carry its commit SHA.
 * `GENERALSX_RELEASE_ID` is supplied by the deploy script; it falls back to
 * `CF_PAGES_COMMIT_SHA` (set by Pages CI builds) and finally to `dev`.
 *
 * Contains no secret and no retail asset information: service name, release
 * id, and build timestamp only.
 */
import { writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";

const outPath = fileURLToPath(new URL("../dist/health.json", import.meta.url));

const releaseId =
  process.env.GENERALSX_RELEASE_ID?.trim() || process.env.CF_PAGES_COMMIT_SHA?.trim() || "dev";

// `SOURCE_DATE_EPOCH` keeps the artifact reproducible for builds that need a
// byte-identical output (the same convention the native build uses).
const epochSeconds = Number(process.env.SOURCE_DATE_EPOCH);
const builtAt = new Date(
  Number.isFinite(epochSeconds) ? epochSeconds * 1000 : Date.now(),
).toISOString();

const body = `${JSON.stringify(
  { service: "generalsx-launcher", status: "ok", releaseId, builtAt },
  null,
  2,
)}\n`;

await writeFile(outPath, body, "utf8");
// eslint-disable-next-line no-console -- CLI build script output, not app code
console.log(`wrote ${outPath} (releaseId=${releaseId})`);
