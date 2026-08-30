/**
 * Canonical production deployment targets for the GeneralsX browser
 * prototype. Every script and smoke check reads these constants so a hostname
 * is spelled out exactly once.
 *
 * These are public origins only. Nothing here is a secret, an account id, or a
 * zone id — those are supplied by the operator's environment at deploy time
 * (see `scripts/lib.sh`) and never committed.
 */

export interface DeploymentTarget {
  /** Cloudflare Pages project serving the static launcher shell. */
  readonly pagesProject: string;
  /** Cloudflare Worker serving room signaling + TURN credentials. */
  readonly workerName: string;
  /** R2 bucket holding the operator's own authorized assets. */
  readonly assetBucket: string;
  readonly launcherOrigin: string;
  readonly signalingOrigin: string;
  readonly assetOrigin: string;
  /** Branch Pages treats as production (its alias is the custom domain). */
  readonly productionBranch: string;
}

export const PRODUCTION_TARGET: DeploymentTarget = Object.freeze({
  pagesProject: "generalsx-launcher",
  workerName: "generalsx-signaling",
  assetBucket: "generalsx-web-assets",
  launcherOrigin: "https://play.generalsx.org",
  signalingOrigin: "https://signaling.generalsx.org",
  assetOrigin: "https://assets.generalsx.org",
  productionBranch: "main",
});

/** Security-header policy inputs for the production launcher origin. */
export function productionSecurityOptions(target: DeploymentTarget = PRODUCTION_TARGET) {
  return {
    allowedOrigins: [target.launcherOrigin],
    signalingOrigins: [target.signalingOrigin],
    assetOrigins: [target.assetOrigin],
  } as const;
}
