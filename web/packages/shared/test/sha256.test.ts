import { describe, expect, it } from "vitest";
import { Sha256Stream, bytesToHex, digestsEqual, sha256Hex } from "../src/sha256.js";

async function webCryptoSha256(bytes: Uint8Array): Promise<string> {
  const copy = new Uint8Array(new ArrayBuffer(bytes.length));
  copy.set(bytes);
  const digest = await crypto.subtle.digest("SHA-256", copy);
  return bytesToHex(new Uint8Array(digest));
}

describe("Sha256Stream", () => {
  it("matches the FIPS 180-4 test vectors", () => {
    expect(sha256Hex(new Uint8Array(0))).toBe(
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
    );
    expect(sha256Hex(new TextEncoder().encode("abc"))).toBe(
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
    );
    expect(
      sha256Hex(
        new TextEncoder().encode(
          "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        ),
      ),
    ).toBe("248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
  });

  it("agrees with crypto.subtle across chunk boundaries", async () => {
    const payload = new Uint8Array(4096 + 137);
    for (let i = 0; i < payload.length; i += 1) payload[i] = (i * 31 + 7) & 0xff;

    for (const chunkSize of [1, 7, 63, 64, 65, 1000, 4096]) {
      const hasher = new Sha256Stream();
      for (let offset = 0; offset < payload.length; offset += chunkSize) {
        hasher.update(payload.subarray(offset, Math.min(payload.length, offset + chunkSize)));
      }
      expect(hasher.hex(), `chunk size ${chunkSize}`).toBe(await webCryptoSha256(payload));
    }
  });

  it("handles payloads that land exactly on a padding boundary", async () => {
    for (const size of [55, 56, 57, 63, 64, 119, 120]) {
      const payload = new Uint8Array(size).fill(0xab);
      expect(sha256Hex(payload), `size ${size}`).toBe(await webCryptoSha256(payload));
    }
  });

  it("tracks the number of bytes consumed", () => {
    const hasher = new Sha256Stream();
    hasher.update(new Uint8Array(10));
    hasher.update(new Uint8Array(90));
    expect(hasher.byteLength).toBe(100);
  });

  it("ignores empty chunks", () => {
    const hasher = new Sha256Stream();
    hasher.update(new Uint8Array(0));
    hasher.update(new TextEncoder().encode("abc"));
    hasher.update(new Uint8Array(0));
    expect(hasher.hex()).toBe(sha256Hex(new TextEncoder().encode("abc")));
  });

  it("refuses to be reused after finalization", () => {
    const hasher = new Sha256Stream();
    hasher.update(new Uint8Array(4));
    hasher.digest();
    expect(() => hasher.update(new Uint8Array(1))).toThrow(/after digest/);
    expect(() => hasher.digest()).toThrow(/twice/);
  });
});

describe("digestsEqual", () => {
  it("compares digests without early exit", () => {
    const a = "a".repeat(64);
    expect(digestsEqual(a, a)).toBe(true);
    expect(digestsEqual(a, `b${a.slice(1)}`)).toBe(false);
    expect(digestsEqual(a, a.slice(0, 63))).toBe(false);
  });
});
