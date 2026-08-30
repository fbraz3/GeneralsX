/**
 * Post-build step: writes the Cloudflare Pages `_headers` file into the
 * launcher's `dist/` output, using the same security-header policy the
 * Worker applies to its own responses (see `@generalsx-web/shared`).
 *
 * Run automatically by `npm run build` (see package.json). Node's built-in
 * TypeScript support (stable as of the Node version this project targets)
 * lets this run directly with `node`, no extra bundler/loader dependency.
 */
import { writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import {
  renderPagesCacheRules,
  renderPagesHeadersFile,
} from "@generalsx-web/shared/security-headers";

const outPath = fileURLToPath(new URL("../dist/_headers", import.meta.url));

const body =
  renderPagesHeadersFile({
    allowedOrigins: ["https://play.generalsx.org"],
    signalingOrigins: ["https://signaling.generalsx.org"],
    assetOrigins: ["https://assets.generalsx.org"],
  }) +
  "\n" +
  renderPagesCacheRules();

await writeFile(outPath, body, "utf8");
// eslint-disable-next-line no-console -- CLI build script output, not app code
console.log(`wrote ${outPath}`);
