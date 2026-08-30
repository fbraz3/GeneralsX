---
name: prepare-release-draft
description: Inspect changes since the previous release, generate rich and structured release notes, incorporate custom highlights, and update the GitHub release draft without publishing.
argument-hint: Release tag or draft name, and optional custom highlights/announcements (e.g. "GeneralsX-Beta-18" or "GeneralsX-Beta-18 - highlight new multiplayer fixes and welcome @user")
---

# Prepare & Polish GitHub Release Draft

Generate engaging, highly structured, and accurate release notes for a `GeneralsX` draft release, and update the release draft directly via GitHub CLI without publishing it.

---

## 1. Objectives & Principles

1. **Keep Draft Unreleased**: Ensure the release remains strictly in **draft** mode (`--draft`) throughout the entire process. Never publish or remove the draft flag unless explicitly requested by the user.
2. **Visual & Structural Excellence**: Follow the established `GeneralsX` release style (rich markdown, thematic emojis, clear categorized sections, community port links, and full PR changelog).
3. **Natural Contributor Attribution**: Ensure authors are properly credited on specific features and PR links (`[#PR](...) by @author`), and include their GitHub handles in the changelog.
4. **Custom Highlights & User Intent**: Incorporate any custom announcements, milestones, notices, or specific contributor welcomes provided by the user in the prompt arguments or request.
5. **Accuracy & Clarity**: Accurately describe technical improvements (determinism, platform backends, graphics/audio parity, upstream syncs) in an accessible yet precise manner.

---

## 2. Step-by-Step Workflow

### Step 1: Detect Releases & Tags
1. When running `gh` commands in the sandbox environment, always prepend `env -u GITHUB_TOKEN -u GH_TOKEN` to avoid dummy token authentication errors.
2. List available releases to identify the active draft and the previous published release:
   ```bash
   env -u GITHUB_TOKEN -u GH_TOKEN gh release list --json tagName,name,isDraft,isLatest
   ```
3. If a target tag or draft name was provided (e.g., `GeneralsX-Beta-18`), resolve it to its exact `tagName`. Otherwise, select the first entry where `isDraft` is `true`.
4. Validate that the target release exists and has `isDraft: true`. If `isDraft` is `false` or the release does not exist, abort immediately to protect published releases.
5. Identify the previous published release tag (e.g., `GeneralsX-Beta-17`) to determine the diff range.
6. Assign the resolved tags to shell variables for safe interpolation:
   ```bash
   TARGET_TAG="<RESOLVED_TARGET_TAG>"
   PREVIOUS_TAG="<RESOLVED_PREVIOUS_TAG>"
   ```

### Step 2: Gather Commits & Pull Requests
1. Inspect all commits between the previous release and target release (or `HEAD` if targeting current unreleased checkout):
   ```bash
   git log --oneline "$PREVIOUS_TAG..$TARGET_TAG"
   ```
2. Retrieve PR metadata associated with the commits in this release range without arbitrary limits:
   ```bash
   for SHA in $(git rev-list "$PREVIOUS_TAG..$TARGET_TAG"); do
     env -u GITHUB_TOKEN -u GH_TOKEN gh api "repos/fbraz3/GeneralsX/commits/${SHA}/pulls" --jq '.[] | select(.base.repo.full_name == "fbraz3/GeneralsX") | "\(.number)|\(.title)|\(.user.login)|\(.html_url)"'
   done | sort -u -t'|' -k1,1n
   ```
3. View specific PR descriptions to understand key technical details:
   ```bash
   env -u GITHUB_TOKEN -u GH_TOKEN gh pr view "$PR_NUMBER" --json title,body,author
   ```
4. Check recent worklog entries in `docs/WORKLOG/YYYY-MM-DIARY.md` and active planning documents in `docs/WORKDIR/` for additional context.

### Step 3: Check for Special Highlights & Custom Announcements
1. Review the user prompt and arguments for any special highlights, major announcements, contributor welcomes, or important notices requested.
2. If the user mentioned a specific highlight (e.g., welcoming a new collaborator, a new rendering backend, or a major tournament milestone), craft a dedicated top callout section (`### 🌟 <Highlight Title>`).
3. If no special highlight was requested, proceed directly with standard categorized sections.

### Step 4: Structure Release Notes
Organize the release notes following the official GeneralsX template:

```markdown
> This is a **beta** release. Some bugs are still expected. If you run into any problems, please [open an issue](https://github.com/fbraz3/GeneralsX/issues) so we can investigate.

# What's New

### 🌟 <Custom Highlight / Announcement> (Only if requested or applicable)
...

### 🎯 Cross-Platform Multiplayer & Determinism
- Feature / Fix description ([#PR](https://github.com/fbraz3/GeneralsX/pull/PR) by [@author](https://github.com/author)).

### 🎨 Graphics & Rendering Polish
- ...

### 🖥️ UI & Quality of Life
- ...

### 🔊 Audio Engine (OpenAL & MiniAudio) (If applicable)
- ...

### 🧠 Gameplay & Engine Fixes (Upstream Sync)
- ...

# Getting Started

Follow the [Installation Guide](https://github.com/fbraz3/GeneralsX/blob/main/docs/HOWTO/INSTALLATION.md) to set up GeneralsX on your platform.

## Community Ports based on GeneralsX

If you like GeneralsX, please also take a look into these projects

* [Generals-Mac-iOS-iPad](https://github.com/ammaarreshi/Generals-Mac-iOS-iPad) - iOS port by [ammaarreshi](https://github.com/ammaarreshi)
* [Generals-Android](https://github.com/fadi-labib/Generals-Android) - Android port by [fadi-labib](https://github.com/fadi-labib)
* [GeneralsXWeb](https://github.com/meerzulee/GeneralsXWeb) - Web port by [meerzulee](https://github.com/meerzulee)
* [wasm-generals](https://github.com/origami-ltd/wasm-generals) - WebAssembly Browser port by [ebellumat](https://github.com/ebellumat), playable at [generals.wasm.com.br](https://generals.wasm.com.br)

## Changelog
- <commit/pr list>

**Full Changelog**: https://github.com/fbraz3/GeneralsX/compare/<PREVIOUS_TAG>...<TARGET_TAG>
```

### Step 5: Update the Release Draft
1. Save the generated release notes into a temporary markdown file:
   ```bash
   NOTES_FILE="/tmp/release_notes_${TARGET_TAG}.md"
   ```
2. Update the release notes and title via `gh release edit`:
   ```bash
   env -u GITHUB_TOKEN -u GH_TOKEN gh release edit "$TARGET_TAG" --draft --title "GeneralsX <Beta X>" --notes-file "$NOTES_FILE"
   ```
3. Verify that the release is still in draft state:
   ```bash
   env -u GITHUB_TOKEN -u GH_TOKEN gh release view "$TARGET_TAG" --json name,isDraft,tagName
   ```

---

## 3. Deliverables

Provide a concise summary to the user:
1. **Target Draft & Range**: Specified release tag and compared commit range.
2. **Release Notes Preview**: Key highlights and summary of categorized changes.
3. **Draft Status Confirmation**: Confirmation that the draft has been updated safely without publishing.
