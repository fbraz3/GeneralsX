import type { AssetRole } from "@generalsx-web/shared/manifest";
import type { VfsEntry } from "../assets/vfs.js";
import type { LauncherSettings } from "../ui/settings.js";

const STAGING_CHUNK_BYTES = 4 * 1024 * 1024;
const ENGINE_ROLES = new Set<AssetRole>(["engine-js", "engine-wasm", "engine-data"]);

export interface EngineAssetSource {
  list(): readonly VfsEntry[];
  read(target: string, offset: number, length: number): Promise<Uint8Array>;
  readAll(target: string): Promise<Uint8Array>;
  clearCache(): void;
}

export interface EmscriptenFileSystem {
  mkdirTree(path: string): void;
  writeFile(path: string, data: string | Uint8Array, options?: { canOwn?: boolean }): void;
  open(path: string, flags: string): unknown;
  allocate?(stream: unknown, offset: number, length: number): void;
  write(stream: unknown, data: Uint8Array, offset: number, length: number, position: number): number;
  close(stream: unknown): void;
}

export interface EmscriptenRuntimeGlobals {
  readonly FS: EmscriptenFileSystem;
  readonly ENV: Record<string, string>;
  addRunDependency(id: string): void;
  removeRunDependency(id: string): void;
  abort(reason: string): never;
}

export interface GeneralsXAudioApi {
  bindUserGesture(element: HTMLElement): () => void;
  unlock(): Promise<{ readonly running: boolean }>;
}

export interface EmscriptenModuleConfig {
  canvas: HTMLCanvasElement;
  arguments: string[];
  locateFile(path: string): string;
  print(text: string): void;
  printErr(text: string): void;
  onAbort(reason: unknown): void;
  onRuntimeInitialized(): void;
  preRun: Array<() => void>;
  generalsxAudio?: GeneralsXAudioApi;
}

export interface EngineScriptHost {
  publishModule(module: EmscriptenModuleConfig): void;
  runtimeGlobals(): EmscriptenRuntimeGlobals;
  createObjectUrl(bytes: Uint8Array, mimeType: string): string;
  revokeObjectUrl(url: string): void;
  loadClassicScript(url: string): Promise<void>;
}

export interface LaunchEngineOptions {
  readonly assets: EngineAssetSource;
  readonly canvas: HTMLCanvasElement;
  readonly settings: LauncherSettings;
  readonly arguments?: readonly string[];
  readonly udpReady?: Promise<unknown>;
  readonly host?: EngineScriptHost;
  readonly onStatus?: (status: string) => void;
  readonly onLog?: (text: string) => void;
}

export class EngineBootError extends Error {
  constructor(message: string, readonly rootCause?: unknown) {
    super(message);
    this.name = "EngineBootError";
  }
}

function parentPath(path: string): string {
  const slash = path.lastIndexOf("/");
  return slash <= 0 ? "/" : path.slice(0, slash);
}

function singleton(entries: readonly VfsEntry[], role: AssetRole): VfsEntry {
  const matches = entries.filter((entry) => entry.role === role);
  if (matches.length !== 1) {
    throw new EngineBootError(`expected exactly one ${role} asset, found ${matches.length}`);
  }
  return matches[0]!;
}

export function buildOptionsIni(
  settings: LauncherSettings,
  width: number,
  height: number,
): string {
  const quality = settings.graphicsQuality[0]!.toUpperCase() + settings.graphicsQuality.slice(1);
  const volume = Math.round(Math.min(1, Math.max(0, settings.volume)) * 100);
  return [
    `StaticGameLOD = ${quality}`,
    "UseShadowVolumes = no",
    "UseShadowDecals = no",
    `Resolution = ${Math.max(1, width)} ${Math.max(1, height)}`,
    `SFXVolume = ${volume}`,
    `MusicVolume = ${volume}`,
    `VoiceVolume = ${volume}`,
    "",
  ].join("\n");
}

async function stageStreamingAsset(
  source: EngineAssetSource,
  fs: EmscriptenFileSystem,
  entry: VfsEntry,
  onStatus: (status: string) => void,
): Promise<void> {
  fs.mkdirTree(parentPath(entry.target));
  const stream = fs.open(entry.target, "w+");
  try {
    fs.allocate?.(stream, 0, entry.sizeBytes);
    for (let offset = 0; offset < entry.sizeBytes; offset += STAGING_CHUNK_BYTES) {
      const requested = Math.min(STAGING_CHUNK_BYTES, entry.sizeBytes - offset);
      const chunk = await source.read(entry.target, offset, requested);
      if (chunk.byteLength !== requested) {
        throw new EngineBootError(
          `asset "${entry.path}" became truncated while staging (${offset + chunk.byteLength}/${entry.sizeBytes} bytes)`,
        );
      }
      const written = fs.write(stream, chunk, 0, chunk.byteLength, offset);
      if (written !== chunk.byteLength) {
        throw new EngineBootError(`short write while staging "${entry.path}"`);
      }
      onStatus(`Mounting ${entry.path} (${Math.round(((offset + written) / entry.sizeBytes) * 100)}%)`);
    }
  } finally {
    fs.close(stream);
  }
}

export async function stageEngineAssets(
  source: EngineAssetSource,
  runtime: EmscriptenRuntimeGlobals,
  settings: LauncherSettings,
  canvas: HTMLCanvasElement,
  onStatus: (status: string) => void = () => undefined,
): Promise<void> {
  runtime.ENV.CNC_GENERALS_ZH_PATH = "/game";
  runtime.ENV.CNC_GENERALS_PATH = "/game-base";
  runtime.ENV.HOME = "/home/web_user";

  const fs = runtime.FS;
  fs.mkdirTree("/game");
  fs.mkdirTree("/game-base");
  fs.mkdirTree("/fonts");
  const userDirectory = "/home/web_user/.local/share/GeneralsX/GeneralsZH";
  fs.mkdirTree(userDirectory);
  fs.writeFile(
    `${userDirectory}/Options.ini`,
    buildOptionsIni(settings, canvas.width || 1024, canvas.height || 768),
  );

  const runtimeEntries = source.list().filter((entry) => !ENGINE_ROLES.has(entry.role));
  for (const entry of runtimeEntries) {
    onStatus(`Mounting ${entry.path}`);
    if (entry.streaming) {
      await stageStreamingAsset(source, fs, entry, onStatus);
    } else {
      const bytes = await source.readAll(entry.target);
      if (bytes.byteLength !== entry.sizeBytes) {
        throw new EngineBootError(
          `asset "${entry.path}" became truncated while staging (${bytes.byteLength}/${entry.sizeBytes} bytes)`,
        );
      }
      fs.mkdirTree(parentPath(entry.target));
      fs.writeFile(entry.target, bytes, { canOwn: true });
    }
  }
  source.clearCache();
}

function browserScriptHost(): EngineScriptHost {
  return {
    publishModule(module) {
      window.Module = module;
    },
    runtimeGlobals() {
      const runtime = globalThis as typeof globalThis & Partial<EmscriptenRuntimeGlobals>;
      if (!runtime.FS || !runtime.ENV || !runtime.addRunDependency || !runtime.removeRunDependency || !runtime.abort) {
        throw new EngineBootError("Emscripten runtime globals are unavailable during preRun");
      }
      return runtime as typeof globalThis & EmscriptenRuntimeGlobals;
    },
    createObjectUrl(bytes, mimeType) {
      return URL.createObjectURL(new Blob([bytes.slice().buffer as ArrayBuffer], { type: mimeType }));
    },
    revokeObjectUrl(url) {
      URL.revokeObjectURL(url);
    },
    loadClassicScript(url) {
      return new Promise<void>((resolve, reject) => {
        const script = document.createElement("script");
        script.src = url;
        script.async = true;
        script.addEventListener("load", () => resolve(), { once: true });
        script.addEventListener("error", () => reject(new EngineBootError("failed to execute the verified engine loader")), {
          once: true,
        });
        document.body.appendChild(script);
      });
    },
  };
}

export async function launchEmscriptenEngine(
  options: LaunchEngineOptions,
): Promise<EmscriptenModuleConfig> {
  const host = options.host ?? browserScriptHost();
  const entries = options.assets.list();
  const scriptEntry = singleton(entries, "engine-js");
  const wasmEntry = singleton(entries, "engine-wasm");
  const dataEntry = entries.find((entry) => entry.role === "engine-data");
  const onStatus = options.onStatus ?? (() => undefined);
  const onLog = options.onLog ?? globalThis.console.log.bind(globalThis.console);

  onStatus("Preparing verified engine files");
  const scriptBytes = await options.assets.readAll(scriptEntry.target);
  const wasmBytes = await options.assets.readAll(wasmEntry.target);
  const dataBytes = dataEntry ? await options.assets.readAll(dataEntry.target) : undefined;
  const scriptUrl = host.createObjectUrl(scriptBytes, "text/javascript");
  const wasmUrl = host.createObjectUrl(wasmBytes, "application/wasm");
  const dataUrl = dataBytes ? host.createObjectUrl(dataBytes, "application/octet-stream") : undefined;
  const objectUrls = [scriptUrl, wasmUrl, ...(dataUrl ? [dataUrl] : [])];
  let urlsRevoked = false;
  const revokeObjectUrls = (): void => {
    if (urlsRevoked) return;
    urlsRevoked = true;
    objectUrls.forEach((url) => host.revokeObjectUrl(url));
  };

  let resolveRuntime!: (module: EmscriptenModuleConfig) => void;
  let rejectRuntime!: (reason: unknown) => void;
  const runtimeReady = new Promise<EmscriptenModuleConfig>((resolve, reject) => {
    resolveRuntime = resolve;
    rejectRuntime = reject;
  });

  const module: EmscriptenModuleConfig = {
    canvas: options.canvas,
    arguments: ["-win", ...(options.arguments ?? [])],
    locateFile(path) {
      if (path.endsWith(".wasm")) return wasmUrl;
      if (dataUrl && path.endsWith(".data")) return dataUrl;
      return path;
    },
    print: onLog,
    printErr: onLog,
    onAbort(reason) {
      revokeObjectUrls();
      rejectRuntime(new EngineBootError(`engine aborted: ${String(reason)}`, reason));
    },
    onRuntimeInitialized() {
      revokeObjectUrls();
      resolveRuntime(module);
    },
    preRun: [
      () => {
        const dependency = "generalsx-runtime-assets";
        let runtime: EmscriptenRuntimeGlobals;
        try {
          runtime = host.runtimeGlobals();
          runtime.addRunDependency(dependency);
        } catch (error) {
          rejectRuntime(error);
          return;
        }
        void stageEngineAssets(options.assets, runtime, options.settings, options.canvas, onStatus)
          .then(() => runtime.removeRunDependency(dependency))
          .catch((error: unknown) => {
            const bootError =
              error instanceof EngineBootError
                ? error
                : new EngineBootError("failed to mount verified game data", error);
            rejectRuntime(bootError);
            runtime.abort(bootError.message);
          });
      },
    ],
  };

  host.publishModule(module);
  if (options.udpReady) {
    onStatus("Connecting multiplayer transport");
    await options.udpReady;
  }
  onStatus("Starting GeneralsX");
  try {
    await host.loadClassicScript(scriptUrl);
  } catch (error) {
    revokeObjectUrls();
    throw error;
  }
  return runtimeReady;
}
