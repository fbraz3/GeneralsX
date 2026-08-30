/**
 * Internal request/response contract between the stateless Worker and a room
 * Durable Object for TURN credential authorization.
 *
 * This path is *not* publicly routable: Durable Object namespaces are only
 * reachable through a binding, and the Worker's public router never proxies
 * an arbitrary path to a stub. It is defined in its own module so both sides
 * share one type and the shape is unit testable without a Workers runtime.
 */

/** Internal URL the Worker uses when asking a room to authorize a grant. The
 * origin is arbitrary and never resolved — Durable Object `fetch()` routes by
 * stub, not by hostname. */
export const TURN_GRANT_PATH = "/internal/turn-grant";
export const TURN_GRANT_URL = `https://room.invalid${TURN_GRANT_PATH}`;

export interface TurnGrantRequestBody {
  /** The opaque room admission token presented by the client. */
  readonly token: string;
}

export type TurnGrantDenialCode = "UNAUTHORIZED" | "RATE_LIMITED";

export type TurnGrantResult =
  | { readonly ok: true; readonly roomId: string; readonly slot: number }
  | {
      readonly ok: false;
      readonly code: TurnGrantDenialCode;
      /** Operator/user-facing reason. Never contains token or key material. */
      readonly detail: string;
      /** Present only for `RATE_LIMITED`; seconds to wait before retrying. */
      readonly retryAfterSeconds?: number;
    };

/** HTTP status the Worker should surface for a denial. */
export function statusForDenial(code: TurnGrantDenialCode): number {
  return code === "RATE_LIMITED" ? 429 : 401;
}
