/**
 * CLI entry point for the post-deploy smoke tests.
 *
 * Usage (from `web/`):
 *   npm run smoke -w @generalsx-web/deploy
 *   npm run smoke -w @generalsx-web/deploy -- --require-turn --release <sha>
 *   npm run smoke -w @generalsx-web/deploy -- --launcher https://<preview>.pages.dev
 *
 * Exits non-zero when any check fails, so it can gate a deployment step.
 */
import { formatSmokeReport, optionsFromTarget, runSmokeChecks, type SmokeOptions } from "./smoke.ts";
import { PRODUCTION_TARGET } from "./targets.ts";

/* eslint-disable no-console -- operator CLI: stdout *is* the interface */

const USAGE = `generalsx deployment smoke test

Options:
  --launcher <origin>    Launcher origin       (default ${PRODUCTION_TARGET.launcherOrigin})
  --signaling <origin>   Signaling origin      (default ${PRODUCTION_TARGET.signalingOrigin})
  --assets <origin>      Asset origin          (default ${PRODUCTION_TARGET.assetOrigin})
  --asset-path <path>    Object probed on the asset origin (default /manifest.json)
  --no-assets            Skip the asset-origin delivery checks
  --require-turn         Fail (not skip) if live TURN credentials cannot be issued
  --release <sha>        Require both surfaces to report this release id
  -h, --help             Show this help
`;

function parseArgs(argv: readonly string[]): SmokeOptions | null {
  const overrides: Record<string, unknown> = {};
  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];
    const next = (): string => {
      const value = argv[index + 1];
      if (value === undefined) throw new Error(`${String(arg)} requires a value`);
      index += 1;
      return value;
    };
    switch (arg) {
      case "-h":
      case "--help":
        return null;
      case "--launcher":
        overrides.launcherOrigin = next();
        break;
      case "--signaling":
        overrides.signalingOrigin = next();
        break;
      case "--assets":
        overrides.assetOrigin = next();
        break;
      case "--asset-path":
        overrides.assetProbePath = next();
        break;
      case "--no-assets":
        overrides.assetOrigin = undefined;
        break;
      case "--require-turn":
        overrides.requireTurn = true;
        break;
      case "--release":
        overrides.expectReleaseId = next();
        break;
      default:
        throw new Error(`unknown argument: ${String(arg)}`);
    }
  }
  return optionsFromTarget(PRODUCTION_TARGET, overrides as Partial<SmokeOptions>);
}

const options = parseArgs(process.argv.slice(2));
if (options === null) {
  console.log(USAGE);
  process.exit(0);
}

console.log(`launcher   ${options.launcherOrigin}`);
console.log(`signaling  ${options.signalingOrigin}`);
console.log(`assets     ${options.assetOrigin ?? "(skipped)"}`);
console.log("");

const report = await runSmokeChecks(options);
console.log(formatSmokeReport(report));
process.exit(report.passed ? 0 : 1);
