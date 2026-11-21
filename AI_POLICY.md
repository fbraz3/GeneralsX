# AI Contribution Policy

`GeneralsX` welcomes AI-assisted contributions. Modern AI coding assistants and agentic workflows are regularly used alongside human engineering, code review, and cross-platform testing to accelerate development on this codebase.

Using AI is fine, **as long as the result meets the same engineering bar as hand-written code**: every change must be reviewed, understood, compiled, and validated.

## 1. AI-Assisted Contributions Are Welcome

- You are welcome to use AI coding assistants, LLMs, or autonomous agent workflows to implement fixes, features, tests, documentation, and pull request descriptions.
- You do **not** need to open a prior Issue or Discussion for focused bug fixes or straightforward improvements—opening a pull request directly is encouraged. (For large architectural changes, starting a Discussion or Issue first is still helpful to align on direction.)

## 2. Responsibility & Verification

**The submitter (the human contributor or the operator running the agent) is responsible for the submission.**

Before opening a pull request, ensure that:

- **It builds and works:** Verify that the code compiles and has been tested on at least one supported target (or clearly state what was and was not tested).
- **No hallucinated code or claims:** Do not submit references to nonexistent APIs, files, or test results that were not actually observed.
- **Project standards are respected:** Follow [AGENTS.md](AGENTS.md) and [CONTRIBUTING.md](CONTRIBUTING.md), including platform isolation (`Core/GameEngineDevice/`), cross-platform determinism in `GameLogic`, and Conventional Commits.

## 3. Disclosure

If a significant portion of a pull request was generated or assisted by AI, briefly note it in the pull request description (for example, which tool/model was used and what was verified). AI-generated PR descriptions are completely fine as long as they accurately reflect the actual diff.

## 4. What We Do Not Accept

To protect reviewer time and codebase health, `GeneralsX` maintainers may close pull requests that:

- **Fail to compile or break CI** without explanation or follow-up.
- **Dump massive, unvetted changesets** (thousands of lines of speculative refactoring or boilerplate) that the submitter cannot explain or validate.
- **Contain hallucinated features, fake test outputs, or ungrounded review replies.**

## 5. Enforcement

`GeneralsX` maintainers reserve the right to close pull requests that violate the quality and verification standards above, or to restrict contributors who repeatedly submit broken or unvetted automated spam.
