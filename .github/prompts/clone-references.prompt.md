---
name: clone-references-from-gitmodules
description: Clone selected Git submodules from .gitmodules into local directories for reference purposes
argument-hint: The specific submodule paths to clone (e.g., 'references/') or 'all' for all submodules; prefix with '--exclude=' to skip items
---

# Clone Git References from .gitmodules

You are assisting with cloning Git submodules to local reference directories, preserving original repositories as standalone clones rather than formal Git submodules.

## Task
Clone one or more Git submodules defined in `.gitmodules` into their designated local directories. The goal is to have reference implementations available without registering them as formal git submodules.

## Steps

1. **Parse .gitmodules**: Identify submodule entries matching the specified scope (e.g., all submodules under `references/` or a specific list).

2. **Create target directories**: Ensure parent directories exist for all target clones.

3. **Clone repositories**: For each submodule, use `git clone --depth 1` (shallow clone for speed) from the URL specified in `.gitmodules` to the designated path.

4. **Handle conflicts**: If a path already exists, skip with `|| true` or prompt for overwrite.

5. **Verify**: List the cloned directories to confirm successful completion.

## Context

The user is typically:
- Setting up a development environment with external reference repositories
- Bootstrapping a project that relies on reference implementations (e.g., Linux port references, modernization examples)
- Using `.gitmodules` as the source of truth for repository URLs and paths

## Template Command

```bash
# Clone specific submodules from .gitmodules (or those matching a scope prefix)
mkdir -p <target-parent-dir> && \
git clone --depth 1 <repo-url-1> <target-path-1> || true && \
git clone --depth 1 <repo-url-2> <target-path-2> || true && \
# ... repeat for each submodule
ls -la <target-parent-dir>
```

## Notes

- Use `--depth 1` to clone only the latest commit (faster and smaller).
- Use `|| true` to ignore errors if paths already exist.
- Preserve the directory structure defined in `.gitmodules` (e.g., `references/fighter19-dxvk-port`).
- Do not register these as formal git submodules unless explicitly required.
