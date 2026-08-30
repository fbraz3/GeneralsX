import { describe, expect, it, vi } from "vitest";
import worker, { type WorkerEnv } from "../src/index.js";

function makeEnv(overrides: Partial<WorkerEnv> = {}): WorkerEnv {
  return {
    ALLOWED_ORIGINS: "https://play.generalsx.org",
    SIGNALING_ORIGIN: "https://signaling.generalsx.org",
    ASSET_ORIGIN: "https://assets.generalsx.org",
    TURN_KEY_ID: "key-123",
    TURN_KEY_API_TOKEN: "secret",
    ROOM_DO: {
      idFromName: vi.fn(),
      get: vi.fn(),
    } as unknown as WorkerEnv["ROOM_DO"],
    ...overrides,
  };
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

describe("worker fetch: /turn-credentials", () => {
  it("rejects non GET/POST methods", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/turn-credentials", { method: "DELETE" }),
      makeEnv(),
    );
    expect(response.status).toBe(405);
  });

  it("returns 500 when TURN secrets are not configured", async () => {
    const response = await worker.fetch(
      new Request("https://api.generalsx.org/turn-credentials"),
      makeEnvWithoutTurnSecrets(),
    );
    expect(response.status).toBe(500);
    expect(response.headers.get("Content-Security-Policy")).toBeTruthy();
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
