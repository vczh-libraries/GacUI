# GitTui

GitTui browses Git repositories in a terminal using GacUI and TuiSkin. Start it inside a working tree or one of its subfolders. A submodule is treated as its own repository.

- CHANGES groups staged, unstaged and new files. Select a file for an inline diff with three context lines; additions have a dark green background and deletions a dark red background. Git displays binary and submodule changes as summaries.
- HISTORY lists commits on the selected local branch. Select a commit, then a file. Merge commits compare with their first parent; root commits compare with the empty tree.
- The branch combo browses history without checking out a branch. CHANGES always describes the current working tree.
- ACTIONS → REFRESH reloads branches, changes and history and clears selections. There is no background file watcher.
- ACTIONS → FETCH ORIGIN runs a pull of the checked-out branch from origin. It requires a clean working tree and no unfinished Git operation. The branch must also be selected in the combo. Despite its label, this action can update files and create a merge commit.
- FETCH ORIGIN and REBASE first tries the same pull. If that attempt creates merge conflicts, it aborts that merge and retries with rebase. Remaining conflicts are reported in the status area and left for resolution in another terminal. Commit or stash work and finish outstanding Git operations externally, then refresh.
- ACTIONS → EXIT closes the terminal UI. Use Tab, arrows and Enter, or terminal mouse input, to navigate. Drag the column dividers to resize panes. The status area supports scrolling and selecting error text.

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

## Linux and macOS build preparation

The supplied `vmake` is authored configuration only. It has not been executed or verified on Linux/macOS. There is deliberately no fabricated `makefile` or `vmake.txt`.

Install the normal VczhLibraries build environment (`VCPROOT`), Git, a C++20 compiler and platform dependencies. Keep matching sibling wGac (Linux) or iGac (macOS) checkouts and build their TUI provider libraries using that repository's supported workflow first. The default paths expect their `build` directories. `WGAC_ROOT`/`WGAC_BUILD` or `IGAC_ROOT`/`IGAC_BUILD` can be exported to use other paths. The platform and GacUI imports must be from matching versions.

On a supported Unix host, run GacUI's `.github/Ubuntu/build.sh` from `Tools/GitView`, then run `Bin/GitTui` from the desired working tree in a UTF-8 terminal. The configuration compiles the same authored/generated application and portable Git sources, replaces Windows sources with their POSIX counterparts, and links the native TUI provider libraries. Linux also needs the packages listed by its `pkg-config` invocation; macOS uses the Cocoa/Quartz/Carbon/UniformTypeIdentifiers frameworks.

## Source layout

`Gaclib` builds the imported libraries and released GacUI/TuiSkin. `GitView/UI` owns the Workflow view-model contracts, layouts and event handlers. `GitView/Source` is generated exclusively by GacBuild, including compressed embedded resources. `GitView/Model` runs and parses Git; `GitView/ViewModel` implements the UI-facing contracts in C++. `GitTui` contains platform entry points. `GitViewTests` exercises parsing, actual disposable repositories and view-model behavior. There is no GUI executable.
