import { describe, expect, it, vi } from "vitest";
import { loadEngineManifest } from "../src/engine-manifest.js";

const validManifest = {
  schemaVersion: 1,
  engineVersion: "test-build",
  assetBaseUrl: "https://assets.generalsx.org",
  engineEntry: "engine.js",
  assets: [],
};

describe("loadEngineManifest", () => {
  it("returns the parsed manifest when it is valid", async () => {
    const fetchImpl = vi.fn(async () => new Response(JSON.stringify(validManifest), { status: 200 }));
    const result = await loadEngineManifest(
      "https://assets.generalsx.org/manifest.json",
      fetchImpl as unknown as typeof fetch,
    );
    expect(result.ok).toBe(true);
  });

  it("rejects a manifest that fails schema validation", async () => {
    const fetchImpl = vi.fn(
      async () => new Response(JSON.stringify({ ...validManifest, schemaVersion: 99 }), { status: 200 }),
    );
    const result = await loadEngineManifest(
      "https://assets.generalsx.org/manifest.json",
      fetchImpl as unknown as typeof fetch,
    );
    expect(result.ok).toBe(false);
  });

  it("reports a non-2xx HTTP status without throwing", async () => {
    const fetchImpl = vi.fn(async () => new Response("nope", { status: 503 }));
    const result = await loadEngineManifest(
      "https://assets.generalsx.org/manifest.json",
      fetchImpl as unknown as typeof fetch,
    );
    expect(result.ok).toBe(false);
    if (!result.ok) expect(result.reason).toContain("503");
  });

  it("reports malformed JSON without throwing", async () => {
    const fetchImpl = vi.fn(async () => new Response("{not json", { status: 200 }));
    const result = await loadEngineManifest(
      "https://assets.generalsx.org/manifest.json",
      fetchImpl as unknown as typeof fetch,
    );
    expect(result.ok).toBe(false);
  });

  it("reports a network failure without throwing", async () => {
    const fetchImpl = vi.fn(async () => {
      throw new Error("offline");
    });
    const result = await loadEngineManifest(
      "https://assets.generalsx.org/manifest.json",
      fetchImpl as unknown as typeof fetch,
    );
    expect(result.ok).toBe(false);
  });
});
