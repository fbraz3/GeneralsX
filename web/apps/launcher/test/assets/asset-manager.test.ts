import { describe, expect, it, vi } from "vitest";
import { AssetIntegrityError, AssetManager, type AssetCacheStore } from "../../src/assets/asset-manager.js";
import type { EngineManifest } from "@generalsx-web/shared/manifest";

function makeInMemoryCache(): AssetCacheStore & { readonly store: Map<string, Response> } {
  const store = new Map<string, Response>();
  return {
    store,
    async match(key: string) {
      return store.get(key);
    },
    async put(key: string, response: Response) {
      store.set(key, response);
    },
    async delete(key: string) {
      return store.delete(key);
    },
  };
}

async function sha256Hex(bytes: ArrayBuffer): Promise<string> {
  const digest = await crypto.subtle.digest("SHA-256", bytes);
  return [...new Uint8Array(digest)].map((b) => b.toString(16).padStart(2, "0")).join("");
}

const ASSET_BYTES = new TextEncoder().encode("fake-engine-bytes").buffer;

async function manifestFor(sha256: string): Promise<EngineManifest> {
  return {
    schemaVersion: 1,
    engineVersion: "test-build",
    assetBaseUrl: "https://assets.generalsx.org",
    engineEntry: "engine.js",
    assets: [{ path: "engine.wasm", sizeBytes: ASSET_BYTES.byteLength, sha256 }],
  };
}

describe("AssetManager.resolveAssetUrl", () => {
  it("stays under the manifest's asset base URL", async () => {
    const manifest = await manifestFor("a".repeat(64));
    const manager = new AssetManager(manifest, { cache: makeInMemoryCache() });
    const url = manager.resolveAssetUrl(manifest.assets[0]!);
    expect(url).toBe("https://assets.generalsx.org/engine.wasm");
  });
});

describe("AssetManager.downloadAsset", () => {
  it("caches an asset whose digest matches the manifest", async () => {
    const goodHash = await sha256Hex(ASSET_BYTES);
    const manifest = await manifestFor(goodHash);
    const cache = makeInMemoryCache();
    const fetchImpl = vi.fn(async () => new Response(ASSET_BYTES.slice(0)));
    const manager = new AssetManager(manifest, { cache, fetchImpl: fetchImpl as unknown as typeof fetch });

    await manager.downloadAsset(manifest.assets[0]!);

    expect(cache.store.size).toBe(1);
    expect(fetchImpl).toHaveBeenCalledTimes(1);
  });

  it("rejects and does not cache an asset with a mismatched digest", async () => {
    const manifest = await manifestFor("b".repeat(64)); // deliberately wrong
    const cache = makeInMemoryCache();
    const fetchImpl = vi.fn(async () => new Response(ASSET_BYTES.slice(0)));
    const manager = new AssetManager(manifest, { cache, fetchImpl: fetchImpl as unknown as typeof fetch });

    await expect(manager.downloadAsset(manifest.assets[0]!)).rejects.toThrow(AssetIntegrityError);
    expect(cache.store.size).toBe(0);
  });

  it("skips the network entirely for an already-cached asset", async () => {
    const goodHash = await sha256Hex(ASSET_BYTES);
    const manifest = await manifestFor(goodHash);
    const cache = makeInMemoryCache();
    const fetchImpl = vi.fn(async () => new Response(ASSET_BYTES.slice(0)));
    const manager = new AssetManager(manifest, { cache, fetchImpl: fetchImpl as unknown as typeof fetch });

    await manager.downloadAsset(manifest.assets[0]!);
    await manager.downloadAsset(manifest.assets[0]!);

    expect(fetchImpl).toHaveBeenCalledTimes(1);
  });

  it("surfaces an HTTP error without caching anything", async () => {
    const manifest = await manifestFor("a".repeat(64));
    const cache = makeInMemoryCache();
    const fetchImpl = vi.fn(async () => new Response("nope", { status: 404 }));
    const manager = new AssetManager(manifest, { cache, fetchImpl: fetchImpl as unknown as typeof fetch });

    await expect(manager.downloadAsset(manifest.assets[0]!)).rejects.toThrow(/404/);
    expect(cache.store.size).toBe(0);
  });
});

describe("AssetManager.downloadAll", () => {
  it("stops at the first failing asset", async () => {
    const goodHash = await sha256Hex(ASSET_BYTES);
    const manifest: EngineManifest = {
      schemaVersion: 1,
      engineVersion: "test-build",
      assetBaseUrl: "https://assets.generalsx.org",
      engineEntry: "engine.js",
      assets: [
        { path: "ok.bin", sizeBytes: ASSET_BYTES.byteLength, sha256: goodHash },
        { path: "bad.bin", sizeBytes: ASSET_BYTES.byteLength, sha256: "c".repeat(64) },
        { path: "unreached.bin", sizeBytes: ASSET_BYTES.byteLength, sha256: goodHash },
      ],
    };
    const cache = makeInMemoryCache();
    const fetchImpl = vi.fn(async () => new Response(ASSET_BYTES.slice(0)));
    const manager = new AssetManager(manifest, { cache, fetchImpl: fetchImpl as unknown as typeof fetch });

    await expect(manager.downloadAll()).rejects.toThrow(AssetIntegrityError);
    expect(fetchImpl).toHaveBeenCalledTimes(2); // ok.bin, then bad.bin; unreached.bin never fetched
  });
});
