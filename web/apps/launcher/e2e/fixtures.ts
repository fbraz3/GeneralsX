/**
 * Synthetic, disposable fixture data for the browser smoke test.
 *
 * IMPORTANT: nothing in this file is derived from, or resembles, retail
 * Command & Conquer: Generals / Zero Hour game data. Every byte is generated
 * at test time from arbitrary text; the "engine manifest" and "asset" names
 * below are placeholders that only need to satisfy the launcher's own
 * `EngineManifest` schema (see `@generalsx-web/shared/manifest`) and its
 * SHA-256 integrity check.
 */
import { createHash } from "node:crypto";
import type { EngineManifest } from "@generalsx-web/shared/manifest";
import { compatibilityFor } from "@generalsx-web/shared/protocol";

export const ASSET_ORIGIN = "https://assets.generalsx.org";
export const SIGNALING_ORIGIN = "https://signaling.generalsx.org";

export interface FixtureAsset {
  readonly path: string;
  readonly bytes: Uint8Array;
  readonly sha256: string;
}

function sha256Hex(bytes: Uint8Array): string {
  return createHash("sha256").update(bytes).digest("hex");
}

function makeFixtureAsset(path: string, content: string): FixtureAsset {
  const bytes = new TextEncoder().encode(content);
  return { path, bytes, sha256: sha256Hex(bytes) };
}

const FAKE_ENGINE_SCRIPT = `
globalThis.ENV = {};
globalThis.FS = {
  mkdirTree() {},
  writeFile() {},
  open() { return {}; },
  allocate() {},
  write(_stream, _data, _offset, length) { return length; },
  close() {}
};
let dependencies = 0;
globalThis.addRunDependency = () => { dependencies += 1; };
globalThis.removeRunDependency = () => {
  dependencies -= 1;
  if (dependencies === 0) queueMicrotask(() => Module.onRuntimeInitialized());
};
globalThis.abort = (reason) => { throw new Error(reason); };
Module.preRun.forEach((callback) => callback());
if (dependencies === 0) queueMicrotask(() => Module.onRuntimeInitialized());
`;

/** Tiny synthetic engine and game-data fixtures — arbitrary bytes, not game data. */
export const FIXTURE_ASSETS: readonly FixtureAsset[] = [
  makeFixtureAsset("engine/GeneralsXZH.js", FAKE_ENGINE_SCRIPT),
  makeFixtureAsset("engine/GeneralsXZH.wasm", "synthetic-wasm-placeholder"),
  makeFixtureAsset("base/Fixture.big", "synthetic-base-archive"),
  makeFixtureAsset("scripts/Scripts.ini", "synthetic-script-fixture"),
];

/** A valid manifest referencing only the synthetic fixture assets above. */
export function buildFixtureManifest(): EngineManifest {
  return {
    schemaVersion: 3,
    engineVersion: "browser-smoke-fixture",
    compatibility: compatibilityFor("zero-hour", true),
    assetsRevision: 1,
    assetBaseUrl: ASSET_ORIGIN,
    assets: FIXTURE_ASSETS.map((asset, index) => {
      const role =
        index === 0 ? "engine-js" : index === 1 ? "engine-wasm" : index === 2 ? "big-base" : "script";
      return {
        path: asset.path,
        role,
        sizeBytes: asset.bytes.byteLength,
        sha256: asset.sha256,
        mount: {
          target:
            index < 2
              ? `/${asset.path}`
              : index === 2
                ? "/game-base/Fixture.big"
                : "/game/Data/Scripts/Scripts.ini",
          order: index * 1000,
          streaming: role === "big-base",
        },
      };
    }),
  };
}

/** Same shape as {@link buildFixtureManifest}, but one asset's declared
 * digest is deliberately wrong, to exercise the integrity-failure path. */
export function buildTamperedFixtureManifest(): EngineManifest {
  const manifest = buildFixtureManifest();
  const [first, ...rest] = manifest.assets;
  if (!first) throw new Error("expected at least one fixture asset");
  return {
    ...manifest,
    engineVersion: "browser-smoke-fixture-tampered",
    assets: [{ ...first, sha256: "0".repeat(64) }, ...rest],
  };
}
