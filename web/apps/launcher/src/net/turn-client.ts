/**
 * Client for the Worker's `/turn-credentials` endpoint. The long-lived TURN
 * API token never leaves the Worker; this module only ever receives the
 * short-lived `iceServers` payload it returns.
 *
 * The endpoint is authorized, not public: it requires the room admission
 * token the signaling server issued in this client's `welcome` message, sent
 * as `Authorization: Bearer <token>`. TURN credentials can therefore only be
 * requested *after* a room join succeeds — see `webrtc-udp-bridge.ts`, which
 * connects first and refreshes ICE servers once admitted.
 */

export interface TurnCredentialsResponse {
  readonly iceServers: RTCIceServer[];
  readonly ttlSeconds: number;
}

/** Thrown for a rejected credential request, carrying the server's machine-
 * readable code so callers can distinguish "not authorized" (rejoin) from
 * "rate limited" (back off) without parsing prose. */
export class TurnCredentialsError extends Error {
  constructor(
    message: string,
    readonly status: number,
    readonly code: string,
    /** Seconds the server asked the client to wait, when it said so. */
    readonly retryAfterSeconds?: number,
  ) {
    super(message);
    this.name = "TurnCredentialsError";
  }
}

function parseRetryAfter(value: string | null): number | undefined {
  if (!value) return undefined;
  const seconds = Number(value);
  return Number.isFinite(seconds) && seconds >= 0 ? seconds : undefined;
}

export async function fetchIceServers(
  workerBaseUrl: string,
  admissionToken: string,
  fetchImpl: typeof fetch = fetch,
): Promise<TurnCredentialsResponse> {
  if (!admissionToken) {
    throw new TurnCredentialsError("no room admission token; join a room first", 0, "UNAUTHORIZED");
  }
  const url = new URL("/turn-credentials", workerBaseUrl);
  const response = await fetchImpl(url.toString(), {
    headers: { Authorization: `Bearer ${admissionToken}` },
  });
  if (!response.ok) {
    // Reading the body is best-effort: an edge or proxy error may not be JSON.
    let code = "UNKNOWN";
    let detail = "";
    try {
      const body = (await response.json()) as { code?: unknown; error?: unknown };
      if (typeof body.code === "string") code = body.code;
      if (typeof body.error === "string") detail = `: ${body.error}`;
    } catch {
      /* fall back to the status-only message */
    }
    const retryAfter = parseRetryAfter(response.headers.get("Retry-After"));
    throw new TurnCredentialsError(
      `failed to fetch TURN credentials: HTTP ${response.status} (${code})${detail}`,
      response.status,
      code,
      retryAfter,
    );
  }
  const body = (await response.json()) as { iceServers?: unknown; ttlSeconds?: unknown };
  if (!Array.isArray(body.iceServers) || typeof body.ttlSeconds !== "number") {
    throw new TurnCredentialsError("malformed TURN credentials response", response.status, "MALFORMED_RESPONSE");
  }
  return { iceServers: body.iceServers as RTCIceServer[], ttlSeconds: body.ttlSeconds };
}
