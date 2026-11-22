---
name: resolve-coderabbit-findings
description: Triage, address, or technically rebut all CodeRabbit review comments on a pull request, implement fixes with cross-platform and base-game parity, reply to comments via GitHub API, and enforce commit standards.
argument-hint: Pull request number, branch, or URL (e.g. "278" or "https://github.com/fbraz3/GeneralsX/pull/278")
agent: Bender
---

# CodeRabbit Findings Resolution Workflow

The canonical instructions for this workflow live in [`.agents/skills/coderabbit-triage/SKILL.md`](../../.agents/skills/coderabbit-triage/SKILL.md).

Before delegating, ensure the working tree is clean (`git status --porcelain`), check out the PR branch (`gh pr checkout <PR_NUMBER>`), and pass any specific review ID from a `#pullrequestreview-<id>` URL to `.agents/skills/coderabbit-triage/SKILL.md` so fixes are applied to the PR branch and the requested review's outside-diff findings are inspected.
