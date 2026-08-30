import { describe, expect, it } from "vitest";
import {
  MAX_MESSAGE_BYTES,
  MAX_ROOM_CAPACITY,
  MIN_ROOM_CAPACITY,
  parseClientMessage,
  validateClientMessage,
} from "../src/protocol.js";

describe("validateClientMessage: join", () => {
  it("accepts a minimal valid join", () => {
    const result = validateClientMessage({ type: "join", roomId: "AB12" });
    expect(result.valid).toBe(true);
    expect(result.message).toEqual({ type: "join", roomId: "AB12" });
  });

  it("accepts join with name and capacity", () => {
    const result = validateClientMessage({
      type: "join",
      roomId: "AB12",
      name: "Player One",
      capacity: 4,
    });
    expect(result.valid).toBe(true);
  });

  it("rejects malformed room ids", () => {
    for (const roomId of ["", "ab", "toolongroomid1234", "has space", "lower"]) {
      expect(validateClientMessage({ type: "join", roomId }).valid).toBe(false);
    }
  });

  it("rejects malformed player names", () => {
    const result = validateClientMessage({ type: "join", roomId: "AB12", name: "<script>" });
    expect(result.valid).toBe(false);
  });

  it("rejects capacity outside the allowed bounds", () => {
    expect(validateClientMessage({ type: "join", roomId: "AB12", capacity: MIN_ROOM_CAPACITY - 1 }).valid).toBe(
      false,
    );
    expect(validateClientMessage({ type: "join", roomId: "AB12", capacity: MAX_ROOM_CAPACITY + 1 }).valid).toBe(
      false,
    );
    expect(validateClientMessage({ type: "join", roomId: "AB12", capacity: 3.5 }).valid).toBe(false);
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
