import { beforeEach, describe, expect, it } from "vitest";
import { CURRENT_COMPATIBILITY } from "@generalsx-web/shared/protocol";
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

function request(name: string, connectionId: string, compatibility = CURRENT_COMPATIBILITY) {
  return { name, connectionId, compatibility };
}

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
    const first = joinRoom(room, request("Alice", "c1"));
    const second = joinRoom(room, request("Bob", "c2"));
    expect(first).toEqual({ ok: true, value: 0 });
    expect(second).toEqual({ ok: true, value: 1 });
  });

  it("marks only the first joiner as host", () => {
    joinRoom(room, request("Alice", "c1"));
    joinRoom(room, request("Bob", "c2"));
    const roster = buildRoster(room);
    expect(roster.find((r) => r.slot === 0)?.isHost).toBe(true);
    expect(roster.find((r) => r.slot === 1)?.isHost).toBe(false);
  });

  it("rejects joins once capacity is reached", () => {
    joinRoom(room, request("Alice", "c1"));
    joinRoom(room, request("Bob", "c2"));
    const third = joinRoom(room, request("Carol", "c3"));
    expect(third).toEqual({ ok: false, error: "ROOM_FULL" });
  });

  it("frees a slot on leave and reassigns it to the next joiner", () => {
    joinRoom(room, request("Alice", "c1"));
    joinRoom(room, request("Bob", "c2"));
    leaveRoom(room, 0);
    expect(findFreeSlot(room)).toBe(0);
    const rejoin = joinRoom(room, request("Carol", "c3"));
    expect(rejoin).toEqual({ ok: true, value: 0 });
  });

  it("keeps the existing host when a non-host occupant leaves", () => {
    joinRoom(room, request("Alice", "c1")); // host, slot 0
    joinRoom(room, request("Bob", "c2")); // slot 1
    leaveRoom(room, 1);
    expect(resolveSlot(room, 0)?.isHost).toBe(true);
  });

  it("promotes the next joiner to host once the room has been fully vacated", () => {
    joinRoom(room, request("Alice", "c1"));
    leaveRoom(room, 0);
    joinRoom(room, request("Bob", "c2"));
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
    joinRoom(room, request("Bob", "c2"));
    const roster = buildRoster(room);
    expect(roster.map((r) => r.slot)).toEqual([0]);
  });

  it("resolveSlot returns undefined for an empty or out-of-range slot", () => {
    expect(resolveSlot(room, 0)).toBeUndefined();
    expect(resolveSlot(room, 99)).toBeUndefined();
  });

  it("isRoomEmpty reflects occupancy", () => {
    expect(isRoomEmpty(room)).toBe(true);
    joinRoom(room, request("Alice", "c1"));
    expect(isRoomEmpty(room)).toBe(false);
  });

  it("rejects mismatched engine, protocol, or determinism versions before assigning a slot", () => {
    expect(joinRoom(room, request("Alice", "c1"))).toEqual({ ok: true, value: 0 });
    for (const compatibility of [
      { ...CURRENT_COMPATIBILITY, engine: 2 },
      { ...CURRENT_COMPATIBILITY, protocol: 2 },
      { ...CURRENT_COMPATIBILITY, determinism: 2 },
    ]) {
      expect(joinRoom(room, request("Bob", `c-${compatibility.engine}${compatibility.protocol}${compatibility.determinism}`, compatibility))).toEqual({
        ok: false,
        error: "INCOMPATIBLE_CLIENT",
      });
    }
    expect(room.slots.size).toBe(1);
  });

  it("accepts a new compatibility profile after the room becomes empty", () => {
    joinRoom(room, request("Alice", "c1"));
    leaveRoom(room, 0);
    expect(joinRoom(room, request("Bob", "c2", { engine: 2, protocol: 3, determinism: 4 }))).toEqual({
      ok: true,
      value: 0,
    });
  });
});
