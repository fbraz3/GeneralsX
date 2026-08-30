/**
 * Pure token-bucket rate limiting for TURN credential issuance.
 *
 * Kept free of any Cloudflare types so the refill arithmetic can be unit
 * tested with an injected clock rather than by waiting in real time.
 *
 * Two buckets are consumed together, and a request must fit in both:
 *
 *  - a **per-seat** bucket, so one player cannot drain a room's allowance;
 *  - a **per-room** bucket, so a full room of colluding clients still has a
 *    bounded aggregate cost.
 *
 * Both are enforced inside the room's Durable Object. That matters: a Durable
 * Object is the single-threaded, single-instance authority for its room, so
 * two concurrent requests cannot both read a stale bucket and both be
 * allowed. A limiter living in the stateless Worker would be per-isolate and
 * therefore trivially bypassed by spreading requests across colos.
 *
 * These limits are keyed by room, and creating a room is free, so they bound
 * what one *room* costs and nothing more: a client that rotates through fresh
 * rooms gets a fresh allowance each time. The room-independent ceiling that
 * closes that hole lives in `../turn/edge-quota.ts`; the two are complementary
 * and both are enforced.
 */
import {
  bucketRetryAfterSeconds,
  createBucket,
  isBucketFull,
  refillBucket,
  type BucketSpec,
  type TokenBucket,
} from "../limits/token-bucket.js";

/** Credential grants a single seat may burst before refill gates it. */
export const TURN_GRANT_SLOT_BURST = 5;
/** Sustained per-seat rate: one grant per interval once the burst is spent. */
export const TURN_GRANT_SLOT_REFILL_MS = 60_000;
/** Credential grants a whole room may burst across all of its seats. */
export const TURN_GRANT_ROOM_BURST = 20;
/** Sustained whole-room refill interval. */
export const TURN_GRANT_ROOM_REFILL_MS = 15_000;

const SLOT_SPEC: BucketSpec = Object.freeze({
  burst: TURN_GRANT_SLOT_BURST,
  refillMs: TURN_GRANT_SLOT_REFILL_MS,
});
const ROOM_SPEC: BucketSpec = Object.freeze({
  burst: TURN_GRANT_ROOM_BURST,
  refillMs: TURN_GRANT_ROOM_REFILL_MS,
});

export interface TurnGrantLimiterState {
  readonly room: TokenBucket;
  readonly perSlot: Map<number, TokenBucket>;
}

export interface TurnGrantDecision {
  readonly allowed: boolean;
  /** Whole seconds the caller should wait before retrying. Always >= 1 when
   * `allowed` is false, so a client never busy-loops on a `Retry-After: 0`. */
  readonly retryAfterSeconds: number;
  /** Which bucket rejected the request, for the operator-facing detail
   * string. `null` when the request was allowed. */
  readonly scope: "slot" | "room" | null;
}

export function createTurnGrantLimiter(nowMs: number): TurnGrantLimiterState {
  return { room: createBucket(ROOM_SPEC, nowMs), perSlot: new Map() };
}

/**
 * Consumes one grant for `slot`, refilling both buckets first.
 *
 * When either bucket is empty the request is denied and **neither** bucket is
 * charged, so a client that is already being throttled cannot deepen the
 * room's deficit by retrying.
 */
export function consumeTurnGrant(
  state: TurnGrantLimiterState,
  slot: number,
  nowMs: number,
): TurnGrantDecision {
  let slotBucket = state.perSlot.get(slot);
  if (!slotBucket) {
    slotBucket = createBucket(SLOT_SPEC, nowMs);
    state.perSlot.set(slot, slotBucket);
  }

  refillBucket(slotBucket, SLOT_SPEC, nowMs);
  refillBucket(state.room, ROOM_SPEC, nowMs);

  if (slotBucket.tokens < 1) {
    return {
      allowed: false,
      retryAfterSeconds: bucketRetryAfterSeconds(slotBucket, SLOT_SPEC),
      scope: "slot",
    };
  }
  if (state.room.tokens < 1) {
    return {
      allowed: false,
      retryAfterSeconds: bucketRetryAfterSeconds(state.room, ROOM_SPEC),
      scope: "room",
    };
  }

  slotBucket.tokens -= 1;
  state.room.tokens -= 1;
  return { allowed: true, retryAfterSeconds: 0, scope: null };
}

/** Drops per-seat buckets that have refilled completely.
 *
 * Only *full* buckets are dropped: a bucket that still carries debt encodes
 * a throttle that must survive the seat being vacated, otherwise a client
 * could reset its own allowance simply by leaving and rejoining the room. A
 * full bucket carries no information, so removing it changes no decision and
 * keeps the map from growing without bound over a long-lived room's churn. */
export function pruneTurnGrantSlots(state: TurnGrantLimiterState, nowMs: number): void {
  for (const [slot, bucket] of state.perSlot) {
    refillBucket(bucket, SLOT_SPEC, nowMs);
    if (isBucketFull(bucket, SLOT_SPEC)) state.perSlot.delete(slot);
  }
}
