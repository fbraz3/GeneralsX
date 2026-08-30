export interface ErrorOverlay {
  readonly element: HTMLElement;
  show(message: string, onRetry?: () => void): void;
  hide(): void;
}

export function createErrorOverlay(container: HTMLElement): ErrorOverlay {
  const element = document.createElement("div");
  element.className = "gx-overlay gx-error-overlay";
  element.setAttribute("role", "alert");
  element.hidden = true;

  const message = document.createElement("p");
  message.className = "gx-error-message";

  const retryButton = document.createElement("button");
  retryButton.type = "button";
  retryButton.className = "gx-button";
  retryButton.textContent = "Retry";
  retryButton.hidden = true;

  element.append(message, retryButton);
  container.appendChild(element);

  return {
    element,
    show(text: string, onRetry?: () => void) {
      message.textContent = text;
      retryButton.hidden = !onRetry;
      retryButton.onclick = onRetry ?? null;
      element.hidden = false;
    },
    hide() {
      element.hidden = true;
      retryButton.onclick = null;
    },
  };
}
