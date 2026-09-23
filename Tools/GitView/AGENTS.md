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

## Linux Builds and Verification

- Each executable has its own build directory and authored `vmake`:
  - `Tools/GitView/Linux/GitTui`: produces `Bin/GitTui`.
  - `Tools/GitView/Linux/GitViewTests`: produces `Bin/GitViewTests`.
- From each directory, run `REPO-ROOT/.github/Ubuntu/build.sh` for incremental builds or `build.sh -f` for full rebuilds. Do not hand-edit generated `makefile` or `vmake.txt`.
- Run `./Bin/GitViewTests /C` from `Linux/GitViewTests` using an installed UTF-8 locale. These tests need Git but do not need native renderer libraries or a desktop session.
- GitTui links the matching sibling wGac checkout's prebuilt `build/WGacTuiControlTest/libWGacTui.a` and `build/WGacShared/libWGac.a`, plus the packages named in its `vmake`. Build these with wGac's supported workflow first. `WGAC_ROOT` and `WGAC_BUILD` can override the default paths.
- Run GitTui in an interactive UTF-8 terminal inside the repository to browse; use its absolute executable path when browsing another repository. Verify Alt labels and selection behavior through terminal input/output.
- On Linux, regenerate with native `Tools/GacGen/Bin/GacGen /P32` and `/P64`, passing the absolute `GitView/UI/Resource.xml` path from `Tools/GacGen`. The resource has no dependencies. Check both architecture logs for `Errors.txt`, then use the sibling Workflow `Tools/CppMerge/Bin/CppMerge <x32-file> <x64-file> <output-file>` for every generated file in `Resource.xml.log/(x32|x64)/Source`, publishing into `GitView/Source`. This reproduces GacBuild's portable output; `/C64` alone hardcodes 64-bit types. Never hand-edit generated C++.
- Both configurations retain macOS support; runtime validation must be recorded separately for each OS.

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
| (readonly text box to display extra information for selected items)              |
+----------------------------------------------------------------------------------+
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

## Keyboard Navigation and Commit Details

Assign Alt labels to every interactive control. Press Alt and then its label: `B` for branches, `A` for actions, `C`/`H` for tab pages, `F` for the active file list, `M` for history commits, `D` for the active diff, and `S` for status. The Actions submenu uses `R` for refresh, `F` for pull, `B` for pull/rebase and `X` for exit.

The current POSIX TUI backend does not report standalone Alt key presses, so Alt-sequence activation is unavailable on Linux/macOS. The labels remain assigned for supported input providers; use Tab, arrow keys, Enter or the mouse on POSIX terminals.

History rows display only the commit subject. The bottom status viewer displays `HASH (author) DATE` and retains it above the selected historical file's information.

Diff groups separated by omitted source lines have one gray horizontal separator list item on the normal background. Touching or overlapping context ranges form one group; separator rows have no line numbers.
