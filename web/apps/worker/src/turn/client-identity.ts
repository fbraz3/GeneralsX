/**
 * Identifies the caller of a Worker request for rate-limiting purposes.
 *
 * Only `CF-Connecting-IP` is trusted. Cloudflare's edge sets that header on
 * every request it forwards to a Worker and *overwrites* any value the client
 * supplied, so it cannot be spoofed by a client that reaches us through
 * Cloudflare. `X-Forwarded-For`, `X-Real-IP`, `Forwarded`, and `True-Client-IP`
 * are all either appended-to or fully client-controlled, so they are never
 * read here: honouring one would let an attacker mint a fresh rate-limit
 * bucket per request simply by varying a header.
 *
 * A request that arrives without `CF-Connecting-IP` did not come through the
 * edge (local `wrangler dev`, or a direct dispatch), so it cannot be
 * attributed to anyone. Rather than guess, every such request shares one
 * deliberately small bucket — see {@link UNIDENTIFIED_CLIENT_KEY}.
 */

/** The only client-address header the edge guarantees and rewrites. */
export const TRUSTED_CLIENT_IP_HEADER = "CF-Connecting-IP";

/** Shared bucket for requests that carry no trusted address. */
export const UNIDENTIFIED_CLIENT_KEY = "unidentified";

/** Longest plausible textual IPv6 address with a zone id; anything longer is
 * malformed and is not used as a map key. */
const MAX_CLIENT_IP_LENGTH = 64;

/**
 * IPv6 bits kept when bucketing.
 *
 * A single residential IPv6 customer is routinely delegated a whole /64 (and
 * often a /56 or /48), so limiting per full 128-bit address would be defeated
 * by picking a new source address per request at zero cost. Aggregating to
 * the /64 makes an IPv6 client no cheaper to rate limit than an IPv4 one.
 */
const IPV6_BUCKET_GROUPS = 4;

export interface ClientIdentity {
  /** Rate-limit bucket key. Never attacker-chosen. */
  readonly key: string;
  /** False when the request carried no trusted address and therefore shares
   * the {@link UNIDENTIFIED_CLIENT_KEY} bucket. */
  readonly identified: boolean;
}

function normalizeIpv6(address: string): string {
  const zoneless = address.split("%")[0] ?? address;
  const expanded = expandIpv6(zoneless);
  if (!expanded) return zoneless.toLowerCase();
  return `${expanded.slice(0, IPV6_BUCKET_GROUPS).join(":")}::/64`;
}

/** Expands `::` so two spellings of one address never land in two buckets.
 * Returns null for anything that is not a plain IPv6 literal. */
function expandIpv6(address: string): string[] | null {
  const halves = address.split("::");
  if (halves.length > 2) return null;
  const head = halves[0] ? halves[0].split(":") : [];
  const tail = halves.length === 2 && halves[1] ? halves[1].split(":") : [];
  if (halves.length === 1) {
    return head.length === 8 ? head.map(padGroup) : null;
  }
  const missing = 8 - head.length - tail.length;
  if (missing < 0) return null;
  return [...head, ...Array<string>(missing).fill("0"), ...tail].map(padGroup);
}

function padGroup(group: string): string {
  return group.toLowerCase().replace(/^0+(?=.)/, "");
}

/**
 * Resolves the rate-limit identity of `request`.
 *
 * The returned key is derived only from an edge-supplied value, so an
 * attacker cannot escape their own bucket. Anything malformed (a comma, which
 * would indicate an `X-Forwarded-For`-style list rather than a genuine edge
 * header, whitespace, or an implausible length) is treated as absent.
 */
export function resolveClientIdentity(request: Request): ClientIdentity {
  const raw = request.headers.get(TRUSTED_CLIENT_IP_HEADER)?.trim() ?? "";
  if (raw.length === 0 || raw.length > MAX_CLIENT_IP_LENGTH || /[\s,]/.test(raw)) {
    return { key: UNIDENTIFIED_CLIENT_KEY, identified: false };
  }
  const key = raw.includes(":") ? normalizeIpv6(raw) : raw;
  return { key, identified: true };
}
