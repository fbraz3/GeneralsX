/**
 * Playwright configuration for the launcher's fixture-based browser smoke
 * test (`e2e/`). This never touches retail game assets or a real Cloudflare
 * deployment: every network request the launcher makes to its configured
 * manifest/asset/signaling origins (see `src/config.ts`) is intercepted and
 * fulfilled by the test itself with synthetic, disposable fixture bytes
 * generated at test time (see `e2e/fixtures.ts`).
 *
 * The launcher's static build is served locally via `vite preview` against
 * the already-built `dist/` output (run `npm run build` first).
 */
import { defineConfig, devices } from "@playwright/test";

export default defineConfig({
  testDir: "./e2e",
  fullyParallel: true,
  forbidOnly: !!process.env["CI"],
  retries: process.env["CI"] ? 1 : 0,
  reporter: process.env["CI"] ? "github" : "list",
  use: {
    baseURL: "http://127.0.0.1:4173",
    trace: "retain-on-failure",
    actionTimeout: 10_000,
  },
  expect: {
    timeout: 10_000,
  },
  projects: [
    {
      name: "chromium",
      use: { ...devices["Desktop Chrome"] },
    },
  ],
  webServer: {
    command: "npm run build && npm run preview",
    url: "http://127.0.0.1:4173",
    reuseExistingServer: !process.env["CI"],
    timeout: 60_000,
  },
});
