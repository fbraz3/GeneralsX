/**
 * Security header policy shared by the Cloudflare Worker (dynamic responses)
 * and the static launcher build (Cloudflare Pages `_headers` file). Kept in
 * one place so COOP/COEP/CORP/CSP/CORS never drift between the two surfaces.
 */

export interface SecurityHeadersOptions {
  /** Origins allowed to fetch/connect cross-origin (CORS + CSP connect-src).
   * Must be exact `scheme://host[:port]` origins, no wildcards. */
  readonly allowedOrigins: readonly string[];
  /** WebSocket signaling origin(s), added to CSP `connect-src` as `wss://`. */
  readonly signalingOrigins?: readonly string[];
  /** Origin serving game assets (R2/CDN), added to CSP `connect-src`. */
  readonly assetOrigins?: readonly string[];
}

const BASE_SECURITY_HEADERS: Readonly<Record<string, string>> = {
  // Required so the launcher can use SharedArrayBuffer for the Emscripten
  // pthreads build (cross-origin isolation).
  "Cross-Origin-Opener-Policy": "same-origin",
  "Cross-Origin-Embedder-Policy": "require-corp",
  "Cross-Origin-Resource-Policy": "same-origin",
  "X-Content-Type-Options": "nosniff",
  "Referrer-Policy": "strict-origin-when-cross-origin",
  "X-Frame-Options": "DENY",
};

function buildCsp(options: SecurityHeadersOptions): string {
  const connectSrc = [
    "'self'",
    "blob:",
    ...(options.signalingOrigins ?? []),
    ...(options.signalingOrigins ?? []).map((origin) => toWebSocketOrigin(origin)),
    ...(options.assetOrigins ?? []),
  ];
  const directives: Record<string, string[]> = {
    "default-src": ["'self'"],
    "script-src": ["'self'", "blob:", "'wasm-unsafe-eval'"],
    "style-src": ["'self'"],
    "img-src": ["'self'", "data:"],
    "font-src": ["'self'"],
    "connect-src": connectSrc,
    "worker-src": ["'self'", "blob:"],
    "frame-ancestors": ["'none'"],
    "base-uri": ["'self'"],
    "object-src": ["'none'"],
  };
  return Object.entries(directives)
    .map(([directive, values]) => `${directive} ${values.join(" ")}`)
    .join("; ");
}

function toWebSocketOrigin(origin: string): string {
  if (origin.startsWith("https://")) return `wss://${origin.slice("https://".length)}`;
  if (origin.startsWith("http://")) return `ws://${origin.slice("http://".length)}`;
  return origin;
}

/** Builds the static (non-CORS) security headers applied to every response. */
export function buildSecurityHeaders(options: SecurityHeadersOptions): Record<string, string> {
  return {
    ...BASE_SECURITY_HEADERS,
    "Content-Security-Policy": buildCsp(options),
  };
}

/**
 * Returns `true` when `requestOrigin` exactly matches one of the configured
 * allowed origins. No wildcard or suffix matching is performed on purpose.
 */
export function isAllowedOrigin(
  requestOrigin: string | null | undefined,
  options: SecurityHeadersOptions,
): boolean {
  if (!requestOrigin) return false;
  return options.allowedOrigins.includes(requestOrigin);
}

/**
 * Builds CORS response headers for a given request origin. Returns an empty
 * object (no CORS headers) when the origin is not on the allowlist, which
 * causes browsers to block the cross-origin read.
 */
export function buildCorsHeaders(
  requestOrigin: string | null | undefined,
  options: SecurityHeadersOptions,
): Record<string, string> {
  if (!isAllowedOrigin(requestOrigin, options)) return {};
  return {
    "Access-Control-Allow-Origin": requestOrigin as string,
    "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type",
    "Access-Control-Max-Age": "600",
    Vary: "Origin",
  };
}

/**
 * Renders a Cloudflare Pages `_headers` file body applying the security
 * headers to every path in `paths` (defaults to the whole site via `/*`).
 */
export function renderPagesHeadersFile(
  options: SecurityHeadersOptions,
  paths: readonly string[] = ["/*"],
): string {
  const headers = buildSecurityHeaders(options);
  return paths
    .map((path) => {
      const lines = Object.entries(headers).map(([name, value]) => `  ${name}: ${value}`);
      return [path, ...lines].join("\n");
    })
    .join("\n\n")
    .concat("\n");
}
