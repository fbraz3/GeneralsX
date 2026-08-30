import { describe, expect, it } from "vitest";
import {
  ASSET_ROLES,
  MAX_ASSET_BYTES,
  assetByRole,
  assetCacheKey,
  assetsByRole,
  isEngineManifest,
  isStreamingRole,
  mountPlan,
  storageRoot,
  totalManifestBytes,
  validateManifest,
  type EngineManifest,
  type ManifestAsset,
} from "../src/manifest.js";

function asset(overrides: Partial<ManifestAsset> = {}): ManifestAsset {
  const base: ManifestAsset = {
    path: "engine/generalsxzh.js",
    role: "engine-js",
    sizeBytes: 1024,
    sha256: "a".repeat(64),
    mount: { target: "/generalsx/engine/generalsxzh.js", order: 0, streaming: false },
  };
  return { ...base, ...overrides };
}

function validManifest(): EngineManifest {
  return {
    schemaVersion: 2,
    engineVersion: "726e1f97c",
    assetsRevision: 3,
    assetBaseUrl: "https://assets.generalsx.org",
    assets: [
      asset(),
      asset({
        path: "engine/generalsxzh.wasm",
        role: "engine-wasm",
        sha256: "b".repeat(64),
        mount: { target: "/generalsx/engine/generalsxzh.wasm", order: 1, streaming: false },
      }),
      asset({
        path: "base/INI.big",
        role: "big-base",
        sizeBytes: 4096,
        sha256: "c".repeat(64),
        mount: { target: "/generalsx/base/INI.big", order: 100, streaming: true },
      }),
      asset({
        path: "expansion/INIZH.big",
        role: "big-expansion",
        sizeBytes: 8192,
        sha256: "d".repeat(64),
        mount: { target: "/generalsx/expansion/INIZH.big", order: 200, streaming: true },
      }),
      asset({
        path: "scripts/Default.ini",
        role: "script",
        sizeBytes: 16,
        sha256: "e".repeat(64),
        mount: { target: "/generalsx/scripts/Default.ini", order: 300, streaming: false },
      }),
      asset({
        path: "fonts/Generals.ttf",
        role: "font",
        sizeBytes: 32,
        sha256: "f".repeat(64),
        mount: { target: "/generalsx/fonts/Generals.ttf", order: 400, streaming: false },
      }),
    ],
  };
}

function withAssets(assets: readonly unknown[]): Record<string, unknown> {
  return { ...validManifest(), assets };
}

function errorPaths(input: unknown): readonly string[] {
  return validateManifest(input).errors.map((error) => error.path);
}

describe("validateManifest", () => {
  it("accepts a well-formed manifest", () => {
    const result = validateManifest(validManifest());
    expect(result.errors).toEqual([]);
    expect(result.valid).toBe(true);
  });

  it("rejects non-object input", () => {
    expect(validateManifest(null).valid).toBe(false);
    expect(validateManifest("nope").valid).toBe(false);
    expect(validateManifest([1, 2]).valid).toBe(false);
  });

  it("rejects the wrong schema version", () => {
    expect(errorPaths({ ...validManifest(), schemaVersion: 1 })).toContain("schemaVersion");
  });

  it("requires a positive integer assets revision", () => {
    expect(errorPaths({ ...validManifest(), assetsRevision: 0 })).toContain("assetsRevision");
    expect(errorPaths({ ...validManifest(), assetsRevision: 1.5 })).toContain("assetsRevision");
    expect(errorPaths({ ...validManifest(), assetsRevision: "3" })).toContain("assetsRevision");
  });

  it("rejects an unsafe engine version", () => {
    expect(errorPaths({ ...validManifest(), engineVersion: "../etc" })).toContain("engineVersion");
    expect(errorPaths({ ...validManifest(), engineVersion: "" })).toContain("engineVersion");
  });

  it("rejects an unsafe asset base URL", () => {
    for (const assetBaseUrl of [
      "http://assets.generalsx.org",
      "https://assets.generalsx.org/",
      "https://assets.generalsx.org/zh/r7/",
      "https://assets.generalsx.org/zh/../r7",
      "https://assets.generalsx.org/zh//r7",
      "https://assets.generalsx.org?x=1",
      "https://assets.generalsx.org#f",
      "https://user:pass@assets.generalsx.org",
      "not a url",
    ]) {
      expect(errorPaths({ ...validManifest(), assetBaseUrl }), assetBaseUrl).toContain("assetBaseUrl");
    }
  });

  it("accepts a canonical path prefix so each revision can own a key prefix", () => {
    for (const assetBaseUrl of [
      "https://assets.generalsx.org",
      "https://assets.generalsx.org/zh/r7",
      "https://assets.generalsx.org:8443/zh/r7",
    ]) {
      expect(errorPaths({ ...validManifest(), assetBaseUrl }), assetBaseUrl).not.toContain("assetBaseUrl");
    }
  });

  it("rejects unsafe relative asset paths", () => {
    for (const path of ["/abs.wasm", "../escape.wasm", "https://evil.example/x.wasm", "", "a//b", "a\\b"]) {
      const result = validateManifest(withAssets([asset({ path }), ...validManifest().assets.slice(1)]));
      expect(result.valid, `expected ${path} to be rejected`).toBe(false);
    }
  });

  it("rejects duplicate asset paths, mount targets, and mount orders", () => {
    const base = validManifest();
    const duplicatePath = withAssets([base.assets[0], { ...base.assets[1], path: base.assets[0]!.path }, ...base.assets.slice(2)]);
    expect(validateManifest(duplicatePath).errors.some((e) => e.message.includes("duplicate asset path"))).toBe(true);

    const duplicateTarget = withAssets([
      base.assets[0],
      { ...base.assets[1], mount: base.assets[0]!.mount },
      ...base.assets.slice(2),
    ]);
    expect(
      validateManifest(duplicateTarget).errors.some((e) => e.message.includes("duplicate mount target")),
    ).toBe(true);

    const duplicateOrder = withAssets([
      base.assets[0],
      { ...base.assets[1], mount: { ...base.assets[1]!.mount, order: 0 } },
      ...base.assets.slice(2),
    ]);
    expect(
      validateManifest(duplicateOrder).errors.some((e) => e.message.includes("duplicate mount order")),
    ).toBe(true);
  });

  it("rejects unsafe mount targets", () => {
    const base = validManifest();
    for (const target of ["relative/path", "/", "/a//b", "/a/../b", "/trailing/", "/a\\b"]) {
      const broken = withAssets([
        { ...base.assets[0], mount: { target, order: 0, streaming: false } },
        ...base.assets.slice(1),
      ]);
      expect(errorPaths(broken), target).toContain("assets[0].mount.target");
    }
  });

  it("rejects an unknown role", () => {
    const base = validManifest();
    expect(errorPaths(withAssets([{ ...base.assets[0], role: "cheat-engine" }, ...base.assets.slice(1)]))).toContain(
      "assets[0].role",
    );
  });

  it("requires streaming to match the role", () => {
    const base = validManifest();
    const scriptStreaming = withAssets([
      ...base.assets.slice(0, 4),
      { ...base.assets[4], mount: { ...base.assets[4]!.mount, streaming: true } },
      base.assets[5],
    ]);
    expect(errorPaths(scriptStreaming)).toContain("assets[4].mount.streaming");

    const archiveNotStreaming = withAssets([
      ...base.assets.slice(0, 2),
      { ...base.assets[2], mount: { ...base.assets[2]!.mount, streaming: false } },
      ...base.assets.slice(3),
    ]);
    expect(errorPaths(archiveNotStreaming)).toContain("assets[2].mount.streaming");
  });

  it("requires exactly one engine-js and one engine-wasm asset", () => {
    const base = validManifest();
    const noEngine = withAssets(base.assets.slice(2));
    const messages = validateManifest(noEngine).errors.map((e) => e.message);
    expect(messages.some((m) => m.includes('exactly one "engine-js"'))).toBe(true);
    expect(messages.some((m) => m.includes('exactly one "engine-wasm"'))).toBe(true);

    const twoWasm = withAssets([
      ...base.assets,
      asset({
        path: "engine/other.wasm",
        role: "engine-wasm",
        sha256: "1".repeat(64),
        mount: { target: "/generalsx/engine/other.wasm", order: 500, streaming: false },
      }),
    ]);
    expect(
      validateManifest(twoWasm).errors.some((e) => e.message.includes('exactly one "engine-wasm"')),
    ).toBe(true);
  });

  it("allows at most one engine-data asset", () => {
    const base = validManifest();
    const twoData = withAssets([
      ...base.assets,
      asset({
        path: "engine/a.data",
        role: "engine-data",
        sha256: "2".repeat(64),
        mount: { target: "/generalsx/engine/a.data", order: 600, streaming: false },
      }),
      asset({
        path: "engine/b.data",
        role: "engine-data",
        sha256: "3".repeat(64),
        mount: { target: "/generalsx/engine/b.data", order: 601, streaming: false },
      }),
    ]);
    expect(validateManifest(twoData).errors.some((e) => e.message.includes('at most one "engine-data"'))).toBe(
      true,
    );
  });

  it("requires at least one base archive", () => {
    const base = validManifest();
    const noBase = withAssets(base.assets.filter((entry) => entry.role !== "big-base"));
    expect(
      validateManifest(noBase).errors.some((e) => e.message.includes('at least one "big-base"')),
    ).toBe(true);
  });

  it("requires expansion archives to mount after base archives", () => {
    const base = validManifest();
    const inverted = withAssets([
      ...base.assets.slice(0, 3),
      { ...base.assets[3], mount: { ...base.assets[3]!.mount, order: 50 } },
      ...base.assets.slice(4),
    ]);
    expect(
      validateManifest(inverted).errors.some((e) => e.message.includes("must exceed every")),
    ).toBe(true);
  });

  it("rejects a malformed sha256 digest", () => {
    const base = validManifest();
    for (const sha256 of ["not-a-hash", "A".repeat(64), "a".repeat(63)]) {
      expect(errorPaths(withAssets([{ ...base.assets[0], sha256 }, ...base.assets.slice(1)])), sha256).toContain(
        "assets[0].sha256",
      );
    }
  });

  it("rejects sizes outside the supported range", () => {
    const base = validManifest();
    for (const sizeBytes of [0, -1, 1.5, MAX_ASSET_BYTES + 1]) {
      expect(
        errorPaths(withAssets([{ ...base.assets[0], sizeBytes }, ...base.assets.slice(1)])),
        String(sizeBytes),
      ).toContain("assets[0].sizeBytes");
    }
  });

  it("accepts a strong etag and rejects a weak one", () => {
    const base = validManifest();
    const strong = withAssets([{ ...base.assets[0], etag: '"abc123"' }, ...base.assets.slice(1)]);
    expect(validateManifest(strong).valid).toBe(true);

    for (const etag of ['W/"abc123"', "abc123", '""']) {
      expect(errorPaths(withAssets([{ ...base.assets[0], etag }, ...base.assets.slice(1)])), etag).toContain(
        "assets[0].etag",
      );
    }
  });

  it("rejects an empty or non-array assets list", () => {
    expect(errorPaths({ ...validManifest(), assets: [] })).toContain("assets");
    expect(errorPaths({ ...validManifest(), assets: "nope" })).toContain("assets");
  });

  it("collects multiple errors in a single pass", () => {
    expect(validateManifest({ schemaVersion: 1 }).errors.length).toBeGreaterThan(1);
  });
});

describe("manifest helpers", () => {
  it("narrows valid manifests", () => {
    expect(isEngineManifest(validManifest())).toBe(true);
    expect(isEngineManifest({})).toBe(false);
  });

  it("sums asset sizes", () => {
    expect(totalManifestBytes(validManifest())).toBe(1024 + 1024 + 4096 + 8192 + 16 + 32);
  });

  it("selects assets by role", () => {
    const manifest = validManifest();
    expect(assetByRole(manifest, "engine-wasm")?.path).toBe("engine/generalsxzh.wasm");
    expect(assetsByRole(manifest, "big-base").map((entry) => entry.path)).toEqual(["base/INI.big"]);
    expect(assetByRole(manifest, "engine-data")).toBeUndefined();
  });

  it("orders the mount plan so expansion archives override base archives", () => {
    const plan = mountPlan(validManifest());
    const baseIndex = plan.findIndex((entry) => entry.role === "big-base");
    const expansionIndex = plan.findIndex((entry) => entry.role === "big-expansion");
    expect(expansionIndex).toBeGreaterThan(baseIndex);
    expect(plan.map((entry) => entry.mount.order)).toEqual([0, 1, 100, 200, 300, 400]);
  });

  it("derives an immutable storage root and content-addressed cache key", () => {
    const manifest = validManifest();
    expect(storageRoot(manifest)).toBe("726e1f97c-r3");
    expect(storageRoot({ ...manifest, assetsRevision: 4 })).toBe("726e1f97c-r4");
    expect(assetCacheKey(manifest.assets[0]!)).toBe(manifest.assets[0]!.sha256);
  });

  it("marks only archive roles as streaming", () => {
    const streaming = ASSET_ROLES.filter((role) => isStreamingRole(role));
    expect(streaming).toEqual(["big-base", "big-expansion"]);
  });
});
