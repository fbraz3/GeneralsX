const resizeObservers = new WeakMap<HTMLCanvasElement, ResizeObserver>();

/** Creates and mounts the WebGL/WebGPU render target canvas used by the
 * Emscripten engine module. Sizing follows the container until the engine
 * claims the backing store. */
export function createGameCanvas(container: HTMLElement): HTMLCanvasElement {
  const canvas = document.createElement("canvas");
  // d8web creates its WebGL2 context against the Emscripten-standard
  // `#canvas` selector.
  canvas.id = "canvas";
  canvas.className = "gx-canvas";
  container.appendChild(canvas);

  const resize = (): void => {
    const ratio = window.devicePixelRatio || 1;
    canvas.width = Math.max(1, Math.round(canvas.clientWidth * ratio));
    canvas.height = Math.max(1, Math.round(canvas.clientHeight * ratio));
  };

  const observer = new ResizeObserver(resize);
  resizeObservers.set(canvas, observer);
  observer.observe(canvas);
  resize();

  return canvas;
}

/** Keeps the engine's fixed-size backbuffer stable while CSS scales the
 * canvas to later viewport changes. Reallocating the backing store after
 * WebGL startup breaks rendering and pointer coordinates. */
export function lockGameCanvasSize(canvas: HTMLCanvasElement): void {
  resizeObservers.get(canvas)?.disconnect();
  resizeObservers.delete(canvas);
}
