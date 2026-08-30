/**
 * Engine + asset manifest schema shared by the launcher and the Cloudflare
 * Worker backend.
 *
 * IMPORTANT: This module never embeds, references, or ships retail game
 * assets. It only describes the *shape* of a manifest document that is
 * published separately (e.g. to an R2 bucket or authorized CDN) by an
 * operator who holds a legitimate license for the referenced files. The
 * manifest itself only carries object keys, sizes, integrity hashes, and the
 * in-engine mount metadata needed to assemble a virtual file system.
 *
 * Publishing contract (operator side):
 *  - every object is uploaded under an immutable key and is never overwritten
 *    in place; a changed file gets a new key and a bumped `assetsRevision`;
 *  - objects are served with a strong `ETag`, `Accept-Ranges: bytes`, and
 *    `Cache-Control: public, max-age=31536000, immutable`;
 *  - the manifest document itself is the only mutable object.
 */
import type { Sha256Hex } from "./sha256.js";
import { isCompatibilityVersion, type CompatibilityVersion } from "./protocol.ts";

export type { Sha256Hex };

const SHA256_HEX_RE = /^[0-9a-f]{64}$/;
const VERSION_RE = /^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$/;
const STRONG_ETAG_RE = /^"[\x21\x23-\x7e]{1,126}"$/;
// eslint-disable-next-line no-control-regex -- explicitly rejecting control characters
const CONTROL_CHARS_RE = /[\x00-\x1f\x7f]/;

/** Largest single asset the launcher is willing to plan for (8 GiB). Guards
 * against a hostile or corrupt manifest asking the client to download an
 * absurd amount of data. */
export const MAX_ASSET_BYTES = 8 * 1024 * 1024 * 1024;

/** Largest total payload described by one manifest (32 GiB). */
export const MAX_MANIFEST_BYTES = 32 * 1024 * 1024 * 1024;

/**
 * Role of an asset within the engine boot sequence. The role decides *how*
 * the launcher hands the file to the engine, which is why it is part of the
 * validated schema rather than inferred from a file extension at runtime.
 *
 * - `engine-js`      Emscripten loader/glue script (exactly one per manifest).
 * - `engine-wasm`    Compiled engine module (exactly one per manifest).
 * - `engine-data`    Optional Emscripten preload package (`.data`).
 * - `big-base`       Base-game BIG archive, range-read from the VFS.
 * - `big-expansion`  Expansion (Zero Hour) BIG archive, overlays the base.
 * - `script`         Loose INI/LUA/STR script file read whole.
 * - `font`           Loose font file read whole.
 */
export const ASSET_ROLES = [
  "engine-js",
  "engine-wasm",
  "engine-data",
  "big-base",
  "big-expansion",
  "script",
  "font",
] as const;

export type AssetRole = (typeof ASSET_ROLES)[number];

/** Roles that are range-read on demand instead of loaded into memory. */
const STREAMING_ROLES: ReadonlySet<AssetRole> = new Set<AssetRole>(["big-base", "big-expansion"]);

/** Roles that must appear exactly once in a manifest. */
const SINGLETON_ROLES: readonly AssetRole[] = ["engine-js", "engine-wasm"];

/** Roles that may appear at most once in a manifest. */
const OPTIONAL_SINGLETON_ROLES: readonly AssetRole[] = ["engine-data"];

function isAssetRole(value: unknown): value is AssetRole {
  return typeof value === "string" && (ASSET_ROLES as readonly string[]).includes(value);
}

/** True when a role's bytes are streamed/range-read rather than buffered. */
export function isStreamingRole(role: AssetRole): boolean {
  return STREAMING_ROLES.has(role);
}

/** Where and in what order an asset is mounted in the engine's VFS. */
export interface AssetMount {
  /** Absolute in-engine path, e.g. `/generals/Data/INI.big`. Unique per
   * manifest, never containing `..`, a backslash, or an empty segment. */
  readonly target: string;
  /** Mount order. Unique per manifest; lower values are mounted first, and
   * every `big-expansion` archive must sort after every `big-base` archive so
   * expansion content overrides base content deterministically. */
  readonly order: number;
  /** `true` for assets the VFS range-reads on demand (BIG archives), `false`
   * for assets that are read whole (engine module, scripts, fonts). Must
   * agree with the asset's role. */
  readonly streaming: boolean;
}

/** A single downloadable asset entry described by the manifest. */
export interface ManifestAsset {
  /** Immutable object key relative to {@link EngineManifest.assetBaseUrl}.
   * Must not contain `..`, a leading slash, or an absolute URL. */
  readonly path: string;
  /** How the launcher hands this file to the engine. */
  readonly role: AssetRole;
  /** Exact byte size. A download that delivers more or fewer bytes is
   * rejected before the digest check even runs. */
  readonly sizeBytes: number;
  /** SHA-256 hex digest of the asset bytes, verified while streaming. */
  readonly sha256: Sha256Hex;
  /** Mount metadata consumed by the launcher's VFS. */
  readonly mount: AssetMount;
  /** Optional strong `ETag` of the published object. When present it is sent
   * as `If-Range` and re-validated on every resumed download; weak
   * validators are rejected because they cannot guarantee byte identity. */
  readonly etag?: string;
}

/** Immutable engine + asset manifest consumed by the launcher shell. */
export interface EngineManifest {
  /** Manifest schema version, bumped on breaking changes. */
  readonly schemaVersion: 3;
  /** Human-readable engine build identifier (e.g. git short SHA or tag). */
  readonly engineVersion: string;
  /** CMake-generated lockstep identity for this exact engine/content/math build. */
  readonly compatibility: CompatibilityVersion;
  /**
   * Monotonically increasing revision of the *asset set*. Together with
   * `engineVersion` it forms the immutable local storage root, so publishing
   * a new revision never reuses a previous revision's cached files and the
   * superseded root can simply be dropped.
   */
  readonly assetsRevision: number;
  /** Origin (plus optional path prefix) the launcher is allowed to download
   * assets from. Must be `https://`, carry no credentials, query, fragment, or
   * dot segments, and must not end with a slash. Every asset URL is resolved
   * against it and rejected if it escapes this prefix. */
  readonly assetBaseUrl: string;
  /** Every asset required before the engine can start. */
  readonly assets: readonly ManifestAsset[];
}

export interface ManifestValidationError {
  readonly path: string;
  readonly message: string;
}

export interface ManifestValidationResult {
  readonly valid: boolean;
  readonly errors: readonly ManifestValidationError[];
}

function isPlainObject(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isSafeRelativePath(path: string): boolean {
  if (path.length === 0 || path.length > 512) return false;
  if (path.startsWith("/")) return false;
  if (path.includes("\\")) return false;
  if (path.includes("..")) return false;
  if (path.includes("//")) return false;
  if (/^[a-z][a-z0-9+.-]*:\/\//i.test(path)) return false; // absolute URL
  if (CONTROL_CHARS_RE.test(path)) return false;
  return true;
}

function isSafeMountTarget(target: string): boolean {
  if (target.length < 2 || target.length > 512) return false;
  if (!target.startsWith("/")) return false;
  if (target.endsWith("/")) return false;
  if (target.includes("\\")) return false;
  if (target.includes("//")) return false;
  if (CONTROL_CHARS_RE.test(target)) return false;
  const segments = target.slice(1).split("/");
  return segments.every((segment) => segment !== "" && segment !== "." && segment !== "..");
}

/** Accepts an `https://` origin with an optional, canonical path prefix (one
 * key prefix per published revision) and nothing else: no credentials, query,
 * fragment, dot segments, or trailing slash. The launcher appends exactly one
 * slash before resolving an asset path against this value, so a trailing slash
 * here would silently widen the confinement check. */
function isAllowedAssetBaseUrl(url: string): boolean {
  let parsed: URL;
  try {
    parsed = new URL(url);
  } catch {
    return false;
  }
  if (parsed.protocol !== "https:") return false;
  if (parsed.search !== "" || parsed.hash !== "") return false;
  if (parsed.username !== "" || parsed.password !== "") return false;
  if (parsed.pathname !== "/") {
    if (parsed.pathname.endsWith("/")) return false;
    const segments = parsed.pathname.slice(1).split("/");
    if (segments.some((segment) => segment === "" || segment === "." || segment === "..")) {
      return false;
    }
  }
  const canonical = parsed.pathname === "/" ? parsed.origin : `${parsed.origin}${parsed.pathname}`;
  return url === canonical;
}

function validateMount(
  mount: unknown,
  role: AssetRole | undefined,
  assetPath: string,
  errors: ManifestValidationError[],
): { target?: string; order?: number } {
  if (!isPlainObject(mount)) {
    errors.push({ path: `${assetPath}.mount`, message: "must be an object" });
    return {};
  }

  const result: { target?: string; order?: number } = {};

  if (typeof mount.target !== "string" || !isSafeMountTarget(mount.target)) {
    errors.push({
      path: `${assetPath}.mount.target`,
      message: "must be an absolute in-engine path without '..', '//', or backslashes",
    });
  } else {
    result.target = mount.target;
  }

  if (typeof mount.order !== "number" || !Number.isInteger(mount.order) || mount.order < 0) {
    errors.push({ path: `${assetPath}.mount.order`, message: "must be a non-negative integer" });
  } else {
    result.order = mount.order;
  }

  if (typeof mount.streaming !== "boolean") {
    errors.push({ path: `${assetPath}.mount.streaming`, message: "must be a boolean" });
  } else if (role !== undefined && mount.streaming !== isStreamingRole(role)) {
    errors.push({
      path: `${assetPath}.mount.streaming`,
      message: `must be ${String(isStreamingRole(role))} for role "${role}"`,
    });
  }

  return result;
}

/**
 * Validates an untrusted manifest document (e.g. fetched JSON) against the
 * {@link EngineManifest} contract. Returns every violation found rather than
 * stopping at the first error, so operators can fix a manifest in one pass.
 */
export function validateManifest(input: unknown): ManifestValidationResult {
  const errors: ManifestValidationError[] = [];

  if (!isPlainObject(input)) {
    return { valid: false, errors: [{ path: "$", message: "manifest must be a JSON object" }] };
  }

  if (input.schemaVersion !== 3) {
    errors.push({ path: "schemaVersion", message: "must equal 3" });
  }

  if (typeof input.engineVersion !== "string" || !VERSION_RE.test(input.engineVersion)) {
    errors.push({
      path: "engineVersion",
      message: "must be a short identifier matching [A-Za-z0-9][A-Za-z0-9._-]{0,63}",
    });
  }

  if (!isCompatibilityVersion(input.compatibility)) {
    errors.push({
      path: "compatibility",
      message: "must contain positive integer engine, protocol, and determinism versions",
    });
  }

  if (
    typeof input.assetsRevision !== "number" ||
    !Number.isInteger(input.assetsRevision) ||
    input.assetsRevision < 1
  ) {
    errors.push({ path: "assetsRevision", message: "must be a positive integer" });
  }

  if (typeof input.assetBaseUrl !== "string" || !isAllowedAssetBaseUrl(input.assetBaseUrl)) {
    errors.push({
      path: "assetBaseUrl",
      message:
        "must be an https:// origin, optionally with a canonical path prefix, " +
        "and no credentials, query, fragment, dot segments, or trailing slash",
    });
  }

  if (!Array.isArray(input.assets)) {
    errors.push({ path: "assets", message: "must be an array" });
    return { valid: false, errors };
  }

  if (input.assets.length === 0) {
    errors.push({ path: "assets", message: "must contain at least one asset" });
  }

  const seenPaths = new Set<string>();
  const seenTargets = new Set<string>();
  const seenOrders = new Set<number>();
  const roleCounts = new Map<AssetRole, number>();
  const baseOrders: number[] = [];
  const expansionOrders: number[] = [];
  let totalBytes = 0;

  input.assets.forEach((asset: unknown, index: number) => {
    const assetPath = `assets[${index}]`;
    if (!isPlainObject(asset)) {
      errors.push({ path: assetPath, message: "must be an object" });
      return;
    }

    if (typeof asset.path !== "string" || !isSafeRelativePath(asset.path)) {
      errors.push({ path: `${assetPath}.path`, message: "must be a safe relative path" });
    } else if (seenPaths.has(asset.path)) {
      errors.push({ path: `${assetPath}.path`, message: `duplicate asset path: ${asset.path}` });
    } else {
      seenPaths.add(asset.path);
    }

    let role: AssetRole | undefined;
    if (!isAssetRole(asset.role)) {
      errors.push({
        path: `${assetPath}.role`,
        message: `must be one of: ${ASSET_ROLES.join(", ")}`,
      });
    } else {
      role = asset.role;
      roleCounts.set(role, (roleCounts.get(role) ?? 0) + 1);
    }

    if (
      typeof asset.sizeBytes !== "number" ||
      !Number.isInteger(asset.sizeBytes) ||
      asset.sizeBytes < 1 ||
      asset.sizeBytes > MAX_ASSET_BYTES
    ) {
      errors.push({
        path: `${assetPath}.sizeBytes`,
        message: `must be an integer between 1 and ${MAX_ASSET_BYTES}`,
      });
    } else {
      totalBytes += asset.sizeBytes;
    }

    if (typeof asset.sha256 !== "string" || !SHA256_HEX_RE.test(asset.sha256)) {
      errors.push({
        path: `${assetPath}.sha256`,
        message: "must be a 64-character lowercase hex SHA-256 digest",
      });
    }

    if (asset.etag !== undefined && (typeof asset.etag !== "string" || !STRONG_ETAG_RE.test(asset.etag))) {
      errors.push({
        path: `${assetPath}.etag`,
        message: 'must be a quoted strong entity tag (weak "W/" tags are rejected)',
      });
    }

    const mount = validateMount(asset.mount, role, assetPath, errors);
    if (mount.target !== undefined) {
      if (seenTargets.has(mount.target)) {
        errors.push({
          path: `${assetPath}.mount.target`,
          message: `duplicate mount target: ${mount.target}`,
        });
      } else {
        seenTargets.add(mount.target);
      }
    }
    if (mount.order !== undefined) {
      if (seenOrders.has(mount.order)) {
        errors.push({
          path: `${assetPath}.mount.order`,
          message: `duplicate mount order: ${mount.order}`,
        });
      } else {
        seenOrders.add(mount.order);
      }
      if (role === "big-base") baseOrders.push(mount.order);
      if (role === "big-expansion") expansionOrders.push(mount.order);
    }
  });

  for (const role of SINGLETON_ROLES) {
    const count = roleCounts.get(role) ?? 0;
    if (count !== 1) {
      errors.push({
        path: "assets",
        message: `must contain exactly one "${role}" asset (found ${count})`,
      });
    }
  }

  for (const role of OPTIONAL_SINGLETON_ROLES) {
    if ((roleCounts.get(role) ?? 0) > 1) {
      errors.push({ path: "assets", message: `must contain at most one "${role}" asset` });
    }
  }

  if ((roleCounts.get("big-base") ?? 0) === 0) {
    errors.push({ path: "assets", message: 'must contain at least one "big-base" asset' });
  }

  if (expansionOrders.length > 0 && baseOrders.length > 0) {
    const maxBase = Math.max(...baseOrders);
    const minExpansion = Math.min(...expansionOrders);
    if (minExpansion < maxBase) {
      errors.push({
        path: "assets",
        message: 'every "big-expansion" mount.order must exceed every "big-base" mount.order',
      });
    }
  }

  if (totalBytes > MAX_MANIFEST_BYTES) {
    errors.push({
      path: "assets",
      message: `total manifest size must not exceed ${MAX_MANIFEST_BYTES} bytes`,
    });
  }

  return { valid: errors.length === 0, errors };
}

/** Type guard combining {@link validateManifest} with a narrowing return type. */
export function isEngineManifest(input: unknown): input is EngineManifest {
  return validateManifest(input).valid;
}

/** Total byte size of every asset described by the manifest. */
export function totalManifestBytes(manifest: EngineManifest): number {
  return manifest.assets.reduce((sum, asset) => sum + asset.sizeBytes, 0);
}

/** Every asset with the given role, in manifest order. */
export function assetsByRole(manifest: EngineManifest, role: AssetRole): readonly ManifestAsset[] {
  return manifest.assets.filter((asset) => asset.role === role);
}

/** The first asset with the given role, or `undefined` when absent. */
export function assetByRole(manifest: EngineManifest, role: AssetRole): ManifestAsset | undefined {
  return manifest.assets.find((asset) => asset.role === role);
}

/**
 * Assets ordered by `mount.order`. This is the exact order in which the
 * launcher mounts files into the engine VFS, so base archives always precede
 * the expansion archives that override them.
 */
export function mountPlan(manifest: EngineManifest): readonly ManifestAsset[] {
  return [...manifest.assets].sort((a, b) => a.mount.order - b.mount.order);
}

/**
 * Immutable local storage root for a manifest revision. Cached files live
 * under this root, so a new engine build or asset revision never reuses (or
 * silently inherits) a previous revision's bytes.
 */
export function storageRoot(manifest: EngineManifest): string {
  return `${manifest.engineVersion}-r${manifest.assetsRevision}`;
}

/**
 * Immutable, content-addressed cache key for one asset. Because the key is
 * derived solely from the verified digest, identical bytes are stored once
 * and a changed file can never collide with a stale cache entry.
 */
export function assetCacheKey(asset: ManifestAsset): string {
  return asset.sha256;
}
