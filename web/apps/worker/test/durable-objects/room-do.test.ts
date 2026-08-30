/// <reference types="@cloudflare/workers-types" />
import { describe, expect, it } from "vitest";
import { RoomDurableObject } from "../../src/durable-objects/room-do.js";
import {
  ADMISSION_TOKEN_TTL_SECONDS,
  createRoomKeyBytes,
  importRoomKey,
  signAdmissionToken,
} from "../../src/room/admission.js";
import { ROOM_KEY_STORAGE_KEY } from "../../src/room/admission-key-store.js";
import { TURN_GRANT_URL, type TurnGrantResult } from "../../src/room/turn-grant.js";

/**
 * The slice of `DurableObjectState` the room object touches, with every write
 * recorded.
 *
 * A Durable Object id can be derived from *any* string, so `idFromName()` on
 * an attacker-supplied — or forged — room id will instantiate an object. What
 * these tests pin down is that doing so costs nothing durable.
 */
function fakeState() {
  const entries = new Map<string, unknown>();
  const writes: string[] = [];
  const pending: Promise<unknown>[] = [];
  const storage = {
    get: <T,>(key: string): Promise<T | undefined> => Promise.resolve(entries.get(key) as T | undefined),
    put: <T,>(key: string, value: T): Promise<void> => {
      writes.push(key);
      entries.set(key, value);
      return Promise.resolve();
    },
    deleteAll: (): Promise<void> => {
      entries.clear();
      return Promise.resolve();
    },
  };
  const state = {
    storage,
    blockConcurrencyWhile: <T,>(fn: () => Promise<T>): Promise<T> => {
      const promise = fn();
      pending.push(promise);
      return promise;
    },
  } as unknown as DurableObjectState;
  return { state, entries, writes, settle: () => Promise.all(pending) };
}

async function makeRoom() {
  const fake = fakeState();
  const room = new RoomDurableObject(fake.state, {});
  await fake.settle();
  return { room, ...fake };
}

async function requestGrant(room: RoomDurableObject, token: string): Promise<TurnGrantResult> {
  const response = await room.fetch(
    new Request(TURN_GRANT_URL, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ token }),
    }),
  );
  expect(response.status).toBe(200);
  return (await response.json()) as TurnGrantResult;
}

async function throwawayToken(roomId = "ABCD"): Promise<string> {
  const key = await importRoomKey(createRoomKeyBytes());
  return signAdmissionToken(key, {
    roomId,
    slot: 0,
    admissionId: "seat-nonce",
    exp: Math.floor(Date.now() / 1000) + ADMISSION_TOKEN_TTL_SECONDS,
  });
}

describe("RoomDurableObject: admission key lifecycle", () => {
  it("writes nothing when constructed for an unknown room id", async () => {
    const { writes, entries } = await makeRoom();
    expect(writes).toEqual([]);
    expect(entries.size).toBe(0);
  });

  it("does not create a key while answering a turn-grant request", async () => {
    // An unauthenticated caller can reach this path with any room id they
    // like. If it created a key, that would be an unauthenticated durable
    // write amplification.
    const { room, writes } = await makeRoom();
    const result = await requestGrant(room, await throwawayToken());
    expect(result.ok).toBe(false);
    expect(writes).toEqual([]);
  });

  it("refuses a grant for a room with no active members", async () => {
    const { room } = await makeRoom();
    const result = await requestGrant(room, await throwawayToken());
    expect(result).toMatchObject({ ok: false, code: "UNAUTHORIZED" });
    if (!result.ok) expect(result.detail).toContain("no active members");
  });

  it("refuses a malformed grant body without writing", async () => {
    const { room, writes } = await makeRoom();
    const response = await room.fetch(new Request(TURN_GRANT_URL, { method: "POST", body: "not json" }));
    expect(response.status).toBe(200);
    expect(((await response.json()) as TurnGrantResult).ok).toBe(false);
    expect(writes).toEqual([]);
  });

  it("refuses a grant request that is not a POST", async () => {
    const { room } = await makeRoom();
    const response = await room.fetch(new Request(TURN_GRANT_URL, { method: "GET" }));
    const result = (await response.json()) as TurnGrantResult;
    expect(result.ok).toBe(false);
  });

  it("refuses a grant with no token at all", async () => {
    const { room } = await makeRoom();
    const result = await requestGrant(room, "");
    expect(result.ok).toBe(false);
  });

  it("loads an existing key on construction without rewriting it", async () => {
    const fake = fakeState();
    fake.entries.set(ROOM_KEY_STORAGE_KEY, createRoomKeyBytes());
    const room = new RoomDurableObject(fake.state, {});
    await fake.settle();
    expect(fake.writes).toEqual([]);
    // The key exists but the room has no members, so a grant is still refused.
    expect((await requestGrant(room, await throwawayToken())).ok).toBe(false);
  });

  it("rejects a WebSocket-less upgrade attempt", async () => {
    const { room, writes } = await makeRoom();
    const response = await room.fetch(new Request("https://signaling.generalsx.org/room?roomId=ABCD"));
    expect(response.status).toBe(426);
    expect(writes).toEqual([]);
  });
});
