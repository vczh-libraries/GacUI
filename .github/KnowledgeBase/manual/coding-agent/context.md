# Preparing Context Files

Copy the coding-agent context from the Release repository into the root of the application repository. The minimum copied set is:
- `.github`: prompts, guidelines, scripts, Linux build helpers, local learning files, task-log files, and the copied knowledge base.
- `AGENTS.md`: common entry instructions used by Codex and other agents that read this file.
- `CLAUDE.md`: the same entry instructions for Claude-style agents.
- `Project.md`: application-specific repository map and verification policy, written by the application owner.

The copied `.github` folder contains several kinds of files:
- `copilot-instructions.md`: the main instruction file. It tells the agent to read `Project.md`, use the knowledge base, and prefer provided scripts.
- `Guidelines`: build, run, debug, source-file, coding, and GacUI resource instructions.
- `prompts`: job prompts such as `ask.prompt.md`, `investigate.prompt.md`, `refine.prompt.md`, and `kb.prompt.md`.
- `Scripts`: Windows PowerShell wrappers for building, executing, debugging, and archiving task logs.
- `Ubuntu`: Linux build wrapper and helper commands.
- `KnowledgeBase`: copied API, design, manual, and learning documents that the agent can read without network access.
- `Learning`: project-local lessons that refine future agent behavior.
- `TaskLogs`: working documents for investigation and knowledge-base jobs.

Keep the copied context files under source control when they describe stable project behavior. Task-log files may be tracked or ignored according to the repository policy, but the files referenced by the prompts should exist before asking the agent to use those prompts. If the application has different build scripts, test commands, project names, generated files, or platform support, update `Project.md` first.

See [Writing Project.md](.././coding-agent/project-md.md) for the expected contents of the application-specific file.

