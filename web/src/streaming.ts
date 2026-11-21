/**
 * Game archives are streamed on demand instead of being packaged into the wasm bundle.
 *
 * The engine reads archives with plain synchronous file reads, and neither browser escape hatch works
 * here: synchronous XHR is banned on the main thread, and Asyncify cannot unwind through the invoke_*
 * JS frames the engine's try/catch scopes create. So reads really are synchronous — a worker fetches
 * 4 MiB chunks into a SharedArrayBuffer while the main thread spin-waits — with an LRU keeping the
 * engine's many small reads off the network.
 */
import type { EmscriptenFS, EmscriptenModule } from "./types";

// 4 MiB chunks meant a 1 KiB read cost 4 MiB and only 48 blocks fitted the cache, so a map load
// thrashed and pulled ~35 GB over ~9000 requests. Smaller blocks, far more of them cached.
const CHUNK_SIZE = 256 * 1024;
// 400 MB, deliberately well under the renderer's comfort zone: the engine's own wasm heap rides
// in the same process, and a JS-side gigabyte tips the tab into a GC spiral (black canvas).
const CACHE_LIMIT = 400 * 1024 * 1024;
const READAHEAD = 8;  // chunks pulled per sequential miss
const READAHEAD_RUN = 32; // once a run is established, pull much more per round trip

export interface ArchiveEntry {
  mount: string;
  name: string;
  url: string;
  size: number;
  /** Set when the player picked their install with the directory picker; reads then bypass the server. */
  file?: File;
  handle?: FileSystemFileHandle;
}

export interface AssetManifest {
  entries: ArchiveEntry[];
  missing: boolean;
  defaultPath: string;
  configPath: string;
}

const workerSource = `
  postMessage("ready");
  onmessage = async (event) => {
    const { url, file, handle, start, end, sab } = event.data;
    const state = new Int32Array(sab, 0, 2);
    const data = new Uint8Array(sab, 8);
    try {
      let bytes;
      if (file) {
        bytes = new Uint8Array(await file.slice(start, end + 1).arrayBuffer());
      } else if (handle) {
        const resolved = await handle.getFile();
        bytes = new Uint8Array(await resolved.slice(start, end + 1).arrayBuffer());
      } else {
        const response = await fetch(url, { headers: { Range: "bytes=" + start + "-" + end } });
        if (!response.ok && response.status !== 206) throw new Error("HTTP " + response.status);
        const body = new Uint8Array(await response.arrayBuffer());
        // A 200 response carries the full file; slice it to the requested range.
        bytes = response.status === 206 ? body : body.subarray(start, end + 1);
      }
      data.set(bytes.subarray(0, data.length));
      state[1] = Math.min(bytes.length, data.length);
      Atomics.store(state, 0, 1);
    } catch {
      state[1] = 0;
      Atomics.store(state, 0, 2);
    }
  };`;

export class ArchiveStreamer {
  private readonly cache = new Map<string, Uint8Array>();
  private readonly byName = new Map<string, ArchiveEntry>();
  private cached = 0;
  private readonly pending = new Set<string>();
  private run = 0;
  private readonly nextIndex = new Map<string, number>();
  private worker: Worker | null = null;
  private buffer: SharedArrayBuffer | null = null;
  /** Resolves once the worker is alive. It must be running before the engine ever spin-waits:
      the main thread never yields afterwards, so a worker started later could never boot. */
  readonly ready: Promise<unknown>;

  constructor(private readonly onError: (message: string) => void) {
    if (!crossOriginIsolated) {
      this.ready = Promise.resolve();
      return;
    }
    this.worker = new Worker(URL.createObjectURL(new Blob([workerSource], { type: "text/javascript" })));
    this.buffer = new SharedArrayBuffer(CHUNK_SIZE * READAHEAD_RUN + 8);
    this.ready = new Promise((resolve) => this.worker?.addEventListener("message", resolve, { once: true }));
  }

  private fetchChunkSync(url: string, index: number, file?: File, handle?: FileSystemFileHandle, chunks = 1): Uint8Array {
    if (!this.worker || !this.buffer) {
      this.onError("SharedArrayBuffer unavailable: archives cannot stream.");
      return new Uint8Array(0);
    }
    const state = new Int32Array(this.buffer, 0, 2);
    Atomics.store(state, 0, 0);
    const start = index * CHUNK_SIZE;
    this.worker.postMessage({
      url: (file || handle) ? "" : new URL(url, location.href).href,
      file,
      handle: file ? undefined : handle,
      start,
      end: start + CHUNK_SIZE * chunks - 1,
      sab: this.buffer,
    });
    const deadline = Date.now() + 60_000;
    while (Atomics.load(state, 0) === 0) {
      if (Date.now() > deadline) {
        this.onError(`Archive fetch timed out: ${url} chunk ${index}`);
        return new Uint8Array(0);
      }
    }
    if (Atomics.load(state, 0) !== 1) {
      this.onError(`Archive fetch failed: ${url} chunk ${index}`);
      return new Uint8Array(0);
    }
    return new Uint8Array(this.buffer.slice(8, 8 + (state[1] ?? 0)));
  }

  private takeChunk(url: string, index: number, file?: File, handle?: FileSystemFileHandle): Uint8Array {
    const key = `${url}#${index}`;
    const cached = this.cache.get(key);
    if (cached) {
      this.cache.delete(key); // re-insert to mark most recently used
      this.cache.set(key, cached);
      return cached;
    }
    // Read ahead only while the file is being read sequentially. Doing it blindly pulled 8x the
    // bytes for scattered reads, and the transfer — not the request count — is the bottleneck.
    const sequential = this.nextIndex.get(url) === index;
    // No reporting from here: this runs inside the engine's synchronous read, and touching the DOM
    // per chunk stalled the boot.
    this.run = sequential ? this.run + 1 : 0;
    // A long sequential run means a map load walking an archive: pull much more per round trip.
    const chunks = sequential ? (this.run > 2 ? READAHEAD_RUN : READAHEAD) : 1;
    const span = this.fetchChunkSync(url, index, file, handle, chunks);
    this.nextIndex.set(url, index + span.length / CHUNK_SIZE);
    for (let offset = 0; offset < span.length; offset += CHUNK_SIZE) {
      const part = span.subarray(offset, Math.min(offset + CHUNK_SIZE, span.length));
      const partKey = `${url}#${index + offset / CHUNK_SIZE}`;
      if (!this.cache.has(partKey)) {
        this.cache.set(partKey, part);
        this.cached += part.length;
      }
    }
    const chunk = this.cache.get(key) ?? span.subarray(0, CHUNK_SIZE);
    while (this.cached > CACHE_LIMIT && this.cache.size > 1) {
      const oldest = this.cache.keys().next().value as string;
      this.cached -= this.cache.get(oldest)?.length ?? 0;
      this.cache.delete(oldest);
    }
    return chunk;
  }

  /** Pull an archive into the cache in the background so reads of it never block the engine.
      Audio is the case that matters: a sound played for the first time otherwise stalls the frame
      on a synchronous fetch, which on a slow link is heard as a stutter. */
  /** Pull an archive through the network into the browser's disk cache without keeping it in memory.
      Reads that miss the LRU then cost a local cache hit instead of a network round trip. */
  async prime(entries: ArchiveEntry[], onProgress: (done: number, name: string) => void = () => {}): Promise<void> {
    let done = 0;
    for (const entry of entries) {
      if (entry.file || entry.handle) continue; // already local
      const step = CHUNK_SIZE * READAHEAD_RUN;
      for (let start = 0; start < entry.size; start += step) {
        const end = Math.min(start + step, entry.size) - 1;
        try {
          const response = await fetch(entry.url, { headers: { Range: `bytes=${start}-${end}` } });
          await response.arrayBuffer(); // read and drop: the browser keeps it, we do not
        } catch {
          return;
        }
        done += end - start + 1;
        onProgress(done, entry.name);
        await new Promise((resolve) => setTimeout(resolve, 0));
      }
    }
  }

  async warm(entries: ArchiveEntry[], budget = 256 * 1024 * 1024,
             onProgress: (done: number, name: string) => void = () => {}): Promise<void> {
    let spent = 0;
    for (const entry of entries) {
      for (let index = 0; index * CHUNK_SIZE < entry.size; index += READAHEAD) {
        if (spent >= budget) return;
        const start = index * CHUNK_SIZE;
        const end = Math.min(start + CHUNK_SIZE * READAHEAD, entry.size) - 1;
        const chunkCount = Math.ceil((end - start + 1) / CHUNK_SIZE);
        let allCached = true;
        for (let offset = 0; offset < chunkCount; offset++) {
          if (!this.cache.has(`${entry.url}#${index + offset}`)) {
            allCached = false;
            break;
          }
        }
        if (allCached) {
          // Already resident chunks still count as progress, or the bar stalls on cached stretches.
          spent += end - start + 1;
          onProgress(spent, entry.name);
          continue;
        }
        try {
          let bytes: Uint8Array;
          if (entry.file) {
            bytes = new Uint8Array(await entry.file.slice(start, end + 1).arrayBuffer());
          } else if (entry.handle) {
            bytes = new Uint8Array(await (await entry.handle.getFile()).slice(start, end + 1).arrayBuffer());
          } else {
            const response = await fetch(entry.url, {
              headers: { Range: `bytes=${start}-${end}` },
            });
            if (!response.ok || (response.status !== 206 && response.status !== 200)) {
              throw new Error(`HTTP ${response.status} fetching ${entry.url}`);
            }
            const body = new Uint8Array(await response.arrayBuffer());
            // A 200 response carries the full file; copy only the requested range so the full buffer can be GC'd.
            bytes = response.status === 206 ? body : body.slice(start, end + 1);
          }
          for (let offset = 0; offset < bytes.length; offset += CHUNK_SIZE) {
            const partKey = `${entry.url}#${index + offset / CHUNK_SIZE}`;
            if (this.cache.has(partKey)) continue;
            const part = bytes.subarray(offset, Math.min(offset + CHUNK_SIZE, bytes.length));
            this.cache.set(partKey, part);
            this.cached += part.length;
          }
          while (this.cached > CACHE_LIMIT && this.cache.size > 1) {
            const oldest = this.cache.keys().next().value as string;
            this.cached -= this.cache.get(oldest)?.length ?? 0;
            this.cache.delete(oldest);
          }
          spent += bytes.length;
          onProgress(spent, entry.name);
        } catch {
          return; // link died; reads fall back to fetching on demand
        }
        await new Promise((resolve) => setTimeout(resolve, 0)); // stay off the engine's back
      }
    }
  }

  /** Queue a chunk for the background warmer without blocking the caller. */
  private schedule(entry: ArchiveEntry, index: number): void {
    const key = `${entry.url}#${index}`;
    // Cap in-flight background reads: firing one per miss opened hundreds of sockets and the server
    // started refusing connections.
    if (this.cache.has(key) || this.pending.has(key) || this.pending.size >= 8) return;
    this.pending.add(key);
    const start = index * CHUNK_SIZE;
    const end = Math.min(start + CHUNK_SIZE * READAHEAD, entry.size) - 1;
    const source = entry.file
      ? entry.file.slice(start, end + 1).arrayBuffer().then((b) => new Uint8Array(b))
      : entry.handle
      ? entry.handle.getFile().then((file) => file.slice(start, end + 1).arrayBuffer()).then((b) => new Uint8Array(b))
      : fetch(entry.url, { headers: { Range: `bytes=${start}-${end}` } }).then(async (response) => {
          if (!response.ok || (response.status !== 206 && response.status !== 200)) {
            throw new Error(`HTTP ${response.status} fetching ${entry.url}`);
          }
          const body = new Uint8Array(await response.arrayBuffer());
          return response.status === 206 ? body : body.slice(start, end + 1);
        });
    void source
      .then((bytes) => {
        for (let offset = 0; offset < bytes.length; offset += CHUNK_SIZE) {
          const partKey = `${entry.url}#${index + offset / CHUNK_SIZE}`;
          if (this.cache.has(partKey)) continue;
          const part = bytes.subarray(offset, Math.min(offset + CHUNK_SIZE, bytes.length));
          this.cache.set(partKey, part);
          this.cached += part.length;
        }
        while (this.cached > CACHE_LIMIT && this.cache.size > 1) {
          const oldest = this.cache.keys().next().value as string;
          this.cached -= this.cache.get(oldest)?.length ?? 0;
          this.cache.delete(oldest);
        }
      })
      .catch(() => {})
      .finally(() => this.pending.delete(key));
  }

  /** 1 if [offset, offset+size) of the named archive is readable without touching the network;
      otherwise kick background fetches for the missing chunks and return 0. Called by the engine
      once per skipped sound — keep it allocation-light, never touch the DOM. */
  ensure(big: string, offset: number, size: number): number {
    const entry = this.byName.get((big.split(/[\\/]/).pop() ?? big).toLowerCase());
    if (!entry || entry.file || entry.handle) return 1; // unknown → behave as before; local disk → fast reads
    const first = Math.floor(offset / CHUNK_SIZE);
    const last = Math.floor((offset + Math.max(1, size) - 1) / CHUNK_SIZE);
    let resident = 1;
    for (let index = first; index <= last; index += 1) {
      const key = `${entry.url}#${index}`;
      const chunk = this.cache.get(key);
      if (chunk) {
        this.cache.delete(key); // re-insert as most recent: pinned while the engine waits on it
        this.cache.set(key, chunk);
        continue;
      }
      resident = 0;
      this.schedule(entry, index);
      index += READAHEAD - 1; // schedule() pulls READAHEAD chunks starting at index
    }
    return resident;
  }

  mount(module: EmscriptenModule, entry: ArchiveEntry): void {
    const FS = module.FS as EmscriptenFS;
    this.byName.set(entry.name.toLowerCase(), entry);
    FS.mkdirTree(entry.mount);
    const node = FS.createFile(entry.mount, entry.name, {}, true, false);
    const size = entry.size;
    Object.defineProperty(node, "usedBytes", { get: () => size });
    node.stream_ops = {
      llseek: (stream, offset, whence) => {
        let position = offset;
        if (whence === 1) position += stream.position;
        else if (whence === 2) position = size + offset;
        if (position < 0) throw new FS.ErrnoError(28);
        return position;
      },
      read: (_stream, buffer, offset, length, position) => {
        const end = Math.min(size, position + length);
        if (position >= end) return 0;
        const firstChunk = Math.floor(position / CHUNK_SIZE);
        const lastChunk = Math.floor((end - 1) / CHUNK_SIZE);
        let written = 0;
        // No silence-for-missing-bytes here: a decoder fed zeros caches a silent sound forever.
        // The engine now skips cold sounds before opening them (GeneralsXFileResident), so any
        // read that reaches this point is expected to block until the real bytes exist.
        for (let index = firstChunk; index <= lastChunk; index += 1) {
          const chunk = this.takeChunk(entry.url, index, entry.file, entry.handle);
          const chunkStart = index * CHUNK_SIZE;
          const from = Math.max(position, chunkStart) - chunkStart;
          const to = Math.min(end, chunkStart + chunk.length) - chunkStart;
          if (to <= from) break; // short chunk: fetch failed or EOF
          buffer.set(chunk.subarray(from, to), offset + written);
          written += to - from;
        }
        return written;
      },
    };
  }
}

export async function loadManifest(): Promise<AssetManifest> {
  const response = await fetch("/GeneralsXAssets");
  return (await response.json()) as AssetManifest;
}

/** Walk a picked directory looking for the two archive sets, wherever they sit inside it. */
export async function findArchiveDirs(
  root: FileSystemDirectoryHandle,
  depth = 3,
): Promise<Map<string, FileSystemDirectoryHandle>> {
  const found = new Map<string, FileSystemDirectoryHandle>();
  const visit = async (directory: FileSystemDirectoryHandle, level: number): Promise<void> => {
    let zeroHour = false;
    let base = false;
    const children: FileSystemDirectoryHandle[] = [];
    for await (const [name, handle] of directory.entries()) {
      if (handle.kind === "directory") {
        children.push(handle as FileSystemDirectoryHandle);
      } else if (name.toLowerCase().endsWith(".big")) {
        // ZH ships *ZH.big archives; the base game does not. That is the whole discriminator.
        if (/zh\.big$/i.test(name)) zeroHour = true;
        else base = true;
      }
    }
    if (zeroHour && !found.has("GeneralsZH")) found.set("GeneralsZH", directory);
    else if (base && !zeroHour && !found.has("Generals")) found.set("Generals", directory);
    if (found.size === 2 || level >= depth) return;
    for (const child of children) {
      await visit(child, level + 1);
      if (found.size === 2) return;
    }
  };
  await visit(root, 0);
  return found;
}

/** Archives from the folders the player picked, read locally with no server involvement. */
export async function localArchives(): Promise<ArchiveEntry[]> {
  try {
    return await readLocalArchives();
  } catch (error) {
    // Never let a permission/IDB failure skip mounting entirely — the caller falls back to the server.
    console.debug("local archives unavailable", error);
    return [];
  }
}

async function readLocalArchives(): Promise<ArchiveEntry[]> {
  const database = await new Promise<IDBDatabase>((resolve, reject) => {
    const request = indexedDB.open("generalsx", 1);
    request.onupgradeneeded = () => request.result.createObjectStore("handles");
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
  // Both reads must be issued before the first await: an IDB transaction closes as soon as the
  // event loop yields, so a second get() after awaiting getFile() throws "transaction has finished".
  const store = database.transaction("handles", "readonly").objectStore("handles");
  const request = (key: string) =>
    new Promise<FileSystemDirectoryHandle | undefined>((resolve) => {
      const get = store.get(key);
      get.onsuccess = () => resolve(get.result as FileSystemDirectoryHandle | undefined);
      get.onerror = () => resolve(undefined);
    });
  const pending = { GeneralsZH: request("GeneralsZH"), Generals: request("Generals") };
  const entries: ArchiveEntry[] = [];
  for (const mount of ["GeneralsZH", "Generals"] as const) {
    const directory = await pending[mount];
    if (!directory) continue;
    if ((await directory.queryPermission?.({ mode: "read" })) !== "granted"
        && (await directory.requestPermission?.({ mode: "read" })) !== "granted") {
      return [];
    }
    for await (const [name, handle] of directory.entries()) {
      if (!name.toLowerCase().endsWith(".big") || handle.kind !== "file") continue;
      const file = await (handle as FileSystemFileHandle).getFile();
      entries.push({ mount: `/${mount}`, name, url: `local:${mount}/${name}`, size: file.size,
                     file, handle: handle as FileSystemFileHandle });
    }
    try {
      const dataDir = await directory.getDirectoryHandle("Data");
      for (const sub of ["Cursors", "Scripts"] as const) {
        try {
          const subDir = await dataDir.getDirectoryHandle(sub);
          for await (const [name, handle] of subDir.entries()) {
            if (handle.kind !== "file") continue;
            const file = await (handle as FileSystemFileHandle).getFile();
            entries.push({
              mount: `/${mount}/Data/${sub}`,
              name,
              url: `local:${mount}/Data/${sub}/${name}`,
              size: file.size,
              file,
              handle: handle as FileSystemFileHandle,
            });
          }
        } catch {
          // Subdirectory absent in this install folder.
        }
      }
    } catch {
      // Data/ absent in this install folder.
    }
  }
  return entries;
}

/** True when the player already picked folders, even if this load cannot read them yet. */
export async function hasSavedFolders(): Promise<boolean> {
  try {
    const database = await new Promise<IDBDatabase>((resolve, reject) => {
      const request = indexedDB.open("generalsx", 1);
      request.onupgradeneeded = () => request.result.createObjectStore("handles");
      request.onsuccess = () => resolve(request.result);
      request.onerror = () => reject(request.error);
    });
    return await new Promise<boolean>((resolve) => {
      const request = database.transaction("handles", "readonly").objectStore("handles").count();
      request.onsuccess = () => resolve(request.result > 0);
      request.onerror = () => resolve(false);
    });
  } catch {
    return false;
  }
}
