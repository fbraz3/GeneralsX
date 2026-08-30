import { beforeEach, describe, expect, it } from "vitest";
import { compatibilityFor } from "@generalsx-web/shared/protocol";
import {
  buildRoster,
  createRoomState,
  findFreeSlot,
  isRoomEmpty,
  isSlotHeldBy,
  joinRoom,
  leaveRoom,
  normalizeCapacity,
  resolveSlot,
  type JoinRequest,
  type RoomState,
} from "../../src/durable-objects/room-logic.js";

const TEST_COMPATIBILITY = compatibilityFor("zero-hour", true);

/** Every join carries both a compatibility profile and a per-seat admission
 * nonce (see `SlotInfo.admissionId`). Default to the matching profile and
 * derive a deterministic nonce from the connection id so these tests stay
 * readable while still exercising the real signature. */
function seat(name: string, connectionId: string, compatibility = TEST_COMPATIBILITY): JoinRequest {
  return { name, connectionId, compatibility, admissionId: `adm-${connectionId}` };
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
    const first = joinRoom(room, seat("Alice", "c1"));
    const second = joinRoom(room, seat("Bob", "c2"));
    expect(first).toEqual({ ok: true, value: 0 });
    expect(second).toEqual({ ok: true, value: 1 });
  });

  it("marks only the first joiner as host", () => {
    joinRoom(room, seat("Alice", "c1"));
    joinRoom(room, seat("Bob", "c2"));
    const roster = buildRoster(room);
    expect(roster.find((r) => r.slot === 0)?.isHost).toBe(true);
    expect(roster.find((r) => r.slot === 1)?.isHost).toBe(false);
  });

  it("rejects joins once capacity is reached", () => {
    joinRoom(room, seat("Alice", "c1"));
    joinRoom(room, seat("Bob", "c2"));
    const third = joinRoom(room, seat("Carol", "c3"));
    expect(third).toEqual({ ok: false, error: "ROOM_FULL" });
  });

  it("frees a slot on leave and reassigns it to the next joiner", () => {
    joinRoom(room, seat("Alice", "c1"));
    joinRoom(room, seat("Bob", "c2"));
    leaveRoom(room, 0);
    expect(findFreeSlot(room)).toBe(0);
    const rejoin = joinRoom(room, seat("Carol", "c3"));
    expect(rejoin).toEqual({ ok: true, value: 0 });
  });

  it("keeps the existing host when a non-host occupant leaves", () => {
    joinRoom(room, seat("Alice", "c1")); // host, slot 0
    joinRoom(room, seat("Bob", "c2")); // slot 1
    leaveRoom(room, 1);
    expect(resolveSlot(room, 0)?.isHost).toBe(true);
  });

  it("promotes the next joiner to host once the room has been fully vacated", () => {
    joinRoom(room, seat("Alice", "c1"));
    leaveRoom(room, 0);
    joinRoom(room, seat("Bob", "c2"));
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
    joinRoom(room, seat("Bob", "c2"));
    const roster = buildRoster(room);
    expect(roster.map((r) => r.slot)).toEqual([0]);
  });

  it("resolveSlot returns undefined for an empty or out-of-range slot", () => {
    expect(resolveSlot(room, 0)).toBeUndefined();
    expect(resolveSlot(room, 99)).toBeUndefined();
  });

  it("isRoomEmpty reflects occupancy", () => {
    expect(isRoomEmpty(room)).toBe(true);
    joinRoom(room, seat("Alice", "c1"));
    expect(isRoomEmpty(room)).toBe(false);
  });

  it("rejects mismatched engine, protocol, or determinism versions before assigning a slot", () => {
    expect(joinRoom(room, seat("Alice", "c1"))).toEqual({ ok: true, value: 0 });
    for (const compatibility of [
      { ...TEST_COMPATIBILITY, engine: TEST_COMPATIBILITY.engine + 100 },
      { ...TEST_COMPATIBILITY, protocol: TEST_COMPATIBILITY.protocol + 100 },
      { ...TEST_COMPATIBILITY, determinism: TEST_COMPATIBILITY.determinism + 100 },
    ]) {
      expect(joinRoom(room, seat("Bob", `c-${compatibility.engine}${compatibility.protocol}${compatibility.determinism}`, compatibility))).toEqual({
        ok: false,
        error: "INCOMPATIBLE_CLIENT",
      });
    }
    expect(room.slots.size).toBe(1);
  });

  it("accepts a new compatibility profile after the room becomes empty", () => {
    joinRoom(room, seat("Alice", "c1"));
    leaveRoom(room, 0);
    expect(joinRoom(room, seat("Bob", "c2", { engine: 2, protocol: 3, determinism: 4 }))).toEqual({
      ok: true,
      value: 0,
    });
  });
});

describe("isSlotHeldBy", () => {
  let room: RoomState;

  beforeEach(() => {
    room = createRoomState("ABCD", 2);
  });

  it("recognizes the admission currently occupying a slot", () => {
    joinRoom(room, seat("Alice", "c1"));
    expect(isSlotHeldBy(room, 0, "adm-c1")).toBe(true);
  });

  it("rejects an empty slot", () => {
    expect(isSlotHeldBy(room, 0, "adm-c1")).toBe(false);
  });

  it("rejects an admission for a slot the holder does not occupy", () => {
    joinRoom(room, seat("Alice", "c1"));
    joinRoom(room, seat("Bob", "c2"));
    expect(isSlotHeldBy(room, 1, "adm-c1")).toBe(false);
  });

  it("revokes an admission as soon as its holder leaves", () => {
    joinRoom(room, seat("Alice", "c1"));
    expect(isSlotHeldBy(room, 0, "adm-c1")).toBe(true);
    leaveRoom(room, 0);
    expect(isSlotHeldBy(room, 0, "adm-c1")).toBe(false);
  });

  it("does not let a departed player's token authorize the seat's new occupant", () => {
    joinRoom(room, seat("Alice", "c1"));
    leaveRoom(room, 0);
    joinRoom(room, seat("Mallory", "c9"));
    expect(isSlotHeldBy(room, 0, "adm-c1")).toBe(false);
    expect(isSlotHeldBy(room, 0, "adm-c9")).toBe(true);
  });
});
