/**
 * Streaming, integrity-verified asset downloader.
 *
 * This module NEVER embeds, bundles, or ships retail game assets. It fetches
 * bytes at runtime from the operator-configured `manifest.assetBaseUrl`,
 * verifies each file against the manifest's exact size and SHA-256 digest
 * while the bytes stream past, and only then publishes them under an
 * immutable, content-addressed name in browser storage.
 *
 * Design guarantees:
 *  - **Bounded memory** — response bodies are consumed chunk by chunk and
 *    written straight to disk; a chunk larger than `maxWriteChunkBytes` is
 *    split, so peak JS memory is a few megabytes regardless of asset size.
 *  - **Atomicity** — bytes land in a temp file and are promoted to their
 *    final name only after size *and* digest verification succeed.
 *  - **Resumability** — an interrupted transfer keeps its temp file plus a
 *    sidecar record; the next run re-hashes the on-disk prefix and issues a
 *    `Range` request validated by `If-Range`/`ETag`, the manifest digest, and
 *    the engine/asset version.
 *  - **Self-healing** — any corrupt, oversized, undersized, or orphaned file
 *    is deleted rather than reused.
 */
import {
  assetCacheKey,
  storageRoot,
  totalManifestBytes,
  type EngineManifest,
  type ManifestAsset,
} from "@generalsx-web/shared/manifest";
import { Sha256Stream, digestsEqual } from "@generalsx-web/shared/sha256";
import {
  AssetCancelledError,
  AssetDownloadError,
  AssetIntegrityError,
  AssetSizeError,
  AssetStorageQuotaError,
  isQuotaExceeded,
} from "./errors.js";
import {
  estimateStorage,
  type AssetFileStore,
  type AssetStorage,
  type AssetStoreKind,
  type AssetWriteStream,
  type StorageEstimate,
} from "./storage.js";
import { AssetVfs, vfsEntriesFor, type AssetVfsOptions } from "./vfs.js";

/** Sidecar record persisted next to every in-flight temp file. */
interface ResumeRecord {
  readonly version: 1;
  readonly sha256: string;
  readonly sizeBytes: number;
  readonly url: string;
  readonly etag?: string;
  readonly engineVersion: string;
  readonly assetsRevision: number;
  readonly bytesWritten: number;
}

export type AssetSource = "cache" | "network" | "resumed";

export interface AssetProgress {
  readonly path: string;
  readonly role: ManifestAsset["role"];
  readonly source: AssetSource;
  /** Bytes of this asset that are verified-or-on-disk so far. */
  readonly loadedBytes: number;
  /** Declared size of this asset. */
  readonly totalBytes: number;
  /** Bytes across every asset in the manifest. */
  readonly overallLoadedBytes: number;
  readonly overallTotalBytes: number;
}

export type AssetProgressCallback = (progress: AssetProgress) => void;

export interface AssetManagerOptions {
  readonly storage: AssetStorage;
  readonly fetchImpl?: typeof fetch;
  /** Maximum number of assets fetched at the same time (default 3). */
  readonly concurrency?: number;
  /** Largest buffer handed to a single store write (default 4 MiB). */
  readonly maxWriteChunkBytes?: number;
  /** Window used when re-hashing a resumed prefix (default 4 MiB). */
  readonly rehashChunkBytes?: number;
  /** Storage estimate provider; injectable for tests. */
  readonly estimateImpl?: () => Promise<StorageEstimate>;
  /** Overrides passed through to the VFS built by `ensureAssets`. */
  readonly vfs?: AssetVfsOptions;
}

export interface EnsureAssetsOptions {
  readonly onProgress?: AssetProgressCallback;
  readonly signal?: AbortSignal;
}

const DEFAULT_CONCURRENCY = 3;
const DEFAULT_MAX_WRITE_CHUNK_BYTES = 4 * 1024 * 1024;
const DEFAULT_REHASH_CHUNK_BYTES = 4 * 1024 * 1024;
const CONTENT_RANGE_RE = /^bytes\s+(\d+)-(\d+)\/(\d+)$/i;

function metaName(key: string): string {
  return `${key}.json`;
}

function normalizeEtag(value: string | null | undefined): string | undefined {
  if (!value) return undefined;
  const trimmed = value.trim();
  return trimmed.length === 0 ? undefined : trimmed;
}

function parseResumeRecord(text: string | undefined): ResumeRecord | undefined {
  if (text === undefined) return undefined;
  try {
    const parsed: unknown = JSON.parse(text);
    if (typeof parsed !== "object" || parsed === null) return undefined;
    const record = parsed as Partial<ResumeRecord>;
    if (
      record.version !== 1 ||
      typeof record.sha256 !== "string" ||
      typeof record.sizeBytes !== "number" ||
      typeof record.url !== "string" ||
      typeof record.engineVersion !== "string" ||
      typeof record.assetsRevision !== "number" ||
      typeof record.bytesWritten !== "number"
    ) {
      return undefined;
    }
    return record as ResumeRecord;
  } catch {
    return undefined;
  }
}

/** Internal marker: the partial download is unusable, restart from zero. */
class RestartFromScratch extends Error {
  constructor(readonly detail: string) {
    super(`restarting download: ${detail}`);
    this.name = "RestartFromScratch";
  }
}

/**
 * Downloads, verifies, caches, and mounts every asset described by an
 * already-validated {@link EngineManifest}. Assets are always fetched from
 * `manifest.assetBaseUrl`, never from a URL supplied at call time.
 */
export class AssetManager {
  private readonly storage: AssetStorage;
  private readonly fetchImpl: typeof fetch;
  private readonly concurrency: number;
  private readonly maxWriteChunkBytes: number;
  private readonly rehashChunkBytes: number;
  private readonly estimateImpl: () => Promise<StorageEstimate>;
  private readonly vfsOptions: AssetVfsOptions;
  private readonly loadedByPath = new Map<string, number>();
  private storePromise: Promise<AssetFileStore> | undefined;

  constructor(
    private readonly manifest: EngineManifest,
    options: AssetManagerOptions,
  ) {
    this.storage = options.storage;
    this.fetchImpl = options.fetchImpl ?? ((...args) => fetch(...args));
    this.concurrency = Math.max(1, options.concurrency ?? DEFAULT_CONCURRENCY);
    this.maxWriteChunkBytes = Math.max(1, options.maxWriteChunkBytes ?? DEFAULT_MAX_WRITE_CHUNK_BYTES);
    this.rehashChunkBytes = Math.max(1, options.rehashChunkBytes ?? DEFAULT_REHASH_CHUNK_BYTES);
    this.estimateImpl = options.estimateImpl ?? estimateStorage;
    this.vfsOptions = options.vfs ?? {};
  }

  /** Backend actually in use (`opfs`, or `memory` when OPFS is unavailable). */
  get storageKind(): AssetStoreKind {
    return this.storage.kind;
  }

  /** Immutable storage root for this manifest revision. */
  get storageRoot(): string {
    return storageRoot(this.manifest);
  }

  /** Fully-qualified, base-URL-confined download URL for one asset. */
  resolveAssetUrl(asset: ManifestAsset): string {
    const base = `${this.manifest.assetBaseUrl}/`;
    const url = new URL(asset.path, base);
    if (!url.href.startsWith(base)) {
      throw new AssetDownloadError(asset.path, "resolved outside the manifest asset base URL");
    }
    return url.toString();
  }

  private store(): Promise<AssetFileStore> {
    this.storePromise ??= this.storage.openStore(this.storageRoot);
    return this.storePromise;
  }

  /** True when the asset is already present and verified in local storage. */
  async isCached(asset: ManifestAsset): Promise<boolean> {
    const store = await this.store();
    const info = await store.statFinal(assetCacheKey(asset));
    return info?.sizeBytes === asset.sizeBytes;
  }

  /**
   * Ensures every manifest asset is present and verified locally, then
   * returns a VFS mounted over the verified files.
   */
  async ensureAssets(options: EnsureAssetsOptions = {}): Promise<AssetVfs> {
    const store = await this.store();
    await this.pruneObsoleteRoots();
    await this.assertCapacity(store);

    const controller = new AbortController();
    const abortOuter = (): void => controller.abort();
    options.signal?.addEventListener("abort", abortOuter, { once: true });
    if (options.signal?.aborted) controller.abort();

    try {
      const perAsset: EnsureAssetsOptions = {
        signal: controller.signal,
        ...(options.onProgress ? { onProgress: options.onProgress } : {}),
      };
      const uniqueAssets: ManifestAsset[] = [];
      const duplicateAssets: ManifestAsset[] = [];
      const sizesByKey = new Map<string, number>();
      for (const asset of this.manifest.assets) {
        const key = assetCacheKey(asset);
        const existingSize = sizesByKey.get(key);
        if (existingSize === undefined) {
          sizesByKey.set(key, asset.sizeBytes);
          uniqueAssets.push(asset);
        } else if (existingSize === asset.sizeBytes) {
          duplicateAssets.push(asset);
        } else {
          throw new AssetDownloadError(
            asset.path,
            "assets with the same digest declared different byte sizes",
          );
        }
      }
      await this.runBounded(
        uniqueAssets,
        async (asset) => {
          await this.downloadAsset(asset, perAsset);
        },
        controller,
      );
      for (const asset of duplicateAssets) {
        this.report(asset, asset.sizeBytes, "cache", options.onProgress);
      }
    } finally {
      options.signal?.removeEventListener("abort", abortOuter);
    }

    await this.collectGarbage(store);
    return new AssetVfs(store, vfsEntriesFor(this.manifest), this.vfsOptions);
  }

  /** Mounts the VFS without downloading anything (all assets must be cached). */
  async mountVfs(): Promise<AssetVfs> {
    const store = await this.store();
    return new AssetVfs(store, vfsEntriesFor(this.manifest), this.vfsOptions);
  }

  /**
   * Downloads and verifies a single asset. On success the bytes are published
   * atomically under their content-addressed name; on any verification
   * failure the partial data is deleted instead of cached.
   */
  async downloadAsset(asset: ManifestAsset, options: EnsureAssetsOptions = {}): Promise<void> {
    const store = await this.store();
    const key = assetCacheKey(asset);

    const existing = await store.statFinal(key);
    if (existing?.sizeBytes === asset.sizeBytes) {
      await this.discardPartial(store, key);
      this.report(asset, asset.sizeBytes, "cache", options.onProgress);
      return;
    }
    if (existing) {
      // A truncated or oversized final file can only be corruption; never
      // hand it to the engine.
      await store.deleteFinal(key);
    }

    let forceFresh = false;
    for (let attempt = 0; attempt < 2; attempt += 1) {
      if (!forceFresh && (await this.recoverCompletedTemp(store, asset, key))) {
        this.report(asset, asset.sizeBytes, "resumed", options.onProgress);
        return;
      }
      try {
        await this.attemptDownload(store, asset, key, forceFresh, options);
        return;
      } catch (error) {
        if (error instanceof RestartFromScratch) {
          await this.discardPartial(store, key);
          forceFresh = true;
          continue;
        }
        throw error;
      }
    }
    throw new AssetDownloadError(asset.path, "range validation failed twice");
  }

  /**
   * Recovers from a crash between "temp file fully written" and "promoted":
   * a complete, compatible temp file is re-hashed and published without
   * touching the network. A digest mismatch deletes it instead.
   */
  private async recoverCompletedTemp(
    store: AssetFileStore,
    asset: ManifestAsset,
    key: string,
  ): Promise<boolean> {
    const temp = await store.statTemp(key);
    if (temp?.sizeBytes !== asset.sizeBytes) return false;

    const record = parseResumeRecord(await store.readMeta(metaName(key)));
    if (
      record === undefined ||
      record.sha256 !== asset.sha256 ||
      record.sizeBytes !== asset.sizeBytes ||
      record.engineVersion !== this.manifest.engineVersion ||
      record.assetsRevision !== this.manifest.assetsRevision
    ) {
      await this.discardPartial(store, key);
      return false;
    }

    const hasher = new Sha256Stream();
    if (!(await this.rehashTemp(store, key, temp.sizeBytes, hasher))) {
      await this.discardPartial(store, key);
      return false;
    }
    if (!digestsEqual(hasher.hex(), asset.sha256)) {
      await this.discardPartial(store, key);
      return false;
    }

    await store.promote(key);
    const published = await store.statFinal(key);
    if (published?.sizeBytes !== asset.sizeBytes) {
      await store.deleteFinal(key).catch(() => undefined);
      await this.discardPartial(store, key);
      return false;
    }
    await store.deleteMeta(metaName(key));
    return true;
  }

  private async attemptDownload(
    store: AssetFileStore,
    asset: ManifestAsset,
    key: string,
    forceFresh: boolean,
    options: EnsureAssetsOptions,
  ): Promise<void> {
    const url = this.resolveAssetUrl(asset);
    const hasher = new Sha256Stream();
    let offset = forceFresh ? 0 : await this.resumeOffset(store, asset, key, url, hasher);

    const headers = new Headers();
    if (offset > 0) {
      headers.set("Range", `bytes=${offset}-`);
      if (asset.etag) headers.set("If-Range", asset.etag);
    }

    const response = await this.fetchImpl(url, {
      headers,
      credentials: "omit",
      redirect: "follow",
      ...(options.signal ? { signal: options.signal } : {}),
    }).catch((error: unknown) => {
      if (options.signal?.aborted) throw new AssetCancelledError(asset.path);
      throw new AssetDownloadError(asset.path, error instanceof Error ? error.message : "network error");
    });

    if (response.status === 416) {
      await response.body?.cancel("range rejected").catch(() => undefined);
      throw new RestartFromScratch("server rejected the resume range");
    }
    if (!response.ok) {
      await response.body?.cancel("http error").catch(() => undefined);
      throw new AssetDownloadError(asset.path, `HTTP ${response.status}`, response.status);
    }

    const responseEtag = normalizeEtag(response.headers.get("etag"));
    if (asset.etag && responseEtag && responseEtag !== asset.etag) {
      await this.discardPartial(store, key);
      throw new AssetDownloadError(
        asset.path,
        `published object changed (expected ETag ${asset.etag}, got ${responseEtag}); refresh the manifest`,
      );
    }

    let source: AssetSource = "network";
    if (offset > 0) {
      if (response.status === 206) {
        try {
          this.validateContentRange(asset, response, offset);
        } catch (error) {
          await response.body?.cancel("bad range").catch(() => undefined);
          throw error;
        }
        source = "resumed";
      } else {
        // The server ignored `Range`; fall back to a clean full download.
        offset = 0;
      }
    } else if (response.status === 206) {
      await response.body?.cancel("unexpected partial").catch(() => undefined);
      throw new AssetDownloadError(asset.path, "unexpected 206 response for a full request", 206);
    }

    if (offset === 0) {
      const declared = response.headers.get("content-length");
      if (declared !== null && Number(declared) !== asset.sizeBytes) {
        await this.discardPartial(store, key);
        throw new AssetSizeError(asset.path, asset.sizeBytes, Number(declared));
      }
    }

    const freshHasher = offset === 0 ? new Sha256Stream() : hasher;
    const body = response.body;
    if (!body) {
      throw new AssetDownloadError(asset.path, "response carried no body");
    }

    const writer = await store.openTemp(key, { append: offset > 0 });
    let written = offset;
    const reader = body.getReader();

    try {
      for (;;) {
        if (options.signal?.aborted) {
          await reader.cancel("cancelled").catch(() => undefined);
          await writer.close();
          await this.saveResumeRecord(store, asset, key, url, written);
          throw new AssetCancelledError(asset.path);
        }

        const { done, value } = await reader.read();
        if (done) break;
        if (!value || value.length === 0) continue;

        if (written + value.length > asset.sizeBytes) {
          await reader.cancel("oversized").catch(() => undefined);
          await writer.abort();
          await this.discardPartial(store, key);
          throw new AssetSizeError(asset.path, asset.sizeBytes, written + value.length);
        }

        for (let cursor = 0; cursor < value.length; cursor += this.maxWriteChunkBytes) {
          const slice = value.subarray(cursor, Math.min(value.length, cursor + this.maxWriteChunkBytes));
          freshHasher.update(slice);
          await writer.write(slice);
          written += slice.length;
        }

        this.report(asset, written, source, options.onProgress);
      }
      await writer.close();
    } catch (error) {
      if (error instanceof AssetCancelledError || error instanceof AssetSizeError) {
        // These paths already committed or discarded their own bytes.
        throw error;
      }
      if (isQuotaExceeded(error)) {
        await writer.close().catch(() => undefined);
        await reader.cancel("quota").catch(() => undefined);
        await this.saveResumeRecord(store, asset, key, url, written);
        throw new AssetStorageQuotaError(asset.path, asset.sizeBytes, error);
      }
      // A dropped connection is the common case: commit what arrived and
      // record the resume point so the next attempt continues from here.
      await writer.close().catch(() => undefined);
      await reader.cancel("stream error").catch(() => undefined);
      await this.saveResumeRecord(store, asset, key, url, written);
      throw new AssetDownloadError(
        asset.path,
        error instanceof Error ? error.message : "stream failed",
      );
    }

    if (written !== asset.sizeBytes) {
      await this.discardPartial(store, key);
      throw new AssetSizeError(asset.path, asset.sizeBytes, written);
    }

    const digest = freshHasher.hex();
    if (!digestsEqual(digest, asset.sha256)) {
      await this.discardPartial(store, key);
      throw new AssetIntegrityError(asset.path, asset.sha256, digest);
    }

    const onDisk = await store.statTemp(key);
    if (onDisk?.sizeBytes !== asset.sizeBytes) {
      await this.discardPartial(store, key);
      throw new AssetSizeError(asset.path, asset.sizeBytes, onDisk?.sizeBytes ?? 0);
    }

    await store.promote(key);

    const published = await store.statFinal(key);
    if (published?.sizeBytes !== asset.sizeBytes) {
      // Only reachable on the non-atomic copy fallback; never expose a torn
      // file to the engine.
      await store.deleteFinal(key);
      await this.discardPartial(store, key);
      throw new AssetSizeError(asset.path, asset.sizeBytes, published?.sizeBytes ?? 0);
    }

    await store.deleteMeta(metaName(key));
    this.report(asset, asset.sizeBytes, source, options.onProgress);
  }

  /**
   * Decides whether an existing temp file can be resumed. The sidecar must
   * agree with the manifest on digest, size, URL, ETag, engine version, and
   * asset revision, and the on-disk prefix is re-hashed before a single byte
   * is appended.
   */
  private async resumeOffset(
    store: AssetFileStore,
    asset: ManifestAsset,
    key: string,
    url: string,
    hasher: Sha256Stream,
  ): Promise<number> {
    const temp = await store.statTemp(key);
    if (!temp || temp.sizeBytes === 0) {
      await this.discardPartial(store, key);
      return 0;
    }

    const record = parseResumeRecord(await store.readMeta(metaName(key)));
    const compatible =
      record !== undefined &&
      record.sha256 === asset.sha256 &&
      record.sizeBytes === asset.sizeBytes &&
      record.url === url &&
      record.etag === asset.etag &&
      record.engineVersion === this.manifest.engineVersion &&
      record.assetsRevision === this.manifest.assetsRevision;

    if (!compatible || temp.sizeBytes >= asset.sizeBytes) {
      await this.discardPartial(store, key);
      return 0;
    }

    if (!(await this.rehashTemp(store, key, temp.sizeBytes, hasher))) {
      await this.discardPartial(store, key);
      return 0;
    }

    return temp.sizeBytes;
  }

  /** Feeds `length` bytes of a temp file through `hasher` in bounded windows. */
  private async rehashTemp(
    store: AssetFileStore,
    key: string,
    length: number,
    hasher: Sha256Stream,
  ): Promise<boolean> {
    try {
      for (let offset = 0; offset < length; offset += this.rehashChunkBytes) {
        const window = Math.min(this.rehashChunkBytes, length - offset);
        const bytes = await store.readTemp(key, offset, window);
        if (bytes.length !== window) return false;
        hasher.update(bytes);
      }
      return true;
    } catch {
      return false;
    }
  }

  private validateContentRange(asset: ManifestAsset, response: Response, offset: number): void {
    const header = response.headers.get("content-range");
    const match = header ? CONTENT_RANGE_RE.exec(header.trim()) : null;
    if (!match) throw new RestartFromScratch(`unparsable Content-Range: ${header ?? "<missing>"}`);

    const start = Number(match[1]);
    const end = Number(match[2]);
    const total = Number(match[3]);
    if (start !== offset) throw new RestartFromScratch(`Content-Range started at ${start}, expected ${offset}`);
    if (total !== asset.sizeBytes) {
      throw new RestartFromScratch(`Content-Range total ${total}, manifest says ${asset.sizeBytes}`);
    }
    if (end !== asset.sizeBytes - 1) {
      throw new RestartFromScratch(`Content-Range ended at ${end}, expected ${asset.sizeBytes - 1}`);
    }
  }

  private async saveResumeRecord(
    store: AssetFileStore,
    asset: ManifestAsset,
    key: string,
    url: string,
    bytesWritten: number,
  ): Promise<void> {
    const record: ResumeRecord = {
      version: 1,
      sha256: asset.sha256,
      sizeBytes: asset.sizeBytes,
      url,
      ...(asset.etag === undefined ? {} : { etag: asset.etag }),
      engineVersion: this.manifest.engineVersion,
      assetsRevision: this.manifest.assetsRevision,
      bytesWritten,
    };
    await store.writeMeta(metaName(key), JSON.stringify(record)).catch(() => undefined);
  }

  /** Deletes the temp file and its sidecar for a key. */
  private async discardPartial(store: AssetFileStore, key: string): Promise<void> {
    await store.deleteTemp(key).catch(() => undefined);
    await store.deleteMeta(metaName(key)).catch(() => undefined);
  }

  private report(
    asset: ManifestAsset,
    loadedBytes: number,
    source: AssetSource,
    onProgress?: AssetProgressCallback,
  ): void {
    this.loadedByPath.set(asset.path, loadedBytes);
    if (!onProgress) return;
    let overall = 0;
    for (const bytes of this.loadedByPath.values()) overall += bytes;
    onProgress({
      path: asset.path,
      role: asset.role,
      source,
      loadedBytes,
      totalBytes: asset.sizeBytes,
      overallLoadedBytes: overall,
      overallTotalBytes: totalManifestBytes(this.manifest),
    });
  }

  /** Runs `worker` over `items` with bounded parallelism, failing fast. */
  private async runBounded<T>(
    items: readonly T[],
    worker: (item: T) => Promise<void>,
    controller: AbortController,
  ): Promise<void> {
    let cursor = 0;
    let failure: unknown;

    const runners = Array.from({ length: Math.min(this.concurrency, items.length) }, async () => {
      for (;;) {
        if (failure !== undefined) return;
        const index = cursor;
        cursor += 1;
        const item = items[index];
        if (item === undefined) return;
        try {
          await worker(item);
        } catch (error) {
          failure ??= error;
          // Stop sibling transfers as soon as one asset fails.
          controller.abort();
          return;
        }
      }
    });

    await Promise.all(runners);
    if (failure !== undefined) throw failure;
  }

  /** Drops storage roots belonging to superseded manifest revisions. */
  private async pruneObsoleteRoots(): Promise<void> {
    const current = this.storageRoot;
    let roots: readonly string[];
    try {
      roots = await this.storage.listRoots();
    } catch {
      return;
    }
    for (const root of roots) {
      if (root === current) continue;
      await this.storage.deleteRoot(root).catch(() => undefined);
    }
  }

  /** Removes files in the current root that the manifest no longer lists. */
  private async collectGarbage(store: AssetFileStore): Promise<void> {
    const wanted = new Set(this.manifest.assets.map((asset) => assetCacheKey(asset)));
    const wantedMeta = new Set([...wanted].map(metaName));

    for (const name of await store.listFinal().catch(() => [])) {
      if (!wanted.has(name)) await store.deleteFinal(name).catch(() => undefined);
    }
    for (const name of await store.listTemp().catch(() => [])) {
      if (!wanted.has(name)) await store.deleteTemp(name).catch(() => undefined);
      else if (await store.statFinal(name)) await store.deleteTemp(name).catch(() => undefined);
    }
    for (const name of wantedMeta) {
      const key = name.slice(0, -".json".length);
      if (await store.statFinal(key)) await store.deleteMeta(name).catch(() => undefined);
    }
  }

  /**
   * Fails fast when the browser's own storage estimate says the manifest
   * cannot possibly fit, instead of downloading gigabytes and dying on the
   * last file.
   */
  private async assertCapacity(store: AssetFileStore): Promise<void> {
    const estimate = await this.estimateImpl().catch(() => ({}) as StorageEstimate);
    if (estimate.quotaBytes === undefined || estimate.usageBytes === undefined) return;

    let required = 0;
    for (const asset of this.manifest.assets) {
      const info = await store.statFinal(assetCacheKey(asset));
      if (info?.sizeBytes !== asset.sizeBytes) required += asset.sizeBytes;
    }

    const free = estimate.quotaBytes - estimate.usageBytes;
    if (required > free) {
      throw new AssetStorageQuotaError("manifest", required);
    }
  }
}

export { AssetVfs, vfsEntriesFor } from "./vfs.js";
export type { VfsEntry } from "./vfs.js";
export type { AssetWriteStream };
