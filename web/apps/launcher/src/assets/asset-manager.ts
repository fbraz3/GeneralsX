/**
 * Integrity-aware asset downloader/cache scaffold.
 *
 * This module NEVER embeds, bundles, or ships retail game assets. It only
 * knows how to fetch bytes from the operator-configured
 * `manifest.assetBaseUrl` at *runtime*, verify each file's SHA-256 digest
 * against the signed manifest, and persist verified bytes in the browser's
 * Cache Storage API. A mismatched digest is treated as tampering/corruption
 * and is never written to the cache.
 */
import type { EngineManifest, ManifestAsset } from "@generalsx-web/shared/manifest";

/** Minimal Cache-Storage-like surface, so tests can inject an in-memory
 * fake instead of depending on the browser's real `caches` global. */
export interface AssetCacheStore {
  match(key: string): Promise<Response | undefined>;
  put(key: string, response: Response): Promise<void>;
  delete(key: string): Promise<boolean>;
}

/** Cache Storage API-backed implementation, used in real browser builds. */
export class CacheStorageAssetStore implements AssetCacheStore {
  constructor(private readonly cacheName: string) {}

  private async open(): Promise<Cache> {
    return caches.open(this.cacheName);
  }

  async match(key: string): Promise<Response | undefined> {
    const cache = await this.open();
    return cache.match(key);
  }

  async put(key: string, response: Response): Promise<void> {
    const cache = await this.open();
    await cache.put(key, response);
  }

  async delete(key: string): Promise<boolean> {
    const cache = await this.open();
    return cache.delete(key);
  }
}

export interface AssetProgress {
  readonly path: string;
  readonly loadedBytes: number;
  readonly totalBytes: number;
}

export type AssetProgressCallback = (progress: AssetProgress) => void;

export class AssetIntegrityError extends Error {
  constructor(
    readonly path: string,
    readonly expectedSha256: string,
    readonly actualSha256: string,
  ) {
    super(`asset "${path}" failed integrity verification`);
    this.name = "AssetIntegrityError";
  }
}

function bufferToHex(buffer: ArrayBuffer): string {
  return [...new Uint8Array(buffer)].map((byte) => byte.toString(16).padStart(2, "0")).join("");
}

async function sha256Hex(bytes: ArrayBuffer): Promise<string> {
  const digest = await crypto.subtle.digest("SHA-256", bytes);
  return bufferToHex(digest);
}

export interface AssetManagerOptions {
  readonly cache: AssetCacheStore;
  readonly fetchImpl?: typeof fetch;
  readonly digestImpl?: (bytes: ArrayBuffer) => Promise<string>;
}

/**
 * Downloads and verifies assets described by an already-validated
 * {@link EngineManifest}. Every asset is fetched strictly from
 * `manifest.assetBaseUrl` (never an arbitrary URL supplied at call time).
 */
export class AssetManager {
  private readonly cache: AssetCacheStore;
  private readonly fetchImpl: typeof fetch;
  private readonly digestImpl: (bytes: ArrayBuffer) => Promise<string>;

  constructor(
    private readonly manifest: EngineManifest,
    options: AssetManagerOptions,
  ) {
    this.cache = options.cache;
    this.fetchImpl = options.fetchImpl ?? fetch;
    this.digestImpl = options.digestImpl ?? sha256Hex;
  }

  /** Builds the fully-qualified, same-origin-enforced URL for one asset. */
  resolveAssetUrl(asset: ManifestAsset): string {
    // `asset.path` is validated (no "..", no leading slash, no absolute
    // URL) by `validateManifest` before this class is ever constructed with
    // a trusted manifest, but `new URL` below still guards against a path
    // escaping `assetBaseUrl` at request time.
    return new URL(asset.path, `${this.manifest.assetBaseUrl}/`).toString();
  }

  private cacheKey(asset: ManifestAsset): string {
    return `${this.manifest.assetBaseUrl}/${this.manifest.engineVersion}/${asset.path}`;
  }

  /** Returns a previously verified, cached asset, if any. */
  async getCached(asset: ManifestAsset): Promise<Response | undefined> {
    return this.cache.match(this.cacheKey(asset));
  }

  /**
   * Downloads one asset, verifies its SHA-256 digest against the manifest,
   * and writes it to the cache only on success. Throws
   * {@link AssetIntegrityError} on a digest mismatch without caching the
   * corrupted/tampered bytes.
   */
  async downloadAsset(asset: ManifestAsset, onProgress?: AssetProgressCallback): Promise<void> {
    const cached = await this.getCached(asset);
    if (cached) {
      onProgress?.({ path: asset.path, loadedBytes: asset.sizeBytes, totalBytes: asset.sizeBytes });
      return;
    }

    const response = await this.fetchImpl(this.resolveAssetUrl(asset));
    if (!response.ok) {
      throw new Error(`failed to download asset "${asset.path}": HTTP ${response.status}`);
    }
    const bytes = await response.arrayBuffer();
    onProgress?.({ path: asset.path, loadedBytes: bytes.byteLength, totalBytes: asset.sizeBytes });

    const actualSha256 = await this.digestImpl(bytes);
    if (actualSha256 !== asset.sha256) {
      throw new AssetIntegrityError(asset.path, asset.sha256, actualSha256);
    }

    await this.cache.put(this.cacheKey(asset), new Response(bytes));
  }

  /** Downloads every manifest asset in order, sequentially, surfacing
   * progress as it goes. Stops at the first failure. */
  async downloadAll(onProgress?: AssetProgressCallback): Promise<void> {
    for (const asset of this.manifest.assets) {
      await this.downloadAsset(asset, onProgress);
    }
  }
}
