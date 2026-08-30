/**
 * Global, room-independent ceiling on TURN credential issuance.
 *
 * The per-room and per-seat limits in `../room/turn-rate-limit.ts` are keyed
 * by a room id, and rooms are free to create: a client that repeatedly makes
 * a fresh room, joins it, mints a fresh admission token, and asks for
 * credentials gets a fresh allowance every time. Room rotation therefore
 * bypasses room-keyed quotas entirely, however strict they are.
 *
 * This limiter closes that hole by counting what the attacker cannot choose:
 *
 *  - a **per-client** bucket keyed by the edge-supplied `CF-Connecting-IP`
 *    (see `./client-identity.ts` — no client-controlled header is ever read);
 *  - a **global** bucket that caps issuance for the whole deployment, which
 *    is the actual protection for the TURN bill when an attacker does have
 *    many source addresses.
 *
 * Both live in a single Durable Object instance, so unlike a limiter in the
 * stateless Worker they cannot be bypassed by spreading requests across
 * isolates or colos.
 *
 * A rejected request is **not** charged to the global bucket. Otherwise a
 * single throttled client could exhaust the deployment-wide ceiling and deny
 * service to everyone else.
 */
import {
  bucketRetryAfterSeconds,
  createBucket,
  isBucketFull,
  refillBucket,
  type BucketSpec,
  type TokenBucket,
} from "../limits/token-bucket.js";
import { UNIDENTIFIED_CLIENT_KEY } from "./client-identity.js";

/** One address, one household: generous enough for several players behind a
 * shared NAT rejoining rooms, far below what room rotation would need. */
export const TURN_EDGE_CLIENT_SPEC: BucketSpec = Object.freeze({ burst: 30, refillMs: 10_000 });

/** Requests with no trusted address share one small bucket, so a
 * non-edge dispatch path can never become an unmetered side door. */
export const TURN_EDGE_UNIDENTIFIED_SPEC: BucketSpec = Object.freeze({ burst: 5, refillMs: 10_000 });

/** Deployment-wide ceiling: ~1 credential per second sustained, 600 burst. */
export const TURN_EDGE_GLOBAL_SPEC: BucketSpec = Object.freeze({ burst: 600, refillMs: 1_000 });

/**
 * Hard cap on tracked client buckets.
 *
 * The map is the one part of this limiter an attacker can grow, by rotating
 * source addresses. Once it is full, unknown clients are refused rather than
 * admitted, so a spray of one request per address cannot exhaust the Durable
 * Object's memory — and cannot get free credentials either.
 */
export const TURN_EDGE_MAX_TRACKED_CLIENTS = 4_096;

export type TurnEdgeScope = "client" | "global" | "capacity";

export interface TurnEdgeQuotaState {
  readonly global: TokenBucket;
  readonly perClient: Map<string, TokenBucket>;
}

export interface TurnEdgeDecision {
  readonly allowed: boolean;
  /** Whole seconds to wait before retrying; 0 when allowed. */
  readonly retryAfterSeconds: number;
  /** Which ceiling refused the request, for the operator-facing detail. */
  readonly scope: TurnEdgeScope | null;
}

export interface TurnEdgeClient {
  readonly key: string;
  readonly identified: boolean;
}

export function createTurnEdgeQuota(nowMs: number): TurnEdgeQuotaState {
  return { global: createBucket(TURN_EDGE_GLOBAL_SPEC, nowMs), perClient: new Map() };
}

function specFor(client: TurnEdgeClient): BucketSpec {
  return client.identified ? TURN_EDGE_CLIENT_SPEC : TURN_EDGE_UNIDENTIFIED_SPEC;
}

/**
 * Consumes one credential issuance for `client`.
 *
 * Evaluated per-client first: an already-throttled client must not be able to
 * spend the deployment-wide allowance on requests that were going to be
 * refused anyway.
 */
export function consumeTurnEdgeQuota(
  state: TurnEdgeQuotaState,
  client: TurnEdgeClient,
  nowMs: number,
): TurnEdgeDecision {
  const spec = specFor(client);
  let bucket = state.perClient.get(client.key);
  if (!bucket) {
    if (state.perClient.size >= TURN_EDGE_MAX_TRACKED_CLIENTS) {
      pruneTurnEdgeClients(state, nowMs);
    }
    if (state.perClient.size >= TURN_EDGE_MAX_TRACKED_CLIENTS) {
      return {
        allowed: false,
        retryAfterSeconds: Math.ceil(spec.refillMs / 1000),
        scope: "capacity",
      };
    }
    bucket = createBucket(spec, nowMs);
    state.perClient.set(client.key, bucket);
  }

  refillBucket(bucket, spec, nowMs);
  refillBucket(state.global, TURN_EDGE_GLOBAL_SPEC, nowMs);

  if (bucket.tokens < 1) {
    return { allowed: false, retryAfterSeconds: bucketRetryAfterSeconds(bucket, spec), scope: "client" };
  }
  if (state.global.tokens < 1) {
    return {
      allowed: false,
      retryAfterSeconds: bucketRetryAfterSeconds(state.global, TURN_EDGE_GLOBAL_SPEC),
      scope: "global",
    };
  }

  bucket.tokens -= 1;
  state.global.tokens -= 1;
  return { allowed: true, retryAfterSeconds: 0, scope: null };
}

/**
 * Drops per-client buckets that have refilled completely.
 *
 * Only *full* buckets are dropped. A bucket still carrying debt encodes an
 * active throttle, and forgetting it would hand the client a fresh allowance
 * for free — which is exactly the bypass this limiter exists to prevent. The
 * shared {@link UNIDENTIFIED_CLIENT_KEY} bucket is never dropped, because it
 * is not per-client and keeping it costs one entry.
 */
export function pruneTurnEdgeClients(state: TurnEdgeQuotaState, nowMs: number): void {
  for (const [key, bucket] of state.perClient) {
    if (key === UNIDENTIFIED_CLIENT_KEY) continue;
    refillBucket(bucket, TURN_EDGE_CLIENT_SPEC, nowMs);
    if (isBucketFull(bucket, TURN_EDGE_CLIENT_SPEC)) state.perClient.delete(key);
  }
}

/** Internal Worker -> quota Durable Object contract. Never routable from
 * outside: the Worker builds this URL itself and dispatches over a stub. */
export const TURN_EDGE_QUOTA_PATH = "/internal/turn-edge-quota";
export const TURN_EDGE_QUOTA_URL = `https://turn-quota.invalid${TURN_EDGE_QUOTA_PATH}`;

/** Name of the single quota instance. One instance is the point: a global
 * ceiling that is sharded is not a global ceiling. */
export const TURN_EDGE_QUOTA_SINGLETON = "turn-edge-quota-v1";

export interface TurnEdgeQuotaRequestBody {
  readonly client: TurnEdgeClient;
}

export function describeTurnEdgeScope(scope: TurnEdgeScope | null): string {
  switch (scope) {
    case "client":
      return "too many TURN credential requests from this address";
    case "global":
      return "the deployment-wide TURN credential rate limit is saturated";
    case "capacity":
      return "the TURN credential rate limiter is at capacity";
    default:
      return "TURN credential issuance was refused";
  }
}
