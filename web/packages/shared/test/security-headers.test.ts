import { describe, expect, it } from "vitest";
import {
  buildCorsHeaders,
  buildSecurityHeaders,
  isAllowedOrigin,
  renderPagesHeadersFile,
  type SecurityHeadersOptions,
} from "../src/security-headers.js";

const options: SecurityHeadersOptions = {
  allowedOrigins: ["https://play.generalsx.org", "http://localhost:5173"],
  signalingOrigins: ["https://signaling.generalsx.org"],
  assetOrigins: ["https://assets.generalsx.org"],
};

describe("buildSecurityHeaders", () => {
  const headers = buildSecurityHeaders(options);

  it("sets cross-origin isolation headers", () => {
    expect(headers["Cross-Origin-Opener-Policy"]).toBe("same-origin");
    expect(headers["Cross-Origin-Embedder-Policy"]).toBe("require-corp");
    expect(headers["Cross-Origin-Resource-Policy"]).toBe("same-origin");
  });

  it("sets baseline hardening headers", () => {
    expect(headers["X-Content-Type-Options"]).toBe("nosniff");
    expect(headers["X-Frame-Options"]).toBe("DENY");
    expect(headers["Referrer-Policy"]).toBe("strict-origin-when-cross-origin");
  });

  it("builds a CSP that allows the WebSocket signaling origin over wss://", () => {
    const csp = headers["Content-Security-Policy"];
    expect(csp).toBeDefined();
    expect(csp).toContain("wss://signaling.generalsx.org");
    expect(csp).toContain("https://signaling.generalsx.org");
    expect(csp).toContain("https://assets.generalsx.org");
    expect(csp).toContain("default-src 'self'");
    expect(csp).toContain("object-src 'none'");
    expect(csp).toContain("frame-ancestors 'none'");
  });

  it("allows wasm-unsafe-eval for the Emscripten engine only in script-src", () => {
    const csp = headers["Content-Security-Policy"] ?? "";
    expect(csp).toContain("script-src 'self' blob: 'wasm-unsafe-eval'");
  });
});

describe("isAllowedOrigin", () => {
  it("matches exact configured origins only", () => {
    expect(isAllowedOrigin("https://play.generalsx.org", options)).toBe(true);
    expect(isAllowedOrigin("https://evil.example", options)).toBe(false);
    expect(isAllowedOrigin(null, options)).toBe(false);
    expect(isAllowedOrigin(undefined, options)).toBe(false);
  });

  it("does not match a subdomain or suffix of an allowed origin", () => {
    expect(isAllowedOrigin("https://sub.play.generalsx.org", options)).toBe(false);
    expect(isAllowedOrigin("https://play.generalsx.org.evil.example", options)).toBe(false);
  });
});

describe("buildCorsHeaders", () => {
  it("reflects the origin only when allowed", () => {
    const headers = buildCorsHeaders("https://play.generalsx.org", options);
    expect(headers["Access-Control-Allow-Origin"]).toBe("https://play.generalsx.org");
    expect(headers.Vary).toBe("Origin");
  });

  it("returns no CORS headers for a disallowed origin", () => {
    expect(buildCorsHeaders("https://evil.example", options)).toEqual({});
    expect(buildCorsHeaders(null, options)).toEqual({});
  });
});

describe("renderPagesHeadersFile", () => {
  it("renders a Cloudflare Pages _headers file with the security headers", () => {
    const body = renderPagesHeadersFile(options);
    expect(body).toContain("/*");
    expect(body).toContain("  Cross-Origin-Opener-Policy: same-origin");
    expect(body.endsWith("\n")).toBe(true);
  });

  it("supports multiple path sections", () => {
    const body = renderPagesHeadersFile(options, ["/", "/engine/*"]);
    expect(body).toContain("/\n");
    expect(body).toContain("/engine/*\n");
  });
});
