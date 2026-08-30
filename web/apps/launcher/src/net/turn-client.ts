/**
 * Client for the Worker's `/turn-credentials` endpoint. The long-lived TURN
 * API token never leaves the Worker; this module only ever receives the
 * short-lived `iceServers` payload it returns.
 */

export interface TurnCredentialsResponse {
  readonly iceServers: RTCIceServer[];
  readonly ttlSeconds: number;
}

export async function fetchIceServers(
  workerBaseUrl: string,
  fetchImpl: typeof fetch = fetch,
): Promise<TurnCredentialsResponse> {
  const url = new URL("/turn-credentials", workerBaseUrl);
  const response = await fetchImpl(url.toString());
  if (!response.ok) {
    throw new Error(`failed to fetch TURN credentials: HTTP ${response.status}`);
  }
  const body = (await response.json()) as { iceServers?: unknown; ttlSeconds?: unknown };
  if (!Array.isArray(body.iceServers) || typeof body.ttlSeconds !== "number") {
    throw new Error("malformed TURN credentials response");
  }
  return { iceServers: body.iceServers as RTCIceServer[], ttlSeconds: body.ttlSeconds };
}
