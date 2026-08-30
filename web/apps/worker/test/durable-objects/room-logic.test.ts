import { beforeEach, describe, expect, it } from "vitest";
import {
  buildRoster,
  createRoomState,
  findFreeSlot,
  isRoomEmpty,
  joinRoom,
  leaveRoom,
  normalizeCapacity,
  resolveSlot,
  type RoomState,
} from "../../src/durable-objects/room-logic.js";

describe("normalizeCapacity", () => {
  it("defaults to 4 when unset", () => {
    expect(normalizeCapacity(undefined)).toBe(4);
  });

  it("clamps to the supported range", () => {
    expect(normalizeCapacity(1)).toBe(2);
    expect(normalizeCapacity(99)).toBe(8);
    expect(normalizeCapacity(4)).toBe(4);
  });

  it("falls back to the default for non-integers", () => {
    expect(normalizeCapacity(3.5)).toBe(4);
  });
});

describe("room lifecycle", () => {
  let room: RoomState;

  beforeEach(() => {
    room = createRoomState("ABCD", 2);
  });

  it("assigns stable, lowest-first slots", () => {
    const first = joinRoom(room, { name: "Alice", connectionId: "c1" });
    const second = joinRoom(room, { name: "Bob", connectionId: "c2" });
    expect(first).toEqual({ ok: true, value: 0 });
    expect(second).toEqual({ ok: true, value: 1 });
  });

  it("marks only the first joiner as host", () => {
    joinRoom(room, { name: "Alice", connectionId: "c1" });
    joinRoom(room, { name: "Bob", connectionId: "c2" });
    const roster = buildRoster(room);
    expect(roster.find((r) => r.slot === 0)?.isHost).toBe(true);
    expect(roster.find((r) => r.slot === 1)?.isHost).toBe(false);
  });

  it("rejects joins once capacity is reached", () => {
    joinRoom(room, { name: "Alice", connectionId: "c1" });
    joinRoom(room, { name: "Bob", connectionId: "c2" });
    const third = joinRoom(room, { name: "Carol", connectionId: "c3" });
    expect(third).toEqual({ ok: false, error: "ROOM_FULL" });
  });

  it("frees a slot on leave and reassigns it to the next joiner", () => {
    joinRoom(room, { name: "Alice", connectionId: "c1" });
    joinRoom(room, { name: "Bob", connectionId: "c2" });
    leaveRoom(room, 0);
    expect(findFreeSlot(room)).toBe(0);
    const rejoin = joinRoom(room, { name: "Carol", connectionId: "c3" });
    expect(rejoin).toEqual({ ok: true, value: 0 });
  });

  it("keeps the existing host when a non-host occupant leaves", () => {
    joinRoom(room, { name: "Alice", connectionId: "c1" }); // host, slot 0
    joinRoom(room, { name: "Bob", connectionId: "c2" }); // slot 1
    leaveRoom(room, 1);
    expect(resolveSlot(room, 0)?.isHost).toBe(true);
  });

  it("promotes the next joiner to host once the room has been fully vacated", () => {
    joinRoom(room, { name: "Alice", connectionId: "c1" });
    leaveRoom(room, 0);
    joinRoom(room, { name: "Bob", connectionId: "c2" });
    // Host status is derived from "first occupant of an empty room", so once
    // everyone leaves, the next joiner becomes host. Promoting an existing
    // non-host occupant to host on a departure is a higher-layer policy.
    expect(resolveSlot(room, 0)?.isHost).toBe(true);
  });

  it("leaveRoom is idempotent for an already-empty slot", () => {
    expect(() => leaveRoom(room, 0)).not.toThrow();
    expect(isRoomEmpty(room)).toBe(true);
  });

  it("buildRoster returns entries sorted by slot", () => {
    joinRoom(room, { name: "Bob", connectionId: "c2" });
    const roster = buildRoster(room);
    expect(roster.map((r) => r.slot)).toEqual([0]);
  });

  it("resolveSlot returns undefined for an empty or out-of-range slot", () => {
    expect(resolveSlot(room, 0)).toBeUndefined();
    expect(resolveSlot(room, 99)).toBeUndefined();
  });

  it("isRoomEmpty reflects occupancy", () => {
    expect(isRoomEmpty(room)).toBe(true);
    joinRoom(room, { name: "Alice", connectionId: "c1" });
    expect(isRoomEmpty(room)).toBe(false);
  });
});
