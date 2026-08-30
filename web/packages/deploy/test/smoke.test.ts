import { describe, expect, it } from "vitest";
import { buildSecurityHeaders } from "@generalsx-web/shared/security-headers";
import { DEFAULT_RETRY_POLICY, NO_RETRY_POLICY, type RetryPolicy } from "../src/retry.ts";
import {
  formatSmokeReport,
  optionsFromTarget,
  runSmokeChecks,
  type SmokeOptions,
  type SmokeReport,
} from "../src/smoke.ts";
import { PRODUCTION_TARGET } from "../src/targets.ts";

type FetchLike = (input: string, init?: RequestInit) => Promise<Response>;

/** Most cases assert *check* behaviour, not propagation behaviour, so they run
 * with retrying disabled; the retry describe block below drives it directly. */
function run(options: SmokeOptions, fetchImpl: FetchLike): Promise<SmokeReport> {
  return runSmokeChecks(options, { fetchImpl, retryPolicy: NO_RETRY_POLICY });
}

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
  const sentHeaders: Record<string, string>[] = [];
  const doFetch = async (input: string, init?: RequestInit): Promise<Response> => {
    const key = `${init?.method ?? "GET"} ${input}`;
    calls.push(key);
    sentHeaders.push((init?.headers as Record<string, string> | undefined) ?? {});
    const route = routes[key];
    if (!route) throw new Error(`unexpected request: ${key}`);
    const resolved = typeof route === "function" ? route(init) : route;
    const status = resolved.status ?? 200;
    // 204/205/304 must be constructed with a null body.
    const body = status === 204 || status === 205 || status === 304 ? null : (resolved.body ?? "");
    return new Response(body, { status, headers: resolved.headers ?? {} });
  };
  return { doFetch, calls, sentHeaders };
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
    // A correctly deployed Worker refuses both an anonymous request and a
    // forged bearer token, whatever Origin they claim.
    [`GET ${signaling}/turn-credentials`]: {
      status: 401,
      headers: { "Content-Type": "application/json", "WWW-Authenticate": 'Bearer realm="generalsx-room"' },
      body: JSON.stringify({ error: "room admission token required", code: "UNAUTHORIZED" }),
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
    const report = await run(OPTIONS, doFetch);
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
    const report = await run(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    const result = resultFor(report, "launcher-security-headers");
    expect(result.outcome).toBe("fail");
    expect(result.detail).toContain("Cross-Origin-Embedder-Policy");
  });

  it("fails when the deployment does not serve the expected release id", async () => {
    const { doFetch } = stubFetch(healthyRoutes());
    const report = await run({ ...OPTIONS, expectReleaseId: "sha-2" }, doFetch);
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
    const report = await run(OPTIONS, doFetch);
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
    const report = await run(OPTIONS, doFetch);
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
    const report = await run(OPTIONS, doFetch);
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
    const report = await run(withoutAssets, doFetch);
    expect(resultFor(report, "asset-origin-delivery").outcome).toBe("skip");
    expect(calls.some((call) => call.includes(PRODUCTION_TARGET.assetOrigin))).toBe(false);
    expect(report.passed).toBe(true);
  });

  it("passes when TURN credentials are refused without a room admission token", async () => {
    const { doFetch } = stubFetch(healthyRoutes());
    const report = await run(OPTIONS, doFetch);
    expect(resultFor(report, "worker-turn-authorization").outcome).toBe("pass");
  });

  it("fails when TURN credentials are publicly mintable", async () => {
    const signaling = PRODUCTION_TARGET.signalingOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({
        [`GET ${signaling}/turn-credentials`]: {
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ iceServers: [{ urls: ["turn:example"] }], ttlSeconds: 600 }),
        },
      }),
    );
    const report = await run(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    const result = resultFor(report, "worker-turn-authorization");
    expect(result.outcome).toBe("fail");
    expect(result.detail).toContain("expected 401");
  });

  it("still fails when only a forged token is accepted, since CORS never protects a non-browser caller", async () => {
    const signaling = PRODUCTION_TARGET.signalingOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({
        // Anonymous is refused, but any bearer string is honoured — a signature
        // that is never actually verified.
        [`GET ${signaling}/turn-credentials`]: (init) => {
          const headers = init?.headers as Record<string, string> | undefined;
          return headers?.Authorization
            ? { body: JSON.stringify({ iceServers: [], ttlSeconds: 600 }) }
            : { status: 401, headers: { "WWW-Authenticate": "Bearer" } };
        },
      }),
    );
    const report = await run(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "worker-turn-authorization").detail).toContain("forged admission token");
  });

  it("fails a 401 that omits WWW-Authenticate", async () => {
    const signaling = PRODUCTION_TARGET.signalingOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({ [`GET ${signaling}/turn-credentials`]: { status: 401 } }),
    );
    const report = await run(OPTIONS, doFetch);
    expect(resultFor(report, "worker-turn-authorization").detail).toContain("WWW-Authenticate");
  });

  it("carries no real credential, so it can never cause one to be issued", async () => {
    const { doFetch, calls, sentHeaders } = stubFetch(healthyRoutes());
    await run(OPTIONS, doFetch);
    const bearers = calls
      .map((call, index) => ({ call, auth: sentHeaders[index]?.Authorization }))
      .filter((entry) => entry.auth !== undefined);
    // The only token the suite ever sends is the deliberately invalid probe.
    expect(bearers.map((entry) => entry.auth)).toEqual(["Bearer gxa1.not.a.real.token"]);
  });

  it("turns an unreachable origin into a failed check, not an exception", async () => {
    const doFetch = async (): Promise<Response> => {
      throw new Error("ENOTFOUND");
    };
    const report = await run(OPTIONS, doFetch);
    expect(report.passed).toBe(false);
    expect(resultFor(report, "launcher-reachable").detail).toContain("ENOTFOUND");
  });
});

describe("propagation retry", () => {
  /** Collects the backoff schedule instead of waiting it out, and advances a
   * virtual clock so budget arithmetic is exercised deterministically. */
  function fakeClock() {
    const waits: number[] = [];
    let current = 0;
    return {
      waits,
      now: () => current,
      sleep: (ms: number) => {
        waits.push(ms);
        current += ms;
        return Promise.resolve();
      },
    };
  }

  function flakyThen(routes: Record<string, StubRoute>, failures: number, failure: () => never | StubResponse) {
    let remaining = failures;
    const base = stubFetch(routes);
    const doFetch: FetchLike = (input, init) => {
      if (remaining > 0) {
        remaining -= 1;
        const result = failure();
        return Promise.resolve(new Response("", { status: result.status ?? 503 }));
      }
      return base.doFetch(input, init);
    };
    return { doFetch, calls: base.calls };
  }

  it("recovers once DNS propagates, with exponential backoff", async () => {
    const clock = fakeClock();
    let remaining = 2;
    const base = stubFetch(healthyRoutes());
    const doFetch: FetchLike = (input, init) => {
      if (remaining > 0) {
        remaining -= 1;
        return Promise.reject(Object.assign(new Error("fetch failed"), { cause: new Error("getaddrinfo ENOTFOUND") }));
      }
      return base.doFetch(input, init);
    };
    const report = await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    expect(report.passed).toBe(true);
    expect(clock.waits).toEqual([DEFAULT_RETRY_POLICY.initialDelayMs, DEFAULT_RETRY_POLICY.initialDelayMs * 2]);
  });

  it("retries a Cloudflare edge status while a custom domain attaches", async () => {
    const clock = fakeClock();
    const { doFetch } = flakyThen(healthyRoutes(), 1, () => ({ status: 525 }));
    const report = await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    expect(report.passed).toBe(true);
    expect(clock.waits).toHaveLength(1);
  });

  it("does not retry a wrong-but-answered response, so a real defect fails fast", async () => {
    const clock = fakeClock();
    const launcher = PRODUCTION_TARGET.launcherOrigin;
    const { doFetch } = stubFetch(
      healthyRoutes({ [`GET ${launcher}/health.json`]: { status: 404 } }),
    );
    const report = await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    expect(report.passed).toBe(false);
    expect(clock.waits).toEqual([]);
  });

  it("does not retry an assertion failure, only a transport-shaped one", async () => {
    const clock = fakeClock();
    const launcher = PRODUCTION_TARGET.launcherOrigin;
    const { doFetch, calls } = stubFetch(
      healthyRoutes({
        [`GET ${launcher}/`]: {
          headers: { ...SECURITY_HEADERS, "Cross-Origin-Embedder-Policy": "unsafe-none", "Content-Type": "text/html" },
          body: INDEX_HTML,
        },
      }),
    );
    const report = await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    expect(resultFor(report, "launcher-security-headers").outcome).toBe("fail");
    expect(clock.waits).toEqual([]);
    // Exactly one request per check that needs it — never a retried assertion.
    expect(calls.filter((call) => call === `GET ${launcher}/`)).toHaveLength(3);
  });

  it("gives up with an actionable, operator-facing timeout error", async () => {
    const clock = fakeClock();
    const doFetch: FetchLike = () =>
      Promise.reject(Object.assign(new Error("fetch failed"), { cause: new Error("getaddrinfo ENOTFOUND play.generalsx.org") }));
    const report = await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    expect(report.passed).toBe(false);
    const detail = resultFor(report, "launcher-reachable").detail;
    expect(detail).toContain(PRODUCTION_TARGET.launcherOrigin);
    expect(detail).toContain(`${DEFAULT_RETRY_POLICY.attempts} attempt(s)`);
    expect(detail).toContain("propagation may still be in progress");
    expect(detail).toContain("getaddrinfo ENOTFOUND");
    expect(detail).toContain("dig play.generalsx.org");
  });

  it("never exceeds the total waiting budget, however many checks fail", async () => {
    const clock = fakeClock();
    const doFetch: FetchLike = () => Promise.reject(new Error("ENOTFOUND"));
    await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    const waited = clock.waits.reduce((sum, wait) => sum + wait, 0);
    expect(waited).toBeLessThanOrEqual(DEFAULT_RETRY_POLICY.totalBudgetMs);
  });

  it("waits on an unreachable origin once, not once per check that touches it", async () => {
    const clock = fakeClock();
    const attempted: string[] = [];
    const doFetch: FetchLike = (input) => {
      attempted.push(input);
      return Promise.reject(new Error("ENOTFOUND"));
    };
    await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    // Four separate checks probe the launcher origin, but once it has burned
    // its attempts the verdict is cached: no check re-runs the backoff.
    const launcherAttempts = attempted.filter((url) => url.startsWith(PRODUCTION_TARGET.launcherOrigin));
    expect(launcherAttempts).toHaveLength(DEFAULT_RETRY_POLICY.attempts);
  });

  it("shares one budget across origins, so a total outage still terminates promptly", async () => {
    const clock = fakeClock();
    const doFetch: FetchLike = () => Promise.reject(new Error("ENOTFOUND"));
    await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    // The first origins consume most of the budget, so later ones get fewer
    // waits — the run as a whole never exceeds the ceiling.
    expect(clock.waits.reduce((sum, wait) => sum + wait, 0)).toBe(DEFAULT_RETRY_POLICY.totalBudgetMs);
    expect(clock.waits.length).toBeLessThanOrEqual((DEFAULT_RETRY_POLICY.attempts - 1) * 3);
  });

  it("stops retrying an origin that has already answered once", async () => {
    const clock = fakeClock();
    const signaling = PRODUCTION_TARGET.signalingOrigin;
    let readyzCalls = 0;
    const base = stubFetch(healthyRoutes());
    const doFetch: FetchLike = (input, init) => {
      if (input === `${signaling}/readyz`) {
        readyzCalls += 1;
        return Promise.resolve(new Response("", { status: 503 }));
      }
      return base.doFetch(input, init);
    };
    const report = await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, ...clock });
    // `/healthz` already proved the origin resolves, so a 503 from `/readyz` is
    // a genuine not-ready signal and must be reported immediately.
    expect(readyzCalls).toBe(1);
    expect(resultFor(report, "worker-readiness").outcome).toBe("fail");
  });

  it("honours a disabled retry policy", async () => {
    const clock = fakeClock();
    const doFetch: FetchLike = () => Promise.reject(new Error("ENOTFOUND"));
    await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, retryPolicy: NO_RETRY_POLICY, ...clock });
    expect(clock.waits).toEqual([]);
  });

  it("respects a caller-supplied budget shorter than the backoff schedule", async () => {
    const clock = fakeClock();
    const policy: RetryPolicy = { attempts: 10, initialDelayMs: 4_000, maxDelayMs: 4_000, totalBudgetMs: 5_000 };
    const doFetch: FetchLike = () => Promise.reject(new Error("ENOTFOUND"));
    await runSmokeChecks(OPTIONS, { fetchImpl: doFetch, retryPolicy: policy, ...clock });
    expect(clock.waits.reduce((sum, wait) => sum + wait, 0)).toBe(5_000);
  });
});

describe("formatSmokeReport", () => {
  it("summarises pass/fail/skip counts", async () => {
    const { doFetch } = stubFetch(
      healthyRoutes(
        {},
        buildSecurityHeaders({
          allowedOrigins: [PRODUCTION_TARGET.launcherOrigin],
          signalingOrigins: [PRODUCTION_TARGET.signalingOrigin],
        }),
      ),
    );
    const text = formatSmokeReport(await run({ ...OPTIONS, assetOrigin: undefined }, doFetch));
    expect(text).toContain("PASS");
    expect(text).toContain("SKIP");
    expect(text).toMatch(/\d+ passed, 0 failed, \d+ skipped/);
  });
});
