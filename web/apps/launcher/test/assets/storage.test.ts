import { describe, expect, it, vi } from "vitest";
import {
  MemoryAssetStorage,
  OpfsAssetStorage,
  openAssetStorage,
  type AssetFileStore,
  type AssetStorage,
} from "../../src/assets/storage.js";
import { FakeDirectoryHandle } from "./fake-opfs.js";

const encoder = new TextEncoder();

function bytes(text: string): Uint8Array {
  return encoder.encode(text);
}

async function writeTemp(store: AssetFileStore, name: string, chunks: readonly string[]): Promise<void> {
  const writer = await store.openTemp(name, { append: false });
  for (const chunk of chunks) await writer.write(bytes(chunk));
  await writer.close();
}

function backends(): readonly { name: string; make: () => AssetStorage }[] {
  return [
    { name: "opfs", make: () => new OpfsAssetStorage(FakeDirectoryHandle.create()) },
    { name: "opfs (no move support)", make: () => new OpfsAssetStorage(FakeDirectoryHandle.create({ supportsMove: false })) },
    { name: "memory", make: () => new MemoryAssetStorage() },
  ];
}

describe.each(backends())("$name asset store", ({ make }) => {
  it("writes, stats, and range-reads a temp file", async () => {
    const store = await make().openStore("build-r1");
    await writeTemp(store, "abc", ["hello ", "world"]);

    expect(await store.statTemp("abc")).toEqual({ sizeBytes: 11 });
    expect(new TextDecoder().decode(await store.readTemp("abc", 6, 5))).toBe("world");
    expect(await store.statFinal("abc")).toBeUndefined();
  });

  it("promotes a temp file to its final, content-addressed name", async () => {
    const store = await make().openStore("build-r1");
    await writeTemp(store, "abc", ["payload"]);
    await store.promote("abc");

    expect(await store.statFinal("abc")).toEqual({ sizeBytes: 7 });
    expect(await store.statTemp("abc")).toBeUndefined();
    expect(new TextDecoder().decode(await store.readFinal("abc", 0, 7))).toBe("payload");
  });

  it("keeps the final namespace invisible until promotion", async () => {
    const store = await make().openStore("build-r1");
    const writer = await store.openTemp("abc", { append: false });
    await writer.write(bytes("half"));
    expect(await store.statFinal("abc")).toBeUndefined();
    await writer.close();
    expect(await store.statFinal("abc")).toBeUndefined();
  });

  it("appends to an existing temp file when resuming", async () => {
    const store = await make().openStore("build-r1");
    await writeTemp(store, "abc", ["12345"]);

    const writer = await store.openTemp("abc", { append: true });
    await writer.write(bytes("67890"));
    await writer.close();

    expect(await store.statTemp("abc")).toEqual({ sizeBytes: 10 });
    expect(new TextDecoder().decode(await store.readTemp("abc", 0, 10))).toBe("1234567890");
  });

  it("truncates the temp file when not appending", async () => {
    const store = await make().openStore("build-r1");
    await writeTemp(store, "abc", ["1234567890"]);
    await writeTemp(store, "abc", ["xy"]);
    expect(await store.statTemp("abc")).toEqual({ sizeBytes: 2 });
  });

  it("clamps range reads at end of file", async () => {
    const store = await make().openStore("build-r1");
    await writeTemp(store, "abc", ["1234"]);
    await store.promote("abc");
    expect((await store.readFinal("abc", 2, 100)).length).toBe(2);
    expect((await store.readFinal("abc", 9, 4)).length).toBe(0);
  });

  it("round-trips and deletes sidecar metadata", async () => {
    const store = await make().openStore("build-r1");
    expect(await store.readMeta("abc.json")).toBeUndefined();
    await store.writeMeta("abc.json", '{"version":1}');
    expect(await store.readMeta("abc.json")).toBe('{"version":1}');
    await store.writeMeta("abc.json", '{"version":1,"bytesWritten":9}');
    expect(await store.readMeta("abc.json")).toBe('{"version":1,"bytesWritten":9}');
    await store.deleteMeta("abc.json");
    expect(await store.readMeta("abc.json")).toBeUndefined();
  });

  it("lists and deletes entries per namespace", async () => {
    const store = await make().openStore("build-r1");
    await writeTemp(store, "aaa", ["one"]);
    await writeTemp(store, "bbb", ["two"]);
    await store.promote("aaa");

    expect([...(await store.listFinal())].sort()).toEqual(["aaa"]);
    expect([...(await store.listTemp())].sort()).toEqual(["bbb"]);

    await store.deleteFinal("aaa");
    await store.deleteTemp("bbb");
    expect(await store.listFinal()).toEqual([]);
    expect(await store.listTemp()).toEqual([]);
  });

  it("tolerates deleting entries that do not exist", async () => {
    const store = await make().openStore("build-r1");
    await expect(store.deleteFinal("missing")).resolves.toBeUndefined();
    await expect(store.deleteTemp("missing")).resolves.toBeUndefined();
    await expect(store.deleteMeta("missing.json")).resolves.toBeUndefined();
  });

  it("rejects unsafe entry names", async () => {
    const store = await make().openStore("build-r1");
    for (const name of ["../escape", "a/b", "", "..", "a".repeat(200)]) {
      await expect(store.openTemp(name, { append: false })).rejects.toThrow(/unsafe storage entry name/);
    }
  });

  it("isolates roots and deletes superseded revisions", async () => {
    const storage = make();
    const first = await storage.openStore("build-r1");
    await writeTemp(first, "abc", ["old"]);
    await first.promote("abc");

    const second = await storage.openStore("build-r2");
    expect(await second.statFinal("abc")).toBeUndefined();

    expect([...(await storage.listRoots())].sort()).toEqual(["build-r1", "build-r2"]);
    await storage.deleteRoot("build-r1");
    expect(await storage.listRoots()).toEqual(["build-r2"]);
    await expect(storage.deleteRoot("build-r1")).resolves.toBeUndefined();
  });
});

describe("quota handling", () => {
  it("raises QuotaExceededError once the OPFS budget is gone", async () => {
    const storage = new OpfsAssetStorage(FakeDirectoryHandle.create({ quotaBytes: 8 }));
    const store = await storage.openStore("build-r1");
    const writer = await store.openTemp("abc", { append: false });

    await writer.write(new Uint8Array(8));
    await expect(writer.write(new Uint8Array(1))).rejects.toMatchObject({ name: "QuotaExceededError" });
  });

  it("raises QuotaExceededError once the memory budget is gone", async () => {
    const storage = new MemoryAssetStorage(8);
    const store = await storage.openStore("build-r1");
    const writer = await store.openTemp("abc", { append: false });

    await writer.write(new Uint8Array(8));
    await writer.close();
    const second = await store.openTemp("abc", { append: true });
    await expect(second.write(new Uint8Array(4))).rejects.toMatchObject({ name: "QuotaExceededError" });
  });

  it("counts released bytes back against the budget", async () => {
    const storage = new MemoryAssetStorage(16);
    const store = await storage.openStore("build-r1");
    await writeTemp(store, "abc", ["0123456789abcdef"]);
    await store.deleteTemp("abc");
    await expect(writeTemp(store, "def", ["0123456789abcdef"])).resolves.toBeUndefined();
  });
});

describe("OPFS layout", () => {
  it("separates final, temp, and meta namespaces on disk", async () => {
    const root = FakeDirectoryHandle.create();
    const store = await new OpfsAssetStorage(root).openStore("build-r1");

    await writeTemp(store, "abc", ["payload"]);
    await store.writeMeta("abc.json", "{}");
    expect(root.listDir("build-r1/tmp")).toEqual(["abc"]);
    expect(root.listDir("build-r1/meta")).toEqual(["abc.json"]);

    await store.promote("abc");
    expect(root.listDir("build-r1/files")).toEqual(["abc"]);
    expect(root.listDir("build-r1/tmp")).toEqual([]);
    expect(new TextDecoder().decode(root.peek("build-r1/files/abc"))).toBe("payload");
  });

  it("copies instead of renaming when move() is unavailable", async () => {
    const root = FakeDirectoryHandle.create({ supportsMove: false });
    const store = await new OpfsAssetStorage(root).openStore("build-r1");

    await writeTemp(store, "abc", ["a".repeat(1000)]);
    await store.promote("abc");

    expect(root.peek("build-r1/files/abc")?.length).toBe(1000);
    expect(root.listDir("build-r1/tmp")).toEqual([]);
  });

  it("reports a missing temp file when promoting nothing", async () => {
    const storage = new OpfsAssetStorage(FakeDirectoryHandle.create());
    const store = await storage.openStore("build-r1");
    await expect(store.promote("nope")).rejects.toMatchObject({ name: "NotFoundError" });
  });
});

describe("openAssetStorage", () => {
  it("falls back to memory when OPFS is unavailable", async () => {
    const storage = await openAssetStorage();
    expect(storage.kind).toBe("memory");
  });

  it("honours the forced-memory switch", async () => {
    const storage = await openAssetStorage({ forceMemory: true, memoryBudgetBytes: 32 });
    expect(storage.kind).toBe("memory");
    const store = await storage.openStore("build-r1");
    const writer = await store.openTemp("abc", { append: false });
    await expect(writer.write(new Uint8Array(64))).rejects.toMatchObject({ name: "QuotaExceededError" });
  });

  it("uses OPFS when navigator.storage.getDirectory works", async () => {
    const root = FakeDirectoryHandle.create();
    const fakeNavigator = { storage: { getDirectory: async () => root } };
    vi.stubGlobal("navigator", fakeNavigator);
    try {
      const storage = await openAssetStorage();
      expect(storage.kind).toBe("opfs");
    } finally {
      vi.unstubAllGlobals();
    }
  });

  it("falls back to memory when OPFS access is denied", async () => {
    const fakeNavigator = {
      storage: {
        getDirectory: async () => {
          throw new DOMException("denied", "SecurityError");
        },
      },
    };
    vi.stubGlobal("navigator", fakeNavigator);
    try {
      const storage = await openAssetStorage();
      expect(storage.kind).toBe("memory");
    } finally {
      vi.unstubAllGlobals();
    }
  });
});
