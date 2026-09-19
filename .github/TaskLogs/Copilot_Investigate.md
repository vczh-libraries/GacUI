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
