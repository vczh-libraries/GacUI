# GitView

This folder is for a `GitView` tool, a simplified "Github Desktop" but built on top of GacUI with TUI.

Check out and follow [best practices](../BestPractices.md):
- Tool name is `GitView`.
- The TUI version is named `GitTui`.
- The GUI version is currently out of scope, skip this part.

This app is currently doing read only tasks, no side effect should be made, unless explicitly described below.

## Cross Platform Awareness

Features are based on `git` CLI commands, prepare a function to run CLI commands on Windows/Linux/macOS. You could keep the same signature on a header file and use macro guards for different implementation:
- Signature could be `WString RunCliCommand(WString command)`. Use `WString::Unmanaged` for constants.
- Use `VCZH_MSVC` for Windows.
- Use `VCZH_GCC && !VCZH_APPLE` for Linux.
- Use `VCZH_GCC && VCZH_APPLE` for macOS.

## UI Organization

The tool must be launched in the repo folder or its sub folders, please be awared of existance of submodules, the best way is to run `git rev-parse --show-toplevel` directly and remember the absolute path.

UI below is for demonstration only, you should use GacUI with TUI instead of actually render it like below.

### Main Window (CHANGES)

```text
+----------------------+----------------------------+--------------------+---------+
| REPO: <NAME> (label) | BRANCH: <NAME> (combo box) | ACTIONS (dropdown) | (blank) |
+--------------------+-+------------------+---------+--------------------+---------+
| CHANGES (tab page) | HISTORY (tab page) |                                        |
+--------------------+------+-------------+----------------------------------------+
| (tree view)               | (text list)                                          |
| + STAGED FILES            | Normal/DarkGreen/DarkRed background colors           |
|    all staged files       |   for inline diffs                                   |
| + UNSTAGED FILES          | Consecutive non-changed lines could be omitted       |
|    all unstaged files     | 3 lines before and after changes                     |
| + NEW FILES               | So that it works like Github Desktop                 |
|    all untracked files    |                                                      |
|                           | <-- here is a draggable splitter                     |
+---------------------------+------------------------------------------------------+
```

### Main Window (HISTORY)

It has 3 columns instead of two for:
- commit list
- file list in selected commit
- diff in selected file

### ACTIONS drop down

This is a menu bar button with such sub menu:
- `REFRESH`: This app does not watch file changing, click it to reload files in `CHANGES` and commits in `HISTORY`.
  - Everything become unselected.
  - Immediate loading is performed when a commit or file is selected.
- `FETCH ORIGIN`: Do `git pull origin <BRANCH>`, report an error when conflict happens.
- `FETCH ORIGIN and REBASE`: Do `git pull origin <BRANCH>`, rebase when conflict happens, report an error when there are unresolvable conflicts. Users are expected to resolve them with another terminal or tool.

## Localization

No need to do localization, keep everything in English.
