# Copilot CLI Predefined Tools

This file lists the tools available in this Copilot CLI environment, with each tool’s offered description.

## functions.powershell
Runs a PowerShell command in an interactive PowerShell session.
- The "command" parameter does NOT need to be XML-escaped.
- You don't have internet access via this tool.
- You can run Python, Node.js and Go code with `python`, `node` and `go`.
- Each shellId identifies a persistent session. State is saved across calls.
- `initial_wait` must be 30-600 seconds. Give long-running commands time to produce output.
- If a command hasn't completed within initial_wait, it returns partial output and continues running. Use `read_powershell` for more output or `stop_powershell` to stop it.
- You can install Python, JavaScript and Go packages with the `pip`, `npm` and `go` commands.
- Use native PowerShell commands not DOS commands (e.g., use Get-ChildItem rather than dir). DOS commands may not work.

## functions.write_powershell
Sends input to the specified command or PowerShell session.
- This tool can be used to send input to a running PowerShell command or an interactive console app.
- PowerShell commands are run in an interactive PowerShell session with a TTY device and PowerShell command processor.
- shellId (required) must match the shellId used to invoke the async powershell command.
- You can send text, {up}, {down}, {left}, {right}, {enter}, and {backspace} as input.
- Some applications present a list of options to select from. The selection is often denoted using ❯, >, or different formatting.
- When presented with a list of items, make a selection by sending arrow keys like {up} or {down} to move the selection to your chosen item and then {enter} to select it.
- The response will contain any output read after "delay" seconds. Delay should be appropriate for the task and never less than 10 seconds.

## functions.read_powershell
Reads output from a PowerShell command.
- Reads the output of a command running in an "async" PowerShell session.
- The shellId MUST be the same one used to invoke the PowerShell command.
- You can call this tool multiple times to read output produced since the last call.
- Each request has a cost, so provide a reasonable "delay" parameter value for the task, to minimize the need for repeated reads.
- If a read request generates no output, consider using exponential backoff in choosing the delay between reads of the same command.
- Though `write_powershell` accepts ANSI control codes, this tool does not include them in the output.

## functions.stop_powershell
Stops a running PowerShell command.
- Stops a running PowerShell command by terminating the entire PowerShell session and process.
- This tool can be used to stop commands that have not exited on their own.
- Any environment variables defined will have to be redefined after using this tool if the same session ID is used to run a new command.
- The shellId must match the shellId used to invoke the powershell command.

## functions.list_powershell
Lists all active PowerShell sessions.
- Returns information about all currently running PowerShell sessions.
- Useful for discovering shellIds to use with read_powershell, write_powershell, or stop_powershell.
- Shows shellId, command, mode, PID, status, and whether there is unread output.

## functions.view
Tool for viewing files and directories.
- If `path` is an image file, returns the image as base64-encoded data along with its MIME type.
- If `path` is any other type of file, `view` displays the content with line numbers prefixed to each line in the format `N. ` where N is the line number (e.g., `1. `, `2. `, etc.).
- If `path` is a directory, `view` lists non-hidden files and directories up to 2 levels deep
- Path MUST be absolute

## functions.create
Tool for creating new files.
- Creates a new file with the specified content at the given path
- Cannot be used if the specified path already exists
- Parent directories must exist before creating the file
- Path MUST be absolute

## functions.edit
Tool for making string replacements in files.
- Replaces exactly one occurrence of `old_str` with `new_str` in the specified file
- When called multiple times in a single response, edits are independently made in the order calls are specified
- The `old_str` parameter must match EXACTLY one or more consecutive lines from the original file
- If `old_str` is not unique in the file, replacement will not be performed
- Make sure to include enough context in `old_str` to make it unique
- Path MUST be absolute

## functions.web_fetch
Fetches a URL from the internet and returns the page as either markdown or raw HTML. Use this to safely retrieve up-to-date information from HTML web pages.

## functions.report_intent
Use this tool to update the current intent of the session. This is displayed in the user interface and is important to help the user understand what you're doing.
Rules:
- Call this tool ONLY when you are also calling other tools. Do not call this tool in isolation.
- Put this tool call first in your collection of tool calls.
- Always call it at least once per user message (on your first tool-calling turn after a user message).
- Don't then re-call it if the reported intent is still applicable
When to update intent (examples):
- ✅ "Exploring codebase" → "Installing dependencies" (new phase)
- ✅ "Running tests" → "Debugging test failures" (new phase)
- ✅ "Creating hook script" → "Fixing security issue" (new phase)
- ✅ "Creating parser tests" → "Fixing security issue" (new phase)
- ❌ "Installing Pandas 2.2.3" → "Installing Pandas with pip3" (same goal, different tactic: should just have said "Installing Pandas")
- ❌ "Running transformation script" → "Running with python3" (same goal, fallback attempt)
Phrasing guidelines:
- The intent text must be succinct - 4 words max
- Keep it high-level - it should summarize a series of steps and focus on the goal
- Use gerund form
- Bad examples:
- 'I am going to read the codebase and understand it.' (too long and no gerund)
- 'Writing test1.js' (too low-level: describe the goal, not the specific file)
- 'Updating logic' (too vague: at least add one word to hint at what logic)
- Good examples:
- 'Exploring codebase'
- 'Creating parser tests'
- 'Fixing homepage CSS'

## functions.fetch_copilot_cli_documentation
Fetches documentation about you, the GitHub Copilot CLI, and your capabilities. Use this tool when the user asks how to use you, what you can do, or about specific features of the GitHub Copilot CLI.

## functions.update_todo
Use this TODO tool to manage the tasks that must be completed to solve the problem. Use this tool VERY frequently to keep track of your progress towards completing the overall goal.
- Call this tool to make the initial todo list for a complex problem. Then call this tool every time you finish a task and check off the corresponding item in the TODO list. If new tasks are identified, add them to the list as well. Re-planning is allowed if necessary.
- This tool accepts markdown input to track what has been completed, and what still needs to be done.
- This tool does not return meaningful data or make changes to the repository, but helps you organize your work and keeps you on-task.
- Call this tool at the same time as the next necessary tool calls, so that you can keep your TODO list updated while continuing to make progress on the problem.

## functions.grep
Fast and precise code search using ripgrep. Search for patterns in file contents.

## functions.glob
Fast file pattern matching using glob patterns. Find files by name patterns.

## functions.task
Custom agent: Launch specialized agents in separate context windows for specific tasks.

The Task tool launches specialized agents that autonomously handle complex tasks. Each agent type has specific capabilities and tools available to it.

Available agent types:
- explore: Fast agent specialized for exploring codebases and answering questions about code. Use this when you need to quickly find files by patterns, search code for keywords, or answer questions about the codebase. Returns focused answers under 300 words. Safe to call in parallel. (Tools: grep/glob/view, Haiku model)
- task: Agent for executing commands with verbose output (tests, builds, lints, dependency installs). Returns brief summary on success, full output on failure. Keeps main context clean by minimizing successful output. Use for tasks where you only need to know success/failure status. (Tools: All CLI tools, Haiku model)
- general-purpose: Full-capability agent running in a subprocess. Use for complex multi-step tasks requiring the complete toolset and high-quality reasoning. Runs in a separate context window to keep your main conversation clean. (Tools: All CLI tools, Sonnet model)
- code-review: Agent for reviewing code changes with extremely high signal-to-noise ratio. Analyzes staged/unstaged changes and branch diffs. Only surfaces issues that genuinely matter - bugs, security vulnerabilities, logic errors. Never comments on style, formatting, or trivial matters. Will NOT modify code. (Tools: All CLI tools for investigation)

When NOT to use Task tool:
- Reading specific file paths you already know - use view tool instead
- Simple single grep/glob search - use grep/glob tools directly
- Commands where you need immediate full output in your context - use bash directly
- File operations on known files - use edit/create tools directly

Usage notes:
- Can launch multiple explore/code-review agents in parallel (task, general-purpose have side effects)
- Each agent is stateless - provide complete context in your prompt
- Agent results are returned in a single message
- Use explore proactively for codebase questions before making changes

## multi_tool_use.parallel
This tool serves as a wrapper for utilizing multiple tools.

Use this function to run multiple tools simultaneously, but only if they can operate in parallel.
Only tools in the functions namespace are permitted.
Ensure that the parameters provided to each tool are valid according to that tool's specification.
