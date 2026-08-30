import { describe, expect, it } from "vitest";
import {
  consumeTurnGrant,
  createTurnGrantLimiter,
  pruneTurnGrantSlots,
  TURN_GRANT_ROOM_BURST,
  TURN_GRANT_ROOM_REFILL_MS,
  TURN_GRANT_SLOT_BURST,
  TURN_GRANT_SLOT_REFILL_MS,
} from "../../src/room/turn-rate-limit.js";

const T0 = 1_700_000_000_000;

describe("consumeTurnGrant: per-seat bucket", () => {
  it("allows a burst and then throttles the same seat", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) {
      expect(consumeTurnGrant(state, 0, T0).allowed).toBe(true);
    }
    const denied = consumeTurnGrant(state, 0, T0);
    expect(denied.allowed).toBe(false);
    expect(denied.scope).toBe("slot");
  });

  it("asks the caller to wait a whole number of seconds, never zero", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) consumeTurnGrant(state, 0, T0);
    const denied = consumeTurnGrant(state, 0, T0);
    expect(denied.retryAfterSeconds).toBeGreaterThanOrEqual(1);
    expect(Number.isInteger(denied.retryAfterSeconds)).toBe(true);
    expect(denied.retryAfterSeconds).toBeLessThanOrEqual(TURN_GRANT_SLOT_REFILL_MS / 1000);
  });

  it("refills one grant per interval", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) consumeTurnGrant(state, 0, T0);
    expect(consumeTurnGrant(state, 0, T0).allowed).toBe(false);
    expect(consumeTurnGrant(state, 0, T0 + TURN_GRANT_SLOT_REFILL_MS).allowed).toBe(true);
    expect(consumeTurnGrant(state, 0, T0 + TURN_GRANT_SLOT_REFILL_MS).allowed).toBe(false);
  });

  it("never refills beyond the burst, however long the room idles", () => {
    const state = createTurnGrantLimiter(T0);
    const farFuture = T0 + TURN_GRANT_SLOT_REFILL_MS * 10_000;
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) {
      expect(consumeTurnGrant(state, 0, farFuture).allowed).toBe(true);
    }
    expect(consumeTurnGrant(state, 0, farFuture).allowed).toBe(false);
  });

  it("throttles seats independently", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) consumeTurnGrant(state, 0, T0);
    expect(consumeTurnGrant(state, 0, T0).allowed).toBe(false);
    expect(consumeTurnGrant(state, 1, T0).allowed).toBe(true);
  });

  it("does not hand out tokens when the clock appears to move backwards", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) consumeTurnGrant(state, 0, T0);
    expect(consumeTurnGrant(state, 0, T0 - TURN_GRANT_SLOT_REFILL_MS * 5).allowed).toBe(false);
  });
});

describe("consumeTurnGrant: room bucket", () => {
  it("caps the whole room even when every seat is individually within budget", () => {
    const state = createTurnGrantLimiter(T0);
    let allowed = 0;
    // Enough distinct seats that the per-seat bucket can never be the limit.
    for (let slot = 0; slot < 64; slot += 1) {
      if (consumeTurnGrant(state, slot, T0).allowed) allowed += 1;
    }
    expect(allowed).toBe(TURN_GRANT_ROOM_BURST);
  });

  it("reports the room as the throttling scope", () => {
    const state = createTurnGrantLimiter(T0);
    for (let slot = 0; slot < TURN_GRANT_ROOM_BURST; slot += 1) consumeTurnGrant(state, slot, T0);
    const denied = consumeTurnGrant(state, 999, T0);
    expect(denied).toMatchObject({ allowed: false, scope: "room" });
    expect(denied.retryAfterSeconds).toBeGreaterThanOrEqual(1);
  });

  it("refills the room bucket over time", () => {
    const state = createTurnGrantLimiter(T0);
    for (let slot = 0; slot < TURN_GRANT_ROOM_BURST; slot += 1) consumeTurnGrant(state, slot, T0);
    expect(consumeTurnGrant(state, 999, T0).allowed).toBe(false);
    expect(consumeTurnGrant(state, 999, T0 + TURN_GRANT_ROOM_REFILL_MS).allowed).toBe(true);
  });

  it("does not charge the room bucket for a request the seat bucket already rejected", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) consumeTurnGrant(state, 0, T0);
    const roomSpentBefore = state.room.tokens;
    for (let index = 0; index < 50; index += 1) consumeTurnGrant(state, 0, T0);
    expect(state.room.tokens).toBe(roomSpentBefore);
  });
});

describe("pruneTurnGrantSlots", () => {
  it("keeps a throttled seat's debt, so leaving and rejoining cannot reset it", () => {
    const state = createTurnGrantLimiter(T0);
    for (let index = 0; index < TURN_GRANT_SLOT_BURST; index += 1) consumeTurnGrant(state, 0, T0);
    pruneTurnGrantSlots(state, T0);
    expect(state.perSlot.has(0)).toBe(true);
    expect(consumeTurnGrant(state, 0, T0).allowed).toBe(false);
  });

  it("drops a seat once it has refilled completely, since it constrains nothing", () => {
    const state = createTurnGrantLimiter(T0);
    consumeTurnGrant(state, 0, T0);
    pruneTurnGrantSlots(state, T0 + TURN_GRANT_SLOT_REFILL_MS * TURN_GRANT_SLOT_BURST);
    expect(state.perSlot.has(0)).toBe(false);
  });
});
