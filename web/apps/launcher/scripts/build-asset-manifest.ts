/**
 * Operator tool: builds an `EngineManifest` from a locally staged, licensed
 * copy of the game data plus the wasm engine build output.
 *
 * This script reads files from a directory *you* own and prints/writes a
 * manifest describing them. It never uploads anything, never embeds asset
 * bytes, and must never be pointed at a directory inside this repository:
 * retail assets are not distributed here and must not be committed.
 *
 * Usage:
 *   node scripts/build-asset-manifest.ts \
 *     --source /path/to/staging \
 *     --engine-version 2026.08.30-a1b2c3d \
 *     --assets-revision 7 \
 *     --base-url https://assets.generalsx.org \
 *     [--mount-prefix /generalsx] [--out manifest.json]
 *
 * Expected staging layout (see `inferAssetRole`):
 *   engine/generalsxzh.js, engine/generalsxzh.wasm, engine/generalsxzh.data
 *   base/*.big        expansion/*.big
 *   scripts/*.ini|*.scb     fonts/*.ttf
 */
import { createReadStream } from "node:fs";
import { readdir, stat, writeFile } from "node:fs/promises";
import { join, relative, sep } from "node:path";
import { argv, exit, stdout } from "node:process";
import { buildManifest, type SourceFile } from "@generalsx-web/shared/asset-plan";
import { Sha256Stream } from "@generalsx-web/shared/sha256";

interface CliOptions {
  source: string;
  engineVersion: string;
  assetsRevision: number;
  baseUrl: string;
  mountPrefix?: string;
  out?: string;
}

function parseArgs(args: readonly string[]): CliOptions {
  const values = new Map<string, string>();
  for (let i = 0; i < args.length; i += 2) {
    const flag = args[i];
    const value = args[i + 1];
    if (flag === undefined || !flag.startsWith("--") || value === undefined) {
      throw new Error(`malformed argument near "${flag ?? ""}"`);
    }
    values.set(flag.slice(2), value);
  }

  const required = (name: string): string => {
    const value = values.get(name);
    if (value === undefined || value.length === 0) throw new Error(`missing required --${name}`);
    return value;
  };

  const revision = Number(required("assets-revision"));
  if (!Number.isInteger(revision) || revision < 1) {
    throw new Error("--assets-revision must be a positive integer");
  }

  const options: CliOptions = {
    source: required("source"),
    engineVersion: required("engine-version"),
    assetsRevision: revision,
    baseUrl: required("base-url"),
  };
  const mountPrefix = values.get("mount-prefix");
  if (mountPrefix !== undefined) options.mountPrefix = mountPrefix;
  const out = values.get("out");
  if (out !== undefined) options.out = out;
  return options;
}

/** Streams a file through the incremental hasher; memory stays constant. */
async function hashFile(path: string): Promise<string> {
  const hasher = new Sha256Stream();
  for await (const chunk of createReadStream(path, { highWaterMark: 4 * 1024 * 1024 })) {
    hasher.update(chunk as Uint8Array);
  }
  return hasher.hex();
}

async function* walk(root: string, current = root): AsyncGenerator<string> {
  for (const entry of await readdir(current, { withFileTypes: true })) {
    if (entry.name.startsWith(".")) continue;
    const full = join(current, entry.name);
    if (entry.isDirectory()) yield* walk(root, full);
    else if (entry.isFile()) yield full;
  }
}

async function main(): Promise<void> {
  const options = parseArgs(argv.slice(2));
  const files: SourceFile[] = [];

  for await (const absolute of walk(options.source)) {
    const relativePath = relative(options.source, absolute).split(sep).join("/");
    const info = await stat(absolute);
    files.push({
      relativePath,
      sizeBytes: info.size,
      sha256: await hashFile(absolute),
    });
  }

  const plan = buildManifest(files, {
    engineVersion: options.engineVersion,
    assetsRevision: options.assetsRevision,
    assetBaseUrl: options.baseUrl,
    ...(options.mountPrefix === undefined ? {} : { mountPrefix: options.mountPrefix }),
  });

  for (const path of plan.unclassified) {
    console.warn(`skipped (unknown role): ${path}`);
  }
  for (const error of plan.errors) {
    console.error(`invalid manifest at ${error.path}: ${error.message}`);
  }
  if (plan.errors.length > 0) {
    exit(1);
  }

  const json = `${JSON.stringify(plan.manifest, null, 2)}\n`;
  if (options.out === undefined) stdout.write(json);
  else await writeFile(options.out, json, "utf8");
}

await main().catch((error: unknown) => {
  console.error(error instanceof Error ? error.message : String(error));
  exit(1);
});
