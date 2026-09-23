# GitTui

GitTui browses Git repositories in a terminal using GacUI and TuiSkin. Start it inside a working tree or one of its subfolders. A submodule is treated as its own repository.

- CHANGES groups staged, unstaged and new files. Select a file for numbered source rows with three context lines; additions have a dark green background and deletions a dark red background. Deleted rows use old line numbers; other rows use new line numbers. Overlapping or consecutive context is combined; omitted gaps have a gray horizontal separator row. Patch headers are omitted. Binary and submodule changes have brief summaries.
- HISTORY lists commit subjects on the selected local branch. Select a commit, then a file; the bottom status area shows `HASH (author) DATE` and retains it above the selected file information. Merge commits compare with their first parent; root commits compare with the empty tree.
- The branch combo browses history without checking out a branch. CHANGES always describes the current working tree.
- ACTIONS → REFRESH reloads branches, changes and history and clears selections. There is no background file watcher.
- ACTIONS → FETCH ORIGIN runs a pull of the checked-out branch from origin. It requires a clean working tree and no unfinished Git operation. The branch must also be selected in the combo. Despite its label, this action can update files and create a merge commit.
- FETCH ORIGIN and REBASE first tries the same pull. If that attempt creates merge conflicts, it aborts that merge and retries with rebase. Remaining conflicts are reported in the status area and left for resolution in another terminal. Commit or stash work and finish outstanding Git operations externally, then refresh.
- ACTIONS → EXIT closes the terminal UI. Use Tab, arrows and Enter, or terminal mouse input, to navigate. Press Alt, then `B` for branches, `A` for actions, `C`/`H` for tabs, `F` for files, `M` for commits, `D` for the diff or `S` for status. Drag the column dividers to resize panes. Scrollbars hide when content fits. The readonly status area has three text rows, wraps long lines, and supports selecting and vertically scrolling error text.

The current POSIX TUI backend does not report standalone Alt key presses, so Alt-sequence activation is unavailable on Linux/macOS. The labels remain assigned for supported input providers; use Tab, arrow keys, Enter or the mouse on POSIX terminals.

Git must be on PATH. Commands run synchronously; network actions wait for Git to finish. Configure remote credentials before launching. Browsing does not change the index, working tree or checked-out branch. Git hooks and credential helpers configured for an explicitly requested pull retain their usual behavior.

## Windows build

Use Visual Studio 2026's v145 C++ toolset and Windows 10 SDK. The solution supports Debug/Release and Win32/x64. From `Tools/GitView`, using absolute paths appropriate to your checkout:

```powershell
& C:\Code\VczhLibraries\Tools\Tools\GacBuild.ps1 -FileName C:\Code\VczhLibraries\GacUI\Tools\GitView\GacUI.xml
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable GitViewTests -Configuration Debug -Platform x64
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable GitTui -Configuration Debug -Platform x64 -Interactive
```

For normal use, open Windows Terminal in the repository you want to browse and run the built `x64/Debug/GitTui.exe` by its absolute path. No resource files need to accompany the executable.

## Linux and macOS builds

Install the normal VczhLibraries build environment, Git and a C++20 compiler. Each executable has a separate `vmake` directory. From the repository root:

```bash
cd Tools/GitView/Linux/GitViewTests
../../../../.github/Ubuntu/build.sh
LC_ALL=C.UTF-8 ./Bin/GitViewTests /C
cd ../GitTui
../../../../.github/Ubuntu/build.sh
./Bin/GitTui
```

Use an installed UTF-8 locale (for example `en_US.UTF-8` on macOS). Add `-f` to `build.sh` for a full rebuild. The generated `makefile` and `vmake.txt` come from these configurations and the MSBuild source inventories; do not edit them manually.

GitViewTests requires no native renderer libraries or desktop session. GitTui requires matching sibling wGac (Linux) or iGac (macOS) checkouts with their TUI provider libraries built using that repository's supported workflow. Default paths use their `build` directories; export `WGAC_ROOT`/`WGAC_BUILD` or `IGAC_ROOT`/`IGAC_BUILD` to override them. Linux needs the packages listed by `pkg-config` in `Linux/GitTui/vmake`; macOS uses Cocoa/Quartz/Carbon/UniformTypeIdentifiers. GacUI imports and the provider libraries must be from matching versions.

Launch GitTui in an interactive UTF-8 terminal. To browse another working tree, change to that tree and run the absolute path to `Linux/GitTui/Bin/GitTui`. All application resources are embedded. macOS runtime validation is separate from Linux validation.

## Source layout

`Gaclib` builds the imported libraries and released GacUI/TuiSkin. `GitView/UI` owns the Workflow view-model contracts, layouts and event handlers. `GitView/Source` is generated by GacBuild on Windows or native GacGen on Linux, including compressed embedded resources. `GitView/Model` runs and parses Git; `GitView/ViewModel` implements the UI-facing contracts in C++. `GitTui` contains platform entry points. `GitViewTests` exercises parsing, actual disposable repositories and view-model behavior. There is no GUI executable.
