# GitTui: Browse Git in a Terminal

GitTui is the terminal application in `GacUI/Tools/GitView`. It browses working-tree changes and local branch history on Windows, Linux and macOS. Git must be on PATH. Open an interactive terminal in a Git working tree or one of its subfolders, then run `GitTui` by its absolute path. A submodule is treated as its own repository. All application resources are embedded.

## Browse Changes and History

- **CHANGES** groups staged, unstaged and new files. Select a file to read its diff. Added lines have a dark green background and deleted lines have a dark red background. The diff shows line numbers and three context lines around changes; separators mark omitted gaps. Binary and submodule changes have brief summaries.
- **HISTORY** lists commits on the selected local branch. Select a commit and then a file to read its diff. The status area shows the commit hash, author and date above file details. Merge commits compare with their first parent; root commits show their initial contents.
- **BRANCH** selects the history to browse. It does not check out a branch. **CHANGES** continues to describe the current working tree.
- **ACTIONS → REFRESH** reloads branches, changes and history and clears selections. Use it after changing files or running Git externally; GitTui does not watch for changes in the background.

Browsing does not change the index, working tree or checked-out branch. GitTui does not provide staging, committing or conflict resolution.

## Pull from Origin

Despite their labels, the fetch commands perform a pull and can change repository files:

- **FETCH ORIGIN** pulls the checked-out branch from `origin` and can create a merge commit.
- **FETCH ORIGIN and REBASE** first attempts the same pull. If it creates merge conflicts, GitTui aborts that merge and retries with rebase. Remaining conflicts are reported and left for resolution in another terminal.

Both commands require a clean working tree, no unfinished Git operation, and the checked-out branch selected in **BRANCH**. They check the actual checkout again when invoked, including changes made in another terminal. Detached HEAD and branch mismatches are reported without pulling. Configure remote credentials before launching; commands run synchronously and network operations wait for Git. Commit or stash work, resolve conflicts and complete unfinished operations externally, then refresh.

## Navigation and Launch

Use Tab, arrow keys and Enter, or terminal mouse input. Drag column dividers to resize panes. The status area supports selecting and scrolling long error text. **ACTIONS → EXIT** closes the application.

On Windows, press Alt followed by `B` for branches, `A` for actions, `C` or `H` for the tabs, `F` for files, `M` for commits, `D` for the diff, or `S` for status. Standalone Alt activation is unavailable in the current Linux/macOS terminal backend; use the other navigation methods there. Use an interactive UTF-8 terminal on Linux/macOS.

Build instructions are in `GacUI/Tools/GitView/README.md` and `Release/Tools/README.md`. Linux/macOS builds require the matching platform provider libraries described there. Launch from the repository you want to inspect, even when the executable is stored elsewhere.
