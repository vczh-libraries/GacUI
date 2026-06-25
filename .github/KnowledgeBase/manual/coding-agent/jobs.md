# Investigate and Refine Jobs

`AGENTS.md` and `CLAUDE.md` route short request keywords to prompt files in `.github/prompts`. These jobs make agent work repeatable because the agent writes durable task documents, uses the copied knowledge base, and follows the repository's build and debug scripts.

## investigate

Use `investigate` for coding work that needs analysis, implementation, verification, and a record of decisions. It is not limited to bug fixing. It can handle new features, refactoring, behavior changes, regressions, and deep research that may end in source edits.

Common request forms are:
- `investigate repro ...`: start a fresh task and copy the problem or feature request into `Copilot_Investigate.md`.
- `investigate continue ...`: append an update to the current investigation and continue from the existing proposals.
- `investigate report ...`: ask the agent to summarize confirmed proposals and tradeoffs.

The job writes `.github/TaskLogs/Copilot_Investigate.md`. It records the problem description, tests or confirmation criteria, proposed solutions, code changes, and confirmation or denial for each proposal. The prompt expects the agent to build, run tests, and debug when needed.

## refine

Use `refine` after completed work has produced task logs that contain reusable lessons. The job reads archived task documents, extracts user preferences and project-specific lessons, and writes them to:
- `.github/KnowledgeBase/Learning.md`: general learnings across projects.
- `.github/Learning/Learning_Coding.md`: source-code learnings for this repository.
- `.github/Learning/Learning_Testing.md`: test-code and verification learnings for this repository.

The refine job is intentionally document-only. It should not modify source code. It is useful when a correction, review discussion, or debugging result should influence future agent behavior.

## Other Keywords

The copied context also includes `ask` for analysis-only questions and `kb` for drafting or updating knowledge-base documents. If a user request does not start with a known keyword, the root instructions classify ordinary coding work as an `investigate repro` style task.

