# GeneralsX: Instructions for AI Coding Agents

## Context File Loading Guidelines

You **MUST** load the following project-level files in your context before start any work:

- `.github/copilot-instructions.md` - quick reference
- `.github/instructions/generalsx.instructions.md` - Core strategy document
- `.github/instructions/git-commit.instructions.md` - commit message standards
- `.github/instructions/docs.instructions.md` - documentation guidelines
- `.github/instructions/scripts.instructions.md` - script organization rules

Ensure to update the context files list above as new instruction files are added to `github/instructions/*.instructions.md`. Always check the relevant files for understanding specific instructions.

## Context file patterns

You MUST respect the `applyTo` pattern from instruction files if they have one.

Common applyTo Pattern Examples:

| Goal                 | `applyTo` Pattern     | Description                                                  |
|----------------------|-----------------------|--------------------------------------------------------------|
| All Files            | `**`                  | Applies instructions to every file in the workspace.         |
| Specific Language    | `**/*•РУ`             | Triggers only when working with Python files.                |
| Multiple Extensions. | `{**/*.css,**/*.scss` | Targets both standard and Sass stylesheets.                  |
| Folder-Specific      | `src/components/**`   | Applies rules only to files within the components directory. |
| Exclusion            | `**/node_modules/**`  | Explicitly avoids applying rules to external dependencies.   |
