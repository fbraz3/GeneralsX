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
| `dry_run` | boolean | false | Test logic without creating release (no tag created) |

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

### Dry Run - Test Release Notes

```
release_version: GeneralsX-Beta-3
build_notes: |
  - Fixed critical issues
known_issues: |
  - Some edge cases remain
dry_run: true
```

**Dry Run Behavior**:
- No tag is created
- No release is published
- Release notes markdown is generated as an artifact
- Download the artifact to review before publishing for real
- Perfect for testing changelog logic and reviewing notes before commit

## Workflow Behavior

1. **Validation**: Ensures version doesn't already exist (skipped in dry-run)
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
6. **Dry-Run Check**:
   - If `dry_run: true` → skip release creation, generate markdown artifact only
   - If `dry_run: false` → proceed to release creation
7. **Release Creation** (if not dry-run):
   - Uses `gh release create` to publish release
8. **Summary**: Posts results to workflow summary (different messages for dry-run vs real release)

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

## Dry Run Mode

Use dry-run to test the release logic without creating a tag or publishing a release.

**What Happens in Dry Run**:
- ✓ Changelog is generated
- ✓ Release notes markdown is created
- ✓ Markdown file is uploaded as a workflow artifact
- ✗ No tag is created
- ✗ No release is published
- ✗ No GitHub API calls that create/modify data

**Workflow**:
1. Run workflow with `dry_run: true`
2. Workflow completes (summary shows "🧪 Dry Run Completed")
3. Download the markdown artifact from workflow
4. Review release notes in your editor
5. If satisfied, run workflow again with `dry_run: false` to publish for real

**Artifact Location**:
- After dry-run completes, go to workflow run page
- Scroll to "Artifacts" section
- Download `release-notes-GeneralsX-Beta-3` (or your version)
- File is named `GeneralsX-Beta-3-notes.md`

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
2. Selecting names that did not exist before the latest release
3. Creating safe contributor credit lines without exposing email-derived data

Example output:

```
## New Contributors
* NewContributor made their first contribution
```

## Troubleshooting

### "Tag already exists"
The version you specified already has a release. Use a different tag name.

Note: This error won't occur in `dry_run: true` mode since no tag validation happens.

### Changelog shows but PR links are missing
- Ensure commits reference PR numbers in message (e.g., `(#123)`)
- If PR numbers are not in message, manually add PR links to release notes
- Future enhancement: better PR detection patterns

### No new contributors detected
This is expected if:
- No new authors since latest release
- Or all changes were made by contributors already present before the latest release

### Want to preview before publishing?
Use the dry-run mode!
1. Run with `dry_run: true`
2. Workflow artifacts will contain the markdown
3. Download and review
4. If ok, run again with `dry_run: false`

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
