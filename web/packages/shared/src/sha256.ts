/**
 * Incremental (streaming) SHA-256.
 *
 * `crypto.subtle.digest` is one-shot: it requires the *entire* payload to be
 * resident in JavaScript memory before it can produce a digest. The asset
 * pipeline downloads multi-hundred-megabyte archives, so it must hash them
 * chunk by chunk while the bytes are streamed straight to disk. This module
 * provides that incremental primitive with a fixed 64-byte working buffer,
 * which keeps hashing memory constant regardless of payload size.
 *
 * The implementation is a direct transcription of FIPS 180-4 and is verified
 * against `crypto.subtle.digest` in the unit tests.
 */

/** SHA-256 hex digest, always 64 lowercase hex characters. */
export type Sha256Hex = string;

/** Round constants: first 32 bits of the fractional parts of the cube roots
 * of the first 64 primes. */
const K = new Uint32Array([
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
]);

const INITIAL_STATE = new Uint32Array([
  0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
]);

const BLOCK_BYTES = 64;

function rotr(value: number, bits: number): number {
  return ((value >>> bits) | (value << (32 - bits))) >>> 0;
}

const HEX = Array.from({ length: 256 }, (_, byte) => byte.toString(16).padStart(2, "0"));

/** Lowercase hex encoding of arbitrary bytes. */
export function bytesToHex(bytes: Uint8Array): string {
  let out = "";
  for (let i = 0; i < bytes.length; i += 1) {
    out += HEX[bytes[i] as number];
  }
  return out;
}

/**
 * Incremental SHA-256 hasher.
 *
 * Feed it any number of chunks with {@link update}, then call {@link hex} (or
 * {@link digest}) exactly once. The hasher retains only a single 64-byte block
 * plus the 32-byte state, so hashing a 2 GiB archive costs the same memory as
 * hashing a 1 KiB script.
 */
export class Sha256Stream {
  private readonly state = new Uint32Array(INITIAL_STATE);
  private readonly block = new Uint8Array(BLOCK_BYTES);
  private readonly schedule = new Uint32Array(64);
  private blockLength = 0;
  private totalBytes = 0;
  private finished = false;

  /** Total number of bytes consumed so far. */
  get byteLength(): number {
    return this.totalBytes;
  }

  /** Absorbs one chunk. Chunks may be any size, including zero-length. */
  update(chunk: Uint8Array): this {
    if (this.finished) {
      throw new Error("Sha256Stream.update called after digest()");
    }
    this.totalBytes += chunk.length;

    let offset = 0;
    if (this.blockLength > 0) {
      const needed = BLOCK_BYTES - this.blockLength;
      const take = Math.min(needed, chunk.length);
      this.block.set(chunk.subarray(0, take), this.blockLength);
      this.blockLength += take;
      offset = take;
      if (this.blockLength < BLOCK_BYTES) return this;
      this.compress(this.block, 0);
      this.blockLength = 0;
    }

    while (offset + BLOCK_BYTES <= chunk.length) {
      this.compress(chunk, offset);
      offset += BLOCK_BYTES;
    }

    if (offset < chunk.length) {
      const rest = chunk.length - offset;
      this.block.set(chunk.subarray(offset), 0);
      this.blockLength = rest;
    }
    return this;
  }

  /** Finalizes the hash and returns the 32 raw digest bytes. */
  digest(): Uint8Array {
    if (this.finished) {
      throw new Error("Sha256Stream.digest called twice");
    }
    this.finished = true;

    const bitLength = this.totalBytes * 8;
    const tail = new Uint8Array(this.blockLength < 56 ? BLOCK_BYTES : BLOCK_BYTES * 2);
    tail.set(this.block.subarray(0, this.blockLength), 0);
    tail[this.blockLength] = 0x80;

    const view = new DataView(tail.buffer);
    // 64-bit big-endian bit length. Splitting through Math.floor keeps the
    // high word exact for payloads far beyond any realistic asset size.
    view.setUint32(tail.length - 8, Math.floor(bitLength / 0x100000000), false);
    view.setUint32(tail.length - 4, bitLength >>> 0, false);

    for (let offset = 0; offset < tail.length; offset += BLOCK_BYTES) {
      this.compress(tail, offset);
    }

    const out = new Uint8Array(32);
    const outView = new DataView(out.buffer);
    for (let i = 0; i < 8; i += 1) {
      outView.setUint32(i * 4, this.state[i] as number, false);
    }
    return out;
  }

  /** Finalizes the hash and returns its lowercase hex encoding. */
  hex(): Sha256Hex {
    return bytesToHex(this.digest());
  }

  private compress(input: Uint8Array, offset: number): void {
    const w = this.schedule;
    for (let i = 0; i < 16; i += 1) {
      const j = offset + i * 4;
      w[i] =
        (((input[j] as number) << 24) |
          ((input[j + 1] as number) << 16) |
          ((input[j + 2] as number) << 8) |
          (input[j + 3] as number)) >>>
        0;
    }
    for (let i = 16; i < 64; i += 1) {
      const x = w[i - 15] as number;
      const y = w[i - 2] as number;
      const s0 = (rotr(x, 7) ^ rotr(x, 18) ^ (x >>> 3)) >>> 0;
      const s1 = (rotr(y, 17) ^ rotr(y, 19) ^ (y >>> 10)) >>> 0;
      w[i] = (((w[i - 16] as number) + s0 + (w[i - 7] as number) + s1) & 0xffffffff) >>> 0;
    }

    let a = this.state[0] as number;
    let b = this.state[1] as number;
    let c = this.state[2] as number;
    let d = this.state[3] as number;
    let e = this.state[4] as number;
    let f = this.state[5] as number;
    let g = this.state[6] as number;
    let h = this.state[7] as number;

    for (let i = 0; i < 64; i += 1) {
      const S1 = (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) >>> 0;
      const ch = ((e & f) ^ (~e & g)) >>> 0;
      const temp1 = (h + S1 + ch + (K[i] as number) + (w[i] as number)) >>> 0;
      const S0 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) >>> 0;
      const maj = ((a & b) ^ (a & c) ^ (b & c)) >>> 0;
      const temp2 = (S0 + maj) >>> 0;

      h = g;
      g = f;
      f = e;
      e = (d + temp1) >>> 0;
      d = c;
      c = b;
      b = a;
      a = (temp1 + temp2) >>> 0;
    }

    this.state[0] = ((this.state[0] as number) + a) >>> 0;
    this.state[1] = ((this.state[1] as number) + b) >>> 0;
    this.state[2] = ((this.state[2] as number) + c) >>> 0;
    this.state[3] = ((this.state[3] as number) + d) >>> 0;
    this.state[4] = ((this.state[4] as number) + e) >>> 0;
    this.state[5] = ((this.state[5] as number) + f) >>> 0;
    this.state[6] = ((this.state[6] as number) + g) >>> 0;
    this.state[7] = ((this.state[7] as number) + h) >>> 0;
  }
}

/** One-shot convenience wrapper around {@link Sha256Stream}. */
export function sha256Hex(bytes: Uint8Array): Sha256Hex {
  return new Sha256Stream().update(bytes).hex();
}

/**
 * Constant-time-ish comparison of two hex digests. Digest comparison is not
 * a secret-dependent operation here (both values are public), but avoiding an
 * early return keeps the intent explicit and prevents accidental
 * case-sensitivity bugs.
 */
export function digestsEqual(a: string, b: string): boolean {
  if (a.length !== b.length) return false;
  let diff = 0;
  for (let i = 0; i < a.length; i += 1) {
    diff |= a.charCodeAt(i) ^ b.charCodeAt(i);
  }
  return diff === 0;
}
