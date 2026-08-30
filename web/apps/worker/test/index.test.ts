import { describe, expect, it, vi } from "vitest";
import worker, { type WorkerEnv } from "../src/index.js";
import {
  createRoomKeyBytes,
  importRoomKey,
  signAdmissionToken,
  ADMISSION_TOKEN_TTL_SECONDS,
} from "../src/room/admission.js";
import { TURN_GRANT_PATH, type TurnGrantResult } from "../src/room/turn-grant.js";

/** A structurally valid admission token. It is signed with a throwaway key
 * that no room holds, so it is only ever *routable* — every authorization
 * decision in these tests comes from the stubbed Durable Object, exactly as
 * it does in production. */
async function makeRoutableToken(roomId = "ABCD"): Promise<string> {
  const key = await importRoomKey(createRoomKeyBytes());
  return signAdmissionToken(key, {
    roomId,
    slot: 0,
    admissionId: "seat-nonce",
    exp: Math.floor(Date.now() / 1000) + ADMISSION_TOKEN_TTL_SECONDS,
  });
}

/** Builds a ROOM_DO binding whose stub answers the internal turn-grant call
 * with `result`, recording which room id the Worker routed to. */
function makeRoomDo(result: TurnGrantResult | Error = { ok: true, roomId: "ABCD", slot: 0 }) {
  const routedTo: string[] = [];
  const grantPaths: string[] = [];
  const fetchStub = vi.fn(async (request: Request) => {
    grantPaths.push(new URL(request.url).pathname);
    if (result instanceof Error) throw result;
    return new Response(JSON.stringify(result), { status: 200 });
  });
  const binding = {
    idFromName: vi.fn((name: string) => {
      routedTo.push(name);
      return name;
    }),
    get: vi.fn(() => ({ fetch: fetchStub })),
  } as unknown as WorkerEnv["ROOM_DO"];
  return { binding, routedTo, grantPaths, fetchStub };
}

function makeEnv(overrides: Partial<WorkerEnv> = {}): WorkerEnv {
  return {
    ALLOWED_ORIGINS: "https://play.generalsx.org",
    SIGNALING_ORIGIN: "https://signaling.generalsx.org",
    ASSET_ORIGIN: "https://assets.generalsx.org",
    TURN_KEY_ID: "key-123",
    TURN_KEY_API_TOKEN: "secret",
    ROOM_DO: makeRoomDo().binding,
    ...overrides,
  };
}

async function requestTurnCredentials(
  env: WorkerEnv,
  init: { token?: string | undefined; method?: string } = {},
): Promise<Response> {
  const token = init.token === undefined ? await makeRoutableToken() : init.token;
  return worker.fetch(
    new Request("https://api.generalsx.org/turn-credentials", {
      ...(init.method ? { method: init.method } : {}),
      ...(token ? { headers: { Authorization: `Bearer ${token}` } } : {}),
    }),
    env,
  );
}

function makeEnvWithoutTurnSecrets(): WorkerEnv {
  const env = makeEnv();
  const { TURN_KEY_ID: _omitId, TURN_KEY_API_TOKEN: _omitToken, ...rest } = env;
  return rest as WorkerEnv;
}

describe("worker fetch: security headers", () => {
  it("applies COOP/COEP/CORP/CSP headers to a 404 response", async () => {
    const response = await worker.fetch(new Request("https://api.generalsx.org/nope"), makeEnv());
    expect(response.status).toBe(404);
    expect(response.headers.get("Cross-Origin-Opener-Policy")).toBe("same-origin");
    expect(response.headers.get("Cross-Origin-Embedder-Policy")).toBe("require-corp");
    expect(response.headers.get("Cross-Origin-Resource-Policy")).toBe("same-origin");
    expect(response.headers.get("Content-Security-Policy")).toContain("default-src 'self'");
  });

  it("reflects CORS headers only for an allowed origin", async () => {
    const allowed = await worker.fetch(
      new Request("https://api.generalsx.org/nope", { headers: { Origin: "https://play.generalsx.org" } }),
      makeEnv(),
    );
    expect(allowed.headers.get("Access-Control-Allow-Origin")).toBe("https://play.generalsx.org");

    const disallowed = await worker.fetch(
      new Request("https://api.generalsx.org/nope", { headers: { Origin: "https://evil.example" } }),
      makeEnv(),
    );
    expect(disallowed.headers.get("Access-Control-Allow-Origin")).toBeNull();
  });

  it("answers CORS preflight requests without a body", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/turn-credentials", {
        method: "OPTIONS",
        headers: { Origin: "https://play.generalsx.org" },
      }),
      makeEnv(),
    );
    expect(response.status).toBe(204);
    expect(response.headers.get("Access-Control-Allow-Origin")).toBe("https://play.generalsx.org");
  });
});

describe("worker fetch: /turn-credentials authorization", () => {
  it("rejects non GET/POST methods", async () => {
    const response = await requestTurnCredentials(makeEnv(), { method: "DELETE" });
    expect(response.status).toBe(405);
  });

  it("refuses to issue credentials without a room admission token", async () => {
    const roomDo = makeRoomDo();
    const response = await requestTurnCredentials(makeEnv({ ROOM_DO: roomDo.binding }), { token: "" });
    expect(response.status).toBe(401);
    expect(response.headers.get("WWW-Authenticate")).toContain("Bearer");
    await expect(response.json()).resolves.toMatchObject({ code: "UNAUTHORIZED" });
    // The room is never consulted for a request that carries no capability.
    expect(roomDo.fetchStub).not.toHaveBeenCalled();
  });

  it("does not treat an allowed Origin as authorization", async () => {
    // CORS is a browser read restriction, not an authorization mechanism:
    // presenting the launcher's own origin must still get nothing.
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/turn-credentials", {
        headers: { Origin: "https://play.generalsx.org" },
      }),
      makeEnv(),
    );
    expect(response.status).toBe(401);
  });

  it("rejects a malformed token before touching a Durable Object", async () => {
    const roomDo = makeRoomDo();
    const response = await requestTurnCredentials(makeEnv({ ROOM_DO: roomDo.binding }), {
      token: "not-a-real-token",
    });
    expect(response.status).toBe(401);
    expect(roomDo.fetchStub).not.toHaveBeenCalled();
  });

  it("routes the grant to the room named in the token, over the internal path only", async () => {
    const roomDo = makeRoomDo({ ok: true, roomId: "R7K2QX", slot: 2 });
    const token = await makeRoutableToken("R7K2QX");
    await requestTurnCredentials(makeEnv({ ROOM_DO: roomDo.binding }), { token });
    expect(roomDo.routedTo).toEqual(["R7K2QX"]);
    expect(roomDo.grantPaths).toEqual([TURN_GRANT_PATH]);
  });

  it("surfaces a room's refusal as 401 without issuing credentials", async () => {
    const roomDo = makeRoomDo({
      ok: false,
      code: "UNAUTHORIZED",
      detail: "this admission no longer holds a slot in the room",
    });
    const response = await requestTurnCredentials(makeEnv({ ROOM_DO: roomDo.binding }));
    expect(response.status).toBe(401);
    await expect(response.json()).resolves.toMatchObject({
      code: "UNAUTHORIZED",
      error: "this admission no longer holds a slot in the room",
    });
  });

  it("surfaces a rate-limited grant as 429 with Retry-After", async () => {
    const roomDo = makeRoomDo({
      ok: false,
      code: "RATE_LIMITED",
      detail: "too many TURN credential requests for this slot",
      retryAfterSeconds: 42,
    });
    const response = await requestTurnCredentials(makeEnv({ ROOM_DO: roomDo.binding }));
    expect(response.status).toBe(429);
    expect(response.headers.get("Retry-After")).toBe("42");
    await expect(response.json()).resolves.toMatchObject({ code: "RATE_LIMITED" });
  });

  it("fails closed when the room cannot be reached", async () => {
    const roomDo = makeRoomDo(new Error("durable object unreachable"));
    const response = await requestTurnCredentials(makeEnv({ ROOM_DO: roomDo.binding }));
    expect(response.status).toBe(503);
  });

  it("returns 500 when TURN secrets are not configured, only after authorizing", async () => {
    const roomDo = makeRoomDo();
    const env = { ...makeEnvWithoutTurnSecrets(), ROOM_DO: roomDo.binding };
    const response = await requestTurnCredentials(env);
    expect(response.status).toBe(500);
    expect(response.headers.get("Content-Security-Policy")).toBeTruthy();
    expect(roomDo.fetchStub).toHaveBeenCalledOnce();
  });

  it("never caches an issued credential", async () => {
    const roomDo = makeRoomDo();
    const env = makeEnv({
      ROOM_DO: roomDo.binding,
      TURN_KEY_ID: "key-123",
      TURN_KEY_API_TOKEN: "secret",
    });
    const fetchSpy = vi.spyOn(globalThis, "fetch").mockResolvedValue(
      new Response(JSON.stringify({ iceServers: [{ urls: "turn:example" }] }), { status: 200 }),
    );
    try {
      const response = await requestTurnCredentials(env);
      expect(response.status).toBe(200);
      expect(response.headers.get("Cache-Control")).toBe("no-store");
    } finally {
      fetchSpy.mockRestore();
    }
  });
});

describe("worker fetch: /room", () => {
  it("rejects requests from a disallowed Origin before touching the Durable Object", async () => {
    const roomDoGet = vi.fn();
    const env = makeEnv({
      ROOM_DO: {
        idFromName: vi.fn(() => "id"),
        get: roomDoGet,
      } as unknown as WorkerEnv["ROOM_DO"],
    });
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/room?roomId=ABCD", {
        headers: { Origin: "https://evil.example", Upgrade: "websocket" },
      }),
      env,
    );
    expect(response.status).toBe(403);
    expect(roomDoGet).not.toHaveBeenCalled();
  });
});

describe("worker fetch: health endpoints", () => {
  it("serves liveness without consulting any dependency", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/healthz"),
      makeEnvWithoutTurnSecrets(),
    );
    expect(response.status).toBe(200);
    expect(response.headers.get("Cache-Control")).toBe("no-store");
    expect(response.headers.get("Content-Security-Policy")).toBeTruthy();
    await expect(response.json()).resolves.toMatchObject({
      service: "generalsx-signaling",
      status: "ok",
    });
  });

  it("serves readiness as 200 when ready, reporting the release id", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/readyz"),
      makeEnv({ RELEASE_ID: "deadbeef" }),
    );
    expect(response.status).toBe(200);
    expect(response.headers.get("Cache-Control")).toBe("no-store");
    await expect(response.json()).resolves.toMatchObject({
      ready: true,
      status: "ok",
      releaseId: "deadbeef",
    });
  });

  it("serves readiness as 503 when a required binding is missing", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/readyz"),
      makeEnv({ ROOM_DO: undefined as unknown as WorkerEnv["ROOM_DO"] }),
    );
    expect(response.status).toBe(503);
    await expect(response.json()).resolves.toMatchObject({ ready: false, status: "failed" });
  });

  it("rejects non GET/HEAD methods on both probes", async () => {
    for (const path of ["/healthz", "/readyz"]) {
      const response = await worker.fetch(
        new Request(`https://api.generalsx.org${path}`, { method: "POST" }),
        makeEnv(),
      );
      expect(response.status).toBe(405);
    }
  });

  it("never echoes a TURN secret in a probe body", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/readyz"),
      makeEnv({ TURN_KEY_ID: "key-123", TURN_KEY_API_TOKEN: "top-secret" }),
    );
    await expect(response.text()).resolves.not.toContain("top-secret");
  });
});
