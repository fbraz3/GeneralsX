import { describe, expect, it } from "vitest";
import { signAdmissionToken, verifyAdmissionToken } from "../../src/room/admission.js";
import {
  destroyRoomAdmissionKey,
  ensureRoomAdmissionKey,
  loadRoomAdmissionKey,
  ROOM_KEY_STORAGE_KEY,
  type AdmissionKeyStorage,
} from "../../src/room/admission-key-store.js";

/** Records every write, so a test can assert that a code path performed no
 * storage side effect at all — the property that matters for an arbitrary,
 * attacker-supplied Durable Object id. */
function fakeStorage() {
  const entries = new Map<string, unknown>();
  const writes: string[] = [];
  const deletes: number[] = [];
  const storage: AdmissionKeyStorage = {
    get<T>(key: string): Promise<T | undefined> {
      return Promise.resolve(entries.get(key) as T | undefined);
    },
    put<T>(key: string, value: T): Promise<void> {
      writes.push(key);
      entries.set(key, value);
      return Promise.resolve();
    },
    deleteAll(): Promise<void> {
      deletes.push(entries.size);
      entries.clear();
      return Promise.resolve();
    },
  };
  return { storage, entries, writes, deletes };
}

describe("loadRoomAdmissionKey", () => {
  it("returns null for a room that has never admitted anyone", async () => {
    const { storage } = fakeStorage();
    expect(await loadRoomAdmissionKey(storage)).toBeNull();
  });

  it("writes nothing when there is no key", async () => {
    // Constructing a Durable Object for an id peeked out of a forged token
    // must not leave a durable record behind.
    const { storage, writes, entries } = fakeStorage();
    await loadRoomAdmissionKey(storage);
    expect(writes).toEqual([]);
    expect(entries.size).toBe(0);
  });

  it("imports a previously stored key", async () => {
    const { storage } = fakeStorage();
    const created = await ensureRoomAdmissionKey(storage);
    const loaded = await loadRoomAdmissionKey(storage);
    expect(loaded).not.toBeNull();

    const claims = { roomId: "ABCD", slot: 0, admissionId: "seat", exp: Math.floor(Date.now() / 1000) + 60 };
    const token = await signAdmissionToken(created, claims);
    const verified = await verifyAdmissionToken(token, loaded as CryptoKey, { expectedRoomId: "ABCD" });
    expect(verified.ok).toBe(true);
  });

  it("accepts a key stored as an ArrayBuffer", async () => {
    const { storage, entries } = fakeStorage();
    const key = await ensureRoomAdmissionKey(storage);
    const bytes = entries.get(ROOM_KEY_STORAGE_KEY) as Uint8Array;
    entries.set(ROOM_KEY_STORAGE_KEY, bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength));

    const loaded = await loadRoomAdmissionKey(storage);
    const claims = { roomId: "ABCD", slot: 1, admissionId: "seat", exp: Math.floor(Date.now() / 1000) + 60 };
    const token = await signAdmissionToken(key, claims);
    expect((await verifyAdmissionToken(token, loaded as CryptoKey, { expectedRoomId: "ABCD" })).ok).toBe(true);
  });
});

describe("ensureRoomAdmissionKey", () => {
  it("creates and persists a key on first use", async () => {
    const { storage, writes } = fakeStorage();
    await ensureRoomAdmissionKey(storage);
    expect(writes).toEqual([ROOM_KEY_STORAGE_KEY]);
  });

  it("does not rewrite an existing key", async () => {
    const { storage, writes } = fakeStorage();
    await ensureRoomAdmissionKey(storage);
    await ensureRoomAdmissionKey(storage);
    expect(writes).toHaveLength(1);
  });

  it("keeps the same key across calls, so earlier tokens stay valid", async () => {
    const { storage } = fakeStorage();
    const first = await ensureRoomAdmissionKey(storage);
    const claims = { roomId: "ABCD", slot: 0, admissionId: "seat", exp: Math.floor(Date.now() / 1000) + 60 };
    const token = await signAdmissionToken(first, claims);

    const second = await ensureRoomAdmissionKey(storage);
    expect((await verifyAdmissionToken(token, second, { expectedRoomId: "ABCD" })).ok).toBe(true);
  });

  it("gives different rooms different keys", async () => {
    const roomA = fakeStorage();
    const roomB = fakeStorage();
    const keyA = await ensureRoomAdmissionKey(roomA.storage);
    const keyB = await ensureRoomAdmissionKey(roomB.storage);

    const claims = { roomId: "ABCD", slot: 0, admissionId: "seat", exp: Math.floor(Date.now() / 1000) + 60 };
    const token = await signAdmissionToken(keyA, claims);
    expect((await verifyAdmissionToken(token, keyB, { expectedRoomId: "ABCD" })).ok).toBe(false);
  });
});

describe("destroyRoomAdmissionKey", () => {
  it("clears the room's stored state", async () => {
    const { storage, entries } = fakeStorage();
    await ensureRoomAdmissionKey(storage);
    expect(entries.size).toBe(1);

    await destroyRoomAdmissionKey(storage);
    expect(entries.size).toBe(0);
    expect(await loadRoomAdmissionKey(storage)).toBeNull();
  });

  it("makes tokens minted under the old key unverifiable after a rejoin", async () => {
    const { storage } = fakeStorage();
    const old = await ensureRoomAdmissionKey(storage);
    const claims = { roomId: "ABCD", slot: 0, admissionId: "seat", exp: Math.floor(Date.now() / 1000) + 60 };
    const token = await signAdmissionToken(old, claims);

    await destroyRoomAdmissionKey(storage);
    const fresh = await ensureRoomAdmissionKey(storage);
    expect((await verifyAdmissionToken(token, fresh, { expectedRoomId: "ABCD" })).ok).toBe(false);
  });

  it("is safe to call for a room that never had a key", async () => {
    const { storage } = fakeStorage();
    await expect(destroyRoomAdmissionKey(storage)).resolves.toBeUndefined();
  });
});
