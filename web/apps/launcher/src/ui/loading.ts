export interface LoadingOverlay {
  readonly element: HTMLElement;
  setStatus(text: string): void;
  /** `fraction` is clamped to [0, 1]. */
  setProgress(fraction: number): void;
  show(): void;
  hide(): void;
}

export function createLoadingOverlay(container: HTMLElement): LoadingOverlay {
  const element = document.createElement("div");
  element.className = "gx-overlay gx-loading-overlay";
  element.setAttribute("role", "status");
  element.setAttribute("aria-live", "polite");

  const status = document.createElement("p");
  status.className = "gx-loading-status";
  status.textContent = "Loading engine…";

  const progressTrack = document.createElement("div");
  progressTrack.className = "gx-progress-track";
  const progressFill = document.createElement("div");
  progressFill.className = "gx-progress-fill";
  progressTrack.appendChild(progressFill);

  element.append(status, progressTrack);
  container.appendChild(element);

  return {
    element,
    setStatus(text: string) {
      status.textContent = text;
    },
    setProgress(fraction: number) {
      const clamped = Math.min(1, Math.max(0, fraction));
      progressFill.style.width = `${(clamped * 100).toFixed(1)}%`;
    },
    show() {
      element.hidden = false;
    },
    hide() {
      element.hidden = true;
    },
  };
}
