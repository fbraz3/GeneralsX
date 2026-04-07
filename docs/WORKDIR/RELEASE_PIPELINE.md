# Release Pipeline

Automated GitHub Actions workflow for creating GeneralsX releases with automatic changelog generation.

## Features

✅ **Manual Dispatch** — Trigger releases on-demand from GitHub Actions UI  
✅ **Auto Changelog** — Generates changelog from commits since latest release  
✅ **PR Linking** — Automatically links PRs to changelog entries  
✅ **New Contributors** — Detects and credits first-time contributors  
✅ **Build/Known Issues Notes** — Add structured release notes sections  
✅ **Custom Text** — Add personalized release notes  
✅ **Draft/Prerelease Options** — Mark releases appropriately  

## Quick Start

1. Go to **Actions** tab → **Release Pipeline**
2. Click **Run workflow** button
3. Fill in the required field:
   - **release_version**: Tag name (e.g., `GeneralsX-Beta-3`)
4. Optionally fill in sections:
   - **build_notes**: What changed in this build
   - **known_issues**: Known issues in this release
   - **custom_text**: Additional markdown
5. Configure flags if needed:
   - **is_draft**: Leave unchecked for public release
   - **is_prerelease**: Check to mark as prerelease (default: checked)
6. Click **Run workflow**

## Inputs Reference

| Input | Type | Default | Description |
|-------|------|---------|-------------|
| `release_version` | string | — | Release tag name (e.g., `GeneralsX-Beta-3`) |
| `build_notes` | string | — | Build notes section (markdown) |
| `known_issues` | string | — | Known issues section (markdown) |
| `custom_text` | string | — | Additional text to include (markdown) |
| `is_draft` | boolean | false | Create as draft (hidden from releases page) |
| `is_prerelease` | boolean | true | Mark as prerelease (not as latest) |
| `download_artifacts` | boolean | false | Attempt to download and attach artifacts |

## Examples

### Basic Release (just version)

```
release_version: GeneralsX-Beta-3
```

### Release with Build Notes

```
release_version: GeneralsX-Beta-3
build_notes: |
  - Fixed critical multiplayer desync issue
  - Improved Linux performance by 20%
  - Updated DXVK to v2.6
```

### Release with Known Issues

```
release_version: GeneralsX-Beta-3
build_notes: |
  - Fixed renderer crashes on macOS
  - Improved audio synchronization
known_issues: |
  - Replays from Beta-2 are not compatible
  - Some mods may require recompiling
```

### Full Featured Release

```
release_version: GeneralsX-Beta-3
build_notes: |
  - Fixed critical multiplayer desync
  - Updated DXVK to v2.6
  - Improved macOS bundle deployment
known_issues: |
  - Replays from Beta-2 not compatible
  - Some mods may need recompilation
custom_text: |
  ## Highlights
  
  This release focuses on stability and performance improvements.
is_draft: false
is_prerelease: true
```

## Workflow Behavior

1. **Validation**: Ensures version doesn't already exist
2. **Detection**: Finds latest release tag (GeneralsX-Beta-2, etc.)
3. **Changelog Generation**: 
   - Extracts commits since latest release using `git log`
   - Finds associated PRs in commit messages
   - Groups entries and adds PR author credits
4. **New Contributors**: Detects first-time contributors by comparing author lists
5. **Body Assembly**: 
   - Prepends custom text if provided
   - Adds build notes section if provided
   - Adds known issues section if provided
   - Includes beta warning
   - Includes install instructions link
   - Lists "What's Changed" with PR links
   - Credits new contributors
   - Links to full changelog comparison
6. **Preview**: Displays body in workflow logs for verification
7. **Creation**: Uses `gh release create` to publish release
8. **Summary**: Posts results to workflow summary in Actions tab

## About Artifacts

Currently, artifacts must be:
- Downloaded manually from successful build workflow artifacts
- Attached to the release manually via GitHub UI release edit page

To attach artifacts after release creation:
1. Go to the release page
2. Click **Edit** button
3. Scroll to "Attachments" section
4. Drag-drop or click to upload files

Future enhancement: Automatically download and attach artifacts from latest successful builds.

## Changelog Format

Each changelog entry follows this pattern:

```
* fix(component): description by @author in https://github.com/fbraz3/GeneralsX/pull/123
```

For commits without associated PRs:

```
* fix(component): description
```

## New Contributor Detection

The workflow attempts to identify first-time contributors by:
1. Comparing author lists before/after the latest release
2. Looking up GitHub usernames from PR metadata or author info
3. Creating contributor credit lines

Example output:

```
## New Contributors
* @NewContributor made their first contribution in https://github.com/fbraz3/GeneralsX/pull/99
```

## Troubleshooting

### "Tag already exists"
The version you specified already has a release. Use a different tag name.

### Changelog shows but PR links are missing
- Ensure commits reference PR numbers in message (e.g., `(#123)`)
- If PR numbers are not in message, manually add PR links to release notes
- Future enhancement: better PR detection patterns

### No new contributors detected
This is expected if:
- No new authors since latest release
- Or GitHub username couldn't be mapped to author name (manual add needed)

## Manual Post-Release Adjustments

After the workflow completes, you can:

1. **Edit release notes** directly in GitHub UI if needed
2. **Upload artifacts** by dragging/dropping them on the release page
3. **Publish draft** by unchecking "Set as a draft"

## Notes

- All input text fields support **Markdown formatting**
- Inputs like `build_notes`, `known_issues`, and `custom_text` are optional
- The beta warning is always included automatically
- Install instructions link points to `main` branch docs
- Release created via GitHub's `gh` CLI tool (requires GitHub token)

## Future Enhancements

- [ ] Automatic artifact attachment from build workflows
- [ ] Contributor avatars/links in new contributors section
- [ ] Per-category changelog grouping (features, bugfixes, performance, etc.)
- [ ] Better PR detection using GitHub API search
- [ ] Changelog caching to avoid GitHub API rate limits on large histories
