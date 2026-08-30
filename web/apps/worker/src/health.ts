/**
 * Liveness and readiness reporting for the signaling Worker.
 *
 * Kept pure and runtime-agnostic (no `Request`/`Response`, no `fetch`) so it
 * can be unit tested without the Workers runtime, and so every field that
 * leaves the Worker is written down in one auditable place.
 *
 * **No secret value is ever read into a payload.** Secret-backed checks report
 * only whether a binding is present (`configured` / `missing`); the TURN key id
 * and API token themselves never appear in a health response, a log line, or a
 * deployment smoke-test transcript.
 */

/** The subset of the Worker environment health reporting is allowed to see. */
export interface HealthEnv {
  readonly ALLOWED_ORIGINS?: string | undefined;
  readonly SIGNALING_ORIGIN?: string | undefined;
  readonly ASSET_ORIGIN?: string | undefined;
  /** Non-secret immutable release identifier (commit SHA), injected at deploy
   * time with `wrangler deploy --var RELEASE_ID:<sha>`. Lets an operator prove
   * which immutable version is actually serving traffic after a rollback. */
  readonly RELEASE_ID?: string | undefined;
  readonly TURN_KEY_ID?: string | undefined;
  readonly TURN_KEY_API_TOKEN?: string | undefined;
  readonly ROOM_DO?: unknown;
}

export const SERVICE_NAME = "generalsx-signaling";
export const UNKNOWN_RELEASE_ID = "unknown";

/**
 * `ok` — fully configured.
 * `degraded` — serves traffic with reduced capability (documented fallback).
 * `failed` — cannot serve its core function; readiness fails.
 */
export type CheckStatus = "ok" | "degraded" | "failed";

export interface ReadinessCheck {
  readonly name: string;
  readonly status: CheckStatus;
  readonly detail: string;
}

export interface ReadinessReport {
  readonly service: string;
  readonly ready: boolean;
  readonly status: CheckStatus;
  readonly releaseId: string;
  readonly checks: readonly ReadinessCheck[];
}

export interface LivenessReport {
  readonly service: string;
  readonly status: "ok";
  readonly releaseId: string;
}

export function releaseId(env: HealthEnv): string {
  const value = env.RELEASE_ID?.trim();
  return value ? value : UNKNOWN_RELEASE_ID;
}

/**
 * Liveness: "this isolate is running and can build a response". Deliberately
 * dependency-free so a TURN outage or a misconfigured origin list can never
 * make a healthy Worker look dead to an uptime monitor.
 */
export function buildLivenessReport(env: HealthEnv): LivenessReport {
  return { service: SERVICE_NAME, status: "ok", releaseId: releaseId(env) };
}

function countOrigins(value: string | undefined): number {
  return (value ?? "")
    .split(",")
    .map((origin) => origin.trim())
    .filter((origin) => origin.length > 0).length;
}

function checkRoomDurableObject(env: HealthEnv): ReadinessCheck {
  const binding = env.ROOM_DO as { idFromName?: unknown } | undefined;
  const bound = typeof binding?.idFromName === "function";
  return {
    name: "room-durable-object",
    status: bound ? "ok" : "failed",
    detail: bound ? "ROOM_DO binding present" : "ROOM_DO binding missing",
  };
}

function checkAllowedOrigins(env: HealthEnv): ReadinessCheck {
  const count = countOrigins(env.ALLOWED_ORIGINS);
  return {
    name: "allowed-origins",
    status: count > 0 ? "ok" : "failed",
    detail: count > 0 ? `${count} origin(s) allowlisted` : "ALLOWED_ORIGINS is empty",
  };
}

function checkSignalingOrigin(env: HealthEnv): ReadinessCheck {
  const count = countOrigins(env.SIGNALING_ORIGIN);
  return {
    name: "signaling-origin",
    status: count > 0 ? "ok" : "failed",
    detail: count > 0 ? "SIGNALING_ORIGIN configured" : "SIGNALING_ORIGIN is empty",
  };
}

function checkAssetOrigin(env: HealthEnv): ReadinessCheck {
  const count = countOrigins(env.ASSET_ORIGIN);
  return {
    name: "asset-origin",
    status: count > 0 ? "ok" : "degraded",
    detail: count > 0
      ? "ASSET_ORIGIN configured"
      : "ASSET_ORIGIN is empty; CSP connect-src will not allow an asset origin",
  };
}

function checkTurnCredentials(env: HealthEnv): ReadinessCheck {
  const configured = Boolean(env.TURN_KEY_ID && env.TURN_KEY_API_TOKEN);
  return {
    name: "turn-credentials",
    status: configured ? "ok" : "degraded",
    detail: configured
      ? "TURN key id and API token are configured"
      : "TURN secrets missing; peers fall back to direct/STUN ICE only",
  };
}

function checkReleaseId(env: HealthEnv): ReadinessCheck {
  const known = releaseId(env) !== UNKNOWN_RELEASE_ID;
  return {
    name: "release-id",
    status: known ? "ok" : "degraded",
    detail: known
      ? "RELEASE_ID recorded for this version"
      : "RELEASE_ID not set; deploy with --var RELEASE_ID:<commit-sha> to make rollbacks verifiable",
  };
}

function worstStatus(checks: readonly ReadinessCheck[]): CheckStatus {
  if (checks.some((check) => check.status === "failed")) return "failed";
  if (checks.some((check) => check.status === "degraded")) return "degraded";
  return "ok";
}

/**
 * Readiness: "this version can serve room signaling for the configured
 * origins". A `degraded` result is still ready — the launcher documents and
 * surfaces TURN-less direct/STUN fallback — but it is reported so a deployment
 * smoke test can fail the release explicitly with `--require-turn`.
 */
export function buildReadinessReport(env: HealthEnv): ReadinessReport {
  const checks: readonly ReadinessCheck[] = [
    checkRoomDurableObject(env),
    checkAllowedOrigins(env),
    checkSignalingOrigin(env),
    checkAssetOrigin(env),
    checkTurnCredentials(env),
    checkReleaseId(env),
  ];
  const status = worstStatus(checks);
  return {
    service: SERVICE_NAME,
    ready: status !== "failed",
    status,
    releaseId: releaseId(env),
    checks,
  };
}
