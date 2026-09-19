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

- No.1 Repair semantic relationships and text contracts, then verify transitions

## No.1 Repair semantic relationships and text contracts, then verify transitions

Use the existing semantic-node and UI-thread dispatch design. Reparent known popup windows using Microsoft's server-side-provider reparenting contract (retain the popup's HWND host provider in ordinary mode; expose matching logical parent/children in both modes). Traverse selected tab bodies independently of header bounds. Report active grid cells as the selection and focus target, with one consistent focus predicate for property and root queries. Resolve calendar header geometry through the default look's existing day table. Supply Header/Tab orientation and ProgressBar NaN step values.

Return a degenerate visible range for empty/invisible text, implement insertion-point Add/Remove and contiguous single-selection changes, enforce modal/control enablement for all range selection mutations, and derive embedded range enclosure/children from the document's existing object markers. Keep independent embedded text stores independent. Correct bidirectional rectangle merging and horizontal scrolling where existing control APIs permit it.

Audit rejected selection for mutation, edit/password transitions, selection and command event sources, popup capability filtering and stale provider behavior after these baseline fixes. Extend the contract client for each additional observable case. Record unresolved architectural work (composition overrides and remote semantic transport) separately in the companion document; it needs a cross-platform model/transport design and cannot be inferred from a local HWND provider fix.

### CODE CHANGE

Update `WindowsUIAutomationSemantic/Provider/Patterns/Actions/Text/State.Windows.cpp` and their shared private header as required. Extend the existing shared client/launcher and controlled fixture only where runtime evidence needs it. Preserve `TODO_Task_UiaReview.md`; append implementation evidence and remaining work to `TODO_Task_UiaReview_Details.md`. Run the targeted scenario in both modes, both complete UIA suites and the configured UnitTest through repository wrappers.
