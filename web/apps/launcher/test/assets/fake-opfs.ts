/**
 * In-memory stand-in for the Origin Private File System, used by the asset
 * pipeline tests. It implements only the structural interfaces
 * `src/assets/storage.ts` depends on, and can be configured to reproduce the
 * behaviours that matter for correctness:
 *
 *  - engines without `FileSystemFileHandle.move()` (copy-then-delete path),
 *  - a hard storage quota that raises `QuotaExceededError` mid-write,
 *  - swap-file write semantics (nothing is visible until `close()`).
 */
import type {
  OpfsBlobLike,
  OpfsDirectoryHandleLike,
  OpfsFileHandleLike,
  OpfsWritableLike,
} from "../../src/assets/storage.js";

export interface FakeOpfsOptions {
  /** Emulate a browser that exposes atomic `move()` (default true). */
  readonly supportsMove?: boolean;
  /** Total byte budget across every file (default unlimited). */
  readonly quotaBytes?: number;
}

interface FakeState {
  quotaBytes: number;
  totalBytes: number;
}

function notFound(name: string): DOMException {
  return new DOMException(`entry not found: ${name}`, "NotFoundError");
}

class FakeBlob implements OpfsBlobLike {
  constructor(private readonly bytes: Uint8Array) {}

  get size(): number {
    return this.bytes.length;
  }

  slice(start = 0, end = this.bytes.length): OpfsBlobLike {
    return new FakeBlob(this.bytes.slice(start, end));
  }

  async arrayBuffer(): Promise<ArrayBuffer> {
    const copy = this.bytes.slice();
    return copy.buffer as ArrayBuffer;
  }

  async text(): Promise<string> {
    return new TextDecoder().decode(this.bytes);
  }
}

class FakeWritable implements OpfsWritableLike {
  private buffer: Uint8Array;
  private length: number;
  private closed = false;

  constructor(
    private readonly handle: FakeFileHandle,
    keepExistingData: boolean,
    private readonly state: FakeState,
  ) {
    const existing = keepExistingData ? handle.bytes : new Uint8Array(0);
    this.buffer = existing.slice();
    this.length = this.buffer.length;
  }

  async write(data: Uint8Array | { type: "write"; position?: number; data: Uint8Array }): Promise<void> {
    if (this.closed) throw new TypeError("writable is closed");
    const bytes = data instanceof Uint8Array ? data : data.data;
    const position = data instanceof Uint8Array ? this.length : (data.position ?? 0);
    const required = position + bytes.length;

    const growth = Math.max(0, required - this.handle.bytes.length);
    if (this.state.totalBytes + growth > this.state.quotaBytes) {
      throw new DOMException("fake OPFS quota exceeded", "QuotaExceededError");
    }

    if (this.buffer.length < required) {
      const grown = new Uint8Array(required);
      grown.set(this.buffer.subarray(0, this.length), 0);
      this.buffer = grown;
    }
    this.buffer.set(bytes, position);
    this.length = Math.max(this.length, required);
  }

  async truncate(size: number): Promise<void> {
    this.length = Math.min(this.length, size);
  }

  async close(): Promise<void> {
    if (this.closed) return;
    this.closed = true;
    this.handle.replace(this.buffer.subarray(0, this.length));
  }

  async abort(): Promise<void> {
    // Swap-file semantics: discard everything written in this session.
    this.closed = true;
  }
}

export class FakeFileHandle implements OpfsFileHandleLike {
  bytes = new Uint8Array(0);
  move?: (destination: OpfsDirectoryHandleLike, name?: string) => Promise<void>;

  constructor(
    public name: string,
    public parent: FakeDirectoryHandle,
    private readonly state: FakeState,
    supportsMove: boolean,
  ) {
    if (supportsMove) {
      this.move = async (destination: OpfsDirectoryHandleLike, newName?: string): Promise<void> => {
        const target = destination as FakeDirectoryHandle;
        this.parent.detach(this.name);
        this.name = newName ?? this.name;
        this.parent = target;
        target.attach(this);
      };
    }
  }

  replace(bytes: Uint8Array): void {
    this.state.totalBytes += bytes.length - this.bytes.length;
    this.bytes = bytes.slice();
  }

  async getFile(): Promise<OpfsBlobLike> {
    return new FakeBlob(this.bytes);
  }

  async createWritable(options?: { keepExistingData?: boolean }): Promise<OpfsWritableLike> {
    return new FakeWritable(this, options?.keepExistingData === true, this.state);
  }
}

export class FakeDirectoryHandle implements OpfsDirectoryHandleLike {
  private readonly files = new Map<string, FakeFileHandle>();
  private readonly dirs = new Map<string, FakeDirectoryHandle>();

  constructor(
    readonly name: string,
    private readonly state: FakeState,
    private readonly supportsMove: boolean,
  ) {}

  static create(options: FakeOpfsOptions = {}): FakeDirectoryHandle {
    const state: FakeState = {
      quotaBytes: options.quotaBytes ?? Number.POSITIVE_INFINITY,
      totalBytes: 0,
    };
    return new FakeDirectoryHandle("", state, options.supportsMove !== false);
  }

  attach(handle: FakeFileHandle): void {
    this.files.set(handle.name, handle);
  }

  detach(name: string): void {
    this.files.delete(name);
  }

  async getFileHandle(name: string, options?: { create?: boolean }): Promise<OpfsFileHandleLike> {
    const existing = this.files.get(name);
    if (existing) return existing;
    if (options?.create !== true) throw notFound(name);
    const handle = new FakeFileHandle(name, this, this.state, this.supportsMove);
    this.files.set(name, handle);
    return handle;
  }

  async getDirectoryHandle(
    name: string,
    options?: { create?: boolean },
  ): Promise<OpfsDirectoryHandleLike> {
    const existing = this.dirs.get(name);
    if (existing) return existing;
    if (options?.create !== true) throw notFound(name);
    const dir = new FakeDirectoryHandle(name, this.state, this.supportsMove);
    this.dirs.set(name, dir);
    return dir;
  }

  async removeEntry(name: string, options?: { recursive?: boolean }): Promise<void> {
    const file = this.files.get(name);
    if (file) {
      this.state.totalBytes -= file.bytes.length;
      this.files.delete(name);
      return;
    }
    const dir = this.dirs.get(name);
    if (dir) {
      if (options?.recursive !== true && dir.entryCount() > 0) {
        throw new DOMException("directory not empty", "InvalidModificationError");
      }
      dir.release();
      this.dirs.delete(name);
      return;
    }
    throw notFound(name);
  }

  async *keys(): AsyncIterableIterator<string> {
    for (const name of this.dirs.keys()) yield name;
    for (const name of this.files.keys()) yield name;
  }

  /* -- test helpers ------------------------------------------------- */

  entryCount(): number {
    return this.files.size + this.dirs.size;
  }

  release(): void {
    for (const file of this.files.values()) this.state.totalBytes -= file.bytes.length;
    for (const dir of this.dirs.values()) dir.release();
    this.files.clear();
    this.dirs.clear();
  }

  /** Reads a nested path such as `root/files/abc` for assertions. */
  peek(path: string): Uint8Array | undefined {
    const [head, ...rest] = path.split("/");
    if (head === undefined) return undefined;
    if (rest.length === 0) return this.files.get(head)?.bytes;
    return this.dirs.get(head)?.peek(rest.join("/"));
  }

  private ensureDir(segments: readonly string[]): FakeDirectoryHandle {
    const [head, ...rest] = segments;
    if (head === undefined) return this;
    let next = this.dirs.get(head);
    if (!next) {
      next = new FakeDirectoryHandle(head, this.state, this.supportsMove);
      this.dirs.set(head, next);
    }
    return next.ensureDir(rest);
  }

  /** Directly overwrites a nested file, simulating on-disk corruption. */
  poke(path: string, bytes: Uint8Array): void {
    const segments = path.split("/");
    const name = segments.pop();
    if (name === undefined) throw new Error("empty path");
    const dir = this.ensureDir(segments);
    let handle = dir.files.get(name);
    if (!handle) {
      handle = new FakeFileHandle(name, dir, this.state, this.supportsMove);
      dir.files.set(name, handle);
    }
    handle.replace(bytes);
  }

  /** Lists file names inside a nested directory, or `undefined` if missing. */
  listDir(path: string): readonly string[] | undefined {
    if (path === "") return [...this.dirs.keys(), ...this.files.keys()];
    const [head, ...rest] = path.split("/");
    if (head === undefined) return undefined;
    const dir = this.dirs.get(head);
    if (!dir) return undefined;
    return rest.length === 0 ? [...dir.files.keys()] : dir.listDir(rest.join("/"));
  }

  totalBytes(): number {
    return this.state.totalBytes;
  }
}
