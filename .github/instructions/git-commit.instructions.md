---
applyTo: '**'
---

# Git Commit Message Instructions

Use [Conventional Commits](https://www.conventionalcommits.org/) style, adapted for GeneralsX.

## Commit Message Format

```
<type>(optional scope): <description>

[optional body]

[optional footer(s)]
```

### Type

Use one of:

- **feat**: A new feature
- **fix**: A bug fix
- **docs**: Documentation only changes
- **refactor**: Code change that neither fixes a bug nor adds a feature
- **test**: Adding missing tests or correcting existing tests
- **chore**: Build configuration, dependencies, version updates, etc. (no production code changes)
- **build**: Changes to build system or external dependencies (example scopes: cmake, vcpkg, external libs)
- **ci**: Changes to CI/CD configuration or scripts
- **perf**: Code change that improves performance
- **style**: Changes that do not affect code meaning (whitespace, formatting, semicolons, etc.)

**Important**: Do NOT use `@` in the type field.
- ✗ `@refactor(component] message`
- ✓ `refactor(component) message`

### Scope (Optional)

- Affected code, file, directory, or component.
- Can span multiple scopes; omit if needed.
- Use lowercase kebab-case.
- Examples: `graphics`, `audio-openal`, `cmake-presets`, `dxvk-macos`.

### Description/Subject

- Succinct, readable without diff.
- Imperative present tense: "add", not "added" or "adds".
- No leading capital, no trailing period.
- Keep under 50 chars when possible.

### Body (Optional)

- Explain why, trade-offs, and design decisions.
- Separate from subject with blank line.
- Wrap at 72 chars.
- Reference issues when useful.

### Footer (Optional)

- Reference issues or breaking changes.
- Format: `Fixes #<issue>`, `Closes #<issue>`, `Related-to #<issue>`.
- Breaking changes: `BREAKING CHANGE: <description>`.

## Examples

### Simple feature (no scope)
```
feat: add SDL3 event handling for cross-platform input
```

### Fix with scope and body
```
fix(dxvk-macos): correct MoltenVK initialization on ARM64

The Meson build was not passing -arch arm64 explicitly,
causing Rosetta2 to interfere. Updated meson-arm64-native.ini
to enforce native compilation.

Fixes #456
```

### Refactor with scope
```
refactor(graphics): consolidate DirectX8 adapter interfaces
```

### Documentation update
```
docs: update macOS build instructions for Vulkan SDK setup
```

## Squash & Amend

- **Amend**: Modify a previous commit without creating a new one (use before pushing)
  ```bash
  git commit --amend --no-edit
  ```
- **Squash**: Combine multiple commits into one (during interactive rebase)
  ```bash
  git rebase -i HEAD~N
  ```

**Important**: When squashing or amending, ensure the final commit message follows these standards.

## Commit Discipline

- Group logically related changes.
- One feature/fix per commit when possible.
- Keep commits small and reviewable.
- Explain both what and why.

## Quick Reference

| Type | Use Case |
|------|----------|
| `feat` | New feature or capability |
| `fix` | Bug fix (affects functionality) |
| `docs` | README, guides, inline comments |
| `refactor` | Non-functional code reorganization |
| `test` | Test additions/fixes |
| `build` | Build system, CMake, dependencies |
| `ci` | GitHub Actions, CI scripts |
| `perf` | Performance optimization |
| `style` | Formatting, spacing (no logic changes) |

## Pull request guidelines

- PR title follows same format as commit messages.
- PR description gives context and links issues.
- PRs target `main` in `fbraz3/GeneralsX` unless user says otherwise.
- `dxvk-macos` work lives in `references/fbraz3-dxvk` and follows same standards.
- `fbraz3-dxvk` PRs target `generalsx-macos-v2.6` in `fbraz3/dxvk`.

---

**Note**: For GeneralsX code changes, also see `.github/copilot-instructions.md` for the code annotation standard (`// GeneralsX @keyword author DD/MM/YYYY Description`).