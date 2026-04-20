---
applyTo: '**'
---

# Git Commit Message Instructions

Commit message standards based on [Conventional Commits](https://www.conventionalcommits.org/) specification, adapted for GeneralsX project needs.

## Commit Message Format

```
<type>(optional scope): <description>

[optional body]

[optional footer(s)]
```

### Type

Must be one of:

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

- Name of affected code, file, directory, or logical component
- Can span multiple scopes if needed (omit scope in that case)
- Use lowercase with dashes (kebab-case)
- Examples: `graphics`, `audio-openal`, `cmake-presets`, `dxvk-macos`

### Description/Subject

- Succinct description of the change (readable without seeing the diff)
- Use imperative, present tense: "add" not "added" or "adds"
- Do NOT capitalize the first letter
- Do NOT end with a period (.)
- Keep under 50 characters when possible

### Body (Optional)

- Additional context and explanation of **why** the change was made
- Separate from subject with a blank line
- Wrap at 72 characters
- Include motivation, design decisions, or trade-offs
- Reference related issues if applicable (e.g., `Fixes #123`)

### Footer (Optional)

- Reference related issues or breaking changes
- Format: `Fixes #<issue>`, `Closes #<issue>`, `Related-to #<issue>`
- Breaking changes: `BREAKING CHANGE: <description>`

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

- Commit logically related changes together (not by time/pressure)
- One feature/fix per commit when possible
- Keep commits small and reviewable
- Write a commit message that explains **what** and **why**, not just **what** you changed

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

- PR title should follow the same format as commit messages
- the PR description should provide context and link to related issues
- PR targes must be against main branch of `fbraz3/GeneralsX` repo, unless it's a user instruction to do otherwise (e.g., "Merge to `develop` branch" or "Merge to `feature/xyz` branch")
- There is a subproject called `dxvk-macos` located under `references/fbraz3-dxvk` folder, which is a fork of the original DXVK project. Commits related to that subproject should be made in that repository and follow the same commit message standards.
- `fbraz3-dxvk` subproject PRs should target the `generalsx-macos-v2.6` branch of `fbraz3/dxvk` repository, and follow the same commit message standards.

---

**Note**: For GeneralsX code changes, also see `.github/copilot-instructions.md` for the code annotation standard (`// GeneralsX @keyword author DD/MM/YYYY Description`), which complements commit message discipline.