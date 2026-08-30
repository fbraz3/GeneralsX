import { describe, expect, it, vi } from "vitest";
import type { AssetRole } from "@generalsx-web/shared/manifest";
import {
  EngineBootError,
  buildOptionsIni,
  launchEmscriptenEngine,
  stageEngineAssets,
  type EmscriptenFileSystem,
  type EmscriptenModuleConfig,
  type EmscriptenRuntimeGlobals,
  type EngineAssetSource,
  type EngineScriptHost,
} from "../../src/engine/emscripten-loader.js";
import type { VfsEntry } from "../../src/assets/vfs.js";

function entry(
  role: AssetRole,
  target: string,
  sizeBytes: number,
  streaming = false,
): VfsEntry {
  return {
    role,
    target,
    path: target.slice(1),
    sizeBytes,
    sha256: role.padEnd(64, "0"),
    order: 0,
    streaming,
    storageName: role.padEnd(64, "0"),
  };
}

class FakeAssets implements EngineAssetSource {
  readonly reads: Array<{ target: string; offset: number; length: number }> = [];
  cleared = false;

  constructor(
    private readonly entries: readonly VfsEntry[],
    private readonly bytes: ReadonlyMap<string, Uint8Array>,
  ) {}

  list(): readonly VfsEntry[] {
    return this.entries;
  }

  async read(target: string, offset: number, length: number): Promise<Uint8Array> {
    this.reads.push({ target, offset, length });
    return this.bytes.get(target)!.slice(offset, offset + length);
  }

  async readAll(target: string): Promise<Uint8Array> {
    return this.bytes.get(target)!.slice();
  }

  clearCache(): void {
    this.cleared = true;
  }
}

class FakeFileSystem implements EmscriptenFileSystem {
  readonly directories: string[] = [];
  readonly files = new Map<string, Uint8Array | string>();
  private readonly openFiles = new Map<unknown, { path: string; bytes: Uint8Array }>();

  mkdirTree(path: string): void {
    this.directories.push(path);
  }

  writeFile(path: string, data: string | Uint8Array): void {
    this.files.set(path, typeof data === "string" ? data : data.slice());
  }

  open(path: string): unknown {
    const stream = {};
    this.openFiles.set(stream, { path, bytes: new Uint8Array() });
    return stream;
  }

  allocate(stream: unknown, _offset: number, length: number): void {
    this.openFiles.get(stream)!.bytes = new Uint8Array(length);
  }

  write(stream: unknown, data: Uint8Array, offset: number, length: number, position: number): number {
    const file = this.openFiles.get(stream)!;
    const required = position + length;
    if (file.bytes.length < required) {
      const expanded = new Uint8Array(required);
      expanded.set(file.bytes);
      file.bytes = expanded;
    }
    file.bytes.set(data.subarray(offset, offset + length), position);
    return length;
  }

  close(stream: unknown): void {
    const file = this.openFiles.get(stream)!;
    this.files.set(file.path, file.bytes);
    this.openFiles.delete(stream);
  }
}

function fakeRuntime(fs = new FakeFileSystem()): EmscriptenRuntimeGlobals {
  return {
    FS: fs,
    ENV: {},
    addRunDependency: vi.fn(),
    removeRunDependency: vi.fn(),
    abort: vi.fn((reason: string) => {
      throw new Error(reason);
    }) as never,
  };
}

const SETTINGS = { volume: 0.75, graphicsQuality: "medium" as const, playerName: "Ada" };

describe("buildOptionsIni", () => {
  it("maps launcher settings to engine preferences", () => {
    expect(buildOptionsIni(SETTINGS, 1280, 720)).toContain("StaticGameLOD = Medium");
    expect(buildOptionsIni(SETTINGS, 1280, 720)).toContain("Resolution = 1280 720");
    expect(buildOptionsIni(SETTINGS, 1280, 720)).toContain("MusicVolume = 75");
    expect(buildOptionsIni(SETTINGS, 1280, 720)).toContain("UseShadowVolumes = no");
  });
});

describe("stageEngineAssets", () => {
  it("stages scripts whole and archives in bounded chunks at manifest targets", async () => {
    const archiveBytes = new Uint8Array(5 * 1024 * 1024).fill(7);
    const scriptBytes = new TextEncoder().encode("script");
    const entries = [
      entry("engine-js", "/engine/GeneralsXZH.js", 2),
      entry("engine-wasm", "/engine/GeneralsXZH.wasm", 2),
      entry("big-expansion", "/game/INIZH.big", archiveBytes.length, true),
      entry("script", "/game/Data/Scripts/Scripts.ini", scriptBytes.length),
    ];
    const assets = new FakeAssets(
      entries,
      new Map([
        ["/engine/GeneralsXZH.js", new Uint8Array([1, 2])],
        ["/engine/GeneralsXZH.wasm", new Uint8Array([3, 4])],
        ["/game/INIZH.big", archiveBytes],
        ["/game/Data/Scripts/Scripts.ini", scriptBytes],
      ]),
    );
    const fs = new FakeFileSystem();
    const runtime = fakeRuntime(fs);
    const canvas = { width: 1024, height: 768 } as HTMLCanvasElement;
    const progress: Array<[number, number]> = [];

    await stageEngineAssets(
      assets,
      runtime,
      SETTINGS,
      canvas,
      undefined,
      (completed, total) => progress.push([completed, total]),
    );

    expect(runtime.ENV).toMatchObject({
      CNC_GENERALS_ZH_PATH: "/game",
      CNC_GENERALS_PATH: "/game-base",
      HOME: "/home/web_user",
    });
    const stagedArchive = fs.files.get("/game/INIZH.big") as Uint8Array;
    expect(stagedArchive).toHaveLength(archiveBytes.length);
    expect(stagedArchive[0]).toBe(7);
    expect(stagedArchive.at(-1)).toBe(7);
    expect(fs.files.get("/game/Data/Scripts/Scripts.ini")).toEqual(scriptBytes);
    expect(assets.reads).toEqual([
      { target: "/game/INIZH.big", offset: 0, length: 4 * 1024 * 1024 },
      { target: "/game/INIZH.big", offset: 4 * 1024 * 1024, length: 1024 * 1024 },
    ]);
    expect(progress[0]).toEqual([0, archiveBytes.length + scriptBytes.length]);
    expect(progress.at(-1)).toEqual([archiveBytes.length + scriptBytes.length, archiveBytes.length + scriptBytes.length]);
    expect(assets.cleared).toBe(true);
  });

  it("rejects a truncated range read", async () => {
    const archive = entry("big-base", "/game-base/Test.big", 10, true);
    const assets = new FakeAssets([archive], new Map([[archive.target, new Uint8Array(4)]]));
    await expect(
      stageEngineAssets(assets, fakeRuntime(), SETTINGS, { width: 1, height: 1 } as HTMLCanvasElement),
    ).rejects.toThrow(EngineBootError);
  });
});

describe("launchEmscriptenEngine", () => {
  it("publishes Module, waits for transport, mounts assets, and resolves at runtime initialization", async () => {
    const entries = [
      entry("engine-js", "/engine/GeneralsXZH.js", 2),
      entry("engine-wasm", "/engine/GeneralsXZH.wasm", 2),
      entry("font", "/fonts/default.ttf", 2),
    ];
    const assets = new FakeAssets(
      entries,
      new Map([
        [entries[0]!.target, new Uint8Array([1, 2])],
        [entries[1]!.target, new Uint8Array([3, 4])],
        [entries[2]!.target, new Uint8Array([5, 6])],
      ]),
    );
    const runtime = fakeRuntime();
    let module: EmscriptenModuleConfig | undefined;
    const events: string[] = [];
    let allowTransport!: () => void;
    const udpReady = new Promise<void>((resolve) => {
      allowTransport = resolve;
    });
    const host: EngineScriptHost = {
      publishModule(value) {
        module = value;
        events.push("publish");
      },
      runtimeGlobals: () => runtime,
      createObjectUrl: (_bytes, mime) => `blob:${mime}`,
      revokeObjectUrl: (url) => events.push(`revoke:${url}`),
      async loadClassicScript() {
        events.push("script");
        module!.preRun[0]!();
        await vi.waitFor(() => {
          expect(runtime.removeRunDependency).toHaveBeenCalledOnce();
        });
        module!.onRuntimeInitialized();
      },
    };

    const launch = launchEmscriptenEngine({
      assets,
      canvas: { width: 1024, height: 768 } as HTMLCanvasElement,
      settings: SETTINGS,
      udpReady,
      host,
    });
    await vi.waitFor(() => {
      expect(events).toEqual(["publish"]);
    });
    expect(events).toEqual(["publish"]);
    allowTransport();

    const launched = await launch;
    expect(launched).toBe(module);
    expect(events[1]).toBe("script");
    expect(events.filter((event) => event.startsWith("revoke:"))).toHaveLength(2);
    expect(module!.locateFile("GeneralsXZH.wasm")).toBe("blob:application/wasm");
  });

  it("rejects manifests without exactly one engine script and wasm module", async () => {
    const assets = new FakeAssets([], new Map());
    await expect(
      launchEmscriptenEngine({
        assets,
        canvas: {} as HTMLCanvasElement,
        settings: SETTINGS,
        host: {} as EngineScriptHost,
      }),
    ).rejects.toThrow(/engine-js/);
  });
});
