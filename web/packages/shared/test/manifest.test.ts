import { describe, expect, it } from "vitest";
import {
  isEngineManifest,
  totalManifestBytes,
  validateManifest,
  type EngineManifest,
} from "../src/manifest.js";

const validAsset = {
  path: "generalsxzh.wasm",
  sizeBytes: 1024,
  sha256: "a".repeat(64),
};

function validManifest(): EngineManifest {
  return {
    schemaVersion: 1,
    engineVersion: "726e1f97c",
    assetBaseUrl: "https://assets.generalsx.org",
    engineEntry: "generalsxzh.js",
    assets: [validAsset],
  };
}

describe("validateManifest", () => {
  it("accepts a well-formed manifest", () => {
    const result = validateManifest(validManifest());
    expect(result.valid).toBe(true);
    expect(result.errors).toEqual([]);
  });

  it("rejects non-object input", () => {
    expect(validateManifest(null).valid).toBe(false);
    expect(validateManifest("nope").valid).toBe(false);
    expect(validateManifest([1, 2]).valid).toBe(false);
  });

  it("rejects the wrong schema version", () => {
    const result = validateManifest({ ...validManifest(), schemaVersion: 2 });
    expect(result.valid).toBe(false);
    expect(result.errors.some((e) => e.path === "schemaVersion")).toBe(true);
  });

  it("rejects a non-https asset base URL", () => {
    const result = validateManifest({ ...validManifest(), assetBaseUrl: "http://assets.generalsx.org" });
    expect(result.valid).toBe(false);
    expect(result.errors.some((e) => e.path === "assetBaseUrl")).toBe(true);
  });

  it("rejects an asset base URL with a path or query", () => {
    expect(
      validateManifest({ ...validManifest(), assetBaseUrl: "https://assets.generalsx.org/x" }).valid,
    ).toBe(false);
    expect(
      validateManifest({ ...validManifest(), assetBaseUrl: "https://assets.generalsx.org?x=1" }).valid,
    ).toBe(false);
  });

  it("rejects unsafe relative asset paths", () => {
    for (const path of ["/abs.wasm", "../escape.wasm", "https://evil.example/x.wasm", ""]) {
      const result = validateManifest({
        ...validManifest(),
        assets: [{ ...validAsset, path }],
      });
      expect(result.valid, `expected ${path} to be rejected`).toBe(false);
    }
  });

  it("rejects duplicate asset paths", () => {
    const result = validateManifest({
      ...validManifest(),
      assets: [validAsset, validAsset],
    });
    expect(result.valid).toBe(false);
    expect(result.errors.some((e) => e.message.includes("duplicate"))).toBe(true);
  });

  it("rejects a malformed sha256 digest", () => {
    const result = validateManifest({
      ...validManifest(),
      assets: [{ ...validAsset, sha256: "not-a-hash" }],
    });
    expect(result.valid).toBe(false);
    expect(result.errors.some((e) => e.path.endsWith(".sha256"))).toBe(true);
  });

  it("rejects a negative or non-integer size", () => {
    expect(
      validateManifest({ ...validManifest(), assets: [{ ...validAsset, sizeBytes: -1 }] }).valid,
    ).toBe(false);
    expect(
      validateManifest({ ...validManifest(), assets: [{ ...validAsset, sizeBytes: 1.5 }] }).valid,
    ).toBe(false);
  });

  it("collects multiple errors in a single pass", () => {
    const result = validateManifest({ schemaVersion: 2 });
    expect(result.errors.length).toBeGreaterThan(1);
  });
});

describe("isEngineManifest", () => {
  it("narrows valid manifests", () => {
    expect(isEngineManifest(validManifest())).toBe(true);
    expect(isEngineManifest({})).toBe(false);
  });
});

describe("totalManifestBytes", () => {
  it("sums asset sizes", () => {
    const manifest = validManifest();
    expect(totalManifestBytes(manifest)).toBe(1024);
    const bigger: EngineManifest = {
      ...manifest,
      assets: [validAsset, { ...validAsset, path: "other.data", sizeBytes: 2048 }],
    };
    expect(totalManifestBytes(bigger)).toBe(3072);
  });
});
