/// <reference types="@cloudflare/workers-types" />
import {
  buildCorsHeaders,
  buildSecurityHeaders,
  isAllowedOrigin,
  type SecurityHeadersOptions,
} from "@generalsx-web/shared/security-headers";
import { buildLivenessReport, buildReadinessReport } from "./health.js";
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

async function handleTurnCredentials(request: Request, env: WorkerEnv): Promise<Response> {
  if (request.method !== "GET" && request.method !== "POST") {
    return new Response("method not allowed", { status: 405 });
  }
  const url = new URL(request.url);
  const ttlParam = url.searchParams.get("ttl");
  const outcome = await fetchTurnCredentials(env, {
    ...(ttlParam ? { ttlSeconds: Number(ttlParam) } : {}),
  });
  if (!outcome.ok) {
    return new Response(JSON.stringify({ error: outcome.message }), {
      status: outcome.status,
      headers: { "Content-Type": "application/json" },
    });
  }
  return new Response(JSON.stringify({ iceServers: outcome.iceServers, ttlSeconds: outcome.ttlSeconds }), {
    status: 200,
    headers: { "Content-Type": "application/json" },
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
