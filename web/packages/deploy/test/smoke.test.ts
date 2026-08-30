import { describe, expect, it } from "vitest";
import { buildSecurityHeaders } from "@generalsx-web/shared/security-headers";
import {
  formatSmokeReport,
  optionsFromTarget,
  runSmokeChecks,
  type SmokeOptions,
  type SmokeReport,
} from "../src/smoke.ts";
import { PRODUCTION_TARGET } from "../src/targets.ts";

const OPTIONS: SmokeOptions = optionsFromTarget(PRODUCTION_TARGET);

const SECURITY_HEADERS = buildSecurityHeaders({
  allowedOrigins: [PRODUCTION_TARGET.launcherOrigin],
  signalingOrigins: [PRODUCTION_TARGET.signalingOrigin],
  assetOrigins: [PRODUCTION_TARGET.assetOrigin],
});

const INDEX_HTML = '<!doctype html><script type="module" src="/assets/main-abc123.js"></script>';

interface StubResponse {
  readonly status?: number;
  readonly headers?: Record<string, string>;
  readonly body?: string;
}

/** A canned response, or one computed from the request (e.g. origin-dependent
 * CORS reflection). */
type StubRoute = StubResponse | ((init?: RequestInit) => StubResponse);

function requestOrigin(init?: RequestInit): string | undefined {
  const headers = init?.headers as Record<string, string> | undefined;
  return headers?.Origin;
}

/** Builds a fetch double from a `${method} ${url}`-keyed routing table. */
function stubFetch(routes: Record<string, StubRoute>) {
  const calls: string[] = [];
  const doFetch = async (input: string, init?: RequestInit): Promise<Response> => {
    const key = `${init?.method ?? "GET"} ${input}`;
    calls.push(key);
    const route = routes[key];
    if (!route) throw new Error(`unexpected request: ${key}`);
    const resolved = typeof route === "function" ? route(init) : route;
    const status = resolved.status ?? 200;
    // 204/205/304 must be constructed with a null body.
    const body = status === 204 || status === 205 || status === 304 ? null : (resolved.body ?? "");
    return new Response(body, { status, headers: resolved.headers ?? {} });
  };
  return { doFetch, calls };
}

function healthyRoutes(
  overrides: Record<string, StubRoute> = {},
  securityHeaders: Record<string, string> = SECURITY_HEADERS,
): Record<string, StubRoute> {
  const launcher = PRODUCTION_TARGET.launcherOrigin;
  const signaling = PRODUCTION_TARGET.signalingOrigin;
  const assets = PRODUCTION_TARGET.assetOrigin;
  return {
    [`GET ${launcher}/`]: {
      headers: { ...securityHeaders, "Content-Type": "text/html; charset=utf-8" },
      body: INDEX_HTML,
    },
    [`GET ${launcher}/health.json`]: {
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ service: "generalsx-launcher", status: "ok", releaseId: "sha-1" }),
    },
    [`HEAD ${launcher}/assets/main-abc123.js`]: {
      headers: { "Cache-Control": "public, max-age=31536000, immutable" },
    },
    [`GET ${signaling}/healthz`]: {
      headers: { ...securityHeaders, "Content-Type": "application/json" },
      body: JSON.stringify({ service: "generalsx-signaling", status: "ok", releaseId: "sha-1" }),
    },
    [`GET ${signaling}/readyz`]: {
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        service: "generalsx-signaling",
        ready: true,
        status: "ok",
        releaseId: "sha-1",
        checks: [{ name: "turn-credentials", status: "ok", detail: "configured" }],
      }),
    },
    [`OPTIONS ${signaling}/turn-credentials`]: (init) => ({
      status: 204,
      // Mirrors the Worker: reflect the allowlisted origin, nothing else.
      headers: requestOrigin(init) === launcher ? { "Access-Control-Allow-Origin": launcher } : {},
    }),
    [`GET ${signaling}/turn-credentials`]: {
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ iceServers: [{ urls: ["turn:example"] }], ttlSeconds: 600 }),
    },
    [`GET ${assets}/manifest.json`]: {
      status: 206,
      headers: {
        "Content-Range": "bytes 0-0/1024",
        "Accept-Ranges": "bytes",
        ETag: '"strong-etag"',
        "Cache-Control": "public, max-age=31536000, immutable",
        "Access-Control-Allow-Origin": launcher,
        "Access-Control-Expose-Headers": "ETag, Content-Range, Content-Length",
      },
      body: "x",
    },
    ...overrides,
  };
}

function resultFor(report: SmokeReport, name: string) {
  const result = report.results.find((entry) => entry.name === name);
  if (!result) throw new Error(`no smoke result named ${name}`);
  return result;
}

describe("runSmokeChecks", () => {
  it("passes against a correctly configured deployment", async () => {
    const { doFetch } = stubFetch(healthyRoutes());
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(report.passed).toBe(true);
    expect(resultFor(report, "launcher-reachable").outcome).toBe("pass");
    expect(resultFor(report, "launcher-security-headers").outcome).toBe("pass");
    expect(resultFor(report, "launcher-immutable-assets").outcome).toBe("pass");
    expect(resultFor(report, "worker-readiness").outcome).toBe("pass");
    expect(resultFor(report, "asset-origin-delivery").outcome).toBe("pass");
  });

  it("fails when a security header drifts from the shared policy", async () => {
    const launcher = PRODUCTION_TARGET.launcherOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({
        [`GET ${launcher}/`]: {
          headers: {
            ...SECURITY_HEADERS,
            "Cross-Origin-Embedder-Policy": "unsafe-none",
            "Content-Type": "text/html",
          },
          body: INDEX_HTML,
        },
      }),
    );
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    const result = resultFor(report, "launcher-security-headers");
    expect(result.outcome).toBe("fail");
    expect(result.detail).toContain("Cross-Origin-Embedder-Policy");
  });

  it("fails when the deployment does not serve the expected release id", async () => {
    const { doFetch } = stubFetch(healthyRoutes());
    const report = await runSmokeChecks({ ...OPTIONS, expectReleaseId: "sha-2" }, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "launcher-health").detail).toContain("sha-1");
    expect(resultFor(report, "worker-readiness").detail).toContain("sha-1");
  });

  it("fails when readiness reports a failed check", async () => {
    const signaling = PRODUCTION_TARGET.signalingOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({
        [`GET ${signaling}/readyz`]: {
          status: 503,
          body: JSON.stringify({
            ready: false,
            status: "failed",
            releaseId: "sha-1",
            checks: [
              { name: "room-durable-object", status: "failed", detail: "ROOM_DO binding missing" },
            ],
          }),
        },
      }),
    );
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "worker-readiness").detail).toContain("ROOM_DO binding missing");
  });

  it("fails when CORS reflects a disallowed origin", async () => {
    const signaling = PRODUCTION_TARGET.signalingOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({
        [`OPTIONS ${signaling}/turn-credentials`]: {
          status: 204,
          headers: { "Access-Control-Allow-Origin": "*" },
        },
      }),
    );
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "worker-cors-allowlist").outcome).toBe("fail");
  });

  it("rejects a weak ETag on the asset origin", async () => {
    const assets = PRODUCTION_TARGET.assetOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({
        [`GET ${assets}/manifest.json`]: {
          status: 206,
          headers: {
            "Content-Range": "bytes 0-0/1024",
            "Accept-Ranges": "bytes",
            ETag: 'W/"weak-etag"',
            "Cache-Control": "public, max-age=31536000, immutable",
            "Access-Control-Allow-Origin": PRODUCTION_TARGET.launcherOrigin,
            "Access-Control-Expose-Headers": "ETag, Content-Range, Content-Length",
          },
          body: "x",
        },
      }),
    );
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "asset-origin-delivery").detail).toContain("weak ETag");
  });

  it("skips asset checks when no asset origin is configured", async () => {
    const withoutAssets: SmokeOptions = { ...OPTIONS, assetOrigin: undefined };
    const { doFetch, calls } = stubFetch(
      healthyRoutes(
        {},
        buildSecurityHeaders({
          allowedOrigins: [PRODUCTION_TARGET.launcherOrigin],
          signalingOrigins: [PRODUCTION_TARGET.signalingOrigin],
        }),
      ),
    );
    const report = await runSmokeChecks(withoutAssets, doFetch);
    expect(resultFor(report, "asset-origin-delivery").outcome).toBe("skip");
    expect(calls.some((call) => call.includes(PRODUCTION_TARGET.assetOrigin))).toBe(false);
    expect(report.passed).toBe(true);
  });

  it("skips the live TURN check unless it is explicitly required", async () => {
    const { doFetch, calls } = stubFetch(healthyRoutes());
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(resultFor(report, "worker-turn-credentials").outcome).toBe("skip");
    expect(calls).not.toContain(`GET ${PRODUCTION_TARGET.signalingOrigin}/turn-credentials`);
  });

  it("issues and shape-checks a live credential with requireTurn", async () => {
    const { doFetch } = stubFetch(healthyRoutes());
    const report = await runSmokeChecks({ ...OPTIONS, requireTurn: true }, doFetch);
    const result = resultFor(report, "worker-turn-credentials");
    expect(result.outcome).toBe("pass");
    expect(result.detail).not.toContain("turn:example");
  });

  it("turns an unreachable origin into a failed check, not an exception", async () => {
    const doFetch = async (): Promise<Response> => {
      throw new Error("ENOTFOUND");
    };
    const report = await runSmokeChecks(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "launcher-reachable").detail).toContain("ENOTFOUND");
  });
});

describe("formatSmokeReport", () => {
  it("summarises pass/fail/skip counts", async () => {
    const { doFetch } = stubFetch(healthyRoutes());
    const text = formatSmokeReport(await runSmokeChecks(OPTIONS, doFetch));
    expect(text).toContain("PASS");
    expect(text).toContain("SKIP");
    expect(text).toMatch(/\d+ passed, 0 failed, \d+ skipped/);
  });
});
