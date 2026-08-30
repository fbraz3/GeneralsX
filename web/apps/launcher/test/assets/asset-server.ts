/**
 * Mocked asset origin used by the asset pipeline tests.
 *
 * Serves bytes over a real `ReadableStream` so the downloader's streaming,
 * range, resume, and cancellation paths are exercised end to end, and can be
 * configured to misbehave the way a real CDN sometimes does: ignoring
 * `Range`, returning a bogus `Content-Range`, lying about `Content-Length`,
 * dropping the connection mid-transfer, or serving different bytes than the
 * manifest promises.
 */
import { vi } from "vitest";

export interface ServedFile {
  bytes: Uint8Array;
  etag?: string;
  /** Honour `Range` requests (default true). */
  supportsRange?: boolean;
  /** Drop the connection after this many bytes of the current response. */
  failAfterBytes?: number;
  /** Bytes emitted per stream chunk (default 8). */
  chunkSize?: number;
  /** Force a status code, e.g. 404 or 416. */
  status?: number;
  /** Replace the `Content-Range` header with a broken value. */
  contentRangeOverride?: string;
  /** Send a `Content-Length` that disagrees with the body. */
  contentLengthOverride?: number;
  omitContentLength?: boolean;
  /** Override the `ETag` header independently of the manifest value. */
  etagHeaderOverride?: string;
}

/** Patch that may explicitly clear an optional knob with `undefined`. */
export type ServedFilePatch = { [K in keyof ServedFile]?: ServedFile[K] | undefined };

export interface RecordedRequest {
  readonly url: string;
  readonly range: string | null;
  readonly ifRange: string | null;
}

export interface AssetServer {
  readonly fetchImpl: typeof fetch;
  readonly requests: RecordedRequest[];
  /** Highest number of overlapping in-flight responses observed. */
  readonly peakConcurrency: () => number;
  update(path: string, patch: ServedFilePatch): void;
}

const BASE = "https://assets.generalsx.org/";

function headerValue(init: RequestInit | undefined, name: string): string | null {
  const headers = init?.headers;
  if (!headers) return null;
  if (headers instanceof Headers) return headers.get(name);
  return new Headers(headers as HeadersInit).get(name);
}

export function createAssetServer(files: Record<string, ServedFile>): AssetServer {
  const requests: RecordedRequest[] = [];
  let inFlight = 0;
  let peak = 0;

  const fetchImpl = vi.fn(async (input: RequestInfo | URL, init?: RequestInit): Promise<Response> => {
    const url = String(input);
    const range = headerValue(init, "range");
    const ifRange = headerValue(init, "if-range");
    requests.push({ url, range, ifRange });

    const path = url.startsWith(BASE) ? url.slice(BASE.length) : url;
    const file = files[path];
    if (!file) return new Response("not found", { status: 404 });
    if (file.status !== undefined && file.status !== 200 && file.status !== 206) {
      return new Response("error", { status: file.status });
    }

    const supportsRange = file.supportsRange !== false;
    const match = range ? /^bytes=(\d+)-/.exec(range) : null;
    const offset = match && supportsRange ? Number(match[1]) : 0;
    const partial = offset > 0;
    const body = file.bytes.subarray(offset);

    const headers = new Headers();
    if (file.etagHeaderOverride !== undefined) headers.set("ETag", file.etagHeaderOverride);
    else if (file.etag !== undefined) headers.set("ETag", file.etag);
    if (file.omitContentLength !== true) {
      headers.set("Content-Length", String(file.contentLengthOverride ?? body.length));
    }
    if (partial) {
      headers.set(
        "Content-Range",
        file.contentRangeOverride ?? `bytes ${offset}-${file.bytes.length - 1}/${file.bytes.length}`,
      );
    }
    headers.set("Accept-Ranges", "bytes");

    const chunkSize = file.chunkSize ?? 8;
    const failAfter = file.failAfterBytes;

    inFlight += 1;
    peak = Math.max(peak, inFlight);
    let released = false;
    const release = (): void => {
      if (released) return;
      released = true;
      inFlight -= 1;
    };

    let sent = 0;
    const stream = new ReadableStream<Uint8Array>({
      async pull(controller) {
        // Yield to the event loop so parallel downloads actually interleave.
        await Promise.resolve();
        if (failAfter !== undefined && sent >= failAfter) {
          release();
          controller.error(new Error("connection reset"));
          return;
        }
        if (sent >= body.length) {
          release();
          controller.close();
          return;
        }
        const end = Math.min(body.length, sent + chunkSize);
        const capped = failAfter === undefined ? end : Math.min(end, failAfter);
        controller.enqueue(body.slice(sent, capped));
        sent = capped;
      },
      cancel() {
        release();
      },
    });

    return new Response(stream, { status: partial ? 206 : 200, headers });
  });

  return {
    fetchImpl: fetchImpl as unknown as typeof fetch,
    requests,
    peakConcurrency: () => peak,
    update(path, patch) {
      const existing = files[path];
      if (!existing) throw new Error(`unknown served file ${path}`);
      const next: ServedFile = { ...existing };
      const mutable = next as unknown as Record<string, unknown>;
      for (const [key, value] of Object.entries(patch)) {
        if (value === undefined) delete mutable[key];
        else mutable[key] = value;
      }
      files[path] = next;
    },
  };
}
