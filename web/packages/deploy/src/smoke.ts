/**
 * Post-deploy smoke tests for the production browser deployment.
 *
 * Every check is expressed against the *public* surface of a deployment
 * (`https://play.generalsx.org`, `https://signaling.generalsx.org`, and the
 * authorized asset origin), so the same suite validates a fresh deploy, a
 * promoted Pages alias, and a `wrangler rollback`.
 *
 * Design constraints:
 *  - `fetchImpl` is injectable, so the whole suite is unit tested offline.
 *  - Expected security headers are derived from `@generalsx-web/shared`, the
 *    same module that *produces* them, so a policy change can never pass the
 *    smoke test by accident.
 *  - No check sends, requests, or logs a credential. The TURN check asserts
 *    that credentials are *refused* without a room admission token, so it
 *    never causes one to be issued.
 *  - No check downloads a retail asset: the asset-origin probe requests a
 *    single byte (`Range: bytes=0-0`) purely to verify delivery semantics.
 *  - Probes retry propagation-shaped failures with bounded backoff (see
 *    `retry.ts`), so a just-attached custom domain is not reported as broken
 *    while its DNS record and certificate are still rolling out.
 */
import { buildSecurityHeaders } from "@generalsx-web/shared/security-headers";
import {
  createRetryingFetch,
  DEFAULT_RETRY_POLICY,
  PropagationTimeoutError,
  type RetryDeps,
  type RetryPolicy,
} from "./retry.ts";
import { PRODUCTION_TARGET, type DeploymentTarget } from "./targets.ts";

export interface SmokeOptions {
  readonly launcherOrigin: string;
  readonly signalingOrigin: string;
  /** Omit when the deployment has no configured asset origin. */
  readonly assetOrigin?: string | undefined;
  /** Disable only the delivery probe while retaining the configured asset
   * origin in the expected CSP (e.g. before assets have been published). */
  readonly checkAssetOrigin?: boolean | undefined;
  /** Object probed on the asset origin. Must already exist there. */
  readonly assetProbePath?: string | undefined;
  /** Assert the deployment reports this commit SHA — the rollback check. */
  readonly expectReleaseId?: string | undefined;
}

export type SmokeOutcome = "pass" | "fail" | "skip";

export interface SmokeResult {
  readonly name: string;
  readonly outcome: SmokeOutcome;
  readonly detail: string;
}

export interface SmokeReport {
  readonly passed: boolean;
  readonly results: readonly SmokeResult[];
}

type FetchLike = (input: string, init?: RequestInit) => Promise<Response>;

/** Injectable seams. `fetchImpl` keeps the suite unit testable offline;
 * `sleep`/`now` let the retry tests run instantly instead of waiting out a
 * real backoff schedule. */
export interface SmokeDeps extends RetryDeps {
  readonly fetchImpl?: FetchLike | undefined;
  /** Defaults to {@link DEFAULT_RETRY_POLICY}; pass `NO_RETRY_POLICY` to
   * probe a deployment already known to be live. */
  readonly retryPolicy?: RetryPolicy | undefined;
}

const DEFAULT_ASSET_PROBE_PATH = "/manifest.json";
const DISALLOWED_ORIGIN = "https://disallowed.invalid";

export function optionsFromTarget(
  target: DeploymentTarget = PRODUCTION_TARGET,
  overrides: Partial<SmokeOptions> = {},
): SmokeOptions {
  return {
    launcherOrigin: target.launcherOrigin,
    signalingOrigin: target.signalingOrigin,
    assetOrigin: target.assetOrigin,
    ...overrides,
  };
}

function pass(name: string, detail: string): SmokeResult {
  return { name, outcome: "pass", detail };
}

function fail(name: string, detail: string): SmokeResult {
  return { name, outcome: "fail", detail };
}

function skip(name: string, detail: string): SmokeResult {
  return { name, outcome: "skip", detail };
}

function trimOrigin(origin: string): string {
  return origin.replace(/\/+$/, "");
}

/** Runs `body`, converting a transport error into a failed check rather than a
 * thrown exception, so one unreachable origin cannot hide the other results.
 *
 * A `PropagationTimeoutError` already carries the operator-facing summary
 * (origin, attempts, elapsed, underlying failure), so it is reported verbatim
 * rather than being re-wrapped into something less specific. */
async function guard(name: string, body: () => Promise<SmokeResult>): Promise<SmokeResult> {
  try {
    return await body();
  } catch (error) {
    if (error instanceof PropagationTimeoutError) return fail(name, error.message);
    return fail(name, `request failed: ${error instanceof Error ? error.message : String(error)}`);
  }
}

/** Compares the security headers a live origin returned against the policy the
 * shared module defines for this deployment. */
function compareSecurityHeaders(
  response: Response,
  options: SmokeOptions,
): { readonly ok: boolean; readonly detail: string } {
  const expected = buildSecurityHeaders({
    allowedOrigins: [options.launcherOrigin],
    signalingOrigins: [options.signalingOrigin],
    ...(options.assetOrigin ? { assetOrigins: [options.assetOrigin] } : {}),
  });
  const mismatches: string[] = [];
  for (const [name, value] of Object.entries(expected)) {
    const actual = response.headers.get(name);
    if (actual === null) mismatches.push(`${name}: missing`);
    else if (actual !== value) mismatches.push(`${name}: got "${actual}"`);
  }
  return mismatches.length === 0
    ? { ok: true, detail: `${Object.keys(expected).length} headers match the shared policy` }
    : { ok: false, detail: mismatches.join("; ") };
}

async function checkLauncherReachable(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "launcher-reachable";
  return guard(name, async () => {
    const response = await doFetch(`${trimOrigin(options.launcherOrigin)}/`);
    if (response.status !== 200) return fail(name, `expected 200, got ${response.status}`);
    const contentType = response.headers.get("Content-Type") ?? "";
    if (!contentType.includes("text/html")) {
      return fail(name, `expected an HTML document, got Content-Type "${contentType}"`);
    }
    return pass(name, `${options.launcherOrigin} served the launcher shell`);
  });
}

async function checkLauncherSecurityHeaders(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "launcher-security-headers";
  return guard(name, async () => {
    const response = await doFetch(`${trimOrigin(options.launcherOrigin)}/`);
    const comparison = compareSecurityHeaders(response, options);
    return comparison.ok ? pass(name, comparison.detail) : fail(name, comparison.detail);
  });
}

async function checkLauncherHealth(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "launcher-health";
  return guard(name, async () => {
    const response = await doFetch(`${trimOrigin(options.launcherOrigin)}/health.json`);
    if (response.status !== 200) return fail(name, `expected 200, got ${response.status}`);
    const body = (await response.json()) as { service?: unknown; releaseId?: unknown };
    if (body.service !== "generalsx-launcher") {
      return fail(name, `unexpected service field: ${JSON.stringify(body.service)}`);
    }
    const releaseId = typeof body.releaseId === "string" ? body.releaseId : "";
    if (options.expectReleaseId && releaseId !== options.expectReleaseId) {
      return fail(name, `serving releaseId "${releaseId}", expected "${options.expectReleaseId}"`);
    }
    return pass(name, `launcher healthy at releaseId "${releaseId}"`);
  });
}

/** Vite emits content-hashed files under `/assets/`; they must be served with
 * a year-long immutable cache or every boot re-downloads the whole shell. */
async function checkLauncherImmutableAssets(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "launcher-immutable-assets";
  return guard(name, async () => {
    const indexResponse = await doFetch(`${trimOrigin(options.launcherOrigin)}/`);
    const html = await indexResponse.text();
    const match = /\/assets\/[A-Za-z0-9._-]+\.(?:js|css)/.exec(html);
    if (!match) return skip(name, "no hashed /assets/ reference found in index.html");
    const assetUrl = `${trimOrigin(options.launcherOrigin)}${match[0]}`;
    const assetResponse = await doFetch(assetUrl, { method: "HEAD" });
    if (assetResponse.status !== 200) {
      return fail(name, `${match[0]} returned ${assetResponse.status}`);
    }
    const cacheControl = assetResponse.headers.get("Cache-Control") ?? "";
    if (!cacheControl.includes("immutable")) {
      return fail(name, `${match[0]} Cache-Control is "${cacheControl}", expected immutable`);
    }
    return pass(name, `${match[0]} served with "${cacheControl}"`);
  });
}

async function checkWorkerLiveness(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "worker-liveness";
  return guard(name, async () => {
    const response = await doFetch(`${trimOrigin(options.signalingOrigin)}/healthz`);
    if (response.status !== 200) return fail(name, `expected 200, got ${response.status}`);
    const body = (await response.json()) as { service?: unknown; status?: unknown };
    if (body.service !== "generalsx-signaling" || body.status !== "ok") {
      return fail(name, `unexpected liveness payload: ${JSON.stringify(body)}`);
    }
    return pass(name, "signaling worker is live");
  });
}

async function checkWorkerReadiness(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "worker-readiness";
  return guard(name, async () => {
    const response = await doFetch(`${trimOrigin(options.signalingOrigin)}/readyz`);
    const body = (await response.json()) as {
      ready?: unknown;
      status?: unknown;
      releaseId?: unknown;
      checks?: { name?: unknown; status?: unknown; detail?: unknown }[];
    };
    if (response.status !== 200 || body.ready !== true) {
      const failing = (body.checks ?? [])
        .filter((check) => check.status === "failed")
        .map((check) => `${String(check.name)}: ${String(check.detail)}`);
      return fail(
        name,
        `not ready (HTTP ${response.status})${failing.length ? ` — ${failing.join("; ")}` : ""}`,
      );
    }
    const releaseId = typeof body.releaseId === "string" ? body.releaseId : "";
    if (options.expectReleaseId && releaseId !== options.expectReleaseId) {
      return fail(name, `serving releaseId "${releaseId}", expected "${options.expectReleaseId}"`);
    }
    const degraded = (body.checks ?? [])
      .filter((check) => check.status === "degraded")
      .map((check) => String(check.name));
    const suffix = degraded.length ? ` (degraded: ${degraded.join(", ")})` : "";
    return pass(name, `ready at releaseId "${releaseId}"${suffix}`);
  });
}

async function checkWorkerSecurityHeaders(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "worker-security-headers";
  return guard(name, async () => {
    const response = await doFetch(`${trimOrigin(options.signalingOrigin)}/healthz`);
    const comparison = compareSecurityHeaders(response, options);
    return comparison.ok ? pass(name, comparison.detail) : fail(name, comparison.detail);
  });
}

/** The Worker must reflect CORS for the launcher origin and *only* for it. */
async function checkWorkerCorsAllowlist(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "worker-cors-allowlist";
  return guard(name, async () => {
    const base = trimOrigin(options.signalingOrigin);
    const allowed = await doFetch(`${base}/turn-credentials`, {
      method: "OPTIONS",
      headers: { Origin: options.launcherOrigin },
    });
    const reflected = allowed.headers.get("Access-Control-Allow-Origin");
    if (reflected !== options.launcherOrigin) {
      return fail(name, `preflight reflected "${reflected}", expected "${options.launcherOrigin}"`);
    }
    const disallowed = await doFetch(`${base}/turn-credentials`, {
      method: "OPTIONS",
      headers: { Origin: DISALLOWED_ORIGIN },
    });
    const leaked = disallowed.headers.get("Access-Control-Allow-Origin");
    if (leaked !== null) {
      return fail(name, `preflight reflected a disallowed origin as "${leaked}"`);
    }
    return pass(name, "CORS reflects the launcher origin only");
  });
}

/**
 * Verifies TURN credentials are not publicly mintable.
 *
 * This is the one check that must run against the *deployed* Worker rather
 * than a unit test, because it is the only place that proves the live route
 * — not just the handler — refuses an anonymous caller. It deliberately
 * sends an allowed `Origin`: CORS is a browser-side read restriction that
 * curl and any server ignore, so a deployment that relied on it would pass a
 * browser test and still hand credentials to anyone. Presenting the allowed
 * origin and *still* being refused is the assertion that matters.
 *
 * No credential is ever sent or issued here, so this check is safe to run
 * against production on every deploy.
 */
async function checkTurnAuthorization(
  options: SmokeOptions,
  doFetch: FetchLike,
): Promise<SmokeResult> {
  const name = "worker-turn-authorization";
  return guard(name, async () => {
    const url = `${trimOrigin(options.signalingOrigin)}/turn-credentials`;
    const anonymous = await doFetch(url, { headers: { Origin: options.launcherOrigin } });
    if (anonymous.status !== 401) {
      return fail(
        name,
        `an unauthenticated request from an allowed origin got ${anonymous.status}, expected 401 — ` +
          "TURN credentials must require a room admission token",
      );
    }
    if (!anonymous.headers.get("WWW-Authenticate")) {
      return fail(name, "401 response omitted WWW-Authenticate");
    }
    const forged = await doFetch(url, {
      headers: { Origin: options.launcherOrigin, Authorization: "Bearer gxa1.not.a.real.token" },
    });
    if (forged.status !== 401) {
      return fail(name, `a forged admission token got ${forged.status}, expected 401`);
    }
    return pass(name, "credentials require a valid room admission token (anonymous and forged both refused)");
  });
}

/**
 * Verifies the authorized asset origin honours the delivery contract the
 * launcher's resumable downloader depends on: byte ranges, strong ETags,
 * immutable caching, and CORS that exposes the validators.
 */
async function checkAssetOrigin(options: SmokeOptions, doFetch: FetchLike): Promise<SmokeResult> {
  const name = "asset-origin-delivery";
  if (options.checkAssetOrigin === false) return skip(name, "asset-origin delivery check disabled");
  const assetOrigin = options.assetOrigin;
  if (!assetOrigin) return skip(name, "no asset origin configured");
  return guard(name, async () => {
    const probePath = options.assetProbePath ?? DEFAULT_ASSET_PROBE_PATH;
    const response = await doFetch(`${trimOrigin(assetOrigin)}${probePath}`, {
      headers: { Range: "bytes=0-0", Origin: options.launcherOrigin },
    });
    const problems: string[] = [];
    if (response.status !== 206) problems.push(`expected 206 for a range request, got ${response.status}`);
    if (!response.headers.get("Content-Range")) problems.push("missing Content-Range");
    if ((response.headers.get("Accept-Ranges") ?? "") !== "bytes") problems.push("Accept-Ranges is not bytes");
    const etag = response.headers.get("ETag") ?? "";
    if (!etag) problems.push("missing ETag");
    else if (etag.startsWith("W/")) problems.push("weak ETag disables resumable downloads");
    const cacheControl = response.headers.get("Cache-Control") ?? "";
    if (!cacheControl.includes("immutable")) problems.push(`Cache-Control "${cacheControl}" is not immutable`);
    const allowOrigin = response.headers.get("Access-Control-Allow-Origin");
    if (allowOrigin !== options.launcherOrigin) {
      problems.push(`Access-Control-Allow-Origin is "${allowOrigin}"`);
    }
    const exposed = (response.headers.get("Access-Control-Expose-Headers") ?? "").toLowerCase();
    for (const required of ["etag", "content-range", "content-length"]) {
      if (!exposed.includes(required)) problems.push(`Access-Control-Expose-Headers omits ${required}`);
    }
    return problems.length === 0
      ? pass(name, `${probePath} satisfies the resumable-delivery contract`)
      : fail(name, problems.join("; "));
  });
}

/** Runs every check and reports the aggregate result. */
export async function runSmokeChecks(
  options: SmokeOptions,
  deps: SmokeDeps = {},
): Promise<SmokeReport> {
  const baseFetch: FetchLike = deps.fetchImpl ?? ((input, init) => fetch(input, init));
  const doFetch = createRetryingFetch(baseFetch, deps.retryPolicy ?? DEFAULT_RETRY_POLICY, deps);
  const results: SmokeResult[] = [];
  for (const check of [
    checkLauncherReachable,
    checkLauncherSecurityHeaders,
    checkLauncherHealth,
    checkLauncherImmutableAssets,
    checkWorkerLiveness,
    checkWorkerReadiness,
    checkWorkerSecurityHeaders,
    checkWorkerCorsAllowlist,
    checkTurnAuthorization,
    checkAssetOrigin,
  ]) {
    results.push(await check(options, doFetch));
  }
  return { passed: results.every((result) => result.outcome !== "fail"), results };
}

/** Renders a report as a plain-text transcript for CI logs. */
export function formatSmokeReport(report: SmokeReport): string {
  const symbols: Record<SmokeOutcome, string> = { pass: "PASS", fail: "FAIL", skip: "SKIP" };
  const lines = report.results.map(
    (result) => `${symbols[result.outcome].padEnd(4)}  ${result.name.padEnd(28)}  ${result.detail}`,
  );
  const failed = report.results.filter((result) => result.outcome === "fail").length;
  const skipped = report.results.filter((result) => result.outcome === "skip").length;
  const passed = report.results.length - failed - skipped;
  lines.push("", `${passed} passed, ${failed} failed, ${skipped} skipped`);
  return lines.join("\n");
}
