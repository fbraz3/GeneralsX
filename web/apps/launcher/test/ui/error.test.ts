// @vitest-environment happy-dom
import { describe, expect, it, vi } from "vitest";
import { createErrorOverlay } from "../../src/ui/error.js";

describe("createErrorOverlay", () => {
  it("mounts hidden with an alert role", () => {
    const container = document.createElement("div");
    const overlay = createErrorOverlay(container);

    expect(container.contains(overlay.element)).toBe(true);
    expect(overlay.element.getAttribute("role")).toBe("alert");
    expect(overlay.element.hidden).toBe(true);
  });

  it("show() reveals the message and hides the retry button when no callback is given", () => {
    const overlay = createErrorOverlay(document.createElement("div"));

    overlay.show("manifest failed validation");

    expect(overlay.element.hidden).toBe(false);
    expect(overlay.element.querySelector(".gx-error-message")?.textContent).toBe("manifest failed validation");
    const retryButton = overlay.element.querySelector("button") as HTMLButtonElement;
    expect(retryButton.hidden).toBe(true);
  });

  it("show() with a retry callback reveals the button and wires the click handler", () => {
    const overlay = createErrorOverlay(document.createElement("div"));
    const onRetry = vi.fn();

    overlay.show("asset download failed", onRetry);
    const retryButton = overlay.element.querySelector("button") as HTMLButtonElement;

    expect(retryButton.hidden).toBe(false);
    retryButton.click();
    expect(onRetry).toHaveBeenCalledTimes(1);
  });

  it("hide() clears the retry handler so a stale callback cannot fire later", () => {
    const overlay = createErrorOverlay(document.createElement("div"));
    const onRetry = vi.fn();

    overlay.show("network error", onRetry);
    overlay.hide();

    expect(overlay.element.hidden).toBe(true);
    const retryButton = overlay.element.querySelector("button") as HTMLButtonElement;
    retryButton.click();
    expect(onRetry).not.toHaveBeenCalled();
  });
});
