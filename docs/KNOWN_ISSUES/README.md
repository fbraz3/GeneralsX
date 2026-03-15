# Known Issues — DEPRECATED

**⚠️ ARCHIVED DIRECTORY**

This directory is **DEPRECATED**. All issue tracking is now centralized in **GitHub Issues**:  
👉 [`github.com/fbraz3/GeneralsX/issues`](https://github.com/fbraz3/GeneralsX/issues/)

## Migration Policy

- **New issues**: Create directly in GitHub using `gh issue create` command
- **Existing markdown issues**: Migrated to GitHub Issues and removed from this directory
- **Active tracking**: Use GitHub Issues exclusively (labels, assignees, milestones, automation)

## Creating a New Issue (Correct Way)

Use the GitHub CLI:

```bash
gh issue create \
  --title "Brief, actionable title" \
  --body "## Context
...description...

## Goal
...what should be done...

## Acceptance Criteria
- [ ] Item 1
- [ ] Item 2" \
  --label "bug" \
  --label "Linux"
```

See `.github/instructions/docs.instructions.md` for issue creation guidelines.
