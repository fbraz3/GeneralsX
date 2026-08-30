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
    await fetchIceServers(BASE, TOKEN, { fetch: impl });
    expect(calls[0]?.url).toBe("https://signaling.example.com/turn-credentials");
    const headers = calls[0]?.init?.headers as Record<string, string>;
    expect(headers.Authorization).toBe(`Bearer ${TOKEN}`);
  });

  it("never puts the token in the URL, where it could land in a log or Referer", async () => {
    const { impl, calls } = stubFetch(json({ iceServers: [], ttlSeconds: 600 }));
    await fetchIceServers(BASE, TOKEN, { fetch: impl });
    expect(calls[0]?.url).not.toContain(TOKEN);
  });

  it("returns the issued ICE servers and TTL", async () => {
    const iceServers = [{ urls: "turn:turn.example.com", username: "u", credential: "c" }];
    const { impl } = stubFetch(json({ iceServers, ttlSeconds: 600 }));
    await expect(fetchIceServers(BASE, TOKEN, { fetch: impl })).resolves.toEqual({ iceServers, ttlSeconds: 600 });
  });

  it("refuses to make a request at all without a token", async () => {
    const { impl, calls } = stubFetch(json({ iceServers: [], ttlSeconds: 600 }));
    await expect(fetchIceServers(BASE, "", { fetch: impl })).rejects.toThrow(TurnCredentialsError);
    expect(calls).toHaveLength(0);
  });

  it("surfaces the server's machine-readable code for an unauthorized response", async () => {
    const { impl } = stubFetch(
      json({ code: "UNAUTHORIZED", error: "this admission no longer holds a slot" }, { status: 401 }),
    );
    await expect(fetchIceServers(BASE, TOKEN, { fetch: impl })).rejects.toMatchObject({
      status: 401,
      code: "UNAUTHORIZED",
    });
  });

  it("surfaces Retry-After for a rate-limited response", async () => {
    const { impl } = stubFetch(
      json({ code: "RATE_LIMITED", error: "too many requests" }, { status: 429, headers: { "Retry-After": "30" } }),
    );
    await expect(fetchIceServers(BASE, TOKEN, { fetch: impl })).rejects.toMatchObject({
      status: 429,
      code: "RATE_LIMITED",
      retryAfterSeconds: 30,
    });
  });

  it("still reports a usable error when the failure body is not JSON", async () => {
    const { impl } = stubFetch(new Response("<html>edge error</html>", { status: 502 }));
    await expect(fetchIceServers(BASE, TOKEN, { fetch: impl })).rejects.toMatchObject({ status: 502, code: "UNKNOWN" });
  });

  it("rejects a malformed success payload", async () => {
    const { impl } = stubFetch(json({ iceServers: "nope", ttlSeconds: 600 }));
    await expect(fetchIceServers(BASE, TOKEN, { fetch: impl })).rejects.toMatchObject({ code: "MALFORMED_RESPONSE" });
  });
});

describe("fetchIceServers deadlines", () => {
  /** A request that never answers — the failure a timeout exists for, and the
   * one a caller cannot detect for itself. Rejects only when aborted, exactly
   * as a real `fetch` does. */
  function hangingFetch(): { impl: typeof fetch; signals: (AbortSignal | undefined)[] } {
    const signals: (AbortSignal | undefined)[] = [];
    const impl = ((_url: string, init?: RequestInit) => {
      const signal = init?.signal ?? undefined;
      signals.push(signal);
      return new Promise<Response>((_resolve, reject) => {
        signal?.addEventListener("abort", () => {
          reject(new DOMException("The operation was aborted.", "AbortError"));
        });
      });
    }) as unknown as typeof fetch;
    return { impl, signals };
  }

  function controllableTimeout(): { create: (ms: number) => AbortSignal; fire: () => void; requested: number[] } {
    const controller = new AbortController();
    const requested: number[] = [];
    return {
      create: (ms) => {
        requested.push(ms);
        return controller.signal;
      },
      fire: () => controller.abort(new DOMException("Timed out", "TimeoutError")),
      requested,
    };
  }

  it("aborts a request that never answers and reports it as a TIMEOUT", async () => {
    const { impl, signals } = hangingFetch();
    const timeout = controllableTimeout();
    const pending = fetchIceServers(BASE, TOKEN, {
      fetch: impl,
      timeoutMs: 1_234,
      createTimeoutSignal: timeout.create,
    });
    expect(timeout.requested).toEqual([1_234]);
    expect(signals[0]?.aborted).toBe(false);

    timeout.fire();
    await expect(pending).rejects.toMatchObject({ code: "TIMEOUT", status: 0 });
    expect(signals[0]?.aborted).toBe(true);
  });

  it("names the deadline it gave up on, so the failure is actionable", async () => {
    const { impl } = hangingFetch();
    const timeout = controllableTimeout();
    const pending = fetchIceServers(BASE, TOKEN, {
      fetch: impl,
      timeoutMs: 8_000,
      createTimeoutSignal: timeout.create,
    });
    timeout.fire();
    await expect(pending).rejects.toThrow(/no response within 8000ms/);
  });

  it("applies a deadline by default rather than only when asked for one", async () => {
    const { impl, signals } = hangingFetch();
    // Real `AbortSignal.timeout`; the request is left to be collected rather
    // than waited on, since the point is only that a deadline was attached.
    void fetchIceServers(BASE, TOKEN, { fetch: impl }).catch(() => undefined);
    await Promise.resolve();
    expect(signals[0]).toBeInstanceOf(AbortSignal);
  });

  it("honours a caller's cancellation without discarding the deadline", async () => {
    const { impl, signals } = hangingFetch();
    const timeout = controllableTimeout();
    const caller = new AbortController();
    const pending = fetchIceServers(BASE, TOKEN, {
      fetch: impl,
      timeoutMs: 5_000,
      signal: caller.signal,
      createTimeoutSignal: timeout.create,
    });

    caller.abort();
    // Distinguished from a timeout: the caller stopped waiting, so a retry or
    // a warning would be wrong.
    await expect(pending).rejects.toMatchObject({ code: "ABORTED" });
    expect(signals[0]?.aborted).toBe(true);
  });

  it("covers the response body, not just its headers", async () => {
    const timeout = controllableTimeout();
    // Headers arrive; the body never completes.
    const impl = ((_url: string, init?: RequestInit) => {
      const body = new ReadableStream<Uint8Array>({
        start(controller) {
          controller.enqueue(new TextEncoder().encode('{"iceServers":'));
          init?.signal?.addEventListener("abort", () => {
            controller.error(new DOMException("The operation was aborted.", "AbortError"));
          });
        },
      });
      return Promise.resolve(new Response(body, { status: 200, headers: { "Content-Type": "application/json" } }));
    }) as unknown as typeof fetch;

    const pending = fetchIceServers(BASE, TOKEN, {
      fetch: impl,
      timeoutMs: 3_000,
      createTimeoutSignal: timeout.create,
    });
    await Promise.resolve();
    timeout.fire();
    await expect(pending).rejects.toMatchObject({ code: "TIMEOUT" });
  });

  it("leaves a genuine network failure reporting its own cause", async () => {
    const impl = (() => Promise.reject(new TypeError("fetch failed"))) as unknown as typeof fetch;
    await expect(fetchIceServers(BASE, TOKEN, { fetch: impl })).rejects.toThrow(/fetch failed/);
  });
});
