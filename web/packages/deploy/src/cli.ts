/**
 * CLI entry point for the post-deploy smoke tests.
 *
 * Usage (from `web/`):
 *   npm run smoke -w @generalsx-web/deploy
 *   npm run smoke -w @generalsx-web/deploy -- --release <sha>
 *   npm run smoke -w @generalsx-web/deploy -- --launcher https://<preview>.pages.dev
 *   npm run smoke -w @generalsx-web/deploy -- --no-retry        # already-live deployment
 *
 * Exits non-zero when any check fails, so it can gate a deployment step.
 */
import { DEFAULT_RETRY_POLICY, NO_RETRY_POLICY, type RetryPolicy } from "./retry.ts";
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
  --release <sha>        Require both surfaces to report this release id

Propagation retry (DNS/TLS/custom-domain rollout):
  --retry-attempts <n>   Attempts per origin       (default ${DEFAULT_RETRY_POLICY.attempts})
  --retry-budget <sec>   Total seconds spent waiting across the run (default ${DEFAULT_RETRY_POLICY.totalBudgetMs / 1000})
  --no-retry             Fail on the first propagation-shaped error
  -h, --help             Show this help
`;

interface ParsedArgs {
  readonly options: SmokeOptions;
  readonly retryPolicy: RetryPolicy;
}

function positiveNumber(flag: string, raw: string): number {
  const value = Number(raw);
  if (!Number.isFinite(value) || value <= 0) throw new Error(`${flag} requires a positive number, got "${raw}"`);
  return value;
}

function parseArgs(argv: readonly string[]): ParsedArgs | null {
  const overrides: Record<string, unknown> = {};
  let retryPolicy: RetryPolicy = DEFAULT_RETRY_POLICY;
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
        overrides.checkAssetOrigin = false;
        break;
      case "--release":
        overrides.expectReleaseId = next();
        break;
      case "--retry-attempts":
        retryPolicy = { ...retryPolicy, attempts: positiveNumber("--retry-attempts", next()) };
        break;
      case "--retry-budget":
        retryPolicy = { ...retryPolicy, totalBudgetMs: positiveNumber("--retry-budget", next()) * 1000 };
        break;
      case "--no-retry":
        retryPolicy = NO_RETRY_POLICY;
        break;
      default:
        throw new Error(`unknown argument: ${String(arg)}`);
    }
  }
  return {
    options: optionsFromTarget(PRODUCTION_TARGET, overrides as Partial<SmokeOptions>),
    retryPolicy,
  };
}

const parsed = parseArgs(process.argv.slice(2));
if (parsed === null) {
  console.log(USAGE);
  process.exit(0);
}
const { options, retryPolicy } = parsed;

console.log(`launcher   ${options.launcherOrigin}`);
console.log(`signaling  ${options.signalingOrigin}`);
console.log(
  `assets     ${options.checkAssetOrigin === false ? "(skipped)" : (options.assetOrigin ?? "(not configured)")}`,
);
console.log(
  retryPolicy.attempts > 1
    ? `retry      up to ${retryPolicy.attempts} attempts per origin, ${retryPolicy.totalBudgetMs / 1000}s total budget`
    : "retry      disabled",
);
console.log("");

const report = await runSmokeChecks(options, { retryPolicy });
console.log(formatSmokeReport(report));
process.exit(report.passed ? 0 : 1);
