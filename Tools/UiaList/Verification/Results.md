# Implementation verification — 2026-09-13

Environment: Windows 11 Pro 10.0.26200, Windows SDK 10.0.26100.0, MSVC 14.51.36231/v145, 96 DPI. These are observed results, not a declaration that every acceptance scenario in Planning.md has passed.

## Build and generation

| Check | Result |
| --- | --- |
| B01 | GacBuild generated both architectures. An authored-string change appeared in both outputs. Restoring it reproduced the production-file SHA-256 set; an unchanged generation preserved it. |
| B02 | Debug/Release × x64/Win32 built through copilotBuild with zero warnings and errors. A clean Release/Win32 rebuild removed stale incremental-PDB warnings after its output directory changed. |
| B05 | The existing GacUI UnitTest passed 90/90 files and 1,768/1,768 cases on both full runs. No leak report followed the results. Environment-dependent regenerated snapshots were restored. |
| C01/C04 inventory | CheckCatalog matched 175 property IDs, 35 pattern IDs, 44 text attributes, 41 control types, one metadata ID, and 43 translation keys in each locale. Handwritten and generated C++ project/filter inventories are explicit. |
| Source ownership | XML owns all view events and bindings. C++ implements model behavior. Generated Source came from GacBuild; the changed GacUI release pair came from CodePack. |

## Observed GUI behavior

- Processes displays native and synthetic fixture windows. Selecting a window captures its image and traverses Raw View. Native and synthetic property dialogs expose complete catalog rows and acquired pattern cards.
- The normal synthetic fixture exposes 51 raw nodes on this OS, including 41 standard semantic roles and unknown role 59999. Its capability and document nodes expose all 35 standard patterns collectively. The deliberately combined patterns establish adapter coverage, not conformance of those combinations as real controls.
- A native single-line Value edit submitted Unicode text `Inspector value 日本語`; the independent target log observed its edit notification. A native document range opened the 44-attribute workspace and returned a separate Clone reference.
- The x64 modal-ID fix was exercised with real IDs greater than 2³², including the nested read-only property viewer. Win32 also opened both modal levels and captured a 96-DPI window. Normal Debug exits under CDB returned zero without a CRT leak report.
- Sixty native smoke cycles completed across all four inspector/target architecture combinations, as detailed below. Each cycle launched through the repository wrapper, selected the target, opened both modal levels, invoked the native counter exactly once, refreshed, and exited with code zero. Independent WM_COMMAND notifications were the mutation oracle.
- The large fixture returned 11,053 raw nodes: its 11,047 synthetic nodes plus six OS nodes. All 10,000 siblings and the 1,000-level branch were traversed. Scrolling reached the final sibling, stress node 10046. Initial traversal took about 49 seconds.
- After the ownership fix, 25 consecutive complete traversal/replacement cycles passed (41.4–61.7 seconds each, average 47.1 seconds). A separate full-tree shutdown check exposed late detachment of bound observers; moving detachment into RequestClose fixed that case. The final executable-only x64 Debug run closed with all 11,053 nodes loaded, CDB exit code zero, and no CRT leak report.
- Both Release architectures started from directories containing only a renamed copy of the executable, with ordinary system/runtime dependencies. Native window inspection found the main window; HTTP service-state inspection found no Release URL registration. Both processes exited normally with code zero. Other modal/locale combinations in B03 remain separate coverage.
- The final Win32 Debug executable ran from a directory containing only UiaListApp.exe, selected the Win32 native fixture, loaded 26 raw nodes, opened both modal levels, and closed normally. CDB recorded exit code zero at 21:49 PDT, with no CRT leak report. Together with the x64 Debug and both Release runs, all four configurations have executable-only startup evidence; complete B03 locale/modal coverage remains unfinished.

| Inspector | Native target | Consecutive passing cycles | Local evidence |
| --- | --- | ---: | --- |
| Debug/x64 | Debug/x64 | 25 | `smoke-x64.log` |
| Debug/Win32 | Debug/x64 | 25 | `smoke-win32-x64.log` |
| Debug/x64 | Debug/Win32 | 5 | `smoke-x64-win32.log` |
| Debug/Win32 | Debug/Win32 | 5 | `smoke-win32-win32.log` |

The first row preceded the deep-tree lifetime fix; the remaining three rows exercised the final implementation. Deep-tree evidence is in `deep-tree-reruns.log` and `deep-shutdown-fixed.log`; executable-only Win32 Debug evidence is in `deployment-debug-win32.log`. These volatile logs are retained locally and ignored by Git. The checked-in scripts reproduce the input sequence and assertions.

The synthetic action pass accepted these commands through the GUI: GetCurrentSelection, GetClickablePoint, Invoke, Scroll, Expand, GetItem, GetViewName, SetCurrentView, WaitForInputIdle, RemoveFromSelection, Select, row/column header queries, row/column header-item queries, Toggle, Resize, DoDefaultAction, GetIAccessible, FindItemByProperty, Realize, StartListening, Cancel, GetUnderlyingObjectModel, GetCurrentExtendedPropertiesAsArray, GetItemByName, annotation-object/type queries, Zoom, TextRange, GetCurrentGrabbedItems, and Navigate. Returned IAccessible/ObjectModel objects remained opaque. This does not establish every parameter value, version-only combination, or error branch.

## Defects found during verification

1. GacUI's Windows automation endpoint parsed pointer-sized modal IDs through Windows' 32-bit unsigned-long conversion. It now formats/parses 64 bits and rejects values that do not fit the process pointer size before matching registered windows. Both architectures' real modal IDs were exercised.
2. The first capture implementation lacked agile callback identity. The capture handlers now implement the required agile COM contract; repeated capture completed.
3. Acquired pattern interfaces are retained throughout capability description and dispatch. Optional capability disappearance no longer produces a null dereference between repeated acquisitions.
4. Releasing a 1,000-level node ownership chain recursively overflowed the UI thread stack on a window switch. CDB identified repeated NodeViewModel/ObservableList/Ptr destructors. Teardown now retains the graph and removes child ownership from the leaves upward, covering switch, refresh, process-tree replacement, and final destruction. Shutdown performs this in RequestClose while observers are still alive, before destruction of the bound tree. DeepTree.ps1 reproduces replacement; `-Cycles 0 -CloseWithDeepTree` exercises loaded-tree shutdown.
5. An early smoke harness canceled startup Controls after one second. CDB identified the existing fatal HTTP send-error path while inspection workers were idle. The harness uses 15 seconds and restarted the consecutive count; the product's fatal transport policy was preserved.
6. An occupied Debug URL raised `HttpAddUrlToUrlGroup failed` with native result 183. Startup now displays the operation and full URL, and the confirmed process exit is 1. The D2D debug layer breaks during this deliberate fatal shutdown; continuing reaches ExitProcess's requested code. Error testing must keep CDB's second-chance C++ exception break enabled (`sxd eh`); ignoring C++ exceptions is unsuitable evidence for failure behavior.

## Remaining acceptance coverage

Implementation is present, but the full acceptance matrix is **unfinished verification**. Cases without explicit evidence above or below must not be treated as passed. Outstanding coverage includes controlled headless process forests/PID reuse/cross-process descendants; delayed-provider and every failure race; mixed-monitor 150%/200% and negative origins; all OS-language preference combinations and visual layout screenshots; secure/protected desktop cases; SDK Inspect and real browser/MSAA comparisons; every typed editor/draft/recycling and capability-change permutation; all enum values and version-only pattern combinations; real drag/composition/synchronized input state; and all R01–R10 range behavior permutations. The fixture logs text selection/context-menu calls but does not replace standards-conforming behavioral tests.

Desktop screenshot capture was unavailable (`CopyFromScreen` reported an invalid handle), so HTTP layout inspection and successful captured target images are not claimed as complete visual QA.
