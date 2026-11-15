Mirror `TODO_TASK.md` to `iGac`, including to update `FullControlTest` and `GuiWaylandElementRenderer`. Additionally:
- `FullControlTestRawClockInterpretor.(h|cpp)` is added, `RawClockInterpretorBase` could be used to implement the raw clock rendering.
- A `IGuiPlugin` should be created to make the interpretor automatically loaded by both `FullControlTest` and `RemotingTest_Rendering_Cocoa`, they can share the same code now.
Make sure `import.sh` + `syncProj.sh` + `build.sh` is working.
To verify, you should test `FullControlTest` and `RemotingTest_Rendering_Cocoa`, but limit the target to only `RawClock` and `Touching` test pages.

## DETAILS

### Scope and platform names

- In this task, the native renderer class is `GuiCocoaElementRenderer` in `<iGac repo>/Mac/GraphicsElement/CoreGraphics/GuiCocoaElement.h/.cpp`; `GuiWaylandElementRenderer` belongs to wGac. The executable is `RemotingTest_Rendering_macOS`, not `RemotingTest_Rendering_Cocoa`. The local executable is `Test_FullControlTest`, with its entry point in `<iGac repo>/MacFullControlTest/Main.mm`.
- Mirror the implemented GacUI raw-element contract, including the later plugin refactoring in `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawClockInterpretor.h/.cpp`. The older explicit `GuiMain` registration instructions in `<GacUI repo>/TODO_TASK.md` are superseded. Its exclusion of sibling repositories applied to the original Windows task; this task implements the iGac counterpart.
- Both local and remote macOS executables must load the Clock plugin. The current Windows renderer deliberately omits its Clock translation unit; that omission does not apply here. Import the common registry, reflected properties and remote protocol instead of introducing Cocoa-specific duplicates. Core and renderer must use matching protocol code including `ElementDesc_Raw.interpretor`.

### Native renderer and interpretor lifetime

- Make `GuiCocoaElementRenderer` use `GuiRawElementRendererBase` from `<GacUI repo>/Source/GraphicsElement/GuiGraphicsElement.h`. Remove empty initialization, finalization and state-change overrides where they would hide the inherited implementation; preserve the CRTP friendship needed by render-target hooks.
- Finalize in the concrete `GuiCocoaElement` destructor while `Rendering` is still alive. The later element-base finalization must not finalize the same interpretor again. Each renderer owns its own instance; the interpretor borrows the element and owns its detachable event tokens, avoiding an element/renderer/interpretor ownership cycle.
- Select by `Interpretor`, independently of `Id`. Empty and unknown names remove only the renderer-owned handlers; independent manual drawing still runs. Unknown names stay stored and are not retried automatically after late registration. ID-only, data-only and identical updates do not reinstall an unchanged interpretor. The combined setter makes both name and payload visible before initialization.
- Preserve the CoreGraphics callback contract: borrow `CGContextRef` only during the current paint and use its supplied logical bounds. The provider already applies clipping, hosted offsets and backing scale and isolates graphics state, text matrix and current path. Do not retain/release the context, apply backing scale twice or introduce another coordinate flip. Keep interpretor lifetime independent of context/render-target replacement.

### Shared Clock plugin and manual Touching painter

- Synchronize both `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawClockInterpretor.h` and its `.cpp` into `<iGac repo>/MacShared/` through `<iGac repo>/syncProj.sh`, adapting the framework include through that script. `demo::RawClockInterpretorBase::ReadTime` and `DrawClock` have out-of-line definitions; the header alone cannot link. Keep the upstream `VCZH_MSVC` guards excluding Windows concrete painters and registration.
- Implement one shared Cocoa interpretor and factory in a separate local translation unit in `<iGac repo>/MacShared/`, deriving from `RawClockInterpretorBase`, so synchronization cannot overwrite it. Implement the drawing primitives with CoreGraphics, attach `GuiCocoaElement::Rendering` in `Initialize`, and detach in `Finalize`. Preserve upstream clock geometry and colors, using received `HH:mm:ss` data without a native timer or a local-clock read while painting.
- Register exactly `FullControlTest::Clock` through an `IGuiPlugin` and `GUI_REGISTER_PLUGIN`, only during controller-related `Load` after backend setup. Register through the active graphics resource manager to preserve hosted forwarding. Registration must precede local window construction and remote pending-message processing, without a second registration in either entry point.
- Link the portable base and Cocoa plugin implementations exactly once into both executable targets. Keep the application-specific plugin out of the general platform provider. Preserve direct compilation into consumers, or another arrangement that includes the plugin object even without a referenced exported symbol; an unreferenced static-library object may be discarded.
- Regenerate Clock with `Interpretor="FullControlTest::Clock"`, keeping `Id="FullControlTest.Clock"` and its Workflow-owned time updates. Remove obsolete `AttachFullControlTestRawElement` calls and owning Clock attachment variables.
- The current `<iGac repo>/MacShared/FullControlTestRawElement.h/.cpp` and `<iGac repo>/MacShared/FullControlTestRawElementRemote.h/.cpp` implement Clock only. Add the CoreGraphics manual Touching painter, attach `MainWindow::GetTouchingElement()` locally and retain that attachment until before window destruction. Repurpose the remote adapter for Touching using the current GacUI adapter's late enumeration, installation/change/removal handling and cleanup. Keep `Id="FullControlTest.Touching"` and empty `Interpretor`; Clock no longer needs ID-based attachment.
- Match `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawElement.h/.cpp`: Touching data contains `centerX,centerY,axisX,axisY,count`, followed by `count` triples of `pointId,x,y`. The axis rotates/scales a filled SkyBlue 160 by 100 rectangle. Preserve yellow contact rings, bounds and clipping, and prevent duplicate handlers across remote replacement.

### Synchronization and documentation

- Confirm `<GacUI repo>/Release/` and `<GacUI repo>/Test/Resources/Metadata/` match the updated raw APIs, protocol and XML before import/synchronization. Import only copies snapshots; regenerate stale upstream artifacts through their owning GacUI workflow before consuming them.
- Make the result reproducible through `<iGac repo>/import.sh`, `<iGac repo>/syncProj.sh` and `<iGac repo>/build.sh`, including the shared helper copies and CMake source lists. Imports come from GacUI's dependency/release snapshots. Do not hand-edit `<iGac repo>/Import/`, `<iGac repo>/Import-Test/`, `<iGac repo>/Apps/*/Resources/`, `<iGac repo>/Apps/*/Source/` or `<iGac repo>/build/` to repair generated or imported code.
- Migrate the existing `Test_RawElement` consumer in `<iGac repo>/MacTest/RawElement.cpp` and `<iGac repo>/MacTest/CMakeLists.txt` to named Clock selection and link the portable base/plugin once. Its old `ElementDesc_Raw{ id,name,data }` initializer can silently assign the payload to the new interpretor field; populate all four fields correctly and update its assertions. This target must remain buildable through the normal build script.
- Keep the canonical renderer entry point and arguments synchronized from `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp` and `<GacUI repo>/Test/GacUISrc/SharedArguments.h`; keep platform drawing in the shared Cocoa helpers so synchronization preserves it. Audit all old Clock-attachment consumers and positional `ElementDesc_Raw` initializers for the new separate `rawId`, `interpretor` and `data` fields.
- Update `<iGac repo>/doc/OSProvider_Graphics.md` for raw-interpretor selection and lifetime, and `<iGac repo>/readme.md` for changed helper inventory or script behavior. Update hosted/startup documentation if those documented paths change, and record focused remote results in `<iGac repo>/TestMatrix_NativeRenderer.md`. Keep reusable raw-element guidance consistent with Cocoa support through its owning documentation workflow.

## VERIFICATION

These are implementation acceptance checks; this review does not execute them.

### Reproducible build and focused lifetime checks

- On macOS with documented prerequisites, run `<iGac repo>/import.sh`, then `<iGac repo>/syncProj.sh`, then `<iGac repo>/build.sh`; require each to succeed. Verify that the regenerated Clock selects the named factory and both link inventories contain the portable base and native plugin once. Repeat synchronization and an incremental build to detect overwritten helpers, stale inputs and duplicate registration. Also verify `<iGac repo>/build.sh --rebuild` without relying on old build outputs.
- Extend and run the existing `Test_RawElement` fixture in native and hosted modes for empty-to-known, known-to-known, known-to-empty and known-to-missing names; combined name/data updates; data-only/no-op updates; independent instances; and exactly-once cleanup. Include destruction without a target, installation after target attachment and target replacement. A separate manual handler must survive interpretor removal.
- Use fixed clock data to verify interpretor-only repaint without a timer, input or resize masking the result. Compare relevant expectations in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRawElement_Windows.cpp` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Raw.cpp`; Windows coverage does not establish Cocoa behavior.

### Application checks limited to RawClock and Touching

| Run | Launcher and coverage |
| --- | --- |
| Native showcase | `<iGac repo>/test.sh --app:fct`; only `Misc / Raw Clock` and `Misc / Touching` |
| Hosted showcase | `<iGac repo>/test.sh --app:fct --hosted`; the same two pages |
| Remote showcase | Start `<iGac repo>/test_core.sh --app:fct --protocol:minihttp`, then `<iGac repo>/test.sh --app:renderer --port:8889`; the same two pages |

- Run the local showcase separately from Core because both normally use automation port 8888. Use automation `Controls`/`Dom` and `IO` for navigation and state, and capture actual native drawing; a changing payload or a successful launch alone is insufficient.
- On RawClock, verify the face, ticks and colored hands against received time, continued hand movement, resize, switching away/back and palette refresh. Check standard and hosted placement, clipping and backing-scale behavior. Startup must work without explicit Clock registration in application code.
- On Touching, verify the initial rectangle with empty `Interpretor` and controlled payloads for translation, rotation/scale and contact markers. Native Cocoa touch acquisition and native point-command automation are unsupported. Use a controlled local payload fixture for local drawing and synthetic `!PointDown`/`!PointUpdate`/`!PointUp` through Core's `/Automation/RemotingTest_Core/IO` for remote updates, checking payload and CoreGraphics output. Do not claim physical-touch coverage or expand the task to implement native touch.
- Retain Core while replacing the renderer, and exercise takeover with a second renderer on another automation port. Verify full Clock name/data replay, Touching late attachment, handler cleanup without duplicate drawing, continued updates and normal shutdown. Keep verification on these two pages; unrelated RPT, RVMT, TUI and full SOP coverage is unnecessary.
- Record macOS build results, state assertions, visible rendering evidence and any unavailable display/backing-scale checks separately. Stop all owned test/debugger processes after verification.

## REVIEW COMMENTS

No unresolved review comments.
