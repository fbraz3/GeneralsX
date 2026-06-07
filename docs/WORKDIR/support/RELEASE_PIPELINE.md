# Release Pipeline

This workflow runs Linux and macOS builds for Zero Hour and Generals base, collects bundles, generates release notes from local pull requests, and optionally creates a GitHub release.

## Inputs

| Input | Type | Default | Description |
|-------|------|---------|-------------|
| `release_version` | string | — | Target tag/version (for example: `GeneralsX-Beta-3`) |
| `additional_notes` | string | empty | Optional extra notes section |
| `create_release` | boolean | false | When true, creates a GitHub release and uploads assets |
| `is_prerelease` | boolean | false | When true, marks the release as pre-release |
| `dry_run` | boolean | false | Forces no release creation; only generates notes and artifacts |

## Behavior

1. Runs Linux builds for Zero Hour and Generals base (`linux64-deploy`).
2. Runs macOS builds for Zero Hour and Generals base (`macos-vulkan`).
3. Downloads generated bundle artifacts from all platform/game jobs.
4. Produces release assets:
   - `linux-generalsx-linux64-bundle.zip`
   - `linux-generalsxzh-linux64-bundle.zip`
   - `macos-generalsx-app.tar.zip`
   - `macos-generalsxzh-app.tar.zip`
5. Generates release notes with fixed header text plus:
   - `## Additional Notes` (only if provided)
   - `## What's Changed`
   - `## New Contributors` (when applicable)
   - `**Full Changelog**` (when a previous tag exists)
6. Uses only pull requests associated with commits in `fbraz3/GeneralsX` (ignores upstream TheSuperHackers PRs).
7. If `dry_run=true` or `create_release=false`, uploads preview artifacts instead of creating a release.
8. Creates GitHub release only when `create_release=true` and `dry_run=false`.

Dry-run preview policy:
- Generates only one small preview file: `${release_version}-notes.txt`.
- Does not generate normalized zip assets for preview upload.
- Still validates build/download steps and notes generation logic.

## Notes Format

Fixed block:

```markdown
> This is a **beta** release. Some bugs are still expected. If you run into any problems, please [open an issue](https://github.com/fbraz3/GeneralsX/issues) so we can investigate.

# Install Instructions

https://github.com/fbraz3/GeneralsX/blob/main/docs/BUILD/INSTALLATION.md
```

What's changed format:

```markdown
## What's Changed

- $COMMIT_TITLE by @$AUTHOR in $PULL_REQUEST_URL
```

New contributors format:

```markdown
## New Contributors

* @$USERNAME made their first contribution in $PULL_REQUEST_URL
```

Full changelog format:

```markdown
**Full Changelog**: https://github.com/fbraz3/GeneralsX/compare/$LAST_TAG...$CURRENT_TAG
```

## Recommended Usage

1. First run with `create_release=false` to validate output artifacts.
2. Review generated markdown and zip files from workflow artifacts.
3. Run again with `create_release=true` (and optionally `is_prerelease=true`) to publish.
