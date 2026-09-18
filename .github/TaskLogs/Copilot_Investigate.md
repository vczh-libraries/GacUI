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

- No.1 [CONFIRMED] Separate process browsing from inspected targets, make node inspection explicit, and compact persistent provider groups.

## No.1 [CONFIRMED] Separate process browsing from inspected targets, make node inspection explicit, and compact persistent provider groups

Retain native discovery and worker serial/generation checks. Expose process-only hierarchy and a per-process qualifying window list, preserve process PID/creation and inspected HWND/PID identities, and use item events for exact row activation. Keep rendering, popup ownership and event bindings in authored XML. Validate the actual node before inspection and invalidate menu commands when the tree changes. Filter only Unsupported property values in Publish. Remove section expansion behavior entirely; preserve the existing inspection readouts, committed parameter queries and explicit commands. Render full-width titled groups with compact optional rows and measured spacing. Rename the existing scripts, relocate the shared argument header and update inventories/current documentation. Extend existing fixture scripts and run the requested build/runtime matrix.

### CODE CHANGE

Implemented the process-only dropdown and independent own-window list in authored MainWindow.xml and UiaListViewModel. Process browsing preserves the inspected target; activation validates the current process's window, and refresh preserves PID/creation and HWND/PID identities or clears dependent state. The Nodes menu stores the clicked node, invalidates it on busy/tree changes, and validates the exact node and generation in C++ before opening a modal. Double-click retains tree expansion, and modal completion restores keyboard focus asynchronously.

PropertyDialogViewModel::Publish filters only ValueKind::Unsupported and retains key-based selection. PropertyWindow clears its grid selection when the selected property disappears. ActionSection no longer exposes expansion state or performs expansion-triggered inspection; existing readouts, committed getter queries and explicit mutations remain. Shared action/text-range rendering uses persistent full-width GroupBox controls, compact optional rows, five-pixel stack gaps and content insets. References uses a two-row table to keep its list and buttons five pixels apart.

Renamed the two native UIA wrappers/shared files, moved the unchanged parser to Test/GacUISrc/SharedArguments.h, updated every include/project/filter consumer, and deleted AutomationPorts.ps1. Updated current documentation and marked older task requirements superseded. Extended the existing fixture/drivers for qualifying window identities, exact-node menus, supported empty/false/zero/null properties, Unsupported transitions, getter commits and independent command counts. Regenerated UiaList Source only through GacBuild; no shared GacUI source behavior changed.
Implementation refinements: optional action rows use NoLimit while empty, and group/content padding is measured from Controls plus screenshots. The existing smoke now covers exact window rows, headless-parent browsing, external visibility changes, menu dismissal, double-click/Enter, and typed supported values. The native fixture adds optional navigation windows and HelpText becoming unsupported after Toggle. The renamed MTA client must wait for the GacUI provider identity after the HWND appears: repeated immediate metadata-only runs observed FrameworkId=Win32, while a later probe of the same live process passed. Keep the existing identity assertions and timeout; this is a startup readiness fix in the shared test client.

Menu verification found and corrected an implementation issue: WindowsForm::Hide posts WM_CLOSE even for an already closed popup. The unconditional nodeMenu.Hide immediately before ShowPopup therefore asynchronously dismissed the freshly opened menu. CDB traced the only Hide call to the authored right-click handler, and native source confirmed the queued close. Remove that hide-before-show and only issue invalidation hides for an open menu; retain the exact-node/generation validation. This is within the existing proposal.

The Enter-after-modal-close regression also exposed premature focus restoration. GuiWindow::ShowModal invokes the close callback before re-enabling/refocusing the owner, so the immediate nodeTree.SetFocused (and nested propertyGrid.SetFocused) is ineffective. Defer those rendering-only focus calls through InvokeInMainThread after modal completion. Keep the regression waiting for actual dialog closure before sending Enter.

### CONFIRMED

Final GacBuild /P32 and /P64 outputs generated successfully without error files. Final UiaList Debug x64/Win32 and Release x64/Win32 builds passed with zero warnings/errors (26.53, 28.78, 92.78 and 80.09 seconds). GacUISrc Debug x64/Win32 builds passed after header relocation. UnitTest completed 90/90 files and 1,769/1,769 cases with no appended leak report; inspected environmental snapshot differences were restored. CheckCatalog passed 175 properties, 35 patterns, 44 attributes, 41 control types, one metadata ID and 42 synchronized keys per locale. All 18 shared-header project/filter entries resolve; obsolete source/script filenames are absent. The expanded x64 Smoke -GetterRegression -NavigationRegression passed against fixture PID 16360 in 218.78 seconds, with exactly one deliberate native Invoke and exit code zero.

The same full smoke passed on Win32 against fixture PID 11788 in 325.24 seconds, also with one Invoke and exit zero. Both concurrently running fixtures had identical executable names and window captions. Each run established eight navigation fixtures, exactly five qualifying windows, and compared every displayed HWND to independent native enumeration. Headless ancestor browsing, process pruning, repeated refresh, selected-row reactivation, header/background clicks, Enter, collapsed-parent inspection, exact clicked-node inspection, menu cancellation/invalidation, read-only nested details and restored keyboard focus passed. Supported false/zero/empty BSTR/empty array/null properties survived, HelpText disappeared on native Unsupported and returned after Toggle, and each of the three Toggle mutations was logged once. Invalid getter drafts caused no provider call; valid commits, rapid replacement, returned references, StartListening/Cancel and text-range Clone passed their independent log checks.

Additional live checks: editable multiline Cancel produced no SetValue(BSTR); OK produced exactly one logged UTF-16 value `layout value 日本語 中文`. Inline RangeValue editing produced one SetValue(R8) with 41. A 660×480 text-range workspace retained a visible Mixed readout at y=388..408, and its unsupported attribute state remained in the composition. The final text-range command was reachable at y=291..311; its actual caption includes the IUIAutomationTextRange2 prefix.

Measured layout evidence: the 900×650 provider view's actual SetFocus and ShowContextMenu buttons occupy y=188..208 and 213..233, giving a five-pixel gap. Getter editors occupy y=263..293 and 298..328. At 660×480, GetItem editors occupy y=343..373 and 378..408, full-width provider contents span x=31..609, and the final Navigate getter is reachable. Invalid input inserts a 21-pixel validation composition with five pixels of top internal margin, followed by the next editor five pixels below; empty validation contributes no row. Results has operation y=142..158, value y=163..179 and the Details button y=184..204. References now has list bounds y=136..575 and buttons y=580..600: five pixels apart; resizing moves its buttons to y=410..430 while keeping them reachable.

Actual default-size screenshots earlier in this investigation showed persistent titled groups, compact rows and no provider collapse buttons. Subsequent screenshot capture became unavailable: Windows Graphics Capture returned 0x80070057 and PrintWindow produced a black image. Therefore final References/reduced-size screenshots are not claimed; their bounds and interactions were verified through Controls. This remains a visual-verification limit, alongside the previously documented untested DPI/OS-language matrices.

One ad-hoc scroll probe used an incomplete caption and was interrupted while an HTTP request was pending. CDB then located abort/terminate in HttpAutomationService::OnHttpRequestReceived, matching the existing documented fatal response-send policy. This was a harness interruption, not a passing shutdown test. The affected process was terminated and the deep-tree workflow restarted without canceling requests; verbose expected first-chance notifications were suppressed for that stress run. No shared HTTP behavior was changed.

The renamed native UIA launchers were invoked from the repository root, outside Test, with windowless MTA clients. The final shared client passed 4,434 hosted assertions (structure=429, property=339, text=15), and the metadata-only run passed 4,444 assertions (structure=418, property=328, text=13). Both reported normal Window.Close and endpoint release on their respective 8888/8890 ports. Temporary .vcxproj.user files were removed/restored; no settings file remained for either showcase or UiaList. The shared header's Git rename is 100% identical: parsing behavior was not edited.

The final `DeepTree.ps1 -AsPort 8891 -FixtureProcessId 16360 -Cycles 25 -CloseWithDeepTree` run passed all 25 complete traversal/replacement cycles, each observing 11,053 then 51 nodes (40.86–56.31 seconds per cycle). It then loaded the full tree once more and closed. CDB's final event was process exit code 0 at 16:55:13; the completed debugger log contains no CRT leak report or abort. Graphics Capture reported unavailable/canceled capture on the current desktop; this did not prevent raw-tree traversal, replacement or shutdown.

Final wrapper-based argument probes observed Debug's default endpoint on 8888 and normal exit. The x64/Win32 smoke and CDB runs had already exercised explicit 8891. `/AsPort`, `/AsPort:`, `/AsPort:abc`, `/AsPort:0`, `/AsPort:65536` and duplicate `/AsPort:8891 /AsPort:8892` each exited with code 1. Release with `/AsPort:8891` opened its native inspector window, exposed no UiaList endpoint on either 8888 or 8891, and exited normally with code 0. Temporary configuration was removed afterward. A combined probe command was rejected by automatic approval review without a specific reason; the individual probes completed using explicit configuration edits and the existing wrapper.

Final review found no further source changes necessary. UI event/focus/menu behavior remains in authored XML, model identity/filtering behavior remains in C++, and generated changes came only from GacBuild. The proposal is retained with the explicit screenshot/DPI/language verification limits above. Remote changes fetched before delivery affected only ToDo/1.4.1.3.md; rebase does not change tested source.
