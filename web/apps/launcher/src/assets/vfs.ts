/**
 * Bounded-memory virtual file system over the verified asset store.
 *
 * The engine expects to `fopen`/`fseek`/`fread` its way through multi-hundred
 * megabyte BIG archives. Loading those into `ArrayBuffer`s would blow past
 * the wasm heap, so archives stay on disk (OPFS) and are served here as byte
 * ranges through a small, explicitly capped LRU chunk cache. Total resident
 * bytes therefore stay bounded by `cacheBudgetBytes` regardless of how large
 * the mounted archives are.
 */
import { isStreamingRole, type AssetRole, type EngineManifest } from "@generalsx-web/shared/manifest";
import { AssetNotMountedError } from "./errors.js";
import type { AssetFileStore } from "./storage.js";

/** One mounted file, in engine mount order. */
export interface VfsEntry {
  /** Absolute in-engine path, e.g. `/generalsx/base/INI.big`. */
  readonly target: string;
  /** Manifest object key the bytes came from. */
  readonly path: string;
  readonly role: AssetRole;
  readonly sizeBytes: number;
  readonly sha256: string;
  readonly order: number;
  readonly streaming: boolean;
  /** Content-addressed name of the verified file in the store. */
  readonly storageName: string;
}

export interface AssetVfsOptions {
  /** Granularity of range reads and of the LRU cache (default 1 MiB). */
  readonly chunkBytes?: number;
  /** Hard cap on bytes held by the chunk cache (default 32 MiB). */
  readonly cacheBudgetBytes?: number;
  /** Largest file `readAll` will materialise (default 64 MiB). */
  readonly maxWholeFileBytes?: number;
}

const DEFAULT_CHUNK_BYTES = 1024 * 1024;
const DEFAULT_CACHE_BUDGET_BYTES = 32 * 1024 * 1024;
const DEFAULT_MAX_WHOLE_FILE_BYTES = 64 * 1024 * 1024;

export class AssetVfs {
  private readonly entries: readonly VfsEntry[];
  private readonly byTarget: ReadonlyMap<string, VfsEntry>;
  private readonly chunkBytes: number;
  private readonly cacheBudgetBytes: number;
  private readonly maxWholeFileBytes: number;
  /** Insertion-ordered map doubling as an LRU queue. */
  private readonly chunks = new Map<string, Uint8Array>();
  private cachedBytesValue = 0;

  constructor(
    private readonly store: AssetFileStore,
    entries: readonly VfsEntry[],
    options: AssetVfsOptions = {},
  ) {
    this.entries = [...entries].sort((a, b) => a.order - b.order);
    this.byTarget = new Map(this.entries.map((entry) => [entry.target, entry]));
    this.chunkBytes = Math.max(1, options.chunkBytes ?? DEFAULT_CHUNK_BYTES);
    this.cacheBudgetBytes = Math.max(0, options.cacheBudgetBytes ?? DEFAULT_CACHE_BUDGET_BYTES);
    this.maxWholeFileBytes = options.maxWholeFileBytes ?? DEFAULT_MAX_WHOLE_FILE_BYTES;
  }

  /** Every mounted file, ordered so expansion archives follow base archives. */
  list(): readonly VfsEntry[] {
    return this.entries;
  }

  has(target: string): boolean {
    return this.byTarget.has(target);
  }

  stat(target: string): VfsEntry | undefined {
    return this.byTarget.get(target);
  }

  /** Bytes currently held by the chunk cache; never exceeds the budget. */
  get cachedBytes(): number {
    return this.cachedBytesValue;
  }

  /** Drops every cached chunk (e.g. when the engine finishes loading). */
  clearCache(): void {
    this.chunks.clear();
    this.cachedBytesValue = 0;
  }

  /**
   * Reads `length` bytes at `offset` from a mounted file. Reads are served
   * chunk-by-chunk from the store, so peak memory is `chunkBytes` plus the
   * caller's own buffer even for a multi-gigabyte archive.
   */
  async read(target: string, offset: number, length: number): Promise<Uint8Array> {
    const entry = this.require(target);
    if (!Number.isInteger(offset) || offset < 0) throw new RangeError(`invalid offset ${offset}`);
    if (!Number.isInteger(length) || length < 0) throw new RangeError(`invalid length ${length}`);

    const start = Math.min(offset, entry.sizeBytes);
    const end = Math.min(entry.sizeBytes, start + length);
    const out = new Uint8Array(end - start);
    if (out.length === 0) return out;

    let written = 0;
    let cursor = start;
    while (cursor < end) {
      const index = Math.floor(cursor / this.chunkBytes);
      const chunk = await this.chunk(entry, index);
      const chunkStart = index * this.chunkBytes;
      const from = cursor - chunkStart;
      const take = Math.min(chunk.length - from, end - cursor);
      if (take <= 0) break; // store returned a short chunk: file truncated
      out.set(chunk.subarray(from, from + take), written);
      written += take;
      cursor += take;
    }

    return written === out.length ? out : out.subarray(0, written);
  }

  /**
   * Reads a whole file. Only allowed for non-streaming roles (engine module,
   * scripts, fonts) and only below `maxWholeFileBytes`, which is what keeps
   * the "bounded memory" guarantee honest: archives can never be slurped.
   */
  async readAll(target: string): Promise<Uint8Array> {
    const entry = this.require(target);
    if (entry.streaming) {
      throw new RangeError(
        `"${target}" is a streaming asset (role ${entry.role}); use read(target, offset, length)`,
      );
    }
    if (entry.sizeBytes > this.maxWholeFileBytes) {
      throw new RangeError(
        `"${target}" is ${entry.sizeBytes} bytes, above the ${this.maxWholeFileBytes} byte whole-file limit`,
      );
    }
    return this.store.readFinal(entry.storageName, 0, entry.sizeBytes);
  }

  private require(target: string): VfsEntry {
    const entry = this.byTarget.get(target);
    if (!entry) throw new AssetNotMountedError(target);
    return entry;
  }

  private async chunk(entry: VfsEntry, index: number): Promise<Uint8Array> {
    const key = `${entry.storageName}:${index}`;
    const cached = this.chunks.get(key);
    if (cached) {
      // Refresh recency: delete + re-insert moves the key to the tail.
      this.chunks.delete(key);
      this.chunks.set(key, cached);
      return cached;
    }

    const offset = index * this.chunkBytes;
    const length = Math.min(this.chunkBytes, entry.sizeBytes - offset);
    const bytes = await this.store.readFinal(entry.storageName, offset, length);
    this.admit(key, bytes);
    return bytes;
  }

  private admit(key: string, bytes: Uint8Array): void {
    if (bytes.length > this.cacheBudgetBytes) return; // never exceed the budget
    while (this.cachedBytesValue + bytes.length > this.cacheBudgetBytes) {
      const oldest = this.chunks.keys().next();
      if (oldest.done) break;
      const evicted = this.chunks.get(oldest.value);
      this.chunks.delete(oldest.value);
      this.cachedBytesValue -= evicted?.length ?? 0;
    }
    this.chunks.set(key, bytes);
    this.cachedBytesValue += bytes.length;
  }
}

/** Builds the mount table for a manifest, in `mount.order` order. */
export function vfsEntriesFor(manifest: EngineManifest): readonly VfsEntry[] {
  return [...manifest.assets]
    .sort((a, b) => a.mount.order - b.mount.order)
    .map((asset) => ({
      target: asset.mount.target,
      path: asset.path,
      role: asset.role,
      sizeBytes: asset.sizeBytes,
      sha256: asset.sha256,
      order: asset.mount.order,
      streaming: asset.mount.streaming && isStreamingRole(asset.role),
      storageName: asset.sha256,
    }));
}
