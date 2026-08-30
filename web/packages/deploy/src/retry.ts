/**
 * Bounded retry with exponential backoff for post-deploy probes.
 *
 * A deployment's public surface is not reachable at the instant the upload
 * finishes: a freshly attached custom domain has to propagate, its
 * certificate has to be issued, and a new DNS record has to reach whichever
 * resolver the smoke run happens to use. Failing on the first `ENOTFOUND`
 * would make a perfectly good deployment look broken.
 *
 * Three rules keep that from degrading into "retry until something passes":
 *
 *  1. Only *propagation-shaped* failures are retried — transport errors (DNS,
 *     TCP, TLS) and a small allowlist of statuses. A wrong security header, a
 *     stale release id, or a 404 fails on the first attempt, so a real
 *     regression is never hidden behind a two-minute wait.
 *  2. Retrying happens inside the fetch, never around a check's assertions,
 *     so an assertion is evaluated exactly once per response.
 *  3. Waiting is bounded by an attempt count *and* a run-wide wall-clock
 *     budget. Once an origin has answered it is marked reachable and never
 *     waited on again; once it has exhausted the budget, every later probe to
 *     it fails immediately. A run therefore never spends more than
 *     `totalBudgetMs` waiting in total, however many checks it runs.
 *
 * Exhausting the budget raises an error naming the origin, the attempt count,
 * the elapsed time, and the underlying failure, so an operator can tell
 * "still propagating" from "misconfigured" without re-running anything.
 */

export interface RetryPolicy {
  /** Total attempts per origin, including the first. `1` disables retrying. */
  readonly attempts: number;
  readonly initialDelayMs: number;
  readonly maxDelayMs: number;
  /** Wall-clock ceiling on time spent waiting, across the whole run. */
  readonly totalBudgetMs: number;
}

/** Roughly two minutes, which covers ordinary Cloudflare DNS propagation and
 * certificate issuance without letting a broken deployment hang CI. */
export const DEFAULT_RETRY_POLICY: RetryPolicy = Object.freeze({
  attempts: 6,
  initialDelayMs: 2_000,
  maxDelayMs: 20_000,
  totalBudgetMs: 120_000,
});

/** Disables waiting entirely (`--no-retry`), for a re-run against a
 * deployment already known to be live. */
export const NO_RETRY_POLICY: RetryPolicy = Object.freeze({
  attempts: 1,
  initialDelayMs: 0,
  maxDelayMs: 0,
  totalBudgetMs: 0,
});

/**
 * Statuses that plausibly mean "not propagated yet" rather than "wrong".
 *
 * 520-527 are Cloudflare's own edge/origin/TLS family, which is exactly what
 * a half-attached custom domain returns while its certificate is pending.
 * 429 is included so a probe backs off instead of hammering a rate limiter.
 */
const RETRYABLE_STATUSES: readonly number[] = [
  408, 425, 429, 500, 502, 503, 504, 520, 521, 522, 523, 524, 525, 526, 527,
];

export function isRetryableStatus(status: number): boolean {
  return RETRYABLE_STATUSES.includes(status);
}

/** Raised when a probe never got past a propagation-shaped failure. The
 * message is the operator-facing summary. */
export class PropagationTimeoutError extends Error {
  constructor(
    message: string,
    readonly origin: string,
    readonly attempts: number,
    readonly elapsedMs: number,
    readonly lastFailure: string,
  ) {
    super(message);
    this.name = "PropagationTimeoutError";
  }
}

export interface RetryDeps {
  readonly sleep?: (ms: number) => Promise<void>;
  readonly now?: () => number;
}

type FetchLike = (input: string, init?: RequestInit) => Promise<Response>;

const realSleep = (ms: number): Promise<void> => new Promise((resolve) => setTimeout(resolve, ms));

function describeError(error: unknown): string {
  if (error instanceof Error) {
    // Node reports DNS/TLS failures on `cause`; the outer message is usually
    // just "fetch failed", which tells an operator nothing.
    const cause = (error as { cause?: unknown }).cause;
    const causeDetail = cause instanceof Error ? `: ${cause.message}` : "";
    return `${error.message}${causeDetail}`;
  }
  return String(error);
}

function originOf(url: string): string {
  try {
    return new URL(url).origin;
  } catch {
    return url;
  }
}

function hostnameOf(origin: string): string {
  try {
    return new URL(origin).hostname;
  } catch {
    return origin;
  }
}

type OriginState =
  | { readonly kind: "reachable" }
  | { readonly kind: "exhausted"; readonly error: PropagationTimeoutError };

/**
 * Wraps `fetchImpl` so propagation-shaped failures are retried with bounded
 * exponential backoff, sharing one waiting budget across every origin a smoke
 * run touches.
 */
export function createRetryingFetch(
  fetchImpl: FetchLike,
  policy: RetryPolicy = DEFAULT_RETRY_POLICY,
  deps: RetryDeps = {},
): FetchLike {
  const sleep = deps.sleep ?? realSleep;
  const now = deps.now ?? Date.now;
  const states = new Map<string, OriginState>();
  const attemptCap = Math.max(1, policy.attempts);
  let waitedMs = 0;

  return async function retryingFetch(input: string, init?: RequestInit): Promise<Response> {
    const origin = originOf(input);
    const state = states.get(origin);
    // An origin that already answered has demonstrably propagated, so a later
    // failure from it is a real fault rather than a DNS race. And an origin
    // that already burned the budget must not burn it again on every
    // remaining check.
    if (state?.kind === "reachable") return fetchImpl(input, init);
    if (state?.kind === "exhausted") throw state.error;

    const startedAt = now();
    let delay = policy.initialDelayMs;

    for (let attempt = 1; ; attempt += 1) {
      let lastFailure: string;
      try {
        const response = await fetchImpl(input, init);
        if (!isRetryableStatus(response.status)) {
          states.set(origin, { kind: "reachable" });
          return response;
        }
        lastFailure = `HTTP ${response.status}`;
      } catch (error) {
        // Every throw from `fetch` is a transport failure — DNS, connect, or
        // TLS. None of them can be caused by a wrong header or release id.
        lastFailure = describeError(error);
      }

      const elapsed = now() - startedAt;
      const remainingBudget = policy.totalBudgetMs - waitedMs;
      if (attempt >= attemptCap || remainingBudget <= 0) {
        const timeout = new PropagationTimeoutError(
          `${origin} did not become reachable after ${attempt} attempt(s) over ${(elapsed / 1000).toFixed(1)}s. ` +
            "DNS, TLS, or custom-domain propagation may still be in progress — re-run once " +
            `"dig ${hostnameOf(origin)}" resolves and the certificate is active. Last failure: ${lastFailure}`,
          origin,
          attempt,
          elapsed,
          lastFailure,
        );
        states.set(origin, { kind: "exhausted", error: timeout });
        throw timeout;
      }

      const wait = Math.min(delay, remainingBudget);
      await sleep(wait);
      waitedMs += wait;
      delay = Math.min(delay * 2, policy.maxDelayMs);
    }
  };
}
