import { describe, expect, it } from "vitest";
import {
  resolveClientIdentity,
  TRUSTED_CLIENT_IP_HEADER,
  UNIDENTIFIED_CLIENT_KEY,
} from "../../src/turn/client-identity.js";

function request(headers: Record<string, string>): Request {
  return new Request("https://signaling.generalsx.org/turn-credentials", { headers });
}

describe("resolveClientIdentity", () => {
  it("uses the edge-supplied client address", () => {
    const identity = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "203.0.113.7" }));
    expect(identity).toEqual({ key: "203.0.113.7", identified: true });
  });

  it("ignores every client-controlled address header", () => {
    // Each of these can be set freely by the caller, so honouring one would
    // hand an attacker a fresh rate-limit bucket per request.
    for (const header of ["X-Forwarded-For", "X-Real-IP", "True-Client-IP", "Forwarded"]) {
      const identity = resolveClientIdentity(request({ [header]: "203.0.113.7" }));
      expect(identity, header).toEqual({ key: UNIDENTIFIED_CLIENT_KEY, identified: false });
    }
  });

  it("does not let a forged header override the edge one", () => {
    const identity = resolveClientIdentity(
      request({ [TRUSTED_CLIENT_IP_HEADER]: "203.0.113.7", "X-Forwarded-For": "198.51.100.9" }),
    );
    expect(identity.key).toBe("203.0.113.7");
  });

  it("shares one bucket when no trusted address is present", () => {
    const identity = resolveClientIdentity(request({}));
    expect(identity).toEqual({ key: UNIDENTIFIED_CLIENT_KEY, identified: false });
  });

  it("rejects a comma-separated list, which is not a genuine edge value", () => {
    const identity = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "203.0.113.7, 198.51.100.9" }));
    expect(identity.identified).toBe(false);
  });

  it("rejects an implausibly long value rather than using it as a map key", () => {
    const identity = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "a".repeat(200) }));
    expect(identity.identified).toBe(false);
  });

  it("buckets IPv6 by /64, so rotating within a delegation does not escape", () => {
    const first = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8:1:2:3:4:5:6" }));
    const second = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8:1:2:aaaa:bbbb:cccc:dddd" }));
    expect(first.key).toBe(second.key);
    expect(first.identified).toBe(true);
  });

  it("separates distinct IPv6 /64s", () => {
    const first = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8:1:2::1" }));
    const second = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8:1:3::1" }));
    expect(first.key).not.toBe(second.key);
  });

  it("normalises compressed and expanded spellings to one bucket", () => {
    const compressed = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8::1" }));
    const expanded = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:0db8:0:0:0:0:0:1" }));
    expect(compressed.key).toBe(expanded.key);
  });

  it("strips an IPv6 zone id", () => {
    const zoned = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8:1:2::1%eth0" }));
    const plain = resolveClientIdentity(request({ [TRUSTED_CLIENT_IP_HEADER]: "2001:db8:1:2::1" }));
    expect(zoned.key).toBe(plain.key);
  });
});
