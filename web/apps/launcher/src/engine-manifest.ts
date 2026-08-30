/**
 * Fetches and validates the engine manifest referenced by `LauncherConfig`.
 * The manifest is untrusted network input until `validateManifest` passes,
 * even though it is served from an operator-controlled origin.
 */
import { validateManifest, type EngineManifest } from "@generalsx-web/shared/manifest";

export type ManifestLoadResult =
  | { readonly ok: true; readonly manifest: EngineManifest }
  | { readonly ok: false; readonly reason: string };

export async function loadEngineManifest(
  manifestUrl: string,
  // Bound rather than a bare reference: real browsers brand-check `fetch`'s
  // `this` value, so an unbound reference throws "Illegal invocation" the
  // moment it's invoked through a detached local variable (as it always is
  // here, via the `fetchImpl` parameter).
  fetchImpl: typeof fetch = fetch.bind(globalThis),
): Promise<ManifestLoadResult> {
  let response: Response;
  try {
    response = await fetchImpl(manifestUrl, { cache: "no-store" });
  } catch {
    return { ok: false, reason: "network error fetching the engine manifest" };
  }
  if (!response.ok) {
    return { ok: false, reason: `manifest request failed with status ${response.status}` };
  }

  let body: unknown;
  try {
    body = await response.json();
  } catch {
    return { ok: false, reason: "manifest response was not valid JSON" };
  }

  const result = validateManifest(body);
  if (!result.valid) {
    const details = result.errors.map((e) => `${e.path}: ${e.message}`).join("; ");
    return { ok: false, reason: `manifest failed validation: ${details}` };
  }

  return { ok: true, manifest: body as EngineManifest };
}
