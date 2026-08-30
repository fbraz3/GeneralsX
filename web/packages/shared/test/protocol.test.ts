import { describe, expect, it } from "vitest";
import {
  CURRENT_COMPATIBILITY,
  MAX_MESSAGE_BYTES,
  MAX_ROOM_CAPACITY,
  MIN_ROOM_CAPACITY,
  parseClientMessage,
  validateClientMessage,
} from "../src/protocol.js";

function join(overrides: Record<string, unknown> = {}) {
  return { type: "join", roomId: "AB12", compatibility: CURRENT_COMPATIBILITY, ...overrides };
}

describe("validateClientMessage: join", () => {
  it("accepts a minimal valid join", () => {
    const result = validateClientMessage(join());
    expect(result.valid).toBe(true);
    expect(result.message).toEqual(join());
  });

  it("accepts join with name and capacity", () => {
    const result = validateClientMessage(join({
      name: "Player One",
      capacity: 4,
    }));
    expect(result.valid).toBe(true);
  });

  it("rejects malformed room ids", () => {
    for (const roomId of ["", "ab", "toolongroomid1234", "has space", "lower"]) {
      expect(validateClientMessage(join({ roomId })).valid).toBe(false);
    }
  });

  it("rejects malformed player names", () => {
    const result = validateClientMessage(join({ name: "<script>" }));
    expect(result.valid).toBe(false);
  });

  it("rejects capacity outside the allowed bounds", () => {
    expect(validateClientMessage(join({ capacity: MIN_ROOM_CAPACITY - 1 })).valid).toBe(false);
    expect(validateClientMessage(join({ capacity: MAX_ROOM_CAPACITY + 1 })).valid).toBe(false);
    expect(validateClientMessage(join({ capacity: 3.5 })).valid).toBe(false);
  });

  it("requires positive integer compatibility versions", () => {
    expect(validateClientMessage({ type: "join", roomId: "AB12" }).valid).toBe(false);
    expect(validateClientMessage(join({ compatibility: { engine: 1, protocol: 1, determinism: 0 } })).valid).toBe(
      false,
    );
  });
});

describe("validateClientMessage: signaling", () => {
  it("accepts offer/answer/ice with a slot and payload", () => {
    for (const type of ["offer", "answer", "ice"] as const) {
      const result = validateClientMessage({ type, to: 2, payload: { sdp: "v=0" } });
      expect(result.valid, type).toBe(true);
    }
  });

  it("rejects a missing or negative target slot", () => {
    expect(validateClientMessage({ type: "offer", to: -1, payload: {} }).valid).toBe(false);
    expect(validateClientMessage({ type: "offer", payload: {} }).valid).toBe(false);
    expect(validateClientMessage({ type: "offer", to: 1.5, payload: {} }).valid).toBe(false);
  });

  it("rejects a missing payload", () => {
    expect(validateClientMessage({ type: "offer", to: 1 }).valid).toBe(false);
    expect(validateClientMessage({ type: "offer", to: 1, payload: null }).valid).toBe(false);
  });

  it("rejects an oversized string payload", () => {
    const result = validateClientMessage({ type: "offer", to: 1, payload: "x".repeat(20000) });
    expect(result.valid).toBe(false);
  });
});

describe("validateClientMessage: leave and unknown types", () => {
  it("accepts leave", () => {
    expect(validateClientMessage({ type: "leave" })).toEqual({ valid: true, message: { type: "leave" } });
  });

  it("rejects unknown message types", () => {
    expect(validateClientMessage({ type: "hack" }).valid).toBe(false);
  });

  it("rejects non-object input", () => {
    expect(validateClientMessage(null).valid).toBe(false);
    expect(validateClientMessage("join").valid).toBe(false);
    expect(validateClientMessage([1]).valid).toBe(false);
  });
});

describe("parseClientMessage", () => {
  it("parses valid JSON into a validated message", () => {
    const result = parseClientMessage(JSON.stringify({ type: "leave" }));
    expect(result.valid).toBe(true);
  });

  it("rejects invalid JSON", () => {
    expect(parseClientMessage("{not json").valid).toBe(false);
  });

  it("rejects oversized raw messages before parsing", () => {
    const huge = JSON.stringify({ type: "leave", pad: "x".repeat(MAX_MESSAGE_BYTES) });
    expect(parseClientMessage(huge).valid).toBe(false);
  });
});
