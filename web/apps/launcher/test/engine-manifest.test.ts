import { describe, expect, it, vi } from "vitest";
import { sha256Hex } from "@generalsx-web/shared/sha256";
import { loadEngineManifest } from "../src/engine-manifest.js";

const digest = (seed: string): string => sha256Hex(new TextEncoder().encode(seed));

const validManifest = {
  schemaVersion: 2,
  engineVersion: "test-build",
  assetsRevision: 1,
  assetBaseUrl: "https://assets.generalsx.org",
  assets: [
    {
      path: "engine/engine.js",
      role: "engine-js",
      sizeBytes: 128,
      sha256: digest("engine.js"),
      mount: { target: "/generalsx/engine/engine.js", order: 0, streaming: false },
    },
    {
      path: "engine/engine.wasm",
      role: "engine-wasm",
      sizeBytes: 4096,
      sha256: digest("engine.wasm"),
      mount: { target: "/generalsx/engine/engine.wasm", order: 1, streaming: false },
    },
    {
      path: "base/INI.big",
      role: "big-base",
      sizeBytes: 2048,
      sha256: digest("INI.big"),
      mount: { target: "/generalsx/base/INI.big", order: 100, streaming: true },
    },
  ],
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
