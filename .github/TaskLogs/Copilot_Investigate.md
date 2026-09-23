# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Continue to work on the GitView tool, you can check out the current Copilot_Investigate.md to understand the details:
- All controls accepting user interaction should be assigned with alt labels:
  - Branch dropdown
  - Actions (already has)
  - Both tab page
  - File list in Changes tab
  - Commit and file list in history tab
- Currently the author name is displayed after the commit name in the history commit list, remove it and display in the text box instead like this: `COMMIT (author) DATE`.
- Make sure it works under Linux:
  - `vmake` is only for `GitTui`, but there is `GitTui` and `GitViewTests`.
  - I would like you to have both `vmake` in `Tools/GitView/Linux/(GitTui|GitViewTests)`.
  - Update this information in `Tools/GitView/AGENTS.md`.
- commit and push once finishing.

# UPDATES

## UPDATE

Also one thing to fix in the diff rendering, currently diffs are rendered as multiple groups of consecutive lines. But there is no visual separation between each group. You need to add the separation, which should be a simple gray horizontal bar with normal background as a list item. Pay attention to line numbers, when two groups actually have consecutive line numbers, they are seen as one. Hopefully the data structure already have such information.

# TEST [CONFIRMED]

The Linux regression test reproduces the old combined subject/author formatting after the locale prerequisite is fixed. Extend the existing real-repository/view-model test to require subject-only history rows and exact `HASH (author) DATE` status, retained after selecting a historical file. Build GitViewTests and GitTui through the repository Linux build.sh in their new directories, run all focused tests, and exercise navigation, history and diff rendering in a PTY terminal. Check all authored Alt labels; record the existing POSIX standalone-Alt limitation separately. Review generated sources and build inventories. Shared GacUI tests are unnecessary for tool-only changes under Project.md.

# PROPOSALS

- No.1 Alt navigation, separate commit metadata, and independent Linux targets [CONFIRMED]

## No.1 Alt navigation, separate commit metadata, and independent Linux targets

Set distinct Alt labels on the branch combo, tab pages, selectable lists, status viewer and all action menu entries. Keep the author in a separate GitCommit field parsed from NUL-delimited git log output, render only the subject in history, and format the status as `HASH (author) DATE`. Move the GitTui vmake into Linux/GitTui with corrected relative paths; add Linux/GitViewTests without native renderer dependencies. Regenerate XML C++ with the native GacGen entry point and document both Linux build/run workflows in AGENTS.md and README.md.

### CODE CHANGE

Added Alt labels and independent Linux configurations. Native GacGen /P32 and /P64 plus the existing CppMerge preserve the original portable generated C++ types; direct /C64 alone would replace vint with vint64_t and is insufficient for the cross-platform checked-in source. Document this paired generation workflow.

The first Linux test run built but failed creating the Unicode filename fixture: unlike the TUI renderer setup, the test entry point does not initialize LC_CTYPE. The test entry point now initializes it from the environment before running tests. Continue to require an installed UTF-8 locale.

For the follow-up diff request, retain the parser's new-file cursor and context-start indices. Insert one explicit separator DiffLine only when the next context start exceeds the previous context end; touching or overlapping ranges merge naturally. Expose IsSeparator through the authored IDiffLine contract and render a one-row gray TuiBorder on the ordinary background in DiffLineTemplate. Test separated, overlapping and exactly touching context ranges, including insertions/deletions shifting old and new line numbers.


### CONFIRMED

- Both `Tools/GitView/Linux/GitTui` and `Tools/GitView/Linux/GitViewTests` built successfully through `.github/Ubuntu/build.sh` on Linux x64. The successful final compiler logs contain no warnings or errors. Both no-change incremental builds completed without compiling or linking. The generated inventories contain 17 and 18 valid, distinct sources respectively, use the POSIX command implementation and exclude Windows sources. GitViewTests does not link native renderer libraries.
- `LC_ALL=C.UTF-8 ./Bin/GitViewTests /C` passes 1/1 files and 11/11 cases. New assertions verify subject-only commit rows, separate author metadata, exact `HASH (author) DATE` status and retention above the file path. Diff assertions cover one separator for omitted gaps, no separator for overlapping/touching context, insertion/deletion line-number shifts, and normal separator background. Existing Git, Unicode filenames, diff, submodule, worktree and pull/conflict cases also pass.
- Native GacGen `/P32` and `/P64` completed without `Errors.txt`; the existing Workflow CppMerge generated the portable checked-in C++ and reflection files. All 15 interactive authored XML controls have Alt labels. No generated source or build inventory was hand-edited.
- Linux PTY input/output verification at 120x40 and 100x32 used a disposable real repository. Changes displayed a horizontal separator between source lines 7 and 27; History joined lines 7 and 8 and separated lines 14 and 27. Parsed terminal cells confirmed the rule glyph has foreground `#808080`, background `#000000`, including when selected. The bar spans the resized diff viewport, has no line number, and occupies one list row. History showed only commit subjects, with the hash, parenthesized author and ISO date in status, retained after file selection. Mouse selection, branch dropdown arrow/Enter navigation, selection clearing after changing the browsed branch, and normal Actions/Exit shutdown passed (exit 0).
- Existing POSIX limitation: the terminal input decoder does not produce standalone Alt key events, as already documented in `.github/Jobs/DebugTuiControlTestSop.md`. Therefore native Alt-sequence activation is not verified or claimed on Linux/macOS; the assigned labels are ready for supporting providers. README.md and AGENTS.md document this limitation and the ordinary navigation alternatives. Physical desktop input/display and Windows/macOS runtime were not exercised on this Linux host.
- Project.md exempts tool-only changes from the shared GacUI suite; no library or skin source changed. Failed-test fixtures and the terminal fixture were removed after verification. Build outputs remain ignored; only authored files and genuine generated outputs are committed.
