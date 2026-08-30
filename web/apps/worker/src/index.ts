/// <reference types="@cloudflare/workers-types" />
import {
  buildCorsHeaders,
  buildSecurityHeaders,
  isAllowedOrigin,
  type SecurityHeadersOptions,
} from "@generalsx-web/shared/security-headers";
import { buildLivenessReport, buildReadinessReport } from "./health.js";
import { extractBearerToken, peekAdmissionRoomId } from "./room/admission.js";
import { statusForDenial, TURN_GRANT_URL, type TurnGrantResult } from "./room/turn-grant.js";
import { fetchTurnCredentials, type TurnCredentialsEnv } from "./turn/turn-credentials.js";

export { RoomDurableObject } from "./durable-objects/room-do.js";

export interface WorkerEnv extends TurnCredentialsEnv {
  readonly ALLOWED_ORIGINS: string;
  readonly SIGNALING_ORIGIN: string;
  readonly ASSET_ORIGIN: string;
  /** Non-secret commit SHA of the deployed version, injected by the deploy
   * script (`--var RELEASE_ID:<sha>`) so rollbacks are verifiable. */
  readonly RELEASE_ID?: string;
  readonly ROOM_DO: DurableObjectNamespace;
}

function parseOriginList(value: string | undefined): string[] {
  return (value ?? "")
    .split(",")
    .map((origin) => origin.trim())
    .filter((origin) => origin.length > 0);
}

function securityOptions(env: WorkerEnv): SecurityHeadersOptions {
  return {
    allowedOrigins: parseOriginList(env.ALLOWED_ORIGINS),
    signalingOrigins: parseOriginList(env.SIGNALING_ORIGIN),
    assetOrigins: parseOriginList(env.ASSET_ORIGIN),
  };
}

function withHeaders(response: Response, extra: Record<string, string>): Response {
  const headers = new Headers(response.headers);
  for (const [name, value] of Object.entries(extra)) headers.set(name, value);
  return new Response(response.body, { status: response.status, headers });
}

/**
 * Asks the room that issued `token` whether it still authorizes a TURN
 * credential grant, and whether this seat is within its rate limit.
 *
 * The unverified room id in the token only chooses *which* Durable Object is
 * consulted; that object then verifies the signature with its own per-room
 * key, so a forged room id simply routes to a room that will reject it.
 */
async function authorizeTurnGrant(token: string, env: WorkerEnv): Promise<TurnGrantResult> {
  const roomId = peekAdmissionRoomId(token);
  if (roomId === null) {
    return { ok: false, code: "UNAUTHORIZED", detail: "malformed room admission token" };
  }
  const stub = env.ROOM_DO.get(env.ROOM_DO.idFromName(roomId));
  const response = await stub.fetch(
    new Request(TURN_GRANT_URL, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ token }),
    }),
  );
  if (!response.ok) {
    return { ok: false, code: "UNAUTHORIZED", detail: "room authorization is unavailable" };
  }
  return (await response.json()) as TurnGrantResult;
}

/**
 * Issues short-lived TURN credentials to a client that proves, with a room
 * admission token, that it currently holds a seat in a room.
 *
 * The token — not the `Origin` header — is the authorization. CORS is a
 * browser-enforced read restriction and is trivially absent outside a
 * browser, so it can only ever be defense in depth here; without a
 * server-issued capability this endpoint would be an open TURN relay that
 * anyone could bill to this account.
 */
async function handleTurnCredentials(request: Request, env: WorkerEnv): Promise<Response> {
  if (request.method !== "GET" && request.method !== "POST") {
    return new Response("method not allowed", { status: 405 });
  }

  const jsonError = (status: number, code: string, message: string, extra: Record<string, string> = {}): Response =>
    new Response(JSON.stringify({ error: message, code }), {
      status,
      headers: { "Content-Type": "application/json", ...extra },
    });

  const token = extractBearerToken(request.headers.get("Authorization"));
  if (!token) {
    return jsonError(
      401,
      "UNAUTHORIZED",
      "a room admission token is required; join a room and present its token as 'Authorization: Bearer <token>'",
      { "WWW-Authenticate": 'Bearer realm="generalsx-room"' },
    );
  }

  let grant: TurnGrantResult;
  try {
    grant = await authorizeTurnGrant(token, env);
  } catch {
    return jsonError(503, "UNAUTHORIZED", "room authorization is unavailable");
  }
  if (!grant.ok) {
    const retryAfter = grant.retryAfterSeconds;
    return jsonError(statusForDenial(grant.code), grant.code, grant.detail, {
      ...(retryAfter !== undefined ? { "Retry-After": String(retryAfter) } : {}),
    });
  }

  const url = new URL(request.url);
  const ttlParam = url.searchParams.get("ttl");
  const outcome = await fetchTurnCredentials(env, {
    ...(ttlParam ? { ttlSeconds: Number(ttlParam) } : {}),
  });
  if (!outcome.ok) {
    return jsonError(outcome.status, "TURN_UNAVAILABLE", outcome.message);
  }
  return new Response(JSON.stringify({ iceServers: outcome.iceServers, ttlSeconds: outcome.ttlSeconds }), {
    status: 200,
    headers: { "Content-Type": "application/json", "Cache-Control": "no-store" },
  });
}

function handleRoomSocket(request: Request, env: WorkerEnv): Promise<Response> {
  const url = new URL(request.url);
  const roomId = url.searchParams.get("roomId") ?? "";
  const id = env.ROOM_DO.idFromName(roomId);
  const stub = env.ROOM_DO.get(id);
  return stub.fetch(request);
}

/** Health endpoints are unauthenticated but disclose no secret value and are
 * never cached, so an uptime monitor or a post-deploy smoke test always sees
 * the version that is actually serving traffic. */
function jsonProbeResponse(body: unknown, status: number): Response {
  return new Response(JSON.stringify(body), {
    status,
    headers: { "Content-Type": "application/json", "Cache-Control": "no-store" },
  });
}

function handleLiveness(request: Request, env: WorkerEnv): Response {
  if (request.method !== "GET" && request.method !== "HEAD") {
    return new Response("method not allowed", { status: 405 });
  }
  return jsonProbeResponse(buildLivenessReport(env), 200);
}

function handleReadiness(request: Request, env: WorkerEnv): Response {
  if (request.method !== "GET" && request.method !== "HEAD") {
    return new Response("method not allowed", { status: 405 });
  }
  const report = buildReadinessReport(env);
  return jsonProbeResponse(report, report.ready ? 200 : 503);
}

export default {
  async fetch(request: Request, env: WorkerEnv): Promise<Response> {
    const url = new URL(request.url);
    const origin = request.headers.get("Origin");
    const options = securityOptions(env);
    const securityHeaders = buildSecurityHeaders(options);

    if (request.method === "OPTIONS") {
      return new Response(null, { status: 204, headers: buildCorsHeaders(origin, options) });
    }

    if (url.pathname === "/room") {
      if (origin && !isAllowedOrigin(origin, options)) {
        return new Response("origin not allowed", { status: 403 });
      }
      return handleRoomSocket(request, env);
    }

    if (url.pathname === "/healthz") {
      return withHeaders(handleLiveness(request, env), securityHeaders);
    }

    if (url.pathname === "/readyz") {
      return withHeaders(handleReadiness(request, env), securityHeaders);
    }

    if (url.pathname === "/turn-credentials") {
      const response = await handleTurnCredentials(request, env);
      return withHeaders(response, { ...securityHeaders, ...buildCorsHeaders(origin, options) });
    }

    const notFound = new Response("not found", { status: 404 });
    return withHeaders(notFound, { ...securityHeaders, ...buildCorsHeaders(origin, options) });
  },
};
