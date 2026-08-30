import { describe, expect, it } from "vitest";
import {
  parseEngineBuildMetadata,
  verifyEngineBuildMetadata,
  type EngineBuildMetadata,
  type StagedEngineArtifact,
} from "../src/engine-metadata.js";
import { compatibilityFor } from "../src/protocol.js";

const JS_SHA256 = "a".repeat(64);
const WASM_SHA256 = "b".repeat(64);

function metadata(): EngineBuildMetadata {
  return {
    schemaVersion: 1,
    content: "zero-hour",
    deterministicMath: true,
    compatibility: compatibilityFor("zero-hour", true),
    artifacts: {
      "engine-js": { fileName: "GeneralsXZH.js", sha256: JS_SHA256 },
      "engine-wasm": { fileName: "GeneralsXZH.wasm", sha256: WASM_SHA256 },
    },
  };
}

function staged(): StagedEngineArtifact[] {
  return [
    { role: "engine-js", relativePath: "engine/generalsxzh.js", sha256: JS_SHA256 },
    { role: "engine-wasm", relativePath: "engine/generalsxzh.wasm", sha256: WASM_SHA256 },
  ];
}

describe("engine build metadata", () => {
  it("accepts the canonical profile bound to the exact staged engine outputs", () => {
    const parsed = parseEngineBuildMetadata(metadata());
    expect(verifyEngineBuildMetadata(parsed, staged())).toEqual(compatibilityFor("zero-hour", true));
  });

  it("rejects a wrong profile even when its artifact digests match", () => {
    const wrongProfile = {
      ...metadata(),
      compatibility: compatibilityFor("generals", true),
    };
    expect(() => parseEngineBuildMetadata(wrongProfile)).toThrow(/does not match zero-hour/);
  });

  it("rejects metadata generated for a different engine artifact", () => {
    const wrongArtifact = staged().map((artifact) =>
      artifact.role === "engine-wasm" ? { ...artifact, sha256: "c".repeat(64) } : artifact,
    );
    const parsed = parseEngineBuildMetadata(metadata());
    expect(() => verifyEngineBuildMetadata(parsed, wrongArtifact)).toThrow(
      /engine-wasm.*does not match/,
    );
  });

  it("fails closed when metadata or either engine identity is missing", () => {
    expect(() => parseEngineBuildMetadata(undefined)).toThrow(/schemaVersion/);
    const parsed = parseEngineBuildMetadata(metadata());
    expect(() => verifyEngineBuildMetadata(parsed, staged().slice(0, 1))).toThrow(
      /exactly one staged "engine-wasm"/,
    );
  });
});
