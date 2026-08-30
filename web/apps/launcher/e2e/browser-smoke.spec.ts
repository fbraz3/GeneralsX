/**
 * Fixture-based browser smoke test for the GeneralsX launcher shell.
 *
 * This never contacts a real network origin, never deploys anything, and
 * never touches retail game data: every request the launcher makes to its
 * hardcoded manifest/asset origin (`https://assets.generalsx.org`, see
 * `src/config.ts`) is intercepted by Playwright and fulfilled with the
 * synthetic bytes from `./fixtures.ts`. This exercises the real,
 * currently-implemented browser code path — manifest fetch + validation,
 * integrity-checked asset download, and the loading/error overlay adapters
 * — inside an actual browser engine (Cache Storage API, `crypto.subtle`,
 * real DOM), which is exactly what jsdom/happy-dom-based unit tests cannot
 * do.
 *
 * Instantiating the actual Emscripten/WebAssembly engine module remains out
 * of scope (see `web/README.md`); this test only covers the launcher shell.
 */
import { expect, test, type Route } from "@playwright/test";
import { ASSET_ORIGIN, FIXTURE_ASSETS, buildFixtureManifest, buildTamperedFixtureManifest } from "./fixtures.js";

async function fulfillFixtureAssets(route: Route): Promise<void> {
  const url = new URL(route.request().url());
  const fileName = url.pathname.replace(/^\//, "");
  const asset = FIXTURE_ASSETS.find((a) => a.path === fileName);
  if (!asset) {
    await route.fulfill({ status: 404, body: "not found" });
    return;
  }
  await route.fulfill({ status: 200, contentType: "application/octet-stream", body: Buffer.from(asset.bytes) });
}

test.describe("launcher browser boot (fixture assets only)", () => {
  test("loads the engine manifest, downloads fixture assets, and clears the loading overlay", async ({ page }) => {
    await page.route(`${ASSET_ORIGIN}/manifest.json`, async (route) => {
      await route.fulfill({ status: 200, contentType: "application/json", body: JSON.stringify(buildFixtureManifest()) });
    });
    await page.route(`${ASSET_ORIGIN}/fixture-asset-*.bin`, fulfillFixtureAssets);

    await page.goto("/");

    // The room panel only renders once startEngineBoot() reaches the end of
    // its manifest+asset pipeline, so waiting for it is a reliable signal
    // that the whole fixture download/integrity-check path succeeded.
    await expect(page.locator(".gx-room-panel")).toBeVisible();
    await expect(page.locator(".gx-loading-overlay")).toBeHidden();
    await expect(page.locator(".gx-error-overlay")).toBeHidden();
  });

  test("shows the error overlay with a working retry when an asset fails its integrity check", async ({ page }) => {
    let manifestRequests = 0;
    await page.route(`${ASSET_ORIGIN}/manifest.json`, async (route) => {
      manifestRequests += 1;
      // First load: tampered digest (triggers AssetIntegrityError). Retry:
      // a valid manifest, proving the retry callback re-runs the whole
      // boot sequence rather than being a dead button.
      const manifest = manifestRequests === 1 ? buildTamperedFixtureManifest() : buildFixtureManifest();
      await route.fulfill({ status: 200, contentType: "application/json", body: JSON.stringify(manifest) });
    });
    await page.route(`${ASSET_ORIGIN}/fixture-asset-*.bin`, fulfillFixtureAssets);

    await page.goto("/");

    const errorOverlay = page.locator(".gx-error-overlay");
    await expect(errorOverlay).toBeVisible();
    await expect(errorOverlay.locator(".gx-error-message")).toContainText("integrity verification");

    await errorOverlay.locator("button").click();

    await expect(page.locator(".gx-room-panel")).toBeVisible();
    await expect(errorOverlay).toBeHidden();
  });

  test("shows the error overlay when the manifest itself fails validation", async ({ page }) => {
    await page.route(`${ASSET_ORIGIN}/manifest.json`, async (route) => {
      await route.fulfill({
        status: 200,
        contentType: "application/json",
        body: JSON.stringify({ schemaVersion: 99 }),
      });
    });

    await page.goto("/");

    const errorOverlay = page.locator(".gx-error-overlay");
    await expect(errorOverlay).toBeVisible();
    await expect(errorOverlay.locator(".gx-error-message")).toContainText("manifest failed validation");
  });
});
