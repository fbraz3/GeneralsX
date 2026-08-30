import { describe, expect, it } from "vitest";
import {
  bucketRetryAfterSeconds,
  createBucket,
  isBucketFull,
  refillBucket,
  type BucketSpec,
} from "../../src/limits/token-bucket.js";

const SPEC: BucketSpec = { burst: 4, refillMs: 1_000 };

describe("token bucket", () => {
  it("starts full", () => {
    const bucket = createBucket(SPEC, 0);
    expect(bucket.tokens).toBe(SPEC.burst);
    expect(isBucketFull(bucket, SPEC)).toBe(true);
  });

  it("refills proportionally to elapsed time, capped at the burst", () => {
    const bucket = createBucket(SPEC, 0);
    bucket.tokens = 0;
    refillBucket(bucket, SPEC, 2_500);
    expect(bucket.tokens).toBeCloseTo(2.5, 5);

    refillBucket(bucket, SPEC, 1_000_000);
    expect(bucket.tokens).toBe(SPEC.burst);
  });

  it("never adds tokens when the clock moves backwards", () => {
    const bucket = createBucket(SPEC, 10_000);
    bucket.tokens = 1;
    refillBucket(bucket, SPEC, 0);
    expect(bucket.tokens).toBe(1);
  });

  it("reports a retry delay of at least one second", () => {
    const bucket = createBucket(SPEC, 0);
    bucket.tokens = 0.999;
    expect(bucketRetryAfterSeconds(bucket, SPEC)).toBe(1);
  });

  it("rounds the retry delay up to whole seconds", () => {
    const slow: BucketSpec = { burst: 2, refillMs: 60_000 };
    const bucket = createBucket(slow, 0);
    bucket.tokens = 0.5;
    // Half a token missing at one token per 60s is 30s.
    expect(bucketRetryAfterSeconds(bucket, slow)).toBe(30);
  });
});
