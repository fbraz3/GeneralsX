/**
 * Typed errors raised by the asset pipeline.
 *
 * Every failure mode the launcher UI needs to distinguish (tampering, a
 * truncated transfer, a full disk, a user-cancelled boot) gets its own class
 * so callers never have to pattern-match on message text.
 */

/** Base class for every asset pipeline failure. */
export class AssetError extends Error {
  constructor(
    message: string,
    readonly path: string,
  ) {
    super(message);
    this.name = "AssetError";
  }
}

/** The downloaded bytes did not match the manifest's SHA-256 digest. */
export class AssetIntegrityError extends AssetError {
  constructor(
    path: string,
    readonly expectedSha256: string,
    readonly actualSha256: string,
  ) {
    super(`asset "${path}" failed integrity verification`, path);
    this.name = "AssetIntegrityError";
  }
}

/** The transfer delivered more or fewer bytes than the manifest declares. */
export class AssetSizeError extends AssetError {
  constructor(
    path: string,
    readonly expectedBytes: number,
    readonly actualBytes: number,
  ) {
    super(
      `asset "${path}" size mismatch: expected ${expectedBytes} bytes, got ${actualBytes} bytes`,
      path,
    );
    this.name = "AssetSizeError";
  }
}

/** A non-2xx response, or a response the range protocol could not validate. */
export class AssetDownloadError extends AssetError {
  constructor(
    path: string,
    message: string,
    readonly status?: number,
  ) {
    super(`asset "${path}" download failed: ${message}`, path);
    this.name = "AssetDownloadError";
  }
}

/** Browser storage refused the write: quota exceeded or storage unavailable. */
export class AssetStorageQuotaError extends AssetError {
  constructor(
    path: string,
    readonly requiredBytes?: number,
    readonly reason?: unknown,
  ) {
    super(
      `not enough browser storage available to cache asset "${path}"` +
        (requiredBytes === undefined ? "" : ` (${requiredBytes} bytes required)`),
      path,
    );
    this.name = "AssetStorageQuotaError";
  }
}

/** The caller aborted the download via an `AbortSignal`. */
export class AssetCancelledError extends AssetError {
  constructor(path: string) {
    super(`asset "${path}" download was cancelled`, path);
    this.name = "AssetCancelledError";
  }
}

/** A file the engine asked for is not present in the mounted VFS. */
export class AssetNotMountedError extends AssetError {
  constructor(target: string) {
    super(`no asset mounted at "${target}"`, target);
    this.name = "AssetNotMountedError";
  }
}

/** True for the `QuotaExceededError` DOMException raised by OPFS/IndexedDB. */
export function isQuotaExceeded(error: unknown): boolean {
  if (typeof error !== "object" || error === null) return false;
  const name = (error as { name?: unknown }).name;
  return name === "QuotaExceededError" || name === "NS_ERROR_FILE_NO_DEVICE_SPACE";
}
