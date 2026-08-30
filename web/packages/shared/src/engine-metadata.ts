import {
  compatibilityFor,
  isCompatibilityVersion,
  type CompatibilityVersion,
  type GameContentIdentity,
} from "./protocol.ts";

const SHA256_RE = /^[0-9a-f]{64}$/;
const ENGINE_ARTIFACT_ROLES = ["engine-js", "engine-wasm"] as const;

export type EngineArtifactRole = (typeof ENGINE_ARTIFACT_ROLES)[number];

export interface EngineArtifactIdentity {
  readonly fileName: string;
  readonly sha256: string;
}

export interface EngineBuildMetadata {
  readonly schemaVersion: 1;
  readonly content: GameContentIdentity;
  readonly deterministicMath: boolean;
  readonly compatibility: CompatibilityVersion;
  readonly artifacts: Readonly<Record<EngineArtifactRole, EngineArtifactIdentity>>;
}

export interface StagedEngineArtifact {
  readonly role: EngineArtifactRole;
  readonly relativePath: string;
  readonly sha256: string;
}

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function parseArtifact(
  artifacts: Record<string, unknown>,
  role: EngineArtifactRole,
): EngineArtifactIdentity {
  const value = artifacts[role];
  if (!isRecord(value)) throw new Error(`engine metadata is missing "${role}" identity`);
  if (
    typeof value.fileName !== "string" ||
    value.fileName.length === 0 ||
    value.fileName.includes("/") ||
    value.fileName.includes("\\")
  ) {
    throw new Error(`engine metadata "${role}" fileName must be a base file name`);
  }
  if (typeof value.sha256 !== "string" || !SHA256_RE.test(value.sha256)) {
    throw new Error(`engine metadata "${role}" sha256 must be 64 lowercase hexadecimal characters`);
  }
  return Object.freeze({ fileName: value.fileName, sha256: value.sha256 });
}

/**
 * Parses CMake's post-build engine metadata and rejects profiles that do not
 * match the authoritative content/math compatibility matrix.
 */
export function parseEngineBuildMetadata(value: unknown): EngineBuildMetadata {
  if (!isRecord(value) || value.schemaVersion !== 1) {
    throw new Error("engine metadata schemaVersion must be 1");
  }
  if (value.content !== "generals" && value.content !== "zero-hour") {
    throw new Error('engine metadata content must be "generals" or "zero-hour"');
  }
  if (typeof value.deterministicMath !== "boolean") {
    throw new Error("engine metadata deterministicMath must be boolean");
  }
  if (!isCompatibilityVersion(value.compatibility)) {
    throw new Error("engine metadata compatibility is invalid");
  }

  const expected = compatibilityFor(value.content, value.deterministicMath);
  if (
    value.compatibility.engine !== expected.engine ||
    value.compatibility.protocol !== expected.protocol ||
    value.compatibility.determinism !== expected.determinism
  ) {
    throw new Error(
      `engine metadata compatibility does not match ${value.content} ` +
        `${value.deterministicMath ? "GameMath" : "platform-math"} build identity`,
    );
  }
  if (!isRecord(value.artifacts)) throw new Error("engine metadata artifacts are invalid");

  return Object.freeze({
    schemaVersion: 1,
    content: value.content,
    deterministicMath: value.deterministicMath,
    compatibility: expected,
    artifacts: Object.freeze({
      "engine-js": parseArtifact(value.artifacts, "engine-js"),
      "engine-wasm": parseArtifact(value.artifacts, "engine-wasm"),
    }),
  });
}

/**
 * Verifies that metadata was generated for the exact staged Emscripten JS and
 * wasm outputs. The returned profile is safe to copy into the asset manifest,
 * whose asset digests preserve this binding at runtime.
 */
export function verifyEngineBuildMetadata(
  metadata: EngineBuildMetadata,
  stagedArtifacts: readonly StagedEngineArtifact[],
): CompatibilityVersion {
  for (const role of ENGINE_ARTIFACT_ROLES) {
    const candidates = stagedArtifacts.filter((artifact) => artifact.role === role);
    if (candidates.length !== 1) {
      throw new Error(`engine metadata verification requires exactly one staged "${role}" artifact`);
    }
    const staged = candidates[0]!;
    const expected = metadata.artifacts[role];
    if (staged.sha256 !== expected.sha256) {
      throw new Error(
        `engine metadata "${role}" digest for ${expected.fileName} does not match ` +
          `${staged.relativePath}`,
      );
    }
  }
  return metadata.compatibility;
}
