/**
 * Requests short-lived Cloudflare Realtime TURN credentials on behalf of the
 * launcher. The long-lived TURN API token is only ever read from Worker
 * secrets (`env.TURN_KEY_API_TOKEN`) and never forwarded to the client; only
 * the resulting short-lived `iceServers` payload is returned.
 *
 * Secrets are provisioned out of band, e.g.:
 *   wrangler secret put TURN_KEY_ID
 *   wrangler secret put TURN_KEY_API_TOKEN
 */

export const DEFAULT_TURN_TTL_SECONDS = 600;
export const MIN_TURN_TTL_SECONDS = 30;
export const MAX_TURN_TTL_SECONDS = 3600;

export interface TurnCredentialsEnv {
  readonly TURN_KEY_ID?: string;
  readonly TURN_KEY_API_TOKEN?: string;
}

export interface TurnCredentialsRequest {
  readonly url: string;
  readonly init: {
    readonly method: "POST";
    readonly headers: Record<string, string>;
    readonly body: string;
  };
}

/** Clamps a requested TTL (seconds) into the supported range. */
export function normalizeTtlSeconds(requested: number | undefined): number {
  if (requested === undefined || !Number.isFinite(requested)) return DEFAULT_TURN_TTL_SECONDS;
  return Math.min(MAX_TURN_TTL_SECONDS, Math.max(MIN_TURN_TTL_SECONDS, Math.trunc(requested)));
}

/**
 * Builds the outbound request to Cloudflare's TURN credential API. Pure and
 * side-effect free so it can be unit tested without a network call, and so
 * we can assert the secret only ever appears in the `Authorization` header
 * (never in the URL, which could end up in logs).
 */
export function buildTurnCredentialsRequest(
  env: Required<TurnCredentialsEnv>,
  ttlSeconds: number,
): TurnCredentialsRequest {
  return {
    url: `https://rtc.live.cloudflare.com/v1/turn/keys/${encodeURIComponent(env.TURN_KEY_ID)}/credentials/generate-ice-servers`,
    init: {
      method: "POST",
      headers: {
        Authorization: `Bearer ${env.TURN_KEY_API_TOKEN}`,
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ ttl: ttlSeconds }),
    },
  };
}

export type TurnCredentialsOutcome =
  | { readonly ok: true; readonly iceServers: unknown; readonly ttlSeconds: number }
  | { readonly ok: false; readonly status: number; readonly message: string };

/**
 * Fetches short-lived TURN credentials. `fetchImpl` is injectable for tests;
 * production code should pass the ambient Workers `fetch`.
 */
export async function fetchTurnCredentials(
  env: TurnCredentialsEnv,
  options: { readonly ttlSeconds?: number; readonly fetchImpl?: typeof fetch } = {},
): Promise<TurnCredentialsOutcome> {
  if (!env.TURN_KEY_ID || !env.TURN_KEY_API_TOKEN) {
    return { ok: false, status: 500, message: "TURN credentials are not configured" };
  }
  const ttlSeconds = normalizeTtlSeconds(options.ttlSeconds);
  const { url, init } = buildTurnCredentialsRequest(
    { TURN_KEY_ID: env.TURN_KEY_ID, TURN_KEY_API_TOKEN: env.TURN_KEY_API_TOKEN },
    ttlSeconds,
  );
  const doFetch = options.fetchImpl ?? fetch;

  let response: Response;
  try {
    response = await doFetch(url, init);
  } catch {
    return { ok: false, status: 502, message: "failed to reach the TURN credential service" };
  }

  if (!response.ok) {
    return { ok: false, status: 502, message: `TURN credential service responded with ${response.status}` };
  }

  let body: unknown;
  try {
    body = await response.json();
  } catch {
    return { ok: false, status: 502, message: "TURN credential service returned invalid JSON" };
  }

  if (typeof body !== "object" || body === null || !("iceServers" in body)) {
    return { ok: false, status: 502, message: "TURN credential service response is missing iceServers" };
  }

  return { ok: true, iceServers: (body as { iceServers: unknown }).iceServers, ttlSeconds };
}
