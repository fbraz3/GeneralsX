import { defineConfig } from "vite";
import tailwindcss from "@tailwindcss/vite";

// The wasm module, its data and the game archives are produced by CMake / served from disk,
// so the bundler must leave that import alone and resolve it at runtime.
export default defineConfig({
  base: "./",
  plugins: [tailwindcss()],
  build: {
    outDir: "dist",
    emptyOutDir: true,
    target: "es2022",
    rollupOptions: { external: ["./GeneralsXZH.js", "/GeneralsXZH.js"] },
  },
});
