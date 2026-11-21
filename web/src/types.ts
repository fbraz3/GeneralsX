/** Minimal typings for the bits of the emscripten runtime this page actually touches. */

export interface FSStream {
  position: number;
}

export interface FSNode {
  stream_ops: {
    llseek: (stream: FSStream, offset: number, whence: number) => number;
    read: (
      stream: FSStream,
      buffer: Uint8Array,
      offset: number,
      length: number,
      position: number,
    ) => number;
  };
}

export interface EmscriptenFS {
  mkdirTree: (path: string) => void;
  createFile: (
    parent: string,
    name: string,
    properties: object,
    canRead: boolean,
    canWrite: boolean,
  ) => FSNode;
  readFile: (path: string, options: { encoding: "utf8" }) => string;
  writeFile: (path: string, data: string) => void;
  stat: (path: string) => unknown;
  mount: (type: unknown, options: object, path: string) => void;
  syncfs: (populate: boolean, callback: (error?: unknown) => void) => void;
  ErrnoError: new (code: number) => Error;
}

export interface EmscriptenModule {
  FS: EmscriptenFS;
  IDBFS: unknown;
  canvas: HTMLCanvasElement;
  addRunDependency: (id: string) => void;
  removeRunDependency: (id: string) => void;
  _GeneralsXLogicFrame?: () => number;
  _GeneralsXMouseX?: () => number;
  _GeneralsXMouseY?: () => number;
  _GeneralsXSetAudioMuted?: (muted: number) => number;
  _GeneralsXLanSetIdentity?: (client: number) => number;
  _GeneralsXPump?: () => void;
  ccall?: (name: string, ret: string | null, types: string[], args: unknown[]) => number;
}

export type ModuleFactory = (config: Record<string, unknown>) => Promise<EmscriptenModule>;
