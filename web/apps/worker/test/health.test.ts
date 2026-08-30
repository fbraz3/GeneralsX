import { describe, expect, it } from "vitest";
import {
  buildLivenessReport,
  buildReadinessReport,
  releaseId,
  SERVICE_NAME,
  UNKNOWN_RELEASE_ID,
  type HealthEnv,
} from "../src/health.js";

function fullyConfigured(overrides: Partial<HealthEnv> = {}): HealthEnv {
  return {
    ALLOWED_ORIGINS: "https://play.generalsx.org",
    SIGNALING_ORIGIN: "https://signaling.generalsx.org",
    ASSET_ORIGIN: "https://assets.generalsx.org",
    RELEASE_ID: "abc123def456",
    TURN_KEY_ID: "key-123",
    TURN_KEY_API_TOKEN: "super-secret-token",
    ROOM_DO: { idFromName: () => undefined },
    ...overrides,
  };
}

function checkNamed(env: HealthEnv, name: string) {
  const check = buildReadinessReport(env).checks.find((entry) => entry.name === name);
  if (!check) throw new Error(`no readiness check named ${name}`);
  return check;
}

describe("buildLivenessReport", () => {
  it("reports the service and release without touching any dependency", () => {
    expect(buildLivenessReport(fullyConfigured())).toEqual({
      service: SERVICE_NAME,
      status: "ok",
      releaseId: "abc123def456",
    });
  });

  it("stays live even when every dependency is unconfigured", () => {
    expect(buildLivenessReport({}).status).toBe("ok");
    expect(buildLivenessReport({}).releaseId).toBe(UNKNOWN_RELEASE_ID);
  });
});

describe("releaseId", () => {
  it("falls back to a marker when unset or blank", () => {
    expect(releaseId({})).toBe(UNKNOWN_RELEASE_ID);
    expect(releaseId({ RELEASE_ID: "   " })).toBe(UNKNOWN_RELEASE_ID);
    expect(releaseId({ RELEASE_ID: " abc " })).toBe("abc");
  });
});

describe("buildReadinessReport", () => {
  it("is ready and fully ok when everything is configured", () => {
    const report = buildReadinessReport(fullyConfigured());
    expect(report.ready).toBe(true);
    expect(report.status).toBe("ok");
    expect(report.checks.every((check) => check.status === "ok")).toBe(true);
  });

  it("fails readiness without the Durable Object binding", () => {
    const report = buildReadinessReport(fullyConfigured({ ROOM_DO: undefined }));
    expect(report.ready).toBe(false);
    expect(report.status).toBe("failed");
    expect(checkNamed(fullyConfigured({ ROOM_DO: undefined }), "room-durable-object").status).toBe(
      "failed",
    );
  });

  it("fails readiness when the CORS allowlist is empty", () => {
    const report = buildReadinessReport(fullyConfigured({ ALLOWED_ORIGINS: "  ,  " }));
    expect(report.ready).toBe(false);
    expect(checkNamed(fullyConfigured({ ALLOWED_ORIGINS: "" }), "allowed-origins").status).toBe(
      "failed",
    );
  });

  it("stays ready but degraded when TURN secrets are missing", () => {
    const env = fullyConfigured({ TURN_KEY_ID: undefined, TURN_KEY_API_TOKEN: undefined });
    const report = buildReadinessReport(env);
    expect(report.ready).toBe(true);
    expect(report.status).toBe("degraded");
    expect(checkNamed(env, "turn-credentials").status).toBe("degraded");
  });

  it("stays ready but degraded without a release id", () => {
    const env = fullyConfigured({ RELEASE_ID: undefined });
    expect(buildReadinessReport(env).ready).toBe(true);
    expect(checkNamed(env, "release-id").status).toBe("degraded");
  });

  it("never leaks a secret value into the payload", () => {
    const serialized = JSON.stringify(buildReadinessReport(fullyConfigured()));
    expect(serialized).not.toContain("super-secret-token");
    expect(serialized).not.toContain("key-123");
  });
});
