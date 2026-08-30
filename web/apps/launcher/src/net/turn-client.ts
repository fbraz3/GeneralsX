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
 *
 * Every request carries a hard deadline. The bridge holds back roster
 * application, peer creation, and queued inbound signals until this call
 * settles, so a half-open socket that never answers would otherwise stall a
 * room join forever and never reach the documented direct/STUN fallback. A
 * deadline is what makes "a TURN failure is non-fatal" true for the failure
 * mode that matters most: no response at all.
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

/**
 * Deadline for one credential request, covering the response body as well as
 * its headers. Generous enough for a cold Worker isolate on a slow mobile
 * link, short enough that a player whose request is black-holed reaches the
 * direct/STUN fallback while still looking at the join spinner.
 */
export const DEFAULT_TURN_REQUEST_TIMEOUT_MS = 8_000;

export interface TurnRequestOptions {
  /** Injectable for tests; defaults to the global `fetch`. */
  readonly fetch?: typeof fetch;
  /** Hard deadline for the whole request. Defaults to
   * {@link DEFAULT_TURN_REQUEST_TIMEOUT_MS}. */
  readonly timeoutMs?: number;
  /** Caller cancellation (leaving the room, a superseded join). Combined with
   * the deadline rather than replacing it, so imposing one never discards the
   * other. */
  readonly signal?: AbortSignal | null;
  /** Injectable for tests; defaults to `AbortSignal.timeout`, whose timer is
   * a platform timer that fake timers do not drive. */
  readonly createTimeoutSignal?: (ms: number) => AbortSignal;
}

const realTimeoutSignal = (ms: number): AbortSignal => AbortSignal.timeout(ms);

function isAbort(error: unknown): boolean {
  return error instanceof Error && (error.name === "AbortError" || error.name === "TimeoutError");
}

function combineSignals(deadline: AbortSignal, caller: AbortSignal | null | undefined): AbortSignal {
  if (!caller) return deadline;
  if (typeof AbortSignal.any === "function") return AbortSignal.any([caller, deadline]);
  return caller.aborted ? caller : deadline;
}

/** Reports an abort in the same machine-readable shape as an HTTP failure, so
 * a caller never has to tell a `DOMException` apart from a rejection.
 * `TIMEOUT` means *we* stopped waiting; `ABORTED` means the caller did. */
function abortError(deadline: AbortSignal, timeoutMs: number): TurnCredentialsError {
  return deadline.aborted
    ? new TurnCredentialsError(`no response within ${timeoutMs}ms`, 0, "TIMEOUT")
    : new TurnCredentialsError("the request was cancelled", 0, "ABORTED");
}

export async function fetchIceServers(
  workerBaseUrl: string,
  admissionToken: string,
  options: TurnRequestOptions = {},
): Promise<TurnCredentialsResponse> {
  if (!admissionToken) {
    throw new TurnCredentialsError("no room admission token; join a room first", 0, "UNAUTHORIZED");
  }
  const fetchImpl = options.fetch ?? fetch;
  const timeoutMs = options.timeoutMs ?? DEFAULT_TURN_REQUEST_TIMEOUT_MS;
  const deadline = (options.createTimeoutSignal ?? realTimeoutSignal)(timeoutMs);
  const signal = combineSignals(deadline, options.signal);
  const url = new URL("/turn-credentials", workerBaseUrl);

  let response: Response;
  try {
    response = await fetchImpl(url.toString(), {
      headers: { Authorization: `Bearer ${admissionToken}` },
      signal,
    });
  } catch (err) {
    // A network-level failure keeps its own message; only an abort is
    // rewritten, because `AbortError` on its own says nothing about why.
    if (isAbort(err) || signal.aborted) throw abortError(deadline, timeoutMs);
    throw err;
  }
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
  // The deadline covers the body too: a response whose stream never
  // completes stalls a join exactly as unanswered headers would.
  let body: { iceServers?: unknown; ttlSeconds?: unknown };
  try {
    body = (await response.json()) as { iceServers?: unknown; ttlSeconds?: unknown };
  } catch (err) {
    if (isAbort(err) || signal.aborted) throw abortError(deadline, timeoutMs);
    throw new TurnCredentialsError("malformed TURN credentials response", response.status, "MALFORMED_RESPONSE");
  }
  if (!Array.isArray(body.iceServers) || typeof body.ttlSeconds !== "number") {
    throw new TurnCredentialsError("malformed TURN credentials response", response.status, "MALFORMED_RESPONSE");
  }
  return { iceServers: body.iceServers as RTCIceServer[], ttlSeconds: body.ttlSeconds };
}
