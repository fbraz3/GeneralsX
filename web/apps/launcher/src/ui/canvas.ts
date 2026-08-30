/** Creates and mounts the WebGL/WebGPU render target canvas used by the
 * Emscripten engine module. Sizing follows the container via ResizeObserver
 * so the canvas always matches its CSS box in device pixels. */
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
  observer.observe(canvas);
  resize();

  return canvas;
}
