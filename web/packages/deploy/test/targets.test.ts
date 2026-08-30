/**
 * The deployment shell scripts cannot import TypeScript, so they restate the
 * project/bucket names and origins as shell defaults. This test keeps the two
 * copies from drifting — a rename in `targets.ts` that is not mirrored in
 * `scripts/lib.sh` would otherwise deploy to the wrong resource.
 */
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { describe, expect, it } from "vitest";
import { PRODUCTION_TARGET } from "../src/targets.ts";

const libSh = readFileSync(fileURLToPath(new URL("../scripts/lib.sh", import.meta.url)), "utf8");

function shellDefault(variable: string): string | undefined {
  const match = new RegExp(`${variable}="\\$\\{${variable}:-([^}]*)\\}"`).exec(libSh);
  return match?.[1];
}

describe("scripts/lib.sh defaults", () => {
  it.each([
    ["GENERALSX_PAGES_PROJECT", PRODUCTION_TARGET.pagesProject],
    ["GENERALSX_WORKER_NAME", PRODUCTION_TARGET.workerName],
    ["GENERALSX_ASSET_BUCKET", PRODUCTION_TARGET.assetBucket],
    ["GENERALSX_LAUNCHER_ORIGIN", PRODUCTION_TARGET.launcherOrigin],
    ["GENERALSX_SIGNALING_ORIGIN", PRODUCTION_TARGET.signalingOrigin],
    ["GENERALSX_ASSET_ORIGIN", PRODUCTION_TARGET.assetOrigin],
    ["GENERALSX_PRODUCTION_BRANCH", PRODUCTION_TARGET.productionBranch],
  ])("%s matches targets.ts", (variable, expected) => {
    expect(shellDefault(variable)).toBe(expected);
  });
});

describe("deployment scripts", () => {
  const scripts = ["deploy.sh", "deploy-pages.sh", "deploy-worker.sh", "preflight.sh", "provision-r2.sh", "rollback.sh"];

  it.each(scripts)("%s fails fast on unset variables and errors", (script) => {
    const body = readFileSync(fileURLToPath(new URL(`../scripts/${script}`, import.meta.url)), "utf8");
    expect(body).toContain("set -euo pipefail");
  });

  it.each(scripts)("%s never echoes a credential", (script) => {
    const body = readFileSync(fileURLToPath(new URL(`../scripts/${script}`, import.meta.url)), "utf8");
    // Credentials may be *referenced* (passed to curl/wrangler) but must never
    // be expanded into an echo/printf/log line.
    for (const line of body.split("\n")) {
      if (/^\s*(echo|printf|log|ok|warn|die)\b/.test(line)) {
        expect(line).not.toMatch(/\$\{?(CLOUDFLARE_API_TOKEN|TURN_KEY_ID|TURN_KEY_API_TOKEN)\b/);
      }
    }
  });

  it("deploys the worker with --strict so a concurrent deploy cannot be clobbered", () => {
    const body = readFileSync(fileURLToPath(new URL("../scripts/deploy-worker.sh", import.meta.url)), "utf8");
    // The real upload must be strict. The preceding `--dry-run` deliberately
    // is not: it uploads nothing, so there is no remote state to conflict with.
    const upload = body.slice(body.indexOf("Deploying ${GENERALSX_WORKER_NAME}"));
    expect(upload).toMatch(/wrangler deploy \\\n\s*--strict/);
  });

  it("bounds the post-deploy readiness poll so a propagation delay cannot hang a deploy", () => {
    const body = readFileSync(fileURLToPath(new URL("../scripts/deploy-worker.sh", import.meta.url)), "utf8");
    expect(body).toContain("GENERALSX_READY_ATTEMPTS");
    expect(body).toContain("GENERALSX_READY_BUDGET");
    // Both an attempt cap and a wall-clock budget must terminate the loop.
    expect(body).toMatch(/attempt.*-le.*max_attempts/);
    expect(body).toMatch(/waited.*-ge.*budget/);
    expect(body).toContain("--max-time 10");
  });
});
