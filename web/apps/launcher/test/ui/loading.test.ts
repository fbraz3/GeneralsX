// @vitest-environment happy-dom
import { describe, expect, it } from "vitest";
import { createLoadingOverlay } from "../../src/ui/loading.js";

describe("createLoadingOverlay", () => {
  it("mounts hidden by default with the initial status text", () => {
    const container = document.createElement("div");
    const overlay = createLoadingOverlay(container);

    expect(container.contains(overlay.element)).toBe(true);
    expect(overlay.element.getAttribute("role")).toBe("status");
    expect(overlay.element.querySelector(".gx-loading-status")?.textContent).toBe("Loading engine…");
  });

  it("show/hide toggle the hidden attribute", () => {
    const overlay = createLoadingOverlay(document.createElement("div"));

    overlay.show();
    expect(overlay.element.hidden).toBe(false);

    overlay.hide();
    expect(overlay.element.hidden).toBe(true);
  });

  it("setStatus updates the visible status text", () => {
    const overlay = createLoadingOverlay(document.createElement("div"));

    overlay.setStatus("Downloading engine.wasm…");
    expect(overlay.element.querySelector(".gx-loading-status")?.textContent).toBe("Downloading engine.wasm…");
  });

  it("setProgress clamps the fill width to the [0, 1] range", () => {
    const overlay = createLoadingOverlay(document.createElement("div"));
    const fill = overlay.element.querySelector(".gx-progress-fill") as HTMLElement;

    overlay.setProgress(0.5);
    expect(fill.style.width).toBe("50.0%");

    overlay.setProgress(-1);
    expect(fill.style.width).toBe("0.0%");

    overlay.setProgress(2);
    expect(fill.style.width).toBe("100.0%");
  });
});
