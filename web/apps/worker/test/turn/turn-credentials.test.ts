import { describe, expect, it, vi } from "vitest";
import {
  DEFAULT_TURN_TTL_SECONDS,
  MAX_TURN_TTL_SECONDS,
  MIN_TURN_TTL_SECONDS,
  buildTurnCredentialsRequest,
  fetchTurnCredentials,
  normalizeTtlSeconds,
} from "../../src/turn/turn-credentials.js";

describe("normalizeTtlSeconds", () => {
  it("defaults when unset", () => {
    expect(normalizeTtlSeconds(undefined)).toBe(DEFAULT_TURN_TTL_SECONDS);
  });

  it("clamps to the supported range", () => {
    expect(normalizeTtlSeconds(1)).toBe(MIN_TURN_TTL_SECONDS);
    expect(normalizeTtlSeconds(999999)).toBe(MAX_TURN_TTL_SECONDS);
  });

  it("rejects NaN by falling back to the default", () => {
    expect(normalizeTtlSeconds(Number.NaN)).toBe(DEFAULT_TURN_TTL_SECONDS);
  });
});

describe("buildTurnCredentialsRequest", () => {
  const env = { TURN_KEY_ID: "key-123", TURN_KEY_API_TOKEN: "super-secret-token" };

  it("targets the Cloudflare Realtime TURN endpoint for the configured key id", () => {
    const { url } = buildTurnCredentialsRequest(env, 600);
    expect(url).toBe("https://rtc.live.cloudflare.com/v1/turn/keys/key-123/credentials/generate-ice-servers");
  });

  it("sends the ttl in the JSON body", () => {
    const { init } = buildTurnCredentialsRequest(env, 300);
    expect(JSON.parse(init.body)).toEqual({ ttl: 300 });
  });

  it("never leaks the API token into the URL", () => {
    const { url } = buildTurnCredentialsRequest(env, 300);
    expect(url).not.toContain(env.TURN_KEY_API_TOKEN);
  });

  it("only sends the API token in the Authorization header", () => {
    const { init } = buildTurnCredentialsRequest(env, 300);
    expect(init.headers.Authorization).toBe(`Bearer ${env.TURN_KEY_API_TOKEN}`);
  });
});

describe("fetchTurnCredentials", () => {
  it("returns a configuration error when secrets are missing", async () => {
    const outcome = await fetchTurnCredentials({});
    expect(outcome.ok).toBe(false);
  });

  it("returns iceServers on a successful upstream response", async () => {
    const fetchImpl = vi.fn(async () =>
      new Response(JSON.stringify({ iceServers: [{ urls: ["stun:stun.cloudflare.com:3478"] }] }), {
        status: 201,
      }),
    );
    const outcome = await fetchTurnCredentials(
      { TURN_KEY_ID: "key-123", TURN_KEY_API_TOKEN: "secret" },
      { fetchImpl: fetchImpl as unknown as typeof fetch, ttlSeconds: 120 },
    );
    expect(outcome.ok).toBe(true);
    if (outcome.ok) {
      expect(outcome.iceServers).toEqual([{ urls: ["stun:stun.cloudflare.com:3478"] }]);
      expect(outcome.ttlSeconds).toBe(120);
    }
    expect(fetchImpl).toHaveBeenCalledTimes(1);
  });

  it("surfaces an upstream error status without leaking internals", async () => {
    const fetchImpl = vi.fn(async () => new Response("nope", { status: 401 }));
    const outcome = await fetchTurnCredentials(
      { TURN_KEY_ID: "key-123", TURN_KEY_API_TOKEN: "secret" },
      { fetchImpl: fetchImpl as unknown as typeof fetch },
    );
    expect(outcome.ok).toBe(false);
    if (!outcome.ok) {
      expect(outcome.message).not.toContain("secret");
    }
  });

  it("handles a network failure gracefully", async () => {
    const fetchImpl = vi.fn(async () => {
      throw new Error("network down");
    });
    const outcome = await fetchTurnCredentials(
      { TURN_KEY_ID: "key-123", TURN_KEY_API_TOKEN: "secret" },
      { fetchImpl: fetchImpl as unknown as typeof fetch },
    );
    expect(outcome.ok).toBe(false);
  });

  it("handles a malformed upstream response body", async () => {
    const fetchImpl = vi.fn(async () => new Response(JSON.stringify({ notIceServers: true }), { status: 201 }));
    const outcome = await fetchTurnCredentials(
      { TURN_KEY_ID: "key-123", TURN_KEY_API_TOKEN: "secret" },
      { fetchImpl: fetchImpl as unknown as typeof fetch },
    );
    expect(outcome.ok).toBe(false);
  });
});
