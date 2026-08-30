import { describe, expect, it, vi } from "vitest";
import { sha256Hex } from "@generalsx-web/shared/sha256";
import type { EngineManifest, ManifestAsset } from "@generalsx-web/shared/manifest";
import { AssetManager } from "../../src/assets/asset-manager.js";
import {
  AssetCancelledError,
  AssetDownloadError,
  AssetIntegrityError,
  AssetSizeError,
  AssetStorageQuotaError,
} from "../../src/assets/errors.js";
import {
  MemoryAssetStorage,
  OpfsAssetStorage,
  type AssetFileStore,
  type AssetStorage,
  type AssetWriteStream,
} from "../../src/assets/storage.js";
import { createAssetServer, type ServedFile } from "./asset-server.js";
import { FakeDirectoryHandle } from "./fake-opfs.js";

const encoder = new TextEncoder();

function payload(text: string): Uint8Array {
  return encoder.encode(text);
}

const ENGINE_JS = payload("// emscripten glue\n");
const ENGINE_WASM = payload("\0asm-not-a-real-module");
const BASE_BIG = payload("BIGF-base-archive-contents-0123456789");
const EXPANSION_BIG = payload("BIGF-expansion-archive-contents");

function assetFor(
  path: string,
  bytes: Uint8Array,
  role: ManifestAsset["role"],
  order: number,
  extras: Partial<ManifestAsset> = {},
): ManifestAsset {
  const streaming = role === "big-base" || role === "big-expansion";
  return {
    path,
    role,
    sizeBytes: bytes.length,
    sha256: sha256Hex(bytes),
    mount: { target: `/generalsx/${path}`, order, streaming },
    ...extras,
  };
}

function manifestFor(assets: readonly ManifestAsset[], overrides: Partial<EngineManifest> = {}): EngineManifest {
  return {
    schemaVersion: 3,
    engineVersion: "test-build",
    compatibility: { engine: 2, protocol: 2, determinism: 2 },
    assetsRevision: 1,
    assetBaseUrl: "https://assets.generalsx.org",
    assets,
    ...overrides,
  } as EngineManifest;
}

/** Single-asset fixture used by most download-path tests. */
function singleAsset(extras: Partial<ManifestAsset> = {}): {
  manifest: EngineManifest;
  asset: ManifestAsset;
} {
  const asset = assetFor("engine/engine.wasm", ENGINE_WASM, "engine-wasm", 0, extras);
  return { manifest: manifestFor([asset]), asset };
}

function fullManifest(): EngineManifest {
  return manifestFor([
    assetFor("engine/engine.js", ENGINE_JS, "engine-js", 0),
    assetFor("engine/engine.wasm", ENGINE_WASM, "engine-wasm", 1),
    assetFor("base/INI.big", BASE_BIG, "big-base", 100),
    assetFor("expansion/INIZH.big", EXPANSION_BIG, "big-expansion", 200),
  ]);
}

function servedFrom(manifest: EngineManifest, patches: Record<string, Partial<ServedFile>> = {}) {
  const bytesByPath: Record<string, Uint8Array> = {
    "engine/engine.js": ENGINE_JS,
    "engine/engine.wasm": ENGINE_WASM,
    "base/INI.big": BASE_BIG,
    "expansion/INIZH.big": EXPANSION_BIG,
    "scripts/engine-copy.ini": ENGINE_JS,
  };
  const files: Record<string, ServedFile> = {};
  for (const asset of manifest.assets) {
    const bytes = bytesByPath[asset.path] ?? payload(asset.path);
    files[asset.path] = {
      bytes,
      ...(asset.etag === undefined ? {} : { etag: asset.etag }),
      ...(patches[asset.path] ?? {}),
    };
  }
  return createAssetServer(files);
}

function memoryStorage(budgetBytes?: number): AssetStorage {
  return budgetBytes === undefined ? new MemoryAssetStorage() : new MemoryAssetStorage(budgetBytes);
}

function managerFor(
  manifest: EngineManifest,
  server: { fetchImpl: typeof fetch },
  options: Partial<ConstructorParameters<typeof AssetManager>[1]> = {},
): AssetManager {
  return new AssetManager(manifest, {
    storage: memoryStorage(),
    fetchImpl: server.fetchImpl,
    ...options,
  });
}

describe("AssetManager.resolveAssetUrl", () => {
  it("stays under the manifest's asset base URL", () => {
    const { manifest, asset } = singleAsset();
    const manager = managerFor(manifest, servedFrom(manifest));
    expect(manager.resolveAssetUrl(asset)).toBe("https://assets.generalsx.org/engine/engine.wasm");
  });

  it("keeps a revision-scoped path prefix in front of every asset", () => {
    const { manifest, asset } = singleAsset();
    const prefixed = { ...manifest, assetBaseUrl: "https://assets.generalsx.org/zh/r7" };
    const manager = managerFor(prefixed, servedFrom(manifest));
    expect(manager.resolveAssetUrl(asset)).toBe("https://assets.generalsx.org/zh/r7/engine/engine.wasm");
  });

  it("refuses a path that escapes a prefixed asset base URL", () => {
    const { manifest, asset } = singleAsset();
    const prefixed = { ...manifest, assetBaseUrl: "https://assets.generalsx.org/zh/r7" };
    const manager = managerFor(prefixed, servedFrom(manifest));
    expect(() => manager.resolveAssetUrl({ ...asset, path: "../r6/engine/engine.wasm" })).toThrow(
      AssetDownloadError,
    );
  });

  it("refuses a path that escapes the asset base URL", () => {
    const { manifest } = singleAsset();
    const manager = managerFor(manifest, servedFrom(manifest));
    const hostile: ManifestAsset = {
      ...manifest.assets[0]!,
      path: "//evil.example/payload.wasm",
    };
    expect(() => manager.resolveAssetUrl(hostile)).toThrow(AssetDownloadError);
  });
});

describe("AssetManager.downloadAsset", () => {
  it("streams, verifies, and atomically publishes an asset", async () => {
    const { manifest, asset } = singleAsset();
    const server = servedFrom(manifest);
    const storage = memoryStorage();
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await manager.downloadAsset(asset);

    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
    expect(await store.statTemp(asset.sha256)).toBeUndefined();
    expect(await store.readMeta(`${asset.sha256}.json`)).toBeUndefined();
    expect(await manager.isCached(asset)).toBe(true);
  });

  it("reports progress that never exceeds the declared size", async () => {
    const { manifest, asset } = singleAsset();
    const server = servedFrom(manifest, { "engine/engine.wasm": { chunkSize: 4 } });
    const manager = managerFor(manifest, server);
    const seen: number[] = [];

    await manager.downloadAsset(asset, { onProgress: (p) => seen.push(p.loadedBytes) });

    expect(seen.length).toBeGreaterThan(1);
    expect(Math.max(...seen)).toBe(asset.sizeBytes);
    expect(seen).toEqual([...seen].sort((a, b) => a - b));
  });

  it("rejects and discards an asset whose digest does not match", async () => {
    const { manifest, asset } = singleAsset();
    const tampered = { ...asset, sha256: "b".repeat(64) };
    const server = servedFrom(manifest);
    const storage = memoryStorage();
    const manager = new AssetManager(manifestFor([tampered]), { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(tampered)).rejects.toThrow(AssetIntegrityError);

    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(tampered.sha256)).toBeUndefined();
    expect(await store.statTemp(tampered.sha256)).toBeUndefined();
    expect(await store.readMeta(`${tampered.sha256}.json`)).toBeUndefined();
  });

  it("rejects a response that delivers more bytes than the manifest declares", async () => {
    const { manifest, asset } = singleAsset();
    const oversized = payload("\0asm-not-a-real-module-with-extra-tail");
    const server = createAssetServer({
      "engine/engine.wasm": { bytes: oversized, omitContentLength: true, chunkSize: 4 },
    });
    const storage = memoryStorage();
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetSizeError);
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toBeUndefined();
    expect(await store.statTemp(asset.sha256)).toBeUndefined();
  });

  it("rejects a response that delivers fewer bytes than the manifest declares", async () => {
    const { manifest, asset } = singleAsset();
    const server = createAssetServer({
      "engine/engine.wasm": { bytes: ENGINE_WASM.subarray(0, 5), omitContentLength: true },
    });
    const storage = memoryStorage();
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(asset)).rejects.toMatchObject({
      name: "AssetSizeError",
      expectedBytes: asset.sizeBytes,
      actualBytes: 5,
    });
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statTemp(asset.sha256)).toBeUndefined();
  });

  it("rejects a lying Content-Length before streaming any bytes", async () => {
    const { manifest, asset } = singleAsset();
    const server = createAssetServer({
      "engine/engine.wasm": { bytes: ENGINE_WASM, contentLengthOverride: 999_999 },
    });
    const manager = managerFor(manifest, server);

    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetSizeError);
  });

  it("surfaces an HTTP error without caching anything", async () => {
    const { manifest, asset } = singleAsset();
    const server = createAssetServer({});
    const storage = memoryStorage();
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(asset)).rejects.toMatchObject({
      name: "AssetDownloadError",
      status: 404,
    });
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.listFinal()).toEqual([]);
  });

  it("surfaces a network failure as a typed download error", async () => {
    const { manifest, asset } = singleAsset();
    const fetchImpl = vi.fn(async () => {
      throw new TypeError("Failed to fetch");
    });
    const manager = managerFor(manifest, { fetchImpl: fetchImpl as unknown as typeof fetch });

    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetDownloadError);
  });

  it("skips the network entirely for an already-verified asset", async () => {
    const { manifest, asset } = singleAsset();
    const server = servedFrom(manifest);
    const manager = managerFor(manifest, server, { storage: memoryStorage() });

    await manager.downloadAsset(asset);
    await manager.downloadAsset(asset);

    expect(server.requests.length).toBe(1);
  });

  it("re-downloads when the cached final file has the wrong size", async () => {
    const { manifest, asset } = singleAsset();
    const server = servedFrom(manifest);
    const storage = memoryStorage();
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    const store = await storage.openStore(manager.storageRoot);
    const writer = await store.openTemp(asset.sha256, { append: false });
    await writer.write(payload("truncated"));
    await writer.close();
    await store.promote(asset.sha256);

    await manager.downloadAsset(asset);

    expect(server.requests.length).toBe(1);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("rejects an object whose ETag no longer matches the manifest", async () => {
    const { manifest, asset } = singleAsset({ etag: '"v1"' });
    const server = servedFrom(manifest, { "engine/engine.wasm": { etagHeaderOverride: '"v2"' } });
    const manager = managerFor(manifest, server);

    await expect(manager.downloadAsset(asset)).rejects.toThrow(/refresh the manifest/);
  });
});

describe("AssetManager resume and range handling", () => {
  async function interruptedDownload(patch: Partial<ServedFile> = {}) {
    const { manifest, asset } = singleAsset({ etag: '"v1"' });
    const storage = memoryStorage();
    const server = servedFrom(manifest, {
      "engine/engine.wasm": { chunkSize: 4, failAfterBytes: 8, etag: '"v1"', ...patch },
    });
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetDownloadError);
    return { manifest, asset, storage, server, manager };
  }

  it("keeps the partial file and a resume record when the connection drops", async () => {
    const { asset, storage, manager } = await interruptedDownload();
    const store = await storage.openStore(manager.storageRoot);

    expect((await store.statTemp(asset.sha256))?.sizeBytes).toBe(8);
    const record = JSON.parse((await store.readMeta(`${asset.sha256}.json`))!) as Record<string, unknown>;
    expect(record).toMatchObject({
      version: 1,
      sha256: asset.sha256,
      sizeBytes: asset.sizeBytes,
      etag: '"v1"',
      engineVersion: "test-build",
      assetsRevision: 1,
      bytesWritten: 8,
    });
    expect(await store.statFinal(asset.sha256)).toBeUndefined();
  });

  it("resumes with a validated Range request instead of restarting", async () => {
    const { asset, storage, server, manager } = await interruptedDownload();
    server.update("engine/engine.wasm", { failAfterBytes: undefined });

    await manager.downloadAsset(asset);

    expect(server.requests[1]?.range).toBe("bytes=8-");
    expect(server.requests[1]?.ifRange).toBe('"v1"');
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("restarts from zero when the server ignores the Range header", async () => {
    const { asset, storage, server, manager } = await interruptedDownload();
    server.update("engine/engine.wasm", { failAfterBytes: undefined, supportsRange: false });

    await manager.downloadAsset(asset);

    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
    expect(await store.statTemp(asset.sha256)).toBeUndefined();
  });

  it("restarts from zero when Content-Range disagrees with the manifest", async () => {
    const { asset, storage, server, manager } = await interruptedDownload();
    server.update("engine/engine.wasm", {
      failAfterBytes: undefined,
      contentRangeOverride: "bytes 8-99/100",
    });

    await manager.downloadAsset(asset);

    expect(server.requests.length).toBe(3); // interrupted, bad range, clean retry
    expect(server.requests[2]?.range).toBeNull();
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("restarts from zero when the server answers 416", async () => {
    const { asset, storage, server, manager } = await interruptedDownload();
    server.update("engine/engine.wasm", { failAfterBytes: undefined, status: 416 });

    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetDownloadError);

    server.update("engine/engine.wasm", { status: undefined });
    await manager.downloadAsset(asset);
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("discards a partial download from a superseded asset revision", async () => {
    const { manifest, asset, storage, server } = await interruptedDownload();
    server.update("engine/engine.wasm", { failAfterBytes: undefined });

    const nextRevision = new AssetManager({ ...manifest, assetsRevision: 2 }, {
      storage,
      fetchImpl: server.fetchImpl,
    });
    await nextRevision.downloadAsset(asset);

    // A new revision uses a fresh immutable root, so the stale partial from
    // revision 1 can never be appended to.
    expect(server.requests[1]?.range).toBeNull();
    expect(await storage.listRoots()).toContain("test-build-r2");
  });

  it("discards a partial download whose sidecar no longer matches the ETag", async () => {
    const { asset, storage, server } = await interruptedDownload();
    server.update("engine/engine.wasm", { failAfterBytes: undefined, etag: '"v2"' });

    const rotated: ManifestAsset = { ...asset, etag: '"v2"' };
    const manager = new AssetManager(manifestFor([rotated]), { storage, fetchImpl: server.fetchImpl });
    await manager.downloadAsset(rotated);

    expect(server.requests[1]?.range).toBeNull();
    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("discards a partial download that was corrupted on disk", async () => {
    const { asset, storage, server, manager } = await interruptedDownload();
    const store = await storage.openStore(manager.storageRoot);

    const corrupt = await store.openTemp(asset.sha256, { append: false });
    await corrupt.write(payload("XXXXXXXX"));
    await corrupt.close();

    server.update("engine/engine.wasm", { failAfterBytes: undefined });
    // The corrupted prefix is re-hashed and carried into the final digest,
    // which fails verification; the poisoned bytes are then deleted.
    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetIntegrityError);
    expect(await store.statTemp(asset.sha256)).toBeUndefined();

    await manager.downloadAsset(asset);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("ignores a partial file with no sidecar record", async () => {
    const { manifest, asset } = singleAsset();
    const storage = memoryStorage();
    const server = servedFrom(manifest);
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    const store = await storage.openStore(manager.storageRoot);
    const orphan = await store.openTemp(asset.sha256, { append: false });
    await orphan.write(payload("orphaned"));
    await orphan.close();

    await manager.downloadAsset(asset);

    expect(server.requests[0]?.range).toBeNull();
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("publishes a complete temp file left behind by a crash without refetching", async () => {
    const { manifest, asset } = singleAsset();
    const storage = memoryStorage();
    const server = servedFrom(manifest);
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    const store = await storage.openStore(manager.storageRoot);
    const writer = await store.openTemp(asset.sha256, { append: false });
    await writer.write(ENGINE_WASM);
    await writer.close();
    await store.writeMeta(
      `${asset.sha256}.json`,
      JSON.stringify({
        version: 1,
        sha256: asset.sha256,
        sizeBytes: asset.sizeBytes,
        url: manager.resolveAssetUrl(asset),
        engineVersion: "test-build",
        assetsRevision: 1,
        bytesWritten: asset.sizeBytes,
      }),
    );

    await manager.downloadAsset(asset);

    expect(server.requests.length).toBe(0);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
    expect(await store.readMeta(`${asset.sha256}.json`)).toBeUndefined();
  });

  it("re-downloads a complete temp file whose bytes do not match the digest", async () => {
    const { manifest, asset } = singleAsset();
    const storage = memoryStorage();
    const server = servedFrom(manifest);
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    const store = await storage.openStore(manager.storageRoot);
    const writer = await store.openTemp(asset.sha256, { append: false });
    await writer.write(new Uint8Array(asset.sizeBytes).fill(0x41));
    await writer.close();
    await store.writeMeta(
      `${asset.sha256}.json`,
      JSON.stringify({
        version: 1,
        sha256: asset.sha256,
        sizeBytes: asset.sizeBytes,
        url: manager.resolveAssetUrl(asset),
        engineVersion: "test-build",
        assetsRevision: 1,
        bytesWritten: asset.sizeBytes,
      }),
    );

    await manager.downloadAsset(asset);

    expect(server.requests.length).toBe(1);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });
});

describe("AssetManager cancellation", () => {
  it("aborts mid-stream, keeps the partial file, and resumes later", async () => {
    const { manifest, asset } = singleAsset();
    const storage = memoryStorage();
    const server = servedFrom(manifest, { "engine/engine.wasm": { chunkSize: 4 } });
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    const controller = new AbortController();
    let seen = 0;
    const download = manager.downloadAsset(asset, {
      signal: controller.signal,
      onProgress: (progress) => {
        seen = progress.loadedBytes;
        if (seen >= 8) controller.abort();
      },
    });

    await expect(download).rejects.toThrow(AssetCancelledError);

    const store = await storage.openStore(manager.storageRoot);
    expect((await store.statTemp(asset.sha256))?.sizeBytes).toBeGreaterThanOrEqual(8);
    expect(await store.statFinal(asset.sha256)).toBeUndefined();

    await manager.downloadAsset(asset);
    expect(await store.statFinal(asset.sha256)).toEqual({ sizeBytes: asset.sizeBytes });
  });

  it("stops the whole boot when the caller aborts ensureAssets", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest, {
      "engine/engine.js": { chunkSize: 2 },
      "engine/engine.wasm": { chunkSize: 2 },
      "base/INI.big": { chunkSize: 2 },
      "expansion/INIZH.big": { chunkSize: 2 },
    });
    const manager = managerFor(manifest, server, { concurrency: 2 });

    const controller = new AbortController();
    const boot = manager.ensureAssets({
      signal: controller.signal,
      onProgress: () => controller.abort(),
    });

    await expect(boot).rejects.toThrow(AssetCancelledError);
  });

  it("fails immediately when the signal is already aborted", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest);
    const manager = managerFor(manifest, server);

    await expect(manager.ensureAssets({ signal: AbortSignal.abort() })).rejects.toThrow(
      AssetCancelledError,
    );
  });
});

describe("AssetManager storage pressure", () => {
  it("surfaces a quota failure and preserves resume state", async () => {
    const { manifest, asset } = singleAsset();
    const storage = memoryStorage(8);
    const server = servedFrom(manifest, { "engine/engine.wasm": { chunkSize: 4 } });
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(asset)).rejects.toThrow(AssetStorageQuotaError);

    const store = await storage.openStore(manager.storageRoot);
    expect(await store.statFinal(asset.sha256)).toBeUndefined();
    expect(await store.readMeta(`${asset.sha256}.json`)).toBeDefined();
  });

  it("propagates an OPFS quota failure as a typed error", async () => {
    const { manifest, asset } = singleAsset();
    const storage = new OpfsAssetStorage(FakeDirectoryHandle.create({ quotaBytes: 4 }));
    const server = servedFrom(manifest, { "engine/engine.wasm": { chunkSize: 2 } });
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    await expect(manager.downloadAsset(asset)).rejects.toMatchObject({
      name: "AssetStorageQuotaError",
      requiredBytes: asset.sizeBytes,
    });
  });

  it("refuses to start when the browser estimate cannot fit the manifest", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest);
    const manager = managerFor(manifest, server, {
      estimateImpl: async () => ({ usageBytes: 0, quotaBytes: 4 }),
    });

    await expect(manager.ensureAssets()).rejects.toThrow(AssetStorageQuotaError);
    expect(server.requests.length).toBe(0);
  });

  it("starts when the estimate leaves enough headroom", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest);
    const manager = managerFor(manifest, server, {
      estimateImpl: async () => ({ usageBytes: 0, quotaBytes: 10_000_000 }),
    });

    await expect(manager.ensureAssets()).resolves.toBeDefined();
  });
});

describe("AssetManager.ensureAssets", () => {
  it("downloads one content-addressed file for assets with identical bytes", async () => {
    const base = fullManifest();
    const duplicate = assetFor("scripts/engine-copy.ini", ENGINE_JS, "script", 300);
    const manifest = manifestFor([...base.assets, duplicate]);
    const server = servedFrom(manifest);
    const progress: number[] = [];

    const vfs = await managerFor(manifest, server).ensureAssets({
      onProgress: (event) => progress.push(event.overallLoadedBytes),
    });

    expect(server.requests).toHaveLength(4);
    expect(vfs.list()).toHaveLength(5);
    expect(progress.at(-1)).toBe(
      manifest.assets.reduce((total, asset) => total + asset.sizeBytes, 0),
    );
  });

  it("downloads every asset and mounts them in manifest order", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest);
    const manager = managerFor(manifest, server);

    const vfs = await manager.ensureAssets();

    expect(vfs.list().map((entry) => entry.target)).toEqual([
      "/generalsx/engine/engine.js",
      "/generalsx/engine/engine.wasm",
      "/generalsx/base/INI.big",
      "/generalsx/expansion/INIZH.big",
    ]);
    expect(new TextDecoder().decode(await vfs.readAll("/generalsx/engine/engine.js"))).toBe(
      new TextDecoder().decode(ENGINE_JS),
    );
    expect(new TextDecoder().decode(await vfs.read("/generalsx/base/INI.big", 5, 4))).toBe("base");
  });

  it("bounds how many assets download at once", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest, {
      "engine/engine.js": { chunkSize: 2 },
      "engine/engine.wasm": { chunkSize: 2 },
      "base/INI.big": { chunkSize: 2 },
      "expansion/INIZH.big": { chunkSize: 2 },
    });
    const manager = managerFor(manifest, server, { concurrency: 2 });

    await manager.ensureAssets();

    expect(server.requests.length).toBe(4);
    expect(server.peakConcurrency()).toBeLessThanOrEqual(2);
  });

  it("stops sibling downloads as soon as one asset fails", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest, { "engine/engine.wasm": { status: 500 } });
    const manager = managerFor(manifest, server, { concurrency: 1 });

    await expect(manager.ensureAssets()).rejects.toThrow(AssetDownloadError);

    expect(server.requests.map((request) => request.url)).toEqual([
      "https://assets.generalsx.org/engine/engine.js",
      "https://assets.generalsx.org/engine/engine.wasm",
    ]);
  });

  it("reports aggregate progress across the whole manifest", async () => {
    const manifest = fullManifest();
    const server = servedFrom(manifest);
    const manager = managerFor(manifest, server, { concurrency: 1 });
    const totals: number[] = [];

    await manager.ensureAssets({ onProgress: (p) => totals.push(p.overallLoadedBytes) });

    const expected = manifest.assets.reduce((sum, asset) => sum + asset.sizeBytes, 0);
    expect(totals.at(-1)).toBe(expected);
  });

  it("deletes storage roots from superseded revisions", async () => {
    const manifest = fullManifest();
    const storage = memoryStorage();
    const stale = await storage.openStore("test-build-r0");
    const writer = await stale.openTemp("deadbeef", { append: false });
    await writer.write(payload("stale"));
    await writer.close();

    const server = servedFrom(manifest);
    await new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl }).ensureAssets();

    expect(await storage.listRoots()).toEqual(["test-build-r1"]);
  });

  it("garbage-collects files the manifest no longer references", async () => {
    const manifest = fullManifest();
    const storage = memoryStorage();
    const server = servedFrom(manifest);
    const manager = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });

    const store = await storage.openStore(manager.storageRoot);
    const orphan = await store.openTemp("f".repeat(64), { append: false });
    await orphan.write(payload("orphan"));
    await orphan.close();
    await store.promote("f".repeat(64));
    await store.writeMeta(`${"f".repeat(64)}.json`, "{}");

    await manager.ensureAssets();

    expect([...(await store.listFinal())].sort()).toEqual(
      [...manifest.assets.map((asset) => asset.sha256)].sort(),
    );
    expect(await store.listTemp()).toEqual([]);
  });

  it("re-verifies nothing on a warm boot", async () => {
    const manifest = fullManifest();
    const storage = memoryStorage();
    const server = servedFrom(manifest);

    await new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl }).ensureAssets();
    const requestsAfterCold = server.requests.length;

    const warm = new AssetManager(manifest, { storage, fetchImpl: server.fetchImpl });
    const sources: string[] = [];
    await warm.ensureAssets({ onProgress: (p) => sources.push(p.source) });

    expect(server.requests.length).toBe(requestsAfterCold);
    expect(new Set(sources)).toEqual(new Set(["cache"]));
  });
});

describe("AssetManager memory discipline", () => {
  it("never hands the store a buffer larger than the write chunk limit", async () => {
    const bigAsset = assetFor("base/INI.big", BASE_BIG, "big-base", 0);
    const manifest = manifestFor([bigAsset]);
    const server = servedFrom(manifest, { "base/INI.big": { chunkSize: 32 } });

    const writes: number[] = [];
    const storage = recordingStorage(memoryStorage(), writes);
    const manager = new AssetManager(manifest, {
      storage,
      fetchImpl: server.fetchImpl,
      maxWriteChunkBytes: 5,
    });

    await manager.downloadAsset(bigAsset);

    expect(writes.length).toBeGreaterThan(1);
    expect(Math.max(...writes)).toBeLessThanOrEqual(5);
  });
});

/** Wraps a storage provider to record the size of every write. */
function recordingStorage(inner: AssetStorage, writes: number[]): AssetStorage {
  return {
    kind: inner.kind,
    listRoots: () => inner.listRoots(),
    deleteRoot: (root: string) => inner.deleteRoot(root),
    async openStore(root: string): Promise<AssetFileStore> {
      const store = await inner.openStore(root);
      return {
        ...store,
        kind: store.kind,
        root: store.root,
        async openTemp(name: string, options: { readonly append: boolean }): Promise<AssetWriteStream> {
          const writer = await store.openTemp(name, options);
          return {
            async write(chunk: Uint8Array) {
              writes.push(chunk.length);
              await writer.write(chunk);
            },
            close: () => writer.close(),
            abort: () => writer.abort(),
          };
        },
        statFinal: (name: string) => store.statFinal(name),
        readFinal: (name: string, offset: number, length: number) => store.readFinal(name, offset, length),
        deleteFinal: (name: string) => store.deleteFinal(name),
        listFinal: () => store.listFinal(),
        statTemp: (name: string) => store.statTemp(name),
        readTemp: (name: string, offset: number, length: number) => store.readTemp(name, offset, length),
        deleteTemp: (name: string) => store.deleteTemp(name),
        listTemp: () => store.listTemp(),
        readMeta: (name: string) => store.readMeta(name),
        writeMeta: (name: string, contents: string) => store.writeMeta(name, contents),
        deleteMeta: (name: string) => store.deleteMeta(name),
        promote: (name: string) => store.promote(name),
      };
    },
  };
}
