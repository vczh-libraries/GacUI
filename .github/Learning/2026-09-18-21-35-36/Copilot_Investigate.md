# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

TODO_Task_UiaReview.md to fill the gap.

# TEST [CONFIRMED]

Use `TODO_Task_UiaReview.md` with the corrections and verification oracles in `TODO_Task_UiaReview_Details.md`. Preserve the original task file. This new investigation supersedes the previous review-only execution scope: reproduce gaps, implement their solutions and verify the changes. The prior review remains historical evidence, not runtime verification.

Extend the existing `Test/UIA_CppTest_Shared.cs` client with focused contract checks and run through `Test/UIA_CppTest_Shared.ps1` in both hosted CppTest and ordinary CppTest_Metaonly. Start with independently observable failures: combo ancestry and rejected selection, tab body hit testing, header orientation/geometry, grid selection/focus, ProgressBar read-only step values, empty visible text ranges, degenerate text selection and embedded-range relationships. Preserve the existing baseline scenarios and application semantics.

For transitions requiring controlled application state, use the existing Playground and/or unit-test fixtures rather than relying on incidental showcase state. Cover password/edit-mode transitions, event identities/payloads, custom composition semantics, lifecycle and concurrent clients. Record each actual result, including unsupported conditional cases and any environment limitations. Use the repository build/execute/debug wrappers and configured UnitTest. A successful provider call alone is not acceptance; verify the resulting model/rendered state, navigation identities and applicable events.

## Reproduction result

2026-09-18: `Test/UIA_CppTest_Shared.ps1 -Application CppTest -AsPort 8888 -Scenario Review` builds successfully (0 warnings/errors) and fails 12 contract checks against the unchanged provider: empty visible text range, degenerate Add/Remove selection, selected tab body hit testing, embedded range enclosure/children, combo item ancestry, active grid cell enumeration, unique grid focus, Header/Tab orientation and weekday header geometry. The combo rejection check preserves its publicly reported selection in this fixture; it is not a reproduced failure. An initial C# API spelling error was corrected before this baseline run.

# PROPOSALS

- No.1 Repair semantic relationships and text contracts, then verify transitions [CONFIRMED]

## No.1 Repair semantic relationships and text contracts, then verify transitions [CONFIRMED]

Use the existing semantic-node and UI-thread dispatch design. Reparent known popup windows using Microsoft's server-side-provider reparenting contract (retain the popup's HWND host provider in ordinary mode; expose matching logical parent/children in both modes). Traverse selected tab bodies independently of header bounds. Report active grid cells as the selection and focus target, with one consistent focus predicate for property and root queries. Resolve calendar header geometry through the default look's existing day table. Supply Header/Tab orientation and ProgressBar NaN step values.

Return a degenerate visible range for empty/invisible text, implement insertion-point Add/Remove, enforce modal/control enablement for all range selection mutations, and derive embedded range enclosure/children from the document's existing object markers. Keep independent embedded text stores independent. Correct bidirectional rectangle merging and horizontal scrolling where existing control APIs permit it. Nondegenerate Add/Remove retains the existing single-selection restrictions pending additional convention tests.

Audit rejected selection for mutation, edit/password transitions, selection and command event sources, popup capability filtering and stale provider behavior after these baseline fixes. Extend the contract client for each additional observable case. Record unresolved architectural work (composition overrides and remote semantic transport) separately in the companion document; it needs a cross-platform model/transport design and cannot be inferred from a local HWND provider fix.

### CODE CHANGE

Update `WindowsUIAutomationSemantic/Provider/Patterns/Actions/Text/State.Windows.cpp` and their shared private header as required. Extend the existing shared client/launcher and controlled fixture only where runtime evidence needs it. Preserve `TODO_Task_UiaReview.md`; append implementation evidence and remaining work to `TODO_Task_UiaReview_Details.md`. Run the targeted scenario in both modes, both complete UIA suites and the configured UnitTest through repository wrappers.

First implementation refinement: one document object container can contain several semantic controls at the same marker offset. Preserve the specifically requested child on ranges returned by `RangeFromChild` and cloned ranges; choose it as the enclosure only while the current range is still within that object's marker. A marker alone cannot disambiguate multiple embedded siblings. Update popup test discovery to use semantic descendants after ownership is repaired.

The ordinary-mode review scenario now passes all 65 assertions, including the twelve baseline failures. Extend the proposal with explicit `EditModeChanged`/`PasswordCharChanged` control events and reflection registrations, immediate scrubbing of cached Value on entering password mode, and a caret-independent `EnsureTextPositionVisible` API for single-line text scrolling. Add a command-line-selected Playground fixture so password/edit-mode/disabled transitions can be exercised without changing showcase resources. Generate metadata for both architectures after reflection changes. Use normal control events for Invoked and selection notifications so non-UIA actions are observable too.

Additional source corrections in the same proposal: expose Value consistently on document controls with IsReadOnly reflecting edit mode; distinguish the concrete mutex controller from arbitrary button groups; exclude non-dismissable GuiPopup instances from inherited Window/Transform capabilities; realize an already-selected offscreen grid row before opening its editor. Selection enumeration uses selected model indices rather than materializing every unselected row. Hosted hit testing first asks the existing window service for the topmost hosted window, preserving popup stacking after logical reparenting. The client will check popup parent/hit relationships and concurrent MTA reads of the same retained providers/ranges.

Transition testing passed password/read-only/disabled checks, then reproduced a horizontal visibility timeout immediately after replacing short text with a long line. CDB confirmed that the same request works with settled layout (document width 1698, viewport 438, requested end rectangle x=1678..1708, resulting text composition origin x=-1239). The source reads cached composition widths in `GuiDocumentLabel::EnsureRectVisible`; force layout after materializing the requested caret geometry and before applying that scroll. This preserves caret state and removes dependence on a later render tick. Also expose/watch geometry, grid counts, selection capabilities and Window state properties, with rectangle/NaN-aware snapshot equality.

Both ordinary and hosted Playground transition runs now pass 71 assertions. Code review additionally removes the early return that hid tooltips attached to document controls, and rejects AddToSelection of a different grid cell when the grid already has its single active cell. Add a client assertion for that rejection and preserved selection. A broad UnitTest run was still running during one relink; the resulting LNK1168 was an executable lock, not a source error. The shared launcher now has an explicit SkipBuild option for reusing already-built GUI targets while other binaries are under test.

The first complete hosted `All` run passed 1,260 assertions, including popup stacking, modal Invoke, transform operations, theme replacement and normal shutdown. The first complete UnitTest run passed 90/90 files and 1,769/1,769 cases with no appended leak dump. Generated snapshot differences are confined to the file-dialog fixture's working-directory path and the unseeded calendar's current-day highlight in `RefreshThemes/OverridesAndWindows`; restore those incidental artifacts after final verification. Both architectures' metadata was regenerated through `Metadata_Generate`, and the x64 `Metadata_Test` consumer completed successfully. Final verification rebuilds after the last grid/tooltip corrections and repeats the affected contract checks.

The final-build ordinary run reproduces a failure in the newly added second-cell AddToSelection assertion. Separate rejection and selected identity in the client diagnostic before deciding whether the provider still mutates or the client normalizes the action. Keep this check strict until its observed behavior is explained.

CDB confirms that UIA calls `WindowsUIAutomationProvider::SetFocus`, which calls `Select`, before reaching the requested `AddToSelection`; the latter sees the already-replaced active cell. Try declaring provider-owned focus for these custom logical controls, then verify from a fresh process (UIA caches provider options). If implicit focus still occurs, keep focus mutation separate from the selection action instead of weakening the rejection assertion.

Provider-owned focus does not suppress the element-level focus call; discard that experiment. Correct the focus contract instead: synthetic model nodes have no independent focus composition. Advertise focusability only for real controls and the data grid's active cell, which represents the container's actual keyboard target. `SetFocus` validates that predicate and focuses the owning control without selecting any item. Selection remains an explicit SelectionItem operation. This also prevents a rejected operation from changing selection through its implicit focus step. Verify both active/inactive cell focusability and atomic Add rejection through the original managed client; retain the original assertion.

The corrected ordinary `Review` scenario passes all 72 assertions: the different-cell Add is rejected, the previous cell identity is preserved, active/inactive focusability agrees, and the twelve baseline failures remain fixed. Both final Debug x64 and Win32 builds pass with zero warnings/errors. Repeat both full UIA suites and the transition fixture after this final focus change.

Test review found that the historical nested-tab walker only searched direct TabItem children, so the new body Pane reduced its coverage. Update `WalkTabs` to require and enter the selected TabItem's one body Pane before finding nested tabs. Earlier 1,260-assertion runs cover the named scenarios but are not full nested-tab coverage; run the corrected walker in both modes before confirming this proposal.

The final-source ordinary `All` run passed 1,265 assertions before that client-only walker correction was loaded. Its named control/window scenarios passed; supplement it with a corrected ordinary `Walk` run. The ordinary native message-box proxy required the existing owned-button BM_CLICK fallback because SendInput is unavailable on this desktop; do not count that dismissal as native keyboard/input verification.

The final UnitTest run passes 90/90 files and 1,769/1,769 cases, with the pass summary at the end of the completed log and no leak dump. This run additionally changes allocation IDs and recording frame numbers in three asynchronous file-dialog fixtures. A read-only comparison expanded each DOM element reference into its descriptor and removed only allocation IDs/frameId: all 11 changed rendered frames in `InitialName_Save`, `OpenAndClose` and `OpenAndSelect` are identical to HEAD. Restore these recording artifacts together with the already-explained working-directory/current-date differences; no authored snapshot baseline change is needed.

The final hosted `All` run with the corrected nested-tab walker passes 3,250 assertions (structure=464, property=351, text=35), followed by normal process shutdown and endpoint release. All review checks pass in this mode too, including the focus/selection rejection regression. The original task file still has SHA-256 `58E00028A48EC942441CD3820C36479CD3BC0E925E6A89D841D00CB1788DEABE`.

### CONFIRMED

The local repair is confirmed by observable client state, navigation, geometry and event checks, rather than successful HRESULTs alone. The twelve unchanged-provider failures are resolved. The additional grid failure was traced to implicit focus selecting the target before AddToSelection; the final provider keeps focus and selection separate and the unchanged rejection oracle passes.

- Final ordinary `Review`: 72 assertions passed.
- Final hosted `All`, including corrected nested-tab traversal: 3,250 assertions passed.
- Final ordinary named scenarios: 1,265 assertions passed; corrected ordinary `Walk`: 2,339 assertions passed. This separate traversal run supplements the client-only walker correction after the ordinary All client had already loaded.
- Final ordinary and hosted Playground `Transitions`: 71 assertions passed in each. These check password/read-only/disabled transitions, retained ranges, scrolling without caret changes, exact selection event cardinalities/sources, keyboard command notification and four simultaneous MTA workers reading the same providers/ranges.
- All successful UIA runs completed normal Window.Close, process shutdown and endpoint release. The ordinary native message-box dismissal qualification is recorded above.
- Both final solution configurations built with zero warnings/errors. Reflection metadata generation and consumer validation succeeded for both architectures. Final UnitTest passed 90/90 files and 1,769/1,769 cases with no leak output. `git diff --check` is clean.

Keep this implementation. `TODO_Task_UiaReview_Details.md` contains the final execution record, the composition/remote-extension design boundary and remaining conformance coverage. Those broader backlog items are not claimed complete by this local repair, and the original task document is unchanged.
