import { describe, expect, it } from "vitest";
import { sha256Hex } from "@generalsx-web/shared/sha256";
import type { EngineManifest, ManifestAsset } from "@generalsx-web/shared/manifest";
import { AssetVfs, vfsEntriesFor, type VfsEntry } from "../../src/assets/vfs.js";
import { AssetNotMountedError } from "../../src/assets/errors.js";
import { MemoryAssetStorage, type AssetFileStore } from "../../src/assets/storage.js";

const ARCHIVE = new Uint8Array(4096);
for (let i = 0; i < ARCHIVE.length; i += 1) ARCHIVE[i] = i & 0xff;
const SCRIPT = new TextEncoder().encode("Weapon = TankGun\n");

async function seed(files: Record<string, Uint8Array>): Promise<AssetFileStore> {
  const store = await new MemoryAssetStorage().openStore("test-build-r1");
  for (const [name, bytes] of Object.entries(files)) {
    const writer = await store.openTemp(name, { append: false });
    await writer.write(bytes);
    await writer.close();
    await store.promote(name);
  }
  return store;
}

function entry(overrides: Partial<VfsEntry> & Pick<VfsEntry, "target" | "sizeBytes" | "storageName">): VfsEntry {
  return {
    path: "base/INI.big",
    role: "big-base",
    sha256: overrides.storageName,
    order: 0,
    streaming: true,
    ...overrides,
  };
}

const ARCHIVE_NAME = sha256Hex(ARCHIVE);
const SCRIPT_NAME = sha256Hex(SCRIPT);

async function archiveVfs(options?: ConstructorParameters<typeof AssetVfs>[2]): Promise<AssetVfs> {
  const store = await seed({ [ARCHIVE_NAME]: ARCHIVE, [SCRIPT_NAME]: SCRIPT });
  return new AssetVfs(
    store,
    [
      entry({ target: "/generalsx/base/INI.big", sizeBytes: ARCHIVE.length, storageName: ARCHIVE_NAME }),
      entry({
        target: "/generalsx/scripts/Weapon.ini",
        sizeBytes: SCRIPT.length,
        storageName: SCRIPT_NAME,
        path: "scripts/Weapon.ini",
        role: "script",
        streaming: false,
        order: 1,
      }),
    ],
    options,
  );
}

describe("AssetVfs.read", () => {
  it("reads a byte range without materialising the whole archive", async () => {
    const vfs = await archiveVfs({ chunkBytes: 256, cacheBudgetBytes: 512 });
    const slice = await vfs.read("/generalsx/base/INI.big", 1000, 16);
    expect([...slice]).toEqual([...ARCHIVE.subarray(1000, 1016)]);
  });

  it("stitches a read that spans several chunks", async () => {
    const vfs = await archiveVfs({ chunkBytes: 64 });
    const slice = await vfs.read("/generalsx/base/INI.big", 30, 200);
    expect([...slice]).toEqual([...ARCHIVE.subarray(30, 230)]);
  });

  it("clamps reads at end of file", async () => {
    const vfs = await archiveVfs({ chunkBytes: 128 });
    expect((await vfs.read("/generalsx/base/INI.big", 4090, 100)).length).toBe(6);
    expect((await vfs.read("/generalsx/base/INI.big", 99_999, 10)).length).toBe(0);
    expect((await vfs.read("/generalsx/base/INI.big", 0, 0)).length).toBe(0);
  });

  it("rejects negative or fractional ranges", async () => {
    const vfs = await archiveVfs();
    await expect(vfs.read("/generalsx/base/INI.big", -1, 4)).rejects.toThrow(RangeError);
    await expect(vfs.read("/generalsx/base/INI.big", 0, 1.5)).rejects.toThrow(RangeError);
  });

  it("reports unmounted targets", async () => {
    const vfs = await archiveVfs();
    await expect(vfs.read("/generalsx/missing.big", 0, 1)).rejects.toThrow(AssetNotMountedError);
    expect(vfs.has("/generalsx/missing.big")).toBe(false);
    expect(vfs.stat("/generalsx/base/INI.big")?.role).toBe("big-base");
  });
});

describe("AssetVfs chunk cache", () => {
  it("keeps resident bytes under the configured budget", async () => {
    const vfs = await archiveVfs({ chunkBytes: 256, cacheBudgetBytes: 512 });

    for (let offset = 0; offset < ARCHIVE.length; offset += 256) {
      await vfs.read("/generalsx/base/INI.big", offset, 256);
      expect(vfs.cachedBytes).toBeLessThanOrEqual(512);
    }
    expect(vfs.cachedBytes).toBeLessThanOrEqual(512);
  });

  it("serves repeat reads from the cache instead of the store", async () => {
    const store = await seed({ [ARCHIVE_NAME]: ARCHIVE });
    let reads = 0;
    const counting: AssetFileStore = {
      ...store,
      readFinal(name, offset, length) {
        reads += 1;
        return store.readFinal(name, offset, length);
      },
    };
    const vfs = new AssetVfs(
      counting,
      [entry({ target: "/a.big", sizeBytes: ARCHIVE.length, storageName: ARCHIVE_NAME })],
      { chunkBytes: 1024, cacheBudgetBytes: 4096 },
    );

    await vfs.read("/a.big", 0, 16);
    await vfs.read("/a.big", 16, 16);
    expect(reads).toBe(1);

    vfs.clearCache();
    expect(vfs.cachedBytes).toBe(0);
    await vfs.read("/a.big", 0, 16);
    expect(reads).toBe(2);
  });

  it("never caches a chunk larger than the whole budget", async () => {
    const vfs = await archiveVfs({ chunkBytes: 1024, cacheBudgetBytes: 512 });
    await vfs.read("/generalsx/base/INI.big", 0, 1024);
    expect(vfs.cachedBytes).toBe(0);
  });
});

describe("AssetVfs.readAll", () => {
  it("returns the exact bytes of a loose script", async () => {
    const vfs = await archiveVfs();
    expect([...(await vfs.readAll("/generalsx/scripts/Weapon.ini"))]).toEqual([...SCRIPT]);
  });

  it("refuses to buffer a streaming archive", async () => {
    const vfs = await archiveVfs();
    await expect(vfs.readAll("/generalsx/base/INI.big")).rejects.toThrow(/streaming asset/);
  });

  it("refuses files above the whole-file limit", async () => {
    const vfs = await archiveVfs({ maxWholeFileBytes: 4 });
    await expect(vfs.readAll("/generalsx/scripts/Weapon.ini")).rejects.toThrow(/whole-file limit/);
  });
});

describe("vfsEntriesFor", () => {
  it("orders mounts so expansion archives follow base archives", () => {
    const asset = (
      path: string,
      role: ManifestAsset["role"],
      order: number,
      streaming: boolean,
    ): ManifestAsset => ({
      path,
      role,
      sizeBytes: 16,
      sha256: sha256Hex(new TextEncoder().encode(path)),
      mount: { target: `/generalsx/${path}`, order, streaming },
    });

    const manifest = {
      schemaVersion: 2,
      engineVersion: "test-build",
      assetsRevision: 1,
      assetBaseUrl: "https://assets.generalsx.org",
      assets: [
        asset("expansion/INIZH.big", "big-expansion", 200, true),
        asset("engine/engine.js", "engine-js", 0, false),
        asset("base/INI.big", "big-base", 100, true),
      ],
    } as EngineManifest;

    const entries = vfsEntriesFor(manifest);
    expect(entries.map((item) => item.role)).toEqual(["engine-js", "big-base", "big-expansion"]);
    expect(entries.map((item) => item.streaming)).toEqual([false, true, true]);
    expect(entries[0]?.storageName).toBe(entries[0]?.sha256);
  });
});
