// @vitest-environment happy-dom
import { describe, expect, it } from "vitest";
import { createGameCanvas } from "../../src/ui/canvas.js";

describe("createGameCanvas", () => {
  it("mounts exactly one canvas element into the container", () => {
    const container = document.createElement("div");
    const canvas = createGameCanvas(container);

    expect(canvas.tagName).toBe("CANVAS");
    expect(canvas.id).toBe("game-canvas");
    expect(container.querySelectorAll("canvas")).toHaveLength(1);
    expect(container.contains(canvas)).toBe(true);
  });

  it("sizes the backing store to at least 1x1 device pixels", () => {
    const container = document.createElement("div");
    const canvas = createGameCanvas(container);

    // happy-dom reports zero layout size (no real renderer), so the adapter
    // must clamp to a minimum of 1 rather than producing a 0x0 backing store.
    expect(canvas.width).toBeGreaterThanOrEqual(1);
    expect(canvas.height).toBeGreaterThanOrEqual(1);
  });
});
