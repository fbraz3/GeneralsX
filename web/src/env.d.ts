declare module "/GeneralsXZH.js" {
  const factory: (config: Record<string, unknown>) => Promise<unknown>;
  export default factory;
}

interface FileSystemDirectoryHandle {
  queryPermission?: (descriptor?: { mode?: "read" | "readwrite" }) => Promise<PermissionState>;
  requestPermission?: (descriptor?: { mode?: "read" | "readwrite" }) => Promise<PermissionState>;
}
