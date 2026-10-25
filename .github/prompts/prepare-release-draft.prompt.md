---
name: prepare-release-draft
description: Inspect changes since the previous release, generate rich and structured release notes, incorporate custom highlights, and update the GitHub release draft without publishing.
argument-hint: Release tag or draft name, and optional custom highlights/announcements (e.g. "1.0.0" or "1.0.0 - highlight new stable release and welcome @user")
---

# Prepare & Polish GitHub Release Draft

Generate engaging, highly structured, and accurate release notes for a `GeneralsX` draft release, and update the release draft directly via GitHub CLI without publishing it.

---

## 1. Objectives & Principles

1. **Keep Draft Unreleased**: Ensure the release remains strictly in **draft** mode (`--draft`) throughout the entire process. Never publish or remove the draft flag unless explicitly requested by the user.
2. **Visual & Structural Excellence**: Follow the established `GeneralsX` release style (rich markdown, thematic emojis, clear categorized sections, community port links, and full PR changelog).
3. **Natural Contributor Attribution**: Ensure authors are properly credited on specific features and PR links (`[#PR](...) by @author`), and include their GitHub handles in the changelog.
4. **External Contributor Recognition**: Automatically detect external contributors (authors other than the core maintainer `@fbraz3` or automated bots). When external contributors are present in the release range, generate a dedicated and warm `### ❤️ Contributor Acknowledgements` section within `# What's New` thanking them by `@handle` and highlighting their contributions with PR links. If no external contributors participated in the release, omit this section entirely (remove any empty placeholders from CI).
5. **Custom Highlights & User Intent**: Incorporate any custom announcements, milestones, notices, or specific contributor welcomes provided by the user in the prompt arguments or request.
6. **Accuracy & Clarity**: Accurately describe technical improvements (determinism, platform backends, graphics/audio parity, upstream syncs) in an accessible yet precise manner.

---

## 2. Step-by-Step Workflow

### Step 1: Detect Releases & Tags
1. When running `gh` commands in the sandbox environment, always prepend `env -u GITHUB_TOKEN -u GH_TOKEN` to avoid dummy token authentication errors.
2. List available releases to identify the active draft and the previous published release:
   ```bash
   env -u GITHUB_TOKEN -u GH_TOKEN gh release list --json tagName,name,isDraft,isLatest
   ```
3. If a target tag or draft name was provided (e.g., `1.0.0`), resolve it to its exact `tagName`. Otherwise, select the first entry where `isDraft` is `true`.
4. Validate that the target release exists and has `isDraft: true`. If `isDraft` is `false` or the release does not exist, abort immediately to protect published releases.
5. Identify the previous published release tag (e.g., `1.0.0` or `GeneralsX-Beta-19`) to determine the diff range.
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

### Step 3: Identify External Contributors
1. Extract unique authors from the PR list collected in Step 2:
   - Identify author logins excluding the core maintainer (`fbraz3`) and automated bots (`*bot*`, `github-actions[bot]`, `dependabot[bot]`, `coderabbitai[bot]`).
2. Resolve each commit author in `$PREVIOUS_TAG..$TARGET_TAG` to a GitHub login while pairing with commit SHAs, ensuring direct or non-PR commits are accurately attributed without losing commit context:
   ```bash
   # Pair each commit SHA with its resolved GitHub login or fallback author name:
   for SHA in $(git rev-list "$PREVIOUS_TAG..$TARGET_TAG"); do
     LOGIN=$(env -u GITHUB_TOKEN -u GH_TOKEN gh api "repos/fbraz3/GeneralsX/commits/${SHA}" --jq '.author.login // empty')
     if [ -n "$LOGIN" ]; then
       echo "${SHA}|${LOGIN}|github"
     else
       NAME=$(git log -1 --format="%an" "$SHA")
       echo "${SHA}|${NAME}|git"
     fi
   done
   ```
   Do not compare raw Git metadata (names or emails from `git log`) directly with GitHub handles. Do not swallow or mask `gh api` errors with `|| true` or `2>/dev/null`; allow API errors (such as rate limits or network issues) to surface so they can be retried. Use the plain-name fallback strictly when the API call succeeds and returns an empty author login (indicating an unlinked Git author). Derive unique contributors while maintaining the SHA-to-login association so acknowledgements can link to either PRs or specific commits.
3. Evaluate the result:
   - **External Contributors Present**:
     - Prepare a warm, appreciative, and celebratory acknowledgment under `### ❤️ Contributor Acknowledgements` in `# What's New`.
     - Thank each contributor using their linked handle `**[@username](https://github.com/username)**` when GitHub resolved their login, or their plain display name `**Author Name**` (without `@` prefix) when unlinked. Detail what they improved or fixed, referencing their PR (`[#PR](https://github.com/fbraz3/GeneralsX/pull/PR)`) or commit (`[commit](https://github.com/fbraz3/GeneralsX/commit/SHA)`).
     - Example format:
       ```markdown
       ### ❤️ Contributor Acknowledgements
       A special thank you to our community contributors who made this release possible:
       * **[@contributor](https://github.com/contributor)** for configuring mouse wheel zoom speed ([#344](https://github.com/fbraz3/GeneralsX/pull/344))!
       * **Contributor Name** for fixing build script permissions ([commit](https://github.com/fbraz3/GeneralsX/commit/SHA))!
       ```
   - **No External Contributors**:
     - Completely omit the `### ❤️ Contributor Acknowledgements` section from the release notes. Do NOT leave an empty header or placeholder in the final draft.

### Step 4: Check for Special Highlights & Custom Announcements
1. Review the user prompt and arguments for any special highlights, major announcements, contributor welcomes, or important notices requested.
2. If the user mentioned a specific highlight (e.g., welcoming a new collaborator, a new rendering backend, or a major tournament milestone), craft a dedicated top callout section (`### 🌟 <Highlight Title>`).
3. If no special highlight was requested, proceed directly with standard categorized sections.

### Step 5: Structure Release Notes
Organize the release notes following the official GeneralsX template:

```markdown
If you run into any problems, please [open an issue](https://github.com/fbraz3/GeneralsX/issues) so we can investigate.

# Getting Started

GeneralsX lets you play **Command & Conquer: Generals** and **Zero Hour** natively on modern systems with cross-platform online multiplayer.

### 📦 Which file should I download?
* **macOS (Apple Silicon)**: Download `macOS-GeneralsXZH.zip` (Zero Hour) or `macOS-GeneralsX.zip` (Generals).
* **Linux**: Download `Linux-GeneralsXZH.flatpak` (Zero Hour) or `Linux-GeneralsX.flatpak` (Generals).
* **Windows**: Download `Windows-GeneralsXZH.zip` (Zero Hour) or `Windows-GeneralsX.zip` (Generals).

👉 Follow the [Installation & Setup Guide](https://github.com/fbraz3/GeneralsX/wiki/How-to-Install-GeneralsX) on our Wiki to set up your original game data files.

### 🌐 Cross-Platform Multiplayer
Zero Hour includes built-in online lobbies via **GeneralsOnline (NGMP)**. Players on macOS, Linux, and Windows can host, join, and play together without GameSpy, Hamachi, or third-party VLAN tools.

---

# What's New

### 🌟 <Custom Highlight / Announcement> (Only if requested or applicable)
...

### 🌐 Cross-Platform Multiplayer & Networking
- Feature / Fix description ([#PR](https://github.com/fbraz3/GeneralsX/pull/PR) by [@author](https://github.com/author)).

### 🎯 Engine, Gameplay & Determinism
- ...

### 🖥️ UI & Quality of Life
- ...

### 🎨 Graphics & Visuals (DXVK / Vulkan)
- ...

### 🔊 Audio & Video Subsystems (MiniAudio / FFmpeg)
- ...

### 🚀 Platform, Packaging & CI (macOS, Linux, Windows)
- ...

### ❤️ Contributor Acknowledgements (Include ONLY if external contributors exist in this release; omit entirely if none)
A special thank you to our community contributors who made this release possible:
* **[@contributor](https://github.com/contributor)** for <contribution summary> ([#PR](https://github.com/fbraz3/GeneralsX/pull/PR))!
* **<Author Name>** for <direct commit summary> ([commit](https://github.com/fbraz3/GeneralsX/commit/SHA))!

---

## 📱 Ecosystem & Community Ports

If you enjoy GeneralsX, check out these community ports powered by the engine:

* [Generals-Mac-iOS-iPad](https://github.com/ammaarreshi/Generals-Mac-iOS-iPad) - iOS port by [@ammaarreshi](https://github.com/ammaarreshi)
* [Generals-Android](https://github.com/fadi-labib/Generals-Android) - Android port by [@fadi-labib](https://github.com/fadi-labib)
* [wasm-generals](https://github.com/origami-ltd/wasm-generals) - Browser port by [@ebellumat](https://github.com/ebellumat), playable at [generals.wasm.com.br](https://generals.wasm.com.br)

---

## Changelog
- <commit/pr list>

**Full Changelog**: https://github.com/fbraz3/GeneralsX/compare/<PREVIOUS_TAG>...<TARGET_TAG>
```

### Step 6: Update the Release Draft
1. Save the generated release notes into a temporary markdown file:
   ```bash
   NOTES_FILE="/tmp/release_notes_${TARGET_TAG}.md"
   ```
2. Update the release notes and title via `gh release edit`:
   ```bash
   env -u GITHUB_TOKEN -u GH_TOKEN gh release edit "$TARGET_TAG" --draft --title "GeneralsX - $TARGET_TAG" --notes-file "$NOTES_FILE"
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
