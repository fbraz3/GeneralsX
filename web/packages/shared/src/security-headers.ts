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
  // Two years, subdomains included. `preload` is intentionally omitted: it is
  // an irreversible commitment for the whole apex and must be an explicit
  // operator decision, not a side effect of deploying this project.
  "Strict-Transport-Security": "max-age=63072000; includeSubDomains",
  // The launcher needs none of these powerful features. WebRTC DataChannels
  // (the only networking surface) require no camera/microphone permission.
  "Permissions-Policy":
    "accelerometer=(), camera=(), display-capture=(), geolocation=(), gyroscope=(), magnetometer=(), microphone=(), payment=(), usb=()",
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
 *
 * CORS is a browser-enforced *read* restriction, not an authorization
 * mechanism: it is not applied by non-browser clients and `Origin` is
 * forgeable outside a browser. Endpoints that hand out anything valuable must
 * carry their own server-issued credential — see the room admission token on
 * `/turn-credentials` — and treat these headers purely as defense in depth.
 */
export function buildCorsHeaders(
  requestOrigin: string | null | undefined,
  options: SecurityHeadersOptions,
): Record<string, string> {
  if (!isAllowedOrigin(requestOrigin, options)) return {};
  return {
    "Access-Control-Allow-Origin": requestOrigin as string,
    "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
    // `Authorization` carries the room admission token the launcher presents
    // to `/turn-credentials`; without it the browser blocks the preflight.
    "Access-Control-Allow-Headers": "Authorization, Content-Type",
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
  return paths.map((path) => renderHeaderRule(path, headers)).join("\n\n").concat("\n");
}

/** One `_headers` section: a path pattern and the headers applied to it. */
export interface PagesHeaderRule {
  readonly path: string;
  readonly headers: Readonly<Record<string, string>>;
}

/** Immutable caching for content-addressed build output (Vite emits
 * `assets/<name>-<hash>.<ext>`), and revalidation for the entry points so an
 * immutable rollback becomes visible without waiting out a cache TTL. */
export const PAGES_CACHE_RULES: readonly PagesHeaderRule[] = [
  { path: "/assets/*", headers: { "Cache-Control": "public, max-age=31536000, immutable" } },
  { path: "/index.html", headers: { "Cache-Control": "no-cache" } },
  { path: "/health.json", headers: { "Cache-Control": "no-store" } },
];

/**
 * Renders additional `_headers` sections. Cloudflare Pages applies *every*
 * matching rule, so these compose with the site-wide security rule emitted by
 * `renderPagesHeadersFile` instead of restating it per path.
 */
export function renderPagesCacheRules(rules: readonly PagesHeaderRule[] = PAGES_CACHE_RULES): string {
  return rules.map((rule) => renderHeaderRule(rule.path, rule.headers)).join("\n\n").concat("\n");
}

function renderHeaderRule(path: string, headers: Readonly<Record<string, string>>): string {
  const lines = Object.entries(headers).map(([name, value]) => `  ${name}: ${value}`);
  return [path, ...lines].join("\n");
}
