/**
 * A pure token bucket, shared by every rate limiter in this Worker.
 *
 * Deliberately free of Cloudflare types and of any clock of its own: the
 * caller passes `nowMs`, so refill arithmetic is unit tested with an injected
 * clock instead of by waiting in real time.
 */

export interface TokenBucket {
  /** Fractional tokens remaining. */
  tokens: number;
  /** Timestamp the token count is accurate as of. */
  updatedAtMs: number;
}

export interface BucketSpec {
  /** Tokens available at rest, i.e. how large a burst is tolerated. */
  readonly burst: number;
  /** Milliseconds to regenerate one token once the burst is spent. */
  readonly refillMs: number;
}

export function createBucket(spec: BucketSpec, nowMs: number): TokenBucket {
  return { tokens: spec.burst, updatedAtMs: nowMs };
}

export function refillBucket(bucket: TokenBucket, spec: BucketSpec, nowMs: number): void {
  // A clock that appears to move backwards (retries, host clock skew) must
  // never *add* tokens, so elapsed time is floored at zero.
  const elapsed = Math.max(0, nowMs - bucket.updatedAtMs);
  bucket.tokens = Math.min(spec.burst, bucket.tokens + elapsed / spec.refillMs);
  bucket.updatedAtMs = nowMs;
}

/** Whole seconds until the bucket holds a whole token again. Always >= 1, so
 * a client is never told to retry immediately by a `Retry-After: 0`. */
export function bucketRetryAfterSeconds(bucket: TokenBucket, spec: BucketSpec): number {
  const missing = Math.max(0, 1 - bucket.tokens);
  return Math.max(1, Math.ceil((missing * spec.refillMs) / 1000));
}

export function isBucketFull(bucket: TokenBucket, spec: BucketSpec): boolean {
  return bucket.tokens >= spec.burst;
}
