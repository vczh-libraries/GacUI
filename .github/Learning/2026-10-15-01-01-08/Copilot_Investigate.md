# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Fix the date to initialize them to 2026-1-1 so it should stable the snapshot. Run unit test again to push new snapshots. For others, what do you think is the cause of unstable element allocation order? All UI elements are supposed to be created in UI thread, what part in background cause the difference? Is it able to stabilize?

# TEST [CONFIRMED]

- The existing `RefreshThemes preserves overrides and safely snapshots windows` test in `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Theme.cpp` creates a date picker with the constructor's current-date default. The previous unchanged full test run regenerated the two `Application/RefreshThemes/OverridesAndWindows` frames with the highlight moving from October 13 to October 14.
- Set the fixture date before attachment and rendering, then build the Debug x64 solution and run the complete UnitTest suite through the repository wrappers. Require all tests to pass, no skipped files, and no memory leak report after the summary.
- Inspect every changed snapshot and verify both date-picker frames display January 2026 with January 1 selected. Compare file-dialog snapshots against the committed baseline.
- Trace the background filesystem jobs, main-thread callbacks, virtualized row creation, and renderer ID allocation. Separate source-supported scheduling mechanisms from historical platform observations and controlled runtime evidence.

# PROPOSALS

- No.1 Initialize the theme fixture date to January 1, 2026 [CONFIRMED]

## No.1 Initialize the theme fixture date to January 1, 2026

Call `datePicker->SetDate(DateTime::FromDateTime(2026, 1, 1))` immediately after constructing the control in the existing theme-refresh test. This makes both the original and refreshed calendar snapshots independent of the execution date while preserving the production control's current-date default.

### CODE CHANGE

- Added the fixed date initialization in `<GacUI repo>/Test/GacUISrc/UnitTest/TestApplication_Theme.cpp`.
- Regenerated the owning snapshots through UnitTest and reviewed the complete generated diff before committing source and snapshots separately.

### CONFIRMED

- The complete Debug x64 solution build succeeded with zero warnings and zero errors.
- The complete UnitTest run passed 93/93 test files and 1820/1820 test cases. The completed `<GacUI repo>/.github/Scripts/Execute.log` contains no skipped tests or memory leak report after the summary.
- Both `Application/RefreshThemes/OverridesAndWindows/frame_0.json` and `frame_1.json` contain the January 2026 calendar with day 1 selected. Each frame has seven changed values: the month name, four preceding-month day labels, and the old/new selection background colors. No element IDs or DOM structure changed.
- The snapshot commit contains only those two date-picker frames. The source commit includes the fixture initialization and this investigation, with the preceding investigation archived by the repository script.

# ELEMENT ALLOCATION INVESTIGATION

## Reproduced on the same Windows build

The full Debug x64 run after the date-only edit changed all five `ScrollResetOnNavigation` rendering artifacts to the exact blobs previously produced by the Linux run in `d6e54f44a`. The preceding unchanged Windows run retained the other allocation sequence. The earlier Windows/Linux correlation therefore does not establish a platform-specific cause: the same Windows executable configuration can produce both sequences.

The two sequences create the same tree and grid rows with different sequential IDs. The previous structural comparison resolved each DOM element reference to its descriptor and omitted allocation IDs; both changed frames then had identical render trees, including geometry, colors, text, and child order.

## UI-thread allocation confirmed with CDB

A focused debugger execution of `TestApplication_Dialog_File.cpp` passed 28/28 cases. Breakpoints at the `ScrollResetOnNavigation` double-click and `GuiRemoteGraphicsRenderTarget::AllocateNewElementId` captured two allocation paths on the same main thread:

- Element 552: the tree view's deferred `GuiScrollView::CalculateView` callback calls `CallUpdateView`, `RangedItemArrangerBase::OnViewChanged`, and `GuiVirtualRepeatCompositionBase::CreateStyle`. Attaching that row assigns the renderer ID. The debugger identified the control as `GuiBindableTreeView`.
- Element 622: the `FileDialogViewModel::RefreshFiles` UI completion raises `IsLoadingFilesChanged`. The handler from `<GacUI repo>/Source/Utilities/FakeServices/Dialogs/FileDialog.xml` calls `dataGrid.SetViewPosition({x:0 y:0})`; the scroll event calls `CallUpdateView` and creates the grid rows. This path also reaches the same renderer-ID allocator on the main thread.
- In the debugger run, these callbacks were entries 0 and 1 of the same `SharedAsyncService::ExecuteAsyncTasks` batch, whose `firstTaskId` was 37. The resulting five rendering artifacts exactly matched the original committed baseline again.

Relevant implementation:

- `<GacUI repo>/Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp`: `FileDialogTaskQueue` serializes filesystem work on an `InvokeAsync` worker. `RefreshFiles` enumerates folders/files there and posts the UI completion with `InvokeInMainThread`. UI collection mutation and loading-state notifications occur only in that completion.
- `<GacUI repo>/Source/Controls/GuiContainerControls.cpp`: `CalculateView` uses `TryDelayExecuteIfNotDeleted`; scrolling itself calls `CallUpdateView` synchronously.
- `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp`: `TryDelayExecuteIfNotDeleted` posts hosted-control work through `InvokeInMainThread`.
- `<GacUI repo>/Source/Utilities/SharedServices/GuiSharedAsyncService.cpp`: UI callbacks are appended to `taskItems`; each execution pass snapshots the current queue. The ordering between a UI-posted layout callback and a worker-posted completion depends on their enqueue times.
- `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp` and `GuiRemoteGraphics_BasicElements.cpp`: attaching a new renderer allocates `++usedElementIds`.

This is an ordering difference between serialized UI callbacks, not evidence of background-thread UI creation. Serializing filesystem jobs does not order their completions relative to independently queued UI layout work. Rendering-idle detection also does not constitute a barrier for all background tasks.

A pointer-sorted `InvokeAfterRendering` callback map was inspected as another potential source of ordering differences. It was not on either captured allocation stack and is not the diagnosed explanation for this case.

## Stabilization options

For snapshot tests, use a deterministic asynchronous executor: preserve queued/asynchronous behavior, execute mocked background work at explicit harness phases, and deliver its UI completions in a defined order relative to layout and frame capture. This removes dependence on worker speed while retaining production asynchronous behavior. A real-worker regression should separately exercise concurrency and lifetime behavior.

An alternative production change is to define navigation as explicit phases, with a dependency between tree layout and applying/resetting the file grid. Merely retaining the FIFO filesystem queue, adding a sleep, or waiting until the final screenshot looks idle does not fix allocation history. Any scheduling change needs deliberate snapshot review and repeated verification; no scheduler or production file-dialog behavior was changed in this request.

## Artifact handling

The focused debugger run also reordered declarations in 28 `[x64].txt` compiler listings that the full run had left unchanged. These diagnostic-run artifacts were restored explicitly. The five file-dialog rendering artifacts returned to their committed content through the debugger run itself. The retained snapshot changes are only the two theme-refresh calendar frames requested by the user.
