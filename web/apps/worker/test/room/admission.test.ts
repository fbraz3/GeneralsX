import { describe, expect, it } from "vitest";
import {
  ADMISSION_TOKEN_TTL_SECONDS,
  ADMISSION_TOKEN_VERSION,
  createAdmissionId,
  createRoomKeyBytes,
  extractBearerToken,
  importRoomKey,
  peekAdmissionRoomId,
  ROOM_KEY_BYTES,
  signAdmissionToken,
  verifyAdmissionToken,
  type AdmissionClaims,
} from "../../src/room/admission.js";

async function freshKey(): Promise<CryptoKey> {
  return importRoomKey(createRoomKeyBytes());
}

function claims(overrides: Partial<AdmissionClaims> = {}): AdmissionClaims {
  return {
    roomId: "ABCD",
    slot: 0,
    admissionId: "seat-nonce",
    exp: Math.floor(Date.now() / 1000) + ADMISSION_TOKEN_TTL_SECONDS,
    ...overrides,
  };
}

describe("room key material", () => {
  it("generates a full-length key", () => {
    expect(createRoomKeyBytes()).toHaveLength(ROOM_KEY_BYTES);
  });

  it("generates a different key every time", () => {
    const a = [...createRoomKeyBytes()].join(",");
    const b = [...createRoomKeyBytes()].join(",");
    expect(a).not.toBe(b);
  });

  it("imports the key as non-extractable, so it can never be exported or logged", async () => {
    const key = await freshKey();
    expect(key.extractable).toBe(false);
    await expect(crypto.subtle.exportKey("raw", key)).rejects.toThrow();
  });

  it("generates a distinct, hex-encoded admission id per seat", () => {
    const first = createAdmissionId();
    const second = createAdmissionId();
    expect(first).toMatch(/^[0-9a-f]{32}$/);
    expect(first).not.toBe(second);
  });
});

describe("signAdmissionToken / verifyAdmissionToken", () => {
  it("round-trips the claims", async () => {
    const key = await freshKey();
    const token = await signAdmissionToken(key, claims());
    const result = await verifyAdmissionToken(token, key, { expectedRoomId: "ABCD" });
    expect(result).toMatchObject({ ok: true, claims: { roomId: "ABCD", slot: 0, admissionId: "seat-nonce" } });
  });

  it("emits a versioned, three-segment base64url token", async () => {
    const key = await freshKey();
    const token = await signAdmissionToken(key, claims());
    const parts = token.split(".");
    expect(parts).toHaveLength(3);
    expect(parts[0]).toBe(ADMISSION_TOKEN_VERSION);
    expect(token).toMatch(/^[A-Za-z0-9_.-]+$/);
  });

  it("rejects a token signed by a different room's key", async () => {
    const issuer = await freshKey();
    const otherRoom = await freshKey();
    const token = await signAdmissionToken(issuer, claims());
    const result = await verifyAdmissionToken(token, otherRoom, { expectedRoomId: "ABCD" });
    expect(result).toMatchObject({ ok: false, code: "BAD_SIGNATURE" });
  });

  it("rejects a token whose claims were tampered with", async () => {
    const key = await freshKey();
    const token = await signAdmissionToken(key, claims({ slot: 0 }));
    const [version, , mac] = token.split(".") as [string, string, string];
    const forgedPayload = btoa(JSON.stringify(claims({ slot: 7 })))
      .replace(/\+/g, "-")
      .replace(/\//g, "_")
      .replace(/=+$/, "");
    const forged = `${version}.${forgedPayload}.${mac}`;
    const result = await verifyAdmissionToken(forged, key, { expectedRoomId: "ABCD" });
    expect(result).toMatchObject({ ok: false, code: "BAD_SIGNATURE" });
  });

  it("rejects a token minted for another room even under a matching key", async () => {
    const key = await freshKey();
    const token = await signAdmissionToken(key, claims({ roomId: "WXYZ" }));
    const result = await verifyAdmissionToken(token, key, { expectedRoomId: "ABCD" });
    expect(result).toMatchObject({ ok: false, code: "WRONG_ROOM" });
  });

  it("rejects an expired token", async () => {
    const key = await freshKey();
    const issuedAtMs = 1_000_000_000_000;
    const token = await signAdmissionToken(key, claims({ exp: issuedAtMs / 1000 + 60 }));
    const stillValid = await verifyAdmissionToken(token, key, {
      expectedRoomId: "ABCD",
      nowMs: issuedAtMs + 59_000,
    });
    expect(stillValid.ok).toBe(true);
    const expired = await verifyAdmissionToken(token, key, {
      expectedRoomId: "ABCD",
      nowMs: issuedAtMs + 61_000,
    });
    expect(expired).toMatchObject({ ok: false, code: "EXPIRED" });
  });

  it.each([
    ["empty", ""],
    ["not a token", "hello"],
    ["wrong version", "gxa0.YWJj.YWJj"],
    ["too few segments", "gxa1.YWJj"],
    ["too many segments", "gxa1.YWJj.YWJj.YWJj"],
    ["non-base64url signature", "gxa1.YWJj.!!!!"],
  ])("rejects a malformed token (%s)", async (_label, token) => {
    const key = await freshKey();
    const result = await verifyAdmissionToken(token, key, { expectedRoomId: "ABCD" });
    expect(result).toMatchObject({ ok: false, code: "MALFORMED" });
  });

  it("never reports a failure reason that echoes token or key material", async () => {
    const key = await freshKey();
    const otherRoom = await freshKey();
    const token = await signAdmissionToken(key, claims());
    const result = await verifyAdmissionToken(token, otherRoom, { expectedRoomId: "ABCD" });
    expect(result.ok).toBe(false);
    if (!result.ok) expect(result.detail).not.toContain(token.split(".")[2]);
  });
});

describe("peekAdmissionRoomId", () => {
  it("reads the room id for routing without verifying anything", async () => {
    const key = await freshKey();
    const token = await signAdmissionToken(key, claims({ roomId: "R7K2QX" }));
    expect(peekAdmissionRoomId(token)).toBe("R7K2QX");
  });

  it("returns null for anything that is not a well-formed token", () => {
    expect(peekAdmissionRoomId("")).toBeNull();
    expect(peekAdmissionRoomId("nope")).toBeNull();
    expect(peekAdmissionRoomId("gxa1.$$$.YWJj")).toBeNull();
  });
});

describe("extractBearerToken", () => {
  it("extracts a bearer credential", () => {
    expect(extractBearerToken("Bearer abc.def.ghi")).toBe("abc.def.ghi");
  });

  it("tolerates surrounding whitespace and tabs", () => {
    expect(extractBearerToken("  Bearer\tabc  ")).toBe("abc");
  });

  it.each([
    ["missing header", null],
    ["empty header", ""],
    ["wrong scheme", "Basic abc"],
    ["no credential", "Bearer "],
    ["credential with a space", "Bearer abc def"],
  ])("returns null for %s", (_label, header) => {
    expect(extractBearerToken(header)).toBeNull();
  });
});
