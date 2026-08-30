import { describe, expect, it } from "vitest";
import { fetchIceServers, TurnCredentialsError } from "../../src/net/turn-client.js";

const BASE = "https://signaling.example.com";
const TOKEN = "gxa1.payload.signature";

interface Call {
  readonly url: string;
  readonly init: RequestInit | undefined;
}

function stubFetch(response: Response | (() => Response)): { impl: typeof fetch; calls: Call[] } {
  const calls: Call[] = [];
  const impl = ((url: string, init?: RequestInit) => {
    calls.push({ url, init });
    return Promise.resolve(typeof response === "function" ? response() : response);
  }) as unknown as typeof fetch;
  return { impl, calls };
}

function json(body: unknown, init: ResponseInit = {}): Response {
  return new Response(JSON.stringify(body), {
    status: 200,
    headers: { "Content-Type": "application/json" },
    ...init,
  });
}

describe("fetchIceServers", () => {
  it("presents the room admission token as a bearer credential", async () => {
    const { impl, calls } = stubFetch(json({ iceServers: [{ urls: "turn:example" }], ttlSeconds: 600 }));
    await fetchIceServers(BASE, TOKEN, impl);
    expect(calls[0]?.url).toBe("https://signaling.example.com/turn-credentials");
    const headers = calls[0]?.init?.headers as Record<string, string>;
    expect(headers.Authorization).toBe(`Bearer ${TOKEN}`);
  });

  it("never puts the token in the URL, where it could land in a log or Referer", async () => {
    const { impl, calls } = stubFetch(json({ iceServers: [], ttlSeconds: 600 }));
    await fetchIceServers(BASE, TOKEN, impl);
    expect(calls[0]?.url).not.toContain(TOKEN);
  });

  it("returns the issued ICE servers and TTL", async () => {
    const iceServers = [{ urls: "turn:turn.example.com", username: "u", credential: "c" }];
    const { impl } = stubFetch(json({ iceServers, ttlSeconds: 600 }));
    await expect(fetchIceServers(BASE, TOKEN, impl)).resolves.toEqual({ iceServers, ttlSeconds: 600 });
  });

  it("refuses to make a request at all without a token", async () => {
    const { impl, calls } = stubFetch(json({ iceServers: [], ttlSeconds: 600 }));
    await expect(fetchIceServers(BASE, "", impl)).rejects.toThrow(TurnCredentialsError);
    expect(calls).toHaveLength(0);
  });

  it("surfaces the server's machine-readable code for an unauthorized response", async () => {
    const { impl } = stubFetch(
      json({ code: "UNAUTHORIZED", error: "this admission no longer holds a slot" }, { status: 401 }),
    );
    await expect(fetchIceServers(BASE, TOKEN, impl)).rejects.toMatchObject({
      status: 401,
      code: "UNAUTHORIZED",
    });
  });

  it("surfaces Retry-After for a rate-limited response", async () => {
    const { impl } = stubFetch(
      json({ code: "RATE_LIMITED", error: "too many requests" }, { status: 429, headers: { "Retry-After": "30" } }),
    );
    await expect(fetchIceServers(BASE, TOKEN, impl)).rejects.toMatchObject({
      status: 429,
      code: "RATE_LIMITED",
      retryAfterSeconds: 30,
    });
  });

  it("still reports a usable error when the failure body is not JSON", async () => {
    const { impl } = stubFetch(new Response("<html>edge error</html>", { status: 502 }));
    await expect(fetchIceServers(BASE, TOKEN, impl)).rejects.toMatchObject({ status: 502, code: "UNKNOWN" });
  });

  it("rejects a malformed success payload", async () => {
    const { impl } = stubFetch(json({ iceServers: "nope", ttlSeconds: 600 }));
    await expect(fetchIceServers(BASE, TOKEN, impl)).rejects.toMatchObject({ code: "MALFORMED_RESPONSE" });
  });
});
