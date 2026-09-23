# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

You are required to do some clean up test cases in remote protocol based unit test, especially in TUI and EazyLayout. All TUI cases have no frame, Reject cases with a few others in EazyLayout have no frame. Such cases are not allowed to appear in here because the point of remote protocol based unit test is to record rendering result. By the way, other EazyLayout snapshots don’t seem to offer any valuable information either, maybe you could just convert all of them to trivial unit test. You also verify other test cases and see if there is such cases, clean them up and write the rule down in Project.md. If you think those cases are valuable, you can construct these cases without involving remote protocol based unit test framework (aka test cases that use `GacUIUnitTest_StartFast_WithResourceAsText` and other similar functions)

# TEST [CONFIRMED]

- Audit every remote-protocol startup call and wrapper in UnitTest, then inspect tracked snapshot indexes for empty frame lists and non-rendering cases.
- Preserve valuable assertions in ordinary unit tests, with direct controller/resource setup where needed, and remove obsolete snapshot artifacts belonging to converted cases.
- Ensure retained remote rendering cases produce meaningful frames. Build Debug x64 and run the complete UnitTest suite, inspect case counts, leak output and generated snapshots, and repeat the audit for newly created empty outputs.

# PROPOSALS

- No.1 Keep rendering snapshots only for rendering tests [CONFIRMED]

## No.1 Keep rendering snapshots only for rendering tests

The snapshot audit found 34 empty indexes: three live TUI cases and 31 stale EazyLayout compiler/namespace rejection outputs. Existing ordinary compiler and descriptor rejection tests already preserve the latter coverage. The remaining five EazyLayout test bodies (ten resource variants) assert descriptor values, geometry, bindings and events; their snapshots are unnecessary.

Replace the TUI cases' outer snapshot startup with the ordinary compiler-controller application bootstrap already used by resource tests. Run EazyLayout assertions synchronously using the existing in-memory TUI native provider, direct resource compilation/instantiation and explicit layout calculation. Preserve all assertions and payload types, invoke control events and splitter mouse events directly, and retain the existing ordinary rejection tests. Remove both obsolete snapshot folders and document the requirement for useful captured rendering in Project.md.

### CODE CHANGE

- Convert TUI startup and EazyLayout resource tests without using the remote snapshot framework.
- Delete obsolete Tui and EasyLayout snapshot artifacts, including stale compiler outputs.
- Audit all remaining snapshot indexes and startup wrappers; retain rendering and protocol-rendering contract tests.
- Document the scope of snapshot tests and the ordinary-test alternative in Project.md.
- Remove the TUI cases' nested hosted-application swap: the ordinary outer compiler controller has no hosted application to clear or restore. The debugger caught the old unconditional clear after the snapshot wrapper was removed.
- Initialize application plugins with the compiler controller before installing the in-memory TUI services for EazyLayout assertions, matching the existing live TUI cases. Plugin resources require an image service that the minimal TUI test controller intentionally does not provide.
- Use TuiSkin with the in-memory TUI provider. DarkSkin creates graphical elements unsupported by that provider; the geometry assertions use explicit EazyLayout sizes and remain unchanged.

### AUDIT

- Removed 175 tracked files from the Tui and EasyLayout snapshot groups: 34 empty indexes and ten unnecessary EazyLayout rendering recordings, with their associated frame, protocol and compiler files.
- The remaining 756 snapshot indexes all reference existing frame files and contain rendering elements in at least one saved frame. Their groups are Application (54), Controls (691), DomRecovery (2), UnitTestFramework (8), and HelloWorld (1).
- Every remaining source file that starts snapshot recording schedules idle frames, including shared editor and asynchronous-channel helpers. DOM recovery's empty-window test records a real background, and the framework cases exercise captured rendering, clipping, images and IO; they remain snapshot tests. Some deliberately use the unnamed-frame overload.
- Existing ordinary EazyLayout rejection coverage in TestResource.cpp and TestCompositions_EazyLayoutFailures.cpp remains intact. No other empty snapshot group was found.
- Source review confirms that all 50 original EazyLayout assertions and all 291 TUI assertions remain. EazyLayout adds seven setup assertions and replaces simulated remote IO with direct state changes and event dispatch. Its five cases pass under CDB, including all six descriptor syntax/namespace combinations.

### CONFIRMED

Debug x64 builds successfully with zero warnings and zero errors. The complete UnitTest run passes 93/93 files and 1805/1805 cases with no skipped files and no memory-leak report. This includes the converted TUI and EazyLayout cases, ordinary rejection tests, and every retained rendering test.

The final snapshot audit finds 756 indexes and 4348 saved frames, with no empty indexes, missing frame files, or cases lacking rendering elements. Neither removed snapshot group is recreated. The asynchronous ScrollResetOnNavigation case changed only frame IDs and intermediate protocol timing; its saved rendering data is identical after excluding frameId, so that unrelated generated noise was restored to the baseline.

The converted cases retain their useful assertions without starting a remote protocol recorder. EazyLayout explicitly calculates geometry after direct mutations, while native services support the existing controls and bindings. TUI tests continue to assert actual buffer output through their own backend. Project.md now requires useful rendering frames and ordinary tests for coverage that does not need rendering recordings, including removal of obsolete snapshot artifacts when converting cases.
