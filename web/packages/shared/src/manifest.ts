/**
 * Engine + asset manifest schema shared by the launcher and the Cloudflare
 * Worker backend.
 *
 * IMPORTANT: This module never embeds, references, or ships retail game
 * assets. It only describes the *shape* of a manifest document that is
 * published separately (e.g. to an R2 bucket or authorized CDN) by an
 * operator who holds a legitimate license for the referenced files. The
 * manifest itself only carries file names, sizes and integrity hashes.
 */

/** SHA-256 hex digest, always 64 lowercase hex characters. */
export type Sha256Hex = string;

const SHA256_HEX_RE = /^[0-9a-f]{64}$/;

/** A single downloadable asset entry described by the manifest. */
export interface ManifestAsset {
  /** Path relative to the manifest's configured asset base URL. Must not
   * contain `..`, a leading slash, or an absolute URL. */
  readonly path: string;
  /** Exact byte size of the asset, used for progress reporting and to
   * bound the download before the integrity check completes. */
  readonly sizeBytes: number;
  /** SHA-256 hex digest of the asset bytes. Verified client-side before
   * the asset is written to the persistent cache. */
  readonly sha256: Sha256Hex;
}

/** Immutable engine + asset manifest consumed by the launcher shell. */
export interface EngineManifest {
  /** Manifest schema version, bumped on breaking changes. */
  readonly schemaVersion: 1;
  /** Human-readable engine build identifier (e.g. git short SHA or tag). */
  readonly engineVersion: string;
  /** Origin the launcher is allowed to download assets from. Must be an
   * `https://` origin with no path, query, or trailing slash. */
  readonly assetBaseUrl: string;
  /** Path (relative to `assetBaseUrl`) to the Emscripten-generated loader
   * script (the `.js` glue file produced by the build). */
  readonly engineEntry: string;
  /** Ordered list of assets required before the engine can start. */
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
  if (path.startsWith("/") || path.startsWith("\\")) return false;
  if (path.includes("..")) return false;
  if (/^[a-z][a-z0-9+.-]*:\/\//i.test(path)) return false; // absolute URL
  return true;
}

function isHttpsOriginOnly(url: string): boolean {
  try {
    const parsed = new URL(url);
    if (parsed.protocol !== "https:") return false;
    if (parsed.pathname !== "" && parsed.pathname !== "/") return false;
    if (parsed.search || parsed.hash) return false;
    return true;
  } catch {
    return false;
  }
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

  if (input.schemaVersion !== 1) {
    errors.push({ path: "schemaVersion", message: "must equal 1" });
  }

  if (typeof input.engineVersion !== "string" || input.engineVersion.trim().length === 0) {
    errors.push({ path: "engineVersion", message: "must be a non-empty string" });
  }

  if (typeof input.assetBaseUrl !== "string" || !isHttpsOriginOnly(input.assetBaseUrl)) {
    errors.push({
      path: "assetBaseUrl",
      message: "must be an https:// origin with no path, query, or fragment",
    });
  }

  if (typeof input.engineEntry !== "string" || !isSafeRelativePath(input.engineEntry)) {
    errors.push({
      path: "engineEntry",
      message: "must be a safe relative path (no '..', no leading slash, no absolute URL)",
    });
  }

  if (!Array.isArray(input.assets)) {
    errors.push({ path: "assets", message: "must be an array" });
  } else {
    const seenPaths = new Set<string>();
    input.assets.forEach((asset, index) => {
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
      if (
        typeof asset.sizeBytes !== "number" ||
        !Number.isInteger(asset.sizeBytes) ||
        asset.sizeBytes < 0
      ) {
        errors.push({ path: `${assetPath}.sizeBytes`, message: "must be a non-negative integer" });
      }
      if (typeof asset.sha256 !== "string" || !SHA256_HEX_RE.test(asset.sha256)) {
        errors.push({
          path: `${assetPath}.sha256`,
          message: "must be a 64-character lowercase hex SHA-256 digest",
        });
      }
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
