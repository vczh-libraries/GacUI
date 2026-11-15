Mirror `TODO_TASK.md` to `wGac`, including to update `FullControlTest` and `GuiWaylandElementRenderer`. Additionally:
- `FullControlTestRawClockInterpretor.(h|cpp)` is added, `RawClockInterpretorBase` could be used to implement the raw clock rendering.
- A `IGuiPlugin` should be created to make the interpretor automatically loaded by both `FullControlTest` and `RemotingTest_Rendering_Wayland`, they can share the same code now.
Make sure `import.sh` + `syncProj.sh` + `build.sh` is working.
To verify, you should test `FullControlTest` and `RemotingTest_Rendering_Wayland`, but limit the target to only `RawClock` and `Touching` test pages.

## DETAILS

### Scope and current upstream behavior

- Mirror the implemented raw-element contract in GacUI, including the later Clock plugin refactoring. The older explicit `GuiMain` registration instructions in `<GacUI repo>/TODO_TASK.md` have been superseded by `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawClockInterpretor.h/.cpp`. Its exclusion of sibling repositories applied to the original Windows task; this task implements the wGac counterpart.
- The local executable is `Test_FullControlTest`, with its entry point in `<wGac repo>/WGacFullControlTest/Main.cpp`. The remote executable is `RemotingTest_Rendering_Wayland`. Both must load the Clock plugin. The current Windows renderer deliberately omits the Clock translation unit; do not copy that omission into the Wayland renderer.
- Reuse the common registry, reflected properties and remote protocol from the imported GacUI version. Keep Core and renderer on matching protocol code, including `ElementDesc_Raw.interpretor`. Do not introduce a parallel Wayland registry or protocol field.

### Native renderer and interpretor lifetime

- Update `GuiWaylandElementRenderer` in `<wGac repo>/WGac/Renderers/GuiWaylandElement.h/.cpp` to use `GuiRawElementRendererBase` from `<GacUI repo>/Source/GraphicsElement/GuiGraphicsElement.h`. Remove the current empty initialization, finalization and state-change overrides where they would hide the inherited implementation; retain the CRTP friendship needed by render-target hooks.
- Add concrete `GuiWaylandElement` destruction-time finalization while its `Rendering` event still exists, following the Windows raw elements. Finalization through the later element-base destructor must not finalize the same interpretor again. Each renderer owns a fresh interpretor; the interpretor borrows its element and owns only its detachable handler tokens and drawing resources.
- Factory selection uses `Interpretor`, independently of `Id`. Empty or unknown names remove only the renderer-owned interpretor; independent manual handlers continue running. Unknown names stay stored, with no automatic retry after late registration. Data-only, ID-only and identical updates do not reinstall an unchanged interpretor. `SetInterpretorAndData` must expose both new values before initialization.
- Preserve the existing Cairo callback contract: borrow the current `cairo_t*` only during rendering, use the supplied logical bounds, and retain the provider's clip, hosted offset and scale. Do not retain or destroy the context/surface, or apply the scale twice. Preserve drawing state and the current path so one raw painter cannot affect adjacent content. Target changes must not reinstall the interpretor.

### Shared Clock plugin and manual Touching painter

- Synchronize both `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawClockInterpretor.h` and its `.cpp` into `<wGac repo>/WGacShared/` through `<wGac repo>/syncProj.sh`, adapting the framework include through that script. `demo::RawClockInterpretorBase::ReadTime` and `DrawClock` are defined out of line; copying only the header is insufficient. The existing `VCZH_MSVC` guards keep Windows painters and registration out of Unix builds.
- Implement one shared Wayland concrete interpretor and factory in a separate local translation unit in `<wGac repo>/WGacShared/`, so synchronization cannot overwrite it. Derive from `RawClockInterpretorBase`, implement its drawing primitives with Cairo, attach `GuiWaylandElement::Rendering` in `Initialize`, and detach in `Finalize`. Keep the upstream clock geometry and colors. Read the received `HH:mm:ss` payload; do not introduce a native timer or read local time while painting.
- Register exactly `FullControlTest::Clock` through an `IGuiPlugin` and `GUI_REGISTER_PLUGIN`, only in the controller-related `Load` phase after backend setup. Use the active graphics resource manager so hosted forwarding works. Registration must precede local window construction and remote pending-message processing, without a second explicit registration in either entry point.
- Link the portable base and Wayland plugin implementations exactly once into each consuming executable. Keep this application-specific painter out of the general platform provider. An otherwise unreferenced plugin object in a static library can be discarded; preserve the existing direct-source linking approach, or use an object-library arrangement that guarantees inclusion.
- Regenerate the Clock resource with `Interpretor="FullControlTest::Clock"`, retaining `Id="FullControlTest.Clock"` and its Workflow-owned time updates. Remove obsolete `AttachFullControlTestRawElement` calls and Clock attachment ownership from the local entry point and remote adapter.
- The existing `<wGac repo>/WGacShared/FullControlTestRawElement.h/.cpp` and `<wGac repo>/WGacShared/FullControlTestRawElementRemote.h/.cpp` currently implement Clock only. Add the manual Touching painter and repurpose the remote adapter for Touching, following the current GacUI helpers. Attach `MainWindow::GetTouchingElement()` locally and retain the returned attachment until before window destruction. Keep `Id="FullControlTest.Touching"` and an empty `Interpretor`.
- Match the Touching payload and drawing contract in `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawElement.h/.cpp`: `centerX,centerY,axisX,axisY,count`, followed by `count` triples of `pointId,x,y`; the axis transforms a filled SkyBlue 160 by 100 rectangle. Preserve yellow contact rings, current bounds and clipping. The remote adapter must handle late enumeration, installation, ID changes, removal and destruction without duplicate handlers; Clock is no longer an ID-based manual attachment.

### Synchronization and dependent consumers

- Confirm `<GacUI repo>/Release/` and `<GacUI repo>/Test/Resources/Metadata/` match the updated raw APIs, protocol and XML before import/synchronization. Import only copies snapshots; it does not regenerate stale upstream artifacts. If needed, regenerate those inputs through their owning GacUI workflow before consuming them.
- Update the owning script and CMake inventories so `<wGac repo>/import.sh`, `<wGac repo>/syncProj.sh` and `<wGac repo>/build.sh` reproduce the result. Imports come from GacUI's dependency/release snapshots; resources and generated application sources come through synchronization. Do not repair generated copies under `<wGac repo>/Import/`, `<wGac repo>/Import-Test/`, `<wGac repo>/Apps/*/Resources/`, `<wGac repo>/Apps/*/Source/` or `<wGac repo>/build/` by hand.
- Include the focused existing raw-element consumer in the migration: `<wGac repo>/WGacTest/RawElement.cpp` and `<wGac repo>/WGacTest/CMakeLists.txt` still use the old Clock attachment. Convert them to named interpretor selection and link the shared plugin. Audit positional `ElementDesc_Raw` initializers: the old three-field form can silently put the former data string into the new interpretor field. Supply `rawId`, `interpretor` and `data` explicitly in their correct fields.
- Keep the canonical renderer entry point and arguments synchronized from `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp` and `<GacUI repo>/Test/GacUISrc/SharedArguments.h`. Keep platform drawing in the shared Wayland helpers, so another synchronization does not overwrite the implementation.
- Update `<wGac repo>/README.md` and `<wGac repo>/README_CN.md` for the changed raw-element/plugin behavior and any changed source inventory or script usage. Keep reusable raw-element guidance consistent with the new Wayland support through its owning documentation workflow.

## VERIFICATION

These are implementation acceptance checks; this review does not execute them.

### Reproducible build and focused lifetime checks

- On Linux with the documented dependencies, run `<wGac repo>/import.sh`, then `<wGac repo>/syncProj.sh`, then `<wGac repo>/build.sh`. Require each to succeed. Verify that the synchronized resource selects the named Clock factory and that both executable link inventories contain the portable base and native plugin once. Repeat synchronization and an incremental build to detect overwritten helpers, stale inputs or duplicate registration; also verify `<wGac repo>/build.sh --rebuild` succeeds without relying on old build outputs.
- Use the existing `Test_RawElement` fixture for focused native/hosted checks of empty-to-known, known-to-known, known-to-empty and known-to-missing names, combined name/data assignment, data-only/no-op updates, independent instances and exactly-once finalization. Include destruction without a target, installation after target attachment and target recreation. Use fixed clock data for interpretor-only repaint checks so the timer, input or resizing cannot hide a missing repaint. Check a separately attached manual handler still draws after interpretor removal.
- Compare the relevant lifetime and rendering expectations in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRawElement_Windows.cpp` and remote-state expectations in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Raw.cpp`; passing Windows tests alone does not verify the Wayland backend.

### Application checks limited to RawClock and Touching

| Run | Launcher and coverage |
| --- | --- |
| Native showcase | `<wGac repo>/test.sh --app:fct`; only `Misc / Raw Clock` and `Misc / Touching` |
| Hosted showcase | `<wGac repo>/test.sh --app:fct --hosted`; the same two pages |
| Remote showcase | Start `<wGac repo>/test_core.sh --app:fct --protocol:minihttp`, then `<wGac repo>/test.sh --app:renderer --port:8889`; the same two pages |

- Run the local showcase separately from Core because both normally use automation port 8888. Use automation `Controls`/`Dom` and `IO` for navigation and state checks, then capture actual native drawing: changing data alone does not establish that a painter ran.
- For RawClock, verify the face, ticks and colored hands against the received time, including advancing data, resize, switching away/back and palette refresh. Confirm bounds, clipping and hosted placement remain correct and that the application starts without any explicit Clock registration call.
- For Touching, verify the initial rectangle with `Interpretor` empty, then controlled payload changes for translation, rotation/scale and contact markers. Native Wayland touch acquisition is unsupported, and the local/renderer automation adapters do not supply point-command state. Use fixed payloads in the focused local fixture and synthetic `!PointDown`/`!PointUpdate`/`!PointUp` through Core's `/Automation/RemotingTest_Core/IO` for remote behavior. Observe the resulting payload and Wayland drawing; do not claim physical touch support or broaden this task to implement it.
- Replace the remote renderer while retaining Core, and exercise takeover with a second renderer using another automation port. Verify Clock name/data replay, Touching's late attachment and cleanup, no duplicate drawing, continued updates and normal shutdown. Keep this on the two requested pages; unrelated RPT, RVMT, TUI and full SOP coverage is unnecessary.
- Record build results, state assertions, visible rendering evidence and any unavailable display-dependent checks separately. Stop all owned test/debugger processes after verification.

## REVIEW COMMENTS

No unresolved review comments.
