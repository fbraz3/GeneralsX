import { describe, expect, it } from "vitest";
import { buildManifest, inferAssetRole, type SourceFile } from "../src/asset-plan.js";
import type { EngineBuildMetadata } from "../src/engine-metadata.js";
import { validateManifest } from "../src/manifest.js";
import { compatibilityFor } from "../src/protocol.js";

function file(relativePath: string, overrides: Partial<SourceFile> = {}): SourceFile {
  return {
    relativePath,
    sizeBytes: 128,
    sha256: "0".repeat(64),
    ...overrides,
  };
}

const STAGING: readonly SourceFile[] = [
  file("engine/generalsxzh.js", { sha256: "a".repeat(64) }),
  file("engine/generalsxzh.wasm", { sha256: "b".repeat(64) }),
  file("engine/generalsxzh.data", { sha256: "c".repeat(64) }),
  file("base/INI.big", { sha256: "d".repeat(64), sizeBytes: 4096 }),
  file("base/Textures.big", { sha256: "e".repeat(64), sizeBytes: 8192 }),
  file("expansion/INIZH.big", { sha256: "f".repeat(64), sizeBytes: 2048 }),
  file("scripts/Default.ini", { sha256: "1".repeat(64) }),
  file("fonts/Generals.ttf", { sha256: "2".repeat(64) }),
];

const OPTIONS = {
  engineVersion: "2026.08.30-a1b2c3d",
  engineMetadata: {
    schemaVersion: 1,
    content: "zero-hour",
    deterministicMath: true,
    compatibility: compatibilityFor("zero-hour", true),
    artifacts: {
      "engine-js": { fileName: "GeneralsXZH.js", sha256: "a".repeat(64) },
      "engine-wasm": { fileName: "GeneralsXZH.wasm", sha256: "b".repeat(64) },
    },
  } satisfies EngineBuildMetadata,
  assetsRevision: 4,
  assetBaseUrl: "https://assets.generalsx.org",
};

describe("inferAssetRole", () => {
  it("maps the documented staging layout to roles", () => {
    expect(inferAssetRole("engine/generalsxzh.js")).toBe("engine-js");
    expect(inferAssetRole("engine/generalsxzh.wasm")).toBe("engine-wasm");
    expect(inferAssetRole("engine/generalsxzh.data")).toBe("engine-data");
    expect(inferAssetRole("base/INI.big")).toBe("big-base");
    expect(inferAssetRole("expansion/INIZH.big")).toBe("big-expansion");
    expect(inferAssetRole("scripts/Default.ini")).toBe("script");
    expect(inferAssetRole("scripts/SkirmishScripts.scb")).toBe("script");
    expect(inferAssetRole("fonts/Generals.TTF")).toBe("font");
  });

  it("refuses to guess for unclassifiable files", () => {
    expect(inferAssetRole("loose/Unknown.big")).toBeUndefined();
    expect(inferAssetRole("readme")).toBeUndefined();
    expect(inferAssetRole("saves/game.sav")).toBeUndefined();
  });
});

describe("buildManifest", () => {
  it("produces a manifest that passes validation", () => {
    const plan = buildManifest(STAGING, OPTIONS);
    expect(plan.errors).toEqual([]);
    expect(validateManifest(plan.manifest).valid).toBe(true);
    expect(plan.manifest.schemaVersion).toBe(3);
    expect(plan.manifest.assetsRevision).toBe(4);
  });

  it("orders expansion archives after base archives", () => {
    const { manifest } = buildManifest(STAGING, OPTIONS);
    const orders = new Map(manifest.assets.map((asset) => [asset.path, asset.mount.order]));
    expect(orders.get("expansion/INIZH.big")).toBeGreaterThan(orders.get("base/Textures.big")!);
  });

  it("marks archives as streaming and loose files as buffered", () => {
    const { manifest } = buildManifest(STAGING, OPTIONS);
    const byPath = new Map(manifest.assets.map((asset) => [asset.path, asset]));
    expect(byPath.get("base/INI.big")?.mount.streaming).toBe(true);
    expect(byPath.get("expansion/INIZH.big")?.mount.streaming).toBe(true);
    expect(byPath.get("scripts/Default.ini")?.mount.streaming).toBe(false);
    expect(byPath.get("engine/generalsxzh.wasm")?.mount.streaming).toBe(false);
  });

  it("maps roles to the filesystem paths consumed by the engine", () => {
    const { manifest } = buildManifest(STAGING, OPTIONS);
    const byPath = new Map(manifest.assets.map((asset) => [asset.path, asset.mount.target]));
    expect(byPath.get("engine/generalsxzh.js")).toBe("/engine/generalsxzh.js");
    expect(byPath.get("base/INI.big")).toBe("/game-base/INI.big");
    expect(byPath.get("expansion/INIZH.big")).toBe("/game/INIZH.big");
    expect(byPath.get("scripts/Default.ini")).toBe("/game/Data/Scripts/Default.ini");
    expect(byPath.get("fonts/Generals.ttf")).toBe("/fonts/default.ttf");
  });

  it("mounts every asset beneath the configured prefix", () => {
    const { manifest } = buildManifest(STAGING, { ...OPTIONS, mountPrefix: "/zh/" });
    for (const asset of manifest.assets) {
      expect(asset.mount.target).toBe(`/zh/${asset.path}`);
    }
  });

  it("reports unclassified files instead of guessing", () => {
    const plan = buildManifest([...STAGING, file("extras/mystery.bin")], OPTIONS);
    expect(plan.unclassified).toEqual(["extras/mystery.bin"]);
    expect(plan.manifest.assets.some((asset) => asset.path === "extras/mystery.bin")).toBe(false);
  });

  it("honors an explicit role override", () => {
    const plan = buildManifest(
      [...STAGING, file("extras/Custom.big", { role: "big-expansion", sha256: "3".repeat(64) })],
      OPTIONS,
    );
    expect(plan.unclassified).toEqual([]);
    expect(plan.manifest.assets.find((asset) => asset.path === "extras/Custom.big")?.role).toBe(
      "big-expansion",
    );
    expect(plan.errors).toEqual([]);
  });

  it("carries a strong etag through when the operator supplies one", () => {
    const plan = buildManifest(
      STAGING.map((entry) =>
        entry.relativePath === "base/INI.big" ? { ...entry, etag: '"abc-1"' } : entry,
      ),
      OPTIONS,
    );
    expect(plan.manifest.assets.find((asset) => asset.path === "base/INI.big")?.etag).toBe('"abc-1"');
    expect(plan.errors).toEqual([]);
  });

  it("is deterministic regardless of input ordering", () => {
    const forward = buildManifest(STAGING, OPTIONS).manifest;
    const reversed = buildManifest([...STAGING].reverse(), OPTIONS).manifest;
    expect(JSON.stringify(reversed)).toBe(JSON.stringify(forward));
  });

  it("rejects a noncanonical profile before producing a manifest", () => {
    const wrongProfile: EngineBuildMetadata = {
      ...OPTIONS.engineMetadata,
      compatibility: compatibilityFor("generals", true),
    };
    expect(() => buildManifest(STAGING, { ...OPTIONS, engineMetadata: wrongProfile })).toThrow(
      /does not match zero-hour/,
    );
  });

  it("rejects metadata bound to a different staged engine artifact", () => {
    const wrongArtifact: EngineBuildMetadata = {
      ...OPTIONS.engineMetadata,
      artifacts: {
        ...OPTIONS.engineMetadata.artifacts,
        "engine-wasm": {
          ...OPTIONS.engineMetadata.artifacts["engine-wasm"],
          sha256: "9".repeat(64),
        },
      },
    };
    expect(() => buildManifest(STAGING, { ...OPTIONS, engineMetadata: wrongArtifact })).toThrow(
      /engine-wasm.*does not match/,
    );
  });

  it("surfaces validation errors instead of throwing", () => {
    const plan = buildManifest([file("scripts/Default.ini")], OPTIONS);
    expect(plan.errors.length).toBeGreaterThan(0);
    expect(plan.errors.some((error) => error.message.includes('exactly one "engine-js"'))).toBe(true);
  });
});
