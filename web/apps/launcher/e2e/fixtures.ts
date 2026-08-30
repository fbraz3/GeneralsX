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

/** A couple of tiny synthetic "assets" — arbitrary bytes, not game data. */
export const FIXTURE_ASSETS: readonly FixtureAsset[] = [
  makeFixtureAsset("fixture-asset-a.bin", "generalsx-browser-smoke-fixture-a"),
  makeFixtureAsset("fixture-asset-b.bin", "generalsx-browser-smoke-fixture-b"),
];

/** A valid manifest referencing only the synthetic fixture assets above. */
export function buildFixtureManifest(): EngineManifest {
  return {
    schemaVersion: 1,
    engineVersion: "browser-smoke-fixture",
    assetBaseUrl: ASSET_ORIGIN,
    engineEntry: "engine.js",
    assets: FIXTURE_ASSETS.map((asset) => ({
      path: asset.path,
      sizeBytes: asset.bytes.byteLength,
      sha256: asset.sha256,
    })),
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
