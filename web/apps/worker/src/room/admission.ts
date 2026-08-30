/**
 * Room admission tokens: the server-side capability that authorizes a client
 * to mint TURN credentials.
 *
 * Why this exists: TURN relay is a metered, abusable resource. An endpoint
 * that hands out relay credentials to anyone who can issue an HTTP request is
 * an open relay, and CORS does not prevent that — CORS is a *browser* policy
 * that stops a page from *reading* a cross-origin response; it is not applied
 * to curl, a server, or any non-browser client, and `Origin` is trivially
 * forged outside a browser. Authorization therefore has to be a credential
 * the server itself issued.
 *
 * The credential is a short-lived MAC-signed token bound to one seat in one
 * room. It is minted by the room's Durable Object at the moment a player is
 * admitted to a slot, and verified by that same Durable Object — the single
 * authority that knows whether the seat is still held. There is no shared
 * signing secret for an operator to provision or rotate: each room generates
 * its own key in its own Durable Object storage, so a token minted for one
 * room can never validate against another.
 *
 * Token layout (all segments base64url, unpadded):
 *
 *     gxa1.<payload>.<mac>
 *
 * `payload` is the JSON {@link AdmissionClaims}; `mac` is
 * HMAC-SHA-256(`gxa1.<payload>`) under the room key. Claims are readable by
 * the holder — they contain no secret, only the room id, slot, seat nonce,
 * and expiry — and are authenticated, so they cannot be altered.
 *
 * This module is deliberately free of Cloudflare types so it can be unit
 * tested against the standard WebCrypto API.
 */

/** Token version prefix. Bumping this invalidates every outstanding token. */
export const ADMISSION_TOKEN_VERSION = "gxa1";

/**
 * Admission tokens live only long enough for a freshly admitted client to
 * request TURN credentials, and are refreshed by rejoining. Short enough that
 * a leaked token is worth little; long enough to absorb a slow first paint, a
 * suspended tab, or a retry after a transient 5xx.
 */
export const ADMISSION_TOKEN_TTL_SECONDS = 300;

/** Length of the per-seat nonce, in bytes, before hex encoding. */
const ADMISSION_ID_BYTES = 16;
/** Length of the per-room HMAC key, in bytes. */
export const ROOM_KEY_BYTES = 32;

export interface AdmissionClaims {
  /** Room this token admits to. Checked against the verifying room. */
  readonly roomId: string;
  /** Slot the holder occupied when the token was minted. */
  readonly slot: number;
  /** Random per-seat nonce. Rotated on every join, so a token stops
   * verifying the moment its holder leaves and someone else takes the seat. */
  readonly admissionId: string;
  /** Expiry, in seconds since the Unix epoch. */
  readonly exp: number;
}

export type AdmissionFailureCode = "MALFORMED" | "BAD_SIGNATURE" | "EXPIRED" | "WRONG_ROOM";

export type AdmissionVerification =
  | { readonly ok: true; readonly claims: AdmissionClaims }
  | { readonly ok: false; readonly code: AdmissionFailureCode; readonly detail: string };

function toBase64Url(bytes: Uint8Array): string {
  let binary = "";
  for (const byte of bytes) binary += String.fromCharCode(byte);
  return btoa(binary).replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/, "");
}

function fromBase64Url(value: string): Uint8Array | null {
  if (value.length === 0 || !/^[A-Za-z0-9_-]+$/.test(value)) return null;
  const padded = value.replace(/-/g, "+").replace(/_/g, "/").padEnd(Math.ceil(value.length / 4) * 4, "=");
  try {
    const binary = atob(padded);
    const bytes = new Uint8Array(binary.length);
    for (let index = 0; index < binary.length; index += 1) bytes[index] = binary.charCodeAt(index);
    return bytes;
  } catch {
    return null;
  }
}

function encodeUtf8(value: string): Uint8Array {
  return new TextEncoder().encode(value);
}

/** The ambient WebCrypto implementation (Workers, browsers, and Node all
 * expose one). Resolved through a helper rather than a default parameter
 * expression so this module needs no runtime-specific global declaration. */
function ambientCrypto(): Crypto {
  return (globalThis as unknown as { readonly crypto: Crypto }).crypto;
}

/** Fresh random bytes for a room's HMAC key. Never leaves the Durable Object. */
export function createRoomKeyBytes(crypto: Crypto = ambientCrypto()): Uint8Array {
  return crypto.getRandomValues(new Uint8Array(ROOM_KEY_BYTES));
}

/** A random per-seat nonce, hex encoded. */
export function createAdmissionId(crypto: Crypto = ambientCrypto()): string {
  const bytes = crypto.getRandomValues(new Uint8Array(ADMISSION_ID_BYTES));
  return [...bytes].map((byte) => byte.toString(16).padStart(2, "0")).join("");
}

/** Imports raw key bytes as a non-extractable HMAC-SHA-256 signing key. */
export async function importRoomKey(keyBytes: Uint8Array, crypto: Crypto = ambientCrypto()): Promise<CryptoKey> {
  return crypto.subtle.importKey("raw", keyBytes, { name: "HMAC", hash: "SHA-256" }, false, ["sign", "verify"]);
}

/** Mints a signed admission token for one seat. */
export async function signAdmissionToken(
  key: CryptoKey,
  claims: AdmissionClaims,
  crypto: Crypto = ambientCrypto(),
): Promise<string> {
  const payload = toBase64Url(encodeUtf8(JSON.stringify(claims)));
  const signingInput = `${ADMISSION_TOKEN_VERSION}.${payload}`;
  const mac = await crypto.subtle.sign("HMAC", key, encodeUtf8(signingInput));
  return `${signingInput}.${toBase64Url(new Uint8Array(mac))}`;
}

function parseClaims(payload: string): AdmissionClaims | null {
  const bytes = fromBase64Url(payload);
  if (!bytes) return null;
  let parsed: unknown;
  try {
    parsed = JSON.parse(new TextDecoder().decode(bytes));
  } catch {
    return null;
  }
  if (typeof parsed !== "object" || parsed === null) return null;
  const candidate = parsed as Record<string, unknown>;
  if (typeof candidate.roomId !== "string") return null;
  if (typeof candidate.slot !== "number" || !Number.isInteger(candidate.slot) || candidate.slot < 0) return null;
  if (typeof candidate.admissionId !== "string" || candidate.admissionId.length === 0) return null;
  if (typeof candidate.exp !== "number" || !Number.isFinite(candidate.exp)) return null;
  return {
    roomId: candidate.roomId,
    slot: candidate.slot,
    admissionId: candidate.admissionId,
    exp: candidate.exp,
  };
}

/**
 * Reads the room id out of a token *without* verifying it.
 *
 * Used only to pick which room Durable Object should be asked to verify the
 * token. This is safe precisely because it decides nothing: a forged room id
 * routes the request to a different room, whose key will not validate the
 * signature. Never treat the result as authenticated.
 */
export function peekAdmissionRoomId(token: string): string | null {
  const parts = token.split(".");
  if (parts.length !== 3 || parts[0] !== ADMISSION_TOKEN_VERSION) return null;
  return parseClaims(parts[1] as string)?.roomId ?? null;
}

/**
 * Verifies a token's signature, expiry, and room binding.
 *
 * Signature verification is delegated to `crypto.subtle.verify`, which
 * compares the MAC in constant time; this function never compares MAC bytes
 * itself. Seat occupancy is *not* checked here — that is room state, and is
 * enforced by the caller (the Durable Object) after this returns.
 */
export async function verifyAdmissionToken(
  token: string,
  key: CryptoKey,
  options: { readonly expectedRoomId: string; readonly nowMs?: number; readonly crypto?: Crypto },
): Promise<AdmissionVerification> {
  const crypto = options.crypto ?? ambientCrypto();
  const parts = token.split(".");
  if (parts.length !== 3 || parts[0] !== ADMISSION_TOKEN_VERSION) {
    return { ok: false, code: "MALFORMED", detail: "token is not a gxa1 admission token" };
  }
  const [, payload, mac] = parts as [string, string, string];
  const macBytes = fromBase64Url(mac);
  if (!macBytes) {
    return { ok: false, code: "MALFORMED", detail: "token signature is not base64url" };
  }

  const signingInput = `${ADMISSION_TOKEN_VERSION}.${payload}`;
  const verified = await crypto.subtle.verify("HMAC", key, macBytes, encodeUtf8(signingInput));
  if (!verified) {
    return { ok: false, code: "BAD_SIGNATURE", detail: "token signature does not match this room" };
  }

  // Claims are only interpreted *after* the MAC verifies, so unauthenticated
  // input never reaches the authorization logic.
  const claims = parseClaims(payload);
  if (!claims) {
    return { ok: false, code: "MALFORMED", detail: "token claims are malformed" };
  }
  if (claims.roomId !== options.expectedRoomId) {
    return { ok: false, code: "WRONG_ROOM", detail: "token was issued for a different room" };
  }
  const nowSeconds = Math.floor((options.nowMs ?? Date.now()) / 1000);
  if (claims.exp <= nowSeconds) {
    return { ok: false, code: "EXPIRED", detail: "token has expired; rejoin the room to get a new one" };
  }
  return { ok: true, claims };
}

/**
 * Extracts a bearer token from an `Authorization` header.
 *
 * The token travels in a header rather than the query string so it never
 * lands in an access log, a `Referer`, or browser history.
 */
export function extractBearerToken(headerValue: string | null | undefined): string | null {
  if (!headerValue) return null;
  const match = /^Bearer[ \t]+([\x21-\x7e]+)$/.exec(headerValue.trim());
  return match ? (match[1] as string) : null;
}
