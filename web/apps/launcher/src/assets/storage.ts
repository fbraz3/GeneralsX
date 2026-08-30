/**
 * Persistent, bounded-memory storage backend for downloaded assets.
 *
 * The pipeline needs three guarantees the Cache Storage API cannot give us:
 *
 *  1. **Bounded JS memory** — a multi-hundred-megabyte archive must never be
 *     materialised as a single `ArrayBuffer`. OPFS lets us append chunks as
 *     they arrive and later read arbitrary byte ranges back out.
 *  2. **Resumability** — a partially transferred file must survive a reload,
 *     which means writing to a real, addressable temporary file.
 *  3. **Atomic publication** — a file only becomes visible under its final,
 *     content-addressed name after its digest has been verified.
 *
 * Everything here is expressed against small structural interfaces so unit
 * tests can substitute an in-memory filesystem for OPFS.
 */

/** Which backend is in use; surfaced to the UI and to tests. */
export type AssetStoreKind = "opfs" | "memory";

export interface StoredFileInfo {
  readonly sizeBytes: number;
}

/** Append-only sink for one temporary file. */
export interface AssetWriteStream {
  /** Appends one chunk. Rejects with a `QuotaExceededError` DOMException
   * when the backing store is full. */
  write(chunk: Uint8Array): Promise<void>;
  /** Flushes and commits everything written so far. */
  close(): Promise<void>;
  /** Releases the handle without committing further writes. */
  abort(): Promise<void>;
}

/**
 * File operations the asset pipeline needs, split into three namespaces:
 * `final` (verified, content-addressed), `temp` (in-flight), and `meta`
 * (resume sidecars).
 */
export interface AssetFileStore {
  readonly kind: AssetStoreKind;
  readonly root: string;

  statFinal(name: string): Promise<StoredFileInfo | undefined>;
  readFinal(name: string, offset: number, length: number): Promise<Uint8Array>;
  deleteFinal(name: string): Promise<void>;
  listFinal(): Promise<readonly string[]>;

  statTemp(name: string): Promise<StoredFileInfo | undefined>;
  openTemp(name: string, options: { readonly append: boolean }): Promise<AssetWriteStream>;
  readTemp(name: string, offset: number, length: number): Promise<Uint8Array>;
  deleteTemp(name: string): Promise<void>;
  listTemp(): Promise<readonly string[]>;

  readMeta(name: string): Promise<string | undefined>;
  writeMeta(name: string, contents: string): Promise<void>;
  deleteMeta(name: string): Promise<void>;

  /**
   * Publishes `temp/<name>` as `final/<name>`. Uses OPFS `move()` (an atomic
   * rename) when the browser provides it; otherwise falls back to a
   * copy-then-delete that is validated by the caller's size check, so a
   * torn copy is detected and discarded rather than trusted.
   */
  promote(name: string): Promise<void>;
}

/** Root-level storage provider: one root per manifest revision. */
export interface AssetStorage {
  readonly kind: AssetStoreKind;
  openStore(root: string): Promise<AssetFileStore>;
  listRoots(): Promise<readonly string[]>;
  deleteRoot(root: string): Promise<void>;
}

const FINAL_DIR = "files";
const TEMP_DIR = "tmp";
const META_DIR = "meta";
/** Chunk size used when copying between files, bounding peak JS memory. */
const COPY_CHUNK_BYTES = 4 * 1024 * 1024;

function quotaExceeded(message: string): DOMException {
  return new DOMException(message, "QuotaExceededError");
}

function isNotFound(error: unknown): boolean {
  return typeof error === "object" && error !== null && (error as { name?: unknown }).name === "NotFoundError";
}

function assertSafeName(name: string): void {
  if (!/^[A-Za-z0-9._-]{1,128}$/.test(name) || name === "." || name === "..") {
    throw new Error(`unsafe storage entry name: ${name}`);
  }
}

/* ------------------------------------------------------------------ */
/* Minimal structural view of the OPFS handles we depend on.           */
/* The DOM lib does not yet type `FileSystemFileHandle.move`, and the  */
/* tests substitute a fake filesystem, so the pipeline talks to these  */
/* interfaces instead of the concrete DOM types.                       */
/* ------------------------------------------------------------------ */

export interface OpfsBlobLike {
  readonly size: number;
  slice(start?: number, end?: number): OpfsBlobLike;
  arrayBuffer(): Promise<ArrayBuffer>;
  text(): Promise<string>;
}

export interface OpfsWritableLike {
  write(data: Uint8Array | { type: "write"; position?: number; data: Uint8Array }): Promise<void>;
  truncate?(size: number): Promise<void>;
  close(): Promise<void>;
  abort?(reason?: unknown): Promise<void>;
}

export interface OpfsFileHandleLike {
  getFile(): Promise<OpfsBlobLike>;
  createWritable(options?: { keepExistingData?: boolean }): Promise<OpfsWritableLike>;
  move?(destination: OpfsDirectoryHandleLike, name?: string): Promise<void>;
}

export interface OpfsDirectoryHandleLike {
  getFileHandle(name: string, options?: { create?: boolean }): Promise<OpfsFileHandleLike>;
  getDirectoryHandle(name: string, options?: { create?: boolean }): Promise<OpfsDirectoryHandleLike>;
  removeEntry(name: string, options?: { recursive?: boolean }): Promise<void>;
  keys(): AsyncIterableIterator<string>;
}

class OpfsWriteStream implements AssetWriteStream {
  private position: number;
  private closed = false;

  constructor(
    private readonly writable: OpfsWritableLike,
    startPosition: number,
  ) {
    this.position = startPosition;
  }

  async write(chunk: Uint8Array): Promise<void> {
    if (this.closed) throw new Error("write after close");
    if (chunk.length === 0) return;
    await this.writable.write({ type: "write", position: this.position, data: chunk });
    this.position += chunk.length;
  }

  async close(): Promise<void> {
    if (this.closed) return;
    this.closed = true;
    await this.writable.close();
  }

  async abort(): Promise<void> {
    if (this.closed) return;
    this.closed = true;
    if (this.writable.abort) {
      await this.writable.abort("aborted");
      return;
    }
    await this.writable.close();
  }
}

class OpfsAssetFileStore implements AssetFileStore {
  readonly kind = "opfs" as const;

  constructor(
    readonly root: string,
    private readonly dir: OpfsDirectoryHandleLike,
  ) {}

  private async sub(name: string): Promise<OpfsDirectoryHandleLike> {
    return this.dir.getDirectoryHandle(name, { create: true });
  }

  private async handle(
    dirName: string,
    name: string,
    create: boolean,
  ): Promise<OpfsFileHandleLike | undefined> {
    assertSafeName(name);
    const dir = await this.sub(dirName);
    try {
      return await dir.getFileHandle(name, { create });
    } catch (error) {
      if (isNotFound(error)) return undefined;
      throw error;
    }
  }

  private async stat(dirName: string, name: string): Promise<StoredFileInfo | undefined> {
    const handle = await this.handle(dirName, name, false);
    if (!handle) return undefined;
    const file = await handle.getFile();
    return { sizeBytes: file.size };
  }

  private async read(
    dirName: string,
    name: string,
    offset: number,
    length: number,
  ): Promise<Uint8Array> {
    const handle = await this.handle(dirName, name, false);
    if (!handle) throw new DOMException(`missing ${dirName}/${name}`, "NotFoundError");
    const file = await handle.getFile();
    const end = Math.min(file.size, offset + length);
    if (offset >= end) return new Uint8Array(0);
    // Blob slicing is lazy: only the requested window is materialised.
    return new Uint8Array(await file.slice(offset, end).arrayBuffer());
  }

  private async remove(dirName: string, name: string): Promise<void> {
    assertSafeName(name);
    const dir = await this.sub(dirName);
    try {
      await dir.removeEntry(name);
    } catch (error) {
      if (!isNotFound(error)) throw error;
    }
  }

  private async list(dirName: string): Promise<readonly string[]> {
    const dir = await this.sub(dirName);
    const names: string[] = [];
    for await (const key of dir.keys()) names.push(key);
    return names;
  }

  statFinal(name: string): Promise<StoredFileInfo | undefined> {
    return this.stat(FINAL_DIR, name);
  }

  readFinal(name: string, offset: number, length: number): Promise<Uint8Array> {
    return this.read(FINAL_DIR, name, offset, length);
  }

  deleteFinal(name: string): Promise<void> {
    return this.remove(FINAL_DIR, name);
  }

  listFinal(): Promise<readonly string[]> {
    return this.list(FINAL_DIR);
  }

  statTemp(name: string): Promise<StoredFileInfo | undefined> {
    return this.stat(TEMP_DIR, name);
  }

  async openTemp(name: string, options: { readonly append: boolean }): Promise<AssetWriteStream> {
    const handle = await this.handle(TEMP_DIR, name, true);
    if (!handle) throw new Error(`unable to create temp file ${name}`);
    const start = options.append ? (await handle.getFile()).size : 0;
    const writable = await handle.createWritable({ keepExistingData: options.append });
    // Drop any bytes past the resume point so a torn previous write cannot
    // leave a stale tail behind the newly appended data.
    if (options.append && writable.truncate) await writable.truncate(start);
    return new OpfsWriteStream(writable, start);
  }

  readTemp(name: string, offset: number, length: number): Promise<Uint8Array> {
    return this.read(TEMP_DIR, name, offset, length);
  }

  deleteTemp(name: string): Promise<void> {
    return this.remove(TEMP_DIR, name);
  }

  listTemp(): Promise<readonly string[]> {
    return this.list(TEMP_DIR);
  }

  async readMeta(name: string): Promise<string | undefined> {
    const handle = await this.handle(META_DIR, name, false);
    if (!handle) return undefined;
    return (await handle.getFile()).text();
  }

  async writeMeta(name: string, contents: string): Promise<void> {
    const handle = await this.handle(META_DIR, name, true);
    if (!handle) throw new Error(`unable to create meta file ${name}`);
    const writable = await handle.createWritable({ keepExistingData: false });
    await writable.write({ type: "write", position: 0, data: new TextEncoder().encode(contents) });
    await writable.close();
  }

  deleteMeta(name: string): Promise<void> {
    return this.remove(META_DIR, name);
  }

  async promote(name: string): Promise<void> {
    assertSafeName(name);
    const tempDir = await this.sub(TEMP_DIR);
    const finalDir = await this.sub(FINAL_DIR);
    const source = await tempDir.getFileHandle(name);

    if (typeof source.move === "function") {
      await source.move(finalDir, name);
      return;
    }

    // Fallback for engines without `move()`: copy in bounded chunks, then
    // drop the temp file. The caller re-checks the final size afterwards, so
    // an interrupted copy is detected and deleted instead of being trusted.
    const file = await source.getFile();
    const target = await finalDir.getFileHandle(name, { create: true });
    const writable = await target.createWritable({ keepExistingData: false });
    try {
      for (let offset = 0; offset < file.size; offset += COPY_CHUNK_BYTES) {
        const end = Math.min(file.size, offset + COPY_CHUNK_BYTES);
        const bytes = new Uint8Array(await file.slice(offset, end).arrayBuffer());
        await writable.write({ type: "write", position: offset, data: bytes });
      }
      await writable.close();
    } catch (error) {
      if (writable.abort) await writable.abort(error);
      throw error;
    }
    await this.remove(TEMP_DIR, name);
  }
}

/** OPFS-backed storage provider (real browser builds). */
export class OpfsAssetStorage implements AssetStorage {
  readonly kind = "opfs" as const;

  constructor(private readonly base: OpfsDirectoryHandleLike) {}

  async openStore(root: string): Promise<AssetFileStore> {
    assertSafeName(root);
    const dir = await this.base.getDirectoryHandle(root, { create: true });
    return new OpfsAssetFileStore(root, dir);
  }

  async listRoots(): Promise<readonly string[]> {
    const roots: string[] = [];
    for await (const key of this.base.keys()) roots.push(key);
    return roots;
  }

  async deleteRoot(root: string): Promise<void> {
    assertSafeName(root);
    try {
      await this.base.removeEntry(root, { recursive: true });
    } catch (error) {
      if (!isNotFound(error)) throw error;
    }
  }
}

/* ------------------------------------------------------------------ */
/* In-memory fallback                                                  */
/* ------------------------------------------------------------------ */

interface MemoryRoot {
  readonly files: Map<string, Uint8Array>;
  readonly temps: Map<string, Uint8Array>;
  readonly meta: Map<string, string>;
}

class MemoryWriteStream implements AssetWriteStream {
  private buffer: Uint8Array;
  private length: number;
  private closed = false;

  constructor(
    private readonly commit: (bytes: Uint8Array) => void,
    private readonly checkBudget: (extraBytes: number) => void,
    existing: Uint8Array,
  ) {
    this.buffer = existing.slice();
    this.length = existing.length;
  }

  async write(chunk: Uint8Array): Promise<void> {
    if (this.closed) throw new Error("write after close");
    if (chunk.length === 0) return;
    this.checkBudget(chunk.length);
    this.ensureCapacity(this.length + chunk.length);
    this.buffer.set(chunk, this.length);
    this.length += chunk.length;
    // Commit after every chunk so a cancelled transfer leaves exactly the
    // bytes that were written, mirroring OPFS resume semantics.
    this.commit(this.buffer.subarray(0, this.length));
  }

  private ensureCapacity(required: number): void {
    if (this.buffer.length >= required) return;
    const grown = new Uint8Array(Math.max(required, this.buffer.length * 2, 64 * 1024));
    grown.set(this.buffer.subarray(0, this.length), 0);
    this.buffer = grown;
  }

  async close(): Promise<void> {
    if (this.closed) return;
    this.closed = true;
    this.commit(this.buffer.subarray(0, this.length));
  }

  async abort(): Promise<void> {
    this.closed = true;
  }
}

class MemoryAssetFileStore implements AssetFileStore {
  readonly kind = "memory" as const;

  constructor(
    readonly root: string,
    private readonly data: MemoryRoot,
    private readonly budgetBytes: number,
    private readonly usedBytes: () => number,
  ) {}

  private checkBudget(extraBytes: number): void {
    if (this.usedBytes() + extraBytes > this.budgetBytes) {
      throw quotaExceeded(
        `in-memory asset store budget of ${this.budgetBytes} bytes exceeded`,
      );
    }
  }

  private static slice(bytes: Uint8Array | undefined, offset: number, length: number): Uint8Array {
    if (!bytes) throw new DOMException("missing file", "NotFoundError");
    return bytes.slice(offset, Math.min(bytes.length, offset + length));
  }

  async statFinal(name: string): Promise<StoredFileInfo | undefined> {
    const bytes = this.data.files.get(name);
    return bytes ? { sizeBytes: bytes.length } : undefined;
  }

  async readFinal(name: string, offset: number, length: number): Promise<Uint8Array> {
    return MemoryAssetFileStore.slice(this.data.files.get(name), offset, length);
  }

  async deleteFinal(name: string): Promise<void> {
    this.data.files.delete(name);
  }

  async listFinal(): Promise<readonly string[]> {
    return [...this.data.files.keys()];
  }

  async statTemp(name: string): Promise<StoredFileInfo | undefined> {
    const bytes = this.data.temps.get(name);
    return bytes ? { sizeBytes: bytes.length } : undefined;
  }

  async openTemp(name: string, options: { readonly append: boolean }): Promise<AssetWriteStream> {
    assertSafeName(name);
    const existing = options.append ? (this.data.temps.get(name) ?? new Uint8Array(0)) : new Uint8Array(0);
    this.data.temps.set(name, existing);
    return new MemoryWriteStream(
      (bytes) => this.data.temps.set(name, bytes),
      (extra) => this.checkBudget(extra),
      existing,
    );
  }

  async readTemp(name: string, offset: number, length: number): Promise<Uint8Array> {
    return MemoryAssetFileStore.slice(this.data.temps.get(name), offset, length);
  }

  async deleteTemp(name: string): Promise<void> {
    this.data.temps.delete(name);
  }

  async listTemp(): Promise<readonly string[]> {
    return [...this.data.temps.keys()];
  }

  async readMeta(name: string): Promise<string | undefined> {
    return this.data.meta.get(name);
  }

  async writeMeta(name: string, contents: string): Promise<void> {
    assertSafeName(name);
    this.data.meta.set(name, contents);
  }

  async deleteMeta(name: string): Promise<void> {
    this.data.meta.delete(name);
  }

  async promote(name: string): Promise<void> {
    const bytes = this.data.temps.get(name);
    if (!bytes) throw new DOMException(`missing temp file ${name}`, "NotFoundError");
    // Map mutations are synchronous, so publication is atomic by definition.
    this.data.files.set(name, bytes);
    this.data.temps.delete(name);
  }
}

/**
 * Memory-backed fallback used when OPFS is unavailable (older Safari, private
 * browsing, or a hardened embedder). It keeps the pipeline functional but
 * enforces a strict byte budget and reports quota pressure exactly like OPFS,
 * so the launcher can tell the player that assets will not persist.
 */
export class MemoryAssetStorage implements AssetStorage {
  readonly kind = "memory" as const;
  private readonly roots = new Map<string, MemoryRoot>();

  constructor(private readonly budgetBytes = 512 * 1024 * 1024) {}

  private usedBytes(): number {
    let total = 0;
    for (const root of this.roots.values()) {
      for (const bytes of root.files.values()) total += bytes.length;
      for (const bytes of root.temps.values()) total += bytes.length;
    }
    return total;
  }

  async openStore(root: string): Promise<AssetFileStore> {
    assertSafeName(root);
    let data = this.roots.get(root);
    if (!data) {
      data = { files: new Map(), temps: new Map(), meta: new Map() };
      this.roots.set(root, data);
    }
    return new MemoryAssetFileStore(root, data, this.budgetBytes, () => this.usedBytes());
  }

  async listRoots(): Promise<readonly string[]> {
    return [...this.roots.keys()];
  }

  async deleteRoot(root: string): Promise<void> {
    this.roots.delete(root);
  }
}

/** Free/total byte estimate for the origin, when the browser exposes it. */
export interface StorageEstimate {
  readonly usageBytes?: number;
  readonly quotaBytes?: number;
}

export async function estimateStorage(): Promise<StorageEstimate> {
  if (typeof navigator === "undefined" || !navigator.storage?.estimate) return {};
  try {
    const estimate = await navigator.storage.estimate();
    const result: { usageBytes?: number; quotaBytes?: number } = {};
    if (typeof estimate.usage === "number") result.usageBytes = estimate.usage;
    if (typeof estimate.quota === "number") result.quotaBytes = estimate.quota;
    return result;
  } catch {
    return {};
  }
}

/** Asks the browser to exempt the origin from automatic eviction. */
export async function requestPersistentStorage(): Promise<boolean> {
  if (typeof navigator === "undefined" || !navigator.storage?.persist) return false;
  try {
    return await navigator.storage.persist();
  } catch {
    return false;
  }
}

export interface OpenAssetStorageOptions {
  /** Forces the in-memory fallback; used by tests and by `?nopersist` debug
   * boots to exercise the non-OPFS path. */
  readonly forceMemory?: boolean;
  /** Byte budget for the in-memory fallback. */
  readonly memoryBudgetBytes?: number;
  /** Directory name reserved for this pipeline inside OPFS. */
  readonly baseDirectory?: string;
}

/**
 * Opens the best available storage backend: OPFS when the browser supports
 * it, otherwise the in-memory fallback. Detection actually exercises OPFS
 * (rather than sniffing for the API) because some engines expose
 * `getDirectory` but reject it in private-browsing contexts.
 */
export async function openAssetStorage(options: OpenAssetStorageOptions = {}): Promise<AssetStorage> {
  const budget = options.memoryBudgetBytes;
  const memory = (): AssetStorage =>
    budget === undefined ? new MemoryAssetStorage() : new MemoryAssetStorage(budget);

  if (options.forceMemory) return memory();
  if (typeof navigator === "undefined" || !navigator.storage?.getDirectory) return memory();

  try {
    const opfsRoot = await navigator.storage.getDirectory();
    const base = (await (opfsRoot as unknown as OpfsDirectoryHandleLike).getDirectoryHandle(
      options.baseDirectory ?? "generalsx-assets",
      { create: true },
    )) as OpfsDirectoryHandleLike;
    return new OpfsAssetStorage(base);
  } catch {
    return memory();
  }
}
