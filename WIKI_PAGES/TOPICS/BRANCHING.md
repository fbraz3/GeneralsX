# TOPICS/BRANCHING.md

## Summary

Branching workflow — managing changes, merges, and upstream sync.

---

## Branch Types

### Feature Branch

**Purpose:** Develop new feature or fix

**Naming:** `feature/<description>`

**Example:** `feature/dxvk-macOS-fix`

### Bugfix Branch

**Purpose:** Fix specific issue

**Naming:** `bugfix/<issue-number>`

**Example:** `bugfix/132-fps-uncapped`

### Platform Branch

**Purpose:** Platform-specific development

**Naming:** `platform/<platform>/<feature>`

**Example:** `platform/linux64-deploy/case-sensitive-fix`

---

## Branching Rules

### Rule 1: Platform Code Merged

**Platform code in `Core/GameEngineDevice/`:**
- Keep our changes
- Merge upstream only for game logic conflicts

### Rule 2: Game Logic Preserved

**Game logic in `GeneralsMD/Code/GameEngine/`:**
- Keep upstream changes
- Merge our platform fixes carefully

### Rule 3: Build System Merged Carefully

**Build system files:**
- Test both versions
- Verify compatibility
- Merge conflicts carefully

---

## Upstream Sync

### Add Remote

```bash
git remote add thesuperhackers git@github.com:TheSuperHackers/GeneralsGameCode.git
git fetch thesuperhackers
```

### Merge Upstream

```bash
git merge thesuperhackers/main
```

### Conflict Resolution

| File Type | Action |
|-----------|--------|
| Platform code (`Core/GameEngineDevice/`) | Keep ours |
| Game logic (`GeneralsMD/Code/GameEngine/`) | Keep theirs |
| Build system | Merge carefully, test both |

---

## Branching Checklist

### Before Branching

- [ ] Create descriptive branch name
- [ ] Update dev blog diary
- [ ] Document changes in commit message

### During Development

- [ ] Test on both platforms
- [ ] Verify no regressions
- [ ] Update documentation

### Before Merging

- [ ] All tests pass
- [ ] No new platform issues
- [ ] Documentation updated
- [ ] Dev blog diary updated

---

## See Also

- [CONCEPTS/RETAIL.md](../CONCEPTS/RETAIL.md) — Retail compatibility
- [TOPICS/TESTING.md](./TESTING.md) — Testing protocol

---

**Last updated**: 2026-05-18 | **Sources**: AGENTS.md, branching documentation
