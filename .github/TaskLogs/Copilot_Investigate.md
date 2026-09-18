# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

This task mainly focuses on improving `UiaList`.
- `Processes` tab:
  - Besides `Refresh` there should be a combo box taking the whole width, listing all process trees. But windows is not listed in this combo box dropdown.
    - A process is visible in this dropdown when there it or any sub process owns a visible window.
  - The tree view is replaced by a list view, when a process is selected, listing all visible windows directly owns by this process.
    - Columns show title and whatever is currently printed in the list view.
    - Hidden/Visible is no longer needed as hidden ones are not listed.
  - When left click, instead of left double click, jumps to the `UI` tab.
- `Nodes`.
  - Double clicking is no more opening a window, we need a right click context menu with "Inspect". The reason is that, currently double clicking would expand/collapse a node if it is not a leaf node.
- The property window:
  - `Properties` tab: Not supported properties is not listed.
  - `Actions` node:
    - Currently there are buttons for each provider to expand or collapse the view for that provider.
    - Expanding or collapsing is not needed, instead replace it with group boxes.
    - Keep a small margin around the group boxes, like 5, is enough to visibly distince different providers.
  - Provider UI:
    - Currently there are small gaps between properties, which is fine. But around action buttons there are big gaps, use the same gap.
    - You should check what is actual rendering via automation service to know is there big gaps between anything, I don't like big gaps, all gaps should look the same size.

Test scripts.
- Rename `CppTest_Metaonly.ps1` to `UIA_CppTest_Metaonly.ps1`.
- Rename `UIA_Showcase.*` to `UIA_CppTest_Shared.*`.
- Move `AutomationArguments.h` to `Test\GacUISrc\SharedArguments.h`.
- `AutomationPorts.ps1` is not needed, remove it.

Update stale fact in documents affected by this task.

## DETAILS

### Source ownership

- Follow `Tools/UiaList/AGENTS.md`: keep view-model declarations, rendering, bindings, and `-eval` event handlers in `Tools/UiaList/UiaList/UI`; implement non-rendering behavior in `Tools/UiaList/UiaList/ViewModel`. Regenerate `Tools/UiaList/UiaList/Source` through the sibling Tools repository's `Tools/GacBuild.ps1` with `Tools/UiaList/GacUI.xml`; do not edit generated files.
- Keep English, Simplified Chinese, and Japanese strings synchronized, including column headings and the window-selection help text. Reuse the existing localized `Inspect` string. Process names, window titles, and canonical UIA names remain target data.

### Processes

- Place the process combo box beside `Refresh`, filling the remaining row width. Its dropdown preserves the process hierarchy and allows selecting any retained process, including descendants. It contains no window rows or visible synthetic root. Keep executable names and PIDs so duplicate executable names are distinguishable.
- Reuse the existing visibility predicate in `ProcessDiscovery.Windows.cpp`: a qualifying top-level window has the visible style, is not cloaked, and has nonempty bounds. Minimized windows still qualify; their preview can be unavailable without making their UIA nodes unavailable. Retain a process when it or a descendant owns a qualifying window, and prune wholly headless branches.
- The list below the combo box contains only qualifying top-level windows whose owning PID is the selected process. Do not include windows belonging to descendant processes or enumerate child HWNDs into this list. A retained headless ancestor therefore has an empty window list, while its visible descendants remain selectable in the dropdown.
- The current implementation renders window information in tree labels, not list-view columns. Use a details list with title, window class, and pointer-sized HWND columns, retaining the localized untitled fallback. Remove Hidden/Visible labels; preserve the existing deterministic process-name/PID and window-title/HWND ordering.
- Selecting a process only updates the window list and stays on `Processes`. A single left click on a window row activates that exact window and switches to `UI`; keep Enter activation. Clicking the list background or a column header must not activate a previous selection. Returning to `Processes` and clicking an already selected row must still activate it.
- Keep discovery explicit on initialization and `Refresh`. Preserve a surviving selected process by PID and creation time when available, and the inspected window by HWND/PID rather than row index or caption. Clear the list selection when changing processes; browsing another process does not itself activate a new inspected target. On refresh, clear a process selection that no longer exists in the filtered dropdown, and clear the inspection's dependent state when its window no longer qualifies. A surviving inspected target does not require a new capture just because the process list refreshed.
- Preserve the existing worker/UI-thread separation, process request serials, and inspection generations. Repeated refreshes must not publish an older process/window list over a newer one, and window activation retains the existing HWND/PID validation.

### Nodes and property inspection

- Open the context menu for the node under the right click and make `Inspect` act on that node, even if another node was previously selected. Empty space and the synthetic root offer no active inspection command. Dismissal does not open a property window. Preserve normal double-click expand/collapse, Enter inspection, modal ownership, and focus restoration.
- Dismiss or invalidate the menu when its node/tree generation changes. In `UiaListViewModel.cpp`, `InspectNode` currently calls `SelectNode`, which rejects stale nodes, and then uses the existing selected node. Ensure a stale menu command cannot fall through to inspecting a different selection.
- Filter unsupported rows using `native::ValueKind::Unsupported` when `PropertyDialogViewModel::Publish` builds the Properties grid. Do not filter by display text, truthiness, or writability: supported `false`, zero, empty strings/arrays, null, mixed, and read-only values remain visible. Keep the property catalog and native value conversion intact.
- Apply filtering on initial inspection and subsequent refresh/action readback. Restore selection by property key among the remaining rows; clear it when the selected row disappears. Preserve inline editors and multiline/detail dialogs. This Properties-grid rule does not remove unsupported action results, text-range attribute states, or operation/HRESULT diagnostics.

### Provider groups and spacing

- `Actions` refers to the property window's Actions tab. Replace provider header buttons with titled group boxes whose contents are always shown, arranged vertically at the full scroll-container width. Keep the Providers, Results, References, and Text ranges pages and their existing behavior.
- `ActionSectionTemplate` in `ActionTemplates.xml` is also used by text-range workspaces; update that shared rendering consistently. Remove obsolete section expansion state from the authored view-model interface and C++ implementation, then regenerate.
- Preserve parameterless getter readouts, parameterized getter evaluation on valid argument commits, explicit Refresh, and explicit mutation/listening/workspace commands. `ActionSectionViewModel::SetIsExpanded` currently also queues getter queries and a complete inspection; removing the expander must not introduce one complete inspection per displayed group or cause rendering to execute mutations.
- Follow `.github/Guidelines/GacUILayout.md`: target roughly five pixels between visible objects and around groups. Account for accumulated table padding, stack padding, child margins, and group-box content insets. Compact empty status/validation rows so they do not leave large holes, while allowing actual messages to wrap. Check command rows with and without parameters, readouts, references, and text-range groups; changing every XML padding value to the same number is not sufficient verification.

### Scripts, header relocation, and documentation

- The script renames are under `Test`. Rename both `UIA_Showcase.ps1` and `UIA_Showcase.cs` to `UIA_CppTest_Shared.*`; update the hosted wrapper `UIA_CppTest.ps1`, the renamed `UIA_CppTest_Metaonly.ps1`, and the shared script's C# loading path. Preserve scenario arguments, default ports, wrapper-based launching, and the windowless MTA client.
- Move `Test/AutomationArguments.h` to `Test/GacUISrc/SharedArguments.h` and update all includes and explicit project/filter entries. Consumers include `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `CppTest_Rvm`, `GacUI_Host`, `Playground`, `RemotingTest_Core`, `RemotingTest_Rendering_Win32`, and the separate `Tools/UiaList/UiaListApp` project. Check both `Main.cpp` and `GuiMain.cpp` where applicable. The file move does not require renaming `gacui_test::AutomationArguments` or changing `/AsPort` parsing and endpoint behavior.
- Delete `Test/AutomationPorts.ps1` as requested and remove active instructions to run it. Do not introduce a replacement suite solely for this deletion.
- Adapt `Tools/UiaList/Verification/Smoke.ps1`, `DeepTree.ps1`, and `Inspector.ps1` to select the fixture process, single-click its window row, and use Nodes' context-menu `Inspect`. Identify fixtures by PID when captions are ambiguous. Preserve property-value and returned-reference double-clicks, which are separate interactions. Recheck driver assumptions about viewport bounds, scrolling, and getter-field positions after the layout change.
- Update current behavior and commands in `Tools/UiaList/AGENTS.md`, `README.md`, `Planning.md`, and `Verification/README.md`, including their acceptance criteria. Correct stale claims about GacUI lacking native UIA and the fixed inspector automation port where encountered; UiaList's Debug endpoint supports `/AsPort` and defaults to 8888. Keep recorded historical test results factual and distinguish obsolete commands from current instructions. Mark older conflicting task requirements as superseded without rewriting their original wording, and search other active documents for affected names and interactions.

## VERIFICATION

These checks are for implementing the task; this review only updates the task document.

### Generation and build checks

- Run GacBuild for `Tools/UiaList/GacUI.xml`, checking both architecture outputs and generation errors. Build `Tools/UiaList/UiaList.sln` through `copilotBuild.ps1` for Debug/Release and x64/Win32. Run `Verification/CheckCatalog.ps1` to check synchronized localization keys and explicit inventories.
- Build `Test/GacUISrc/GacUISrc.sln` through the repository wrapper for Debug x64 and Win32 to catch relocated-header consumers. Run the required GacUI UnitTest through `copilotExecute.ps1 -Mode UnitTest`, respecting the configured test filter; inspect the completed log and any appended leak report. Follow `Project.md`'s additional generation rules if implementation touches their triggering files.
- Search source, scripts, and project/filter inventories for obsolete filenames and verify all new paths resolve. Historical documentation references must be identifiable as historical rather than runnable current instructions.

### Process selection and navigation

- Verify a process with multiple visible windows, a headless parent with a visible child, a wholly headless branch, duplicate process names/window titles, an untitled window, an owned dialog, hidden/cloaked windows, and a minimized window. The dropdown has precisely the qualifying process hierarchy; each selected process lists only its own qualifying windows with the expected columns and full-width layout.
- Verify dropdown hierarchy navigation and selection without activating a window. Test a single row click, Enter, clicking the selected row again after returning from `UI`, and clicks on the background/header. Only window activation switches to `UI`.
- Add, hide, show, and close windows externally, then refresh. Verify preservation or clearing of process/window identities and dependent state, including an empty list for a retained headless ancestor. Repeat refresh and switch targets during loading; stale completions must not restore old selections or snapshots.

### Inspector behavior and rendered layout

- Right-click an unselected node while another is selected and verify Inspect opens exactly the clicked node. Cover leaves, expanded/collapsed parents, empty space, cancellation, and a tree refresh while the menu is open. Double-click must not open a modal; Enter and focus restoration still work.
- Inspect a target with unsupported properties and valid false/zero/empty/null/mixed values. Verify only unsupported rows disappear, initially and after Refresh/action readback. Cover a selected row disappearing, read-only detail viewing, inline editing, and multiline OK/Cancel.
- Verify provider groups remain visible after refresh/actions and have no collapse buttons. Exercise a parameterless getter, invalid and committed parameterized getter inputs, a mutation, returned-element inspection, and a text-range workspace. Use the independent fixture log to confirm invalid inputs make no provider call and deliberate mutations execute exactly once.
- Use the Debug automation endpoint's Controls output and actual rendered screenshots to inspect spacing. Cover short/long groups, commands with/without parameters, empty/populated validation and status text, Results/References/Text ranges, and scrolled content at default and reduced window sizes. Check visible gaps, full-width groups, clipping, and reachability of the final controls; record observed bounds and visual results rather than relying only on XML values.
- Drive popup menus and modals using their current window IDs from Controls. Use the exact `application/json; charset=utf8` content type, and read resulting state after queued IO before asserting success. Run the adapted smoke/getter regression for Debug x64 and Win32 and the deep-tree replacement workflow; retain independent call-count and normal-shutdown checks.

### Renamed script regression

- Run `Test/UIA_CppTest.ps1` and `Test/UIA_CppTest_Metaonly.ps1` sequentially, invoking them from outside `Test` as well, to verify shared-script/C# resolution and MTA self-launch. Require passing assertions, normal shutdown, released endpoints, and restoration of temporary `.vcxproj.user` settings.
- During inspector verification, use distinct ports such as CppTest 8888, CppTest_Metaonly 8890, and UiaListApp 8891. Confirm the relocated argument parser still honors the chosen automation port and Release UiaList starts no automation endpoint. Retain the documented default and rejection behavior for malformed, duplicate, empty, or out-of-range `/AsPort` arguments.

## REVIEW COMMENTS

No unresolved review comments.

# TEST [CONFIRMED]

Reproduce the current process tree, node double-click modal, unsupported property rows and expandable provider cards using the existing native fixture and Debug automation endpoint. Extend the existing verification scripts for the requested identity, navigation, menu, filtering, getter and spacing contracts. Run the generation, four UiaList builds, both GacUISrc Debug builds, configured UnitTest, catalog checks, fixture workflows and renamed showcase scripts described above. Record runtime evidence and any limits explicitly.

# PROPOSALS

Baseline Debug x64 UiaList and native fixture builds passed with zero warnings/errors. Live Controls showed process-owned window labels with `Visible`, a node double-click opened the Invoke counter property modal, and Actions rendered provider headings as Buttons. SetFocus label bounds were y=209..225 and ShowContextMenu y=269..285 (60-pixel pitch). Getter parameters propertyId and metadataId had y=350..380 and y=442..472 (92-pixel pitch). Authored Publish currently adds every property without testing ValueKind. These observations confirm the requested behavior/layout changes; they do not claim unsupported-row runtime coverage yet.

- No.1 Separate process browsing from inspected targets, make node inspection explicit, and compact persistent provider groups.

## No.1 Separate process browsing from inspected targets, make node inspection explicit, and compact persistent provider groups

Retain native discovery and worker serial/generation checks. Expose process-only hierarchy and a per-process qualifying window list, preserve process PID/creation and inspected HWND/PID identities, and use item events for exact row activation. Keep rendering, popup ownership and event bindings in authored XML. Validate the actual node before inspection and invalidate menu commands when the tree changes. Filter only Unsupported property values in Publish. Remove section expansion behavior entirely; preserve the existing inspection readouts, committed parameter queries and explicit commands. Render full-width titled groups with compact optional rows and measured spacing. Rename the existing scripts, relocate the shared argument header and update inventories/current documentation. Extend existing fixture scripts and run the requested build/runtime matrix.

### CODE CHANGE

Planned changes are in authored UI and ViewModel files, the existing verification scripts/fixture where needed, Test launchers and SharedArguments.h consumers, generated UiaList outputs through GacBuild, and affected documentation. No shared GacUI behavior change is planned.