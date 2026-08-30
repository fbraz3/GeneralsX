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

  it("deploys the worker with --strict, so a dashboard edit is not silently discarded", () => {
    const body = readFileSync(fileURLToPath(new URL("../scripts/deploy-worker.sh", import.meta.url)), "utf8");
    // The real upload must be strict. The preceding `--dry-run` deliberately
    // is not: it uploads nothing, so there is no remote state to conflict with.
    const upload = body.slice(body.indexOf("Deploying ${GENERALSX_WORKER_NAME}"));
    expect(upload).toMatch(/wrangler deploy \\\n\s*--strict/);
  });

  it("does not claim --strict guards concurrent deploys", () => {
    // Wrangler has no compare-and-swap on the deployed version. `--strict`
    // catches configuration drift, not a second operator publishing at the
    // same moment, and documenting otherwise would give false confidence.
    const sources = [
      readFileSync(fileURLToPath(new URL("../scripts/deploy-worker.sh", import.meta.url)), "utf8"),
      readFileSync(fileURLToPath(new URL("../../../apps/worker/wrangler.toml", import.meta.url)), "utf8"),
      readFileSync(fileURLToPath(new URL("../README.md", import.meta.url)), "utf8"),
    ];
    for (const body of sources) {
      for (const line of body.split("\n")) {
        if (!/--strict/.test(line)) continue;
        expect(line).not.toMatch(/concurrent|simultaneous|another operator|someone else/i);
      }
    }
  });

  it("offers an explicit deployed-version precondition for concurrent operators", () => {
    const body = readFileSync(fileURLToPath(new URL("../scripts/deploy-worker.sh", import.meta.url)), "utf8");
    expect(body).toContain("GENERALSX_EXPECTED_RELEASE_ID");
    // It must read what is actually live, and refuse rather than guess when
    // it cannot.
    expect(body).toMatch(/readyz/);
    expect(body).toMatch(/die "deploy aborted by GENERALSX_EXPECTED_RELEASE_ID precondition"/);
    expect(body).toMatch(/refusing to deploy blind/);
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

describe("deploy-web workflow", () => {
  const workflow = readFileSync(
    fileURLToPath(new URL("../../../../.github/workflows/deploy-web.yml", import.meta.url)),
    "utf8",
  );

  it("serializes production deploys, because wrangler cannot", () => {
    expect(workflow).toMatch(/concurrency:\n\s*group: deploy-web-production/);
    // A deploy that has already uploaded must finish its smoke tests; being
    // cancelled mid-run would leave an unverified version live.
    expect(workflow).toMatch(/cancel-in-progress: false/);
  });

  it("passes only flags the smoke CLI still accepts", () => {
    // `--require-turn` was removed when the TURN check became an
    // authorization-refusal check; a stale interpolation would make every
    // production smoke run fail on an unknown argument. Read the accepted
    // flags straight out of the parser rather than restating them, so this
    // guard cannot itself go stale. cli.ts is read as text, never imported:
    // importing it would run a smoke test.
    const cli = readFileSync(fileURLToPath(new URL("../src/cli.ts", import.meta.url)), "utf8");
    const accepted = [...cli.matchAll(/case "(--?[a-z-]+)":/g)].map((match) => match[1]);
    expect(accepted).toContain("--release");
    expect(accepted).not.toContain("--require-turn");

    const invocations = [...workflow.matchAll(/npm run smoke(?:[^\n]|\\\n)*/g)];
    expect(invocations.length).toBeGreaterThan(0);
    for (const invocation of invocations) {
      for (const flag of invocation[0].matchAll(/\s(--[a-z-]+)/g)) {
        expect(accepted, `${flag[1]} is not accepted by the smoke CLI`).toContain(flag[1]);
      }
    }
    expect(workflow).not.toContain("require_turn");
  });

  it("forwards the deployed-version precondition to the deploy script", () => {
    expect(workflow).toContain("GENERALSX_EXPECTED_RELEASE_ID");
    expect(workflow).toContain("expected_release_id");
  });
});
