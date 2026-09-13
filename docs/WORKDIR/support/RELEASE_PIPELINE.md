# Release Pipeline

This workflow runs Linux, macOS, and Windows builds for Zero Hour and Generals base, collects bundles, generates release notes from local pull requests, and creates a GitHub release with automated Semantic Versioning (SemVer).

## Inputs

| Input | Type | Default | Description |
|-------|------|---------|-------------|
| `release_mode` | choice | `Draft` | Release mode: `Draft`, `New Release`, or `Dry Run` |

> [!NOTE]
> The release version is **not** prompted manually. It is automatically computed by `scripts/tooling/release/calculate_next_version.sh` using `base-version.txt` and existing git tags (e.g. `1.0` -> `1.0.0`, `1.0.1`, etc.).

## Behavior

1. Resolves the next release version from `base-version.txt` and existing repository tags via `scripts/tooling/release/calculate_next_version.sh`.
2. Creates and pushes the git tag before starting builds (skipped on `Dry Run`).
3. Runs Linux Flatpak builds for Zero Hour and Generals base (`linux64-deploy`).
4. Runs macOS app bundle builds for Zero Hour and Generals base (`macos-vulkan`).
5. Runs Windows executable builds for Zero Hour and Generals base.
6. Downloads generated bundle artifacts from all platform/game jobs.
7. Produces release assets:
   - `Linux-GeneralsX.flatpak`
   - `Linux-GeneralsXZH.flatpak`
   - `macOS-GeneralsX.tar.zip`
   - `macOS-GeneralsXZH.tar.zip`
   - `Windows-GeneralsX.zip`
   - `Windows-GeneralsXZH.zip`
8. Generates release notes with install instructions, community ports, and local PR changelog.
9. If `release_mode=Dry Run`, uploads a preview artifact `${version}-notes.txt` and avoids pushing tags or creating releases.
10. If `release_mode=Draft` (default), creates a draft GitHub release with attached assets so it can be reviewed and polished via `.github/prompts/prepare-release-draft.prompt.md`.
11. If `release_mode=New Release`, publishes the GitHub release immediately.

## Notes Format

Fixed block:

```markdown
> If you run into any problems, please [open an issue](https://github.com/fbraz3/GeneralsX/issues) so we can investigate.

# Getting Started

Follow the [Installation Guide](https://github.com/fbraz3/GeneralsX/blob/main/docs/HOWTO/INSTALLATION.md) to set up GeneralsX on your platform.
```

## Recommended Usage

1. Ensure `base-version.txt` contains the desired `MAJOR.MINOR` target (e.g., `1.0`).
2. Trigger the `Release Pipeline` workflow with default mode (`Draft`).
3. Once the workflow completes, use the `prepare-release-draft` custom prompt to polish the draft's highlights and release announcement.
4. Review and publish the draft in GitHub Releases.

