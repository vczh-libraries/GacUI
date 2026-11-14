- Add `Interpretor` to `GuiRawElementBase`, now both `Interpretor` and `Data` triggers repaint.
  - Extra `SetInterpretorAndData` method.
  - `GuiDirect2DElementRenderer` and `GuiGDIElementRenderer` reacts to `Interpretor`:
    - When it is empty, the interpretor is removed.
    - When it is not empty, it requests the interpretor factory and the interpretor will subscribe to events of the current `GuiRawElementBase` subclass.
    - Inexisting interpretor is not a failure, it is treated as empty value.
    - `Initialize` and `Finalize` will be called in the renderer. This might need to be implemented in a new base class `GuiRawElementRendererBase`.
  - `IGuiGraphicsResourceManager` from ``GetGuiGraphicsResourceManager` will handle raw element interpretor registration and querying:
    - A `IGuiGraphicsRawInterpretor` and `IGuiGraphicsRawInterpretorFactory` is defined just below `IGuiGraphicsRendererFactory`, the interpretor interface has an `void Initialize(IGuiGraphicsElement*)` and `void Finalize();`.
- The clock in `FullControlTestRawElement.(h|cpp)` will be rewritten to use `IGuiGraphicsRawInterpretorFactory`.
  - The interpretor will be hardcoded "FullControlTest::Clock". The registration is done in `GuiMain` before loading the main window.
  - Keep the Touching page without any interpretor. `FullControlTestRawElementRemote.(h|cpp)` is still needed for its manual drawing attachment.
- To verify, make sure `CppTest` and `RemotingTest_Rendering_Win32` both behaves correctly on the `RawClock` test page.
- DO NOT perform any work on `wGac`, `iGac` and `GacJS`.

## DETAILS

### Raw element state

- In `<GacUI repo>/Source/GraphicsElement/GuiGraphicsElement.h/.cpp`, add an empty-default `WString` property named exactly `Interpretor`, with `GetInterpretor()`, `SetInterpretor(const WString&)` and `SetInterpretorAndData(const WString& interpretor, const WString& data)`.
- Preserve `Id` as independent application metadata: empty and duplicate IDs remain legal, and an ID-only change marks remote state dirty without independently requesting repaint. Factory selection uses `Interpretor`, not `Id`.
- Changed `Interpretor` or `Data` requests repaint through the existing element-state notification. Identical assignments do nothing. The combined setter assigns both fields before one notification, so a newly initialized interpretor observes the new data immediately.
- Data-only and ID-only changes do not recreate an unchanged interpretor. Empty or unregistered names mean no renderer-owned interpretor; an unregistered name remains stored in the property. This does not disable `Rendering` or remove independently attached handlers, including the Touching painter.

### Factory registration and ownership

- Declare the new native interfaces after `IGuiGraphicsRendererFactory` in `<GacUI repo>/Source/GraphicsElement/GuiGraphicsElementInterfaces.h`. Follow the existing factory convention: `IGuiGraphicsRawInterpretorFactory::Create()` creates a fresh interpretor, and the renderer owns that instance and calls its `Initialize(IGuiGraphicsElement*)` and `Finalize()`.
- Add named registration and lookup to `IGuiGraphicsResourceManager` in `<GacUI repo>/Source/GraphicsElement/GuiGraphicsResourceManager.h`: `RegisterRawInterpretorFactory(const WString&, Ptr<IGuiGraphicsRawInterpretorFactory>)` and `GetRawInterpretorFactory(const WString&)`. The common `GuiGraphicsResourceManager` implementation owns factories by string key and returns a borrowed factory pointer, or null for a missing name. Reject duplicate registration consistently with existing renderer factories.
- Forward both operations through `GuiHostedGraphicsResourceManager` in `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedGraphics.h/.cpp`; hosted `CppTest` must use the same registry as its underlying native renderer.
- Register on the UI thread after backend setup and before creating consuming elements. Runtime factory replacement, unregistration and automatic retries after late registration are not required. Native interpretor creation and event attachment also stay on the UI thread.
- Each raw renderer owns its own interpretor; an interpretor borrows its element. The current clock attachment owns a `Ptr<GuiRawElementBase>` in `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawElement.cpp`. Retaining that ownership after migration would create an element-to-renderer-to-interpretor-to-element reference cycle. The separately owned manual Touching attachment retains its existing lifetime model.

### Renderer and event lifetime

- Share interpretor selection and lifetime between `GuiDirect2DElementRenderer` and `GuiGDIElementRenderer`, using `GuiRawElementRendererBase` if appropriate. Finalize and release the previous instance before initializing a replacement. Clearing the name, selecting a missing factory, and final element destruction must each clean up the installed instance exactly once.
- Account for C++ destruction order: `GuiElementBase::~GuiElementBase()` currently calls renderer finalization after the concrete raw element's event members have been destroyed. Arrange interpretor cleanup while the Direct2D/GDI event objects are still alive, for example through the concrete raw element's destruction path. Do not rely solely on that existing base destructor or call interpretor finalization twice.
- Keep interpretor lifetime separate from render-target lifetime. Direct2D target detachment/recreation releases/recreates target-bound brushes through existing before/after events without reinstalling the interpretor. Support installing an interpretor after a target is already attached; the current clock painter initializes missing brushes on first render. Finalization releases brushes and handler tokens even if no further target-change event occurs.
- Preserve direct native event subscriptions. An empty interpretor must still render the manually attached Touching page; replacing or clearing an interpretor detaches only its own handlers.

### Reflection and remote propagation

- Register `Interpretor` and `SetInterpretorAndData` alongside `Id` and `Data` in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionElements.cpp`. Follow the existing property pattern without adding property-changed events. The native interpretor/factory interfaces do not need reflection registration merely to support this task.
- Add an `interpretor` string to `ElementDesc_Raw` in `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Renderer_BasicElements.txt`, preserving numeric protocol `id`, application `rawId`, and `data` as separate fields. Generate schema/serialization outputs through `Metadata_UpdateProtocol`; do not edit generated outputs manually.
- Send the new field from `GuiRawElementRenderer::SendUpdateElementMessages` in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteGraphics_BasicElements.cpp`, including initial state and reconnect/full-state replay. The remote Core only transports the name and data; it does not instantiate Windows painters.
- In `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Elements.cpp`, compare all three application strings, apply the new name/data with the combined setter, and raise one `RawElementChanged` after all state is assigned. Interpretor-only changes must set `needRefresh`, because remote elements have no owning composition; preserve ID-only behavior and suppress identical updates.
- Preserve `GetRawElements()` and the installed/changed/uninstalled events: the manual Touching adapter still depends on them. Audit descriptor aggregate initializers, JSON round trips, protocol logs and snapshot readers for the additional field. Keep snapshot visualization independent of application painter registration and regenerate affected baselines through their owning tools.

### FullControlTest migration and scope

- Add `Interpretor="FullControlTest::Clock"` to the clock in `<GacUI repo>/Test/Resources/App/FullControlTest/RawElementTabPage.xml`, retaining `Id="FullControlTest.Clock"`. Preserve its Workflow-owned timer and `HH:mm:ss` data; native and remote painters draw received data without their own ticking timer.
- Keep the raw element in `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingTabPage.xml` without an interpretor, retaining `Id="FullControlTest.Touching"`, its payload, and `AttachFullControlTestTouchingElement`. This page verifies that manual drawing remains supported.
- Replace only the clock attachment with early factory registration in `<GacUI repo>/Test/GacUISrc/CppTest/Main.cpp`, `<GacUI repo>/Test/GacUISrc/CppTest_Metaonly/Main.cpp`, `<GacUI repo>/Test/GacUISrc/CppTest_Reflection/Main.cpp`, and `<GacUI repo>/Test/GacUISrc/GacUI_Host/Main.cpp`. Register before constructing/loading the main window. Preserve their Touching attachment variables and cleanup; remove only obsolete clock attachment cleanup.
- Retain `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/FullControlTestRawElementRemote.h/.cpp` and their project entries. Remove their clock-specific attachment branch and keep Touching's ID-based reconciliation, late enumeration and handler cleanup. The shared painter source is already compiled by the renderer project and must not be added a second time.
- Register the Windows clock factory in `GuiMain` in `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`, before `ProcessPendingMessages()`. The existing adapter is constructed in `StartClient` before `SetupRawWindows*` establishes the resource manager, so that location is too early for factory registration. Preserve the manual Touching adapter and the shared entry point's non-Windows paths without changing sibling repositories.
- During implementation, update the affected raw-element guidance and the `/Gdi` description in `<GacUI repo>/Project.md` to distinguish factory-selected Clock from manually attached Touching. Follow the canonical-document rules for copied knowledge-base guidance. Do not regenerate, build, test or edit `wGac`, `iGac` or `GacJS` for this task.

## VERIFICATION

These checks are for implementation; this review does not execute the task.

### Focused automated coverage

- Extend `<GacUI repo>/Test/GacUISrc/UnitTest/TestRawElement_Windows.cpp` for both Direct2D and GDI. Count creation, initialization, finalization and destruction across empty-to-known, known-to-known, known-to-empty and known-to-missing transitions; data-only changes and repeated identical assignments must not reinstall. Multiple elements sharing a name receive independent instances. Verify the combined setter exposes both new values during initialization.
- Cover final release without any composition or render target, removal after rendering, late interpretor installation on an existing target, target recreation and retained remote elements. Require exactly-once cleanup, no ownership cycle, no access to destroyed event objects and no Debug memory leaks. Check hosted registration/lookup forwarding.
- Preserve a separately attached drawing handler with empty `Interpretor`; verify it still runs after data updates and after a renderer-owned interpretor is installed and removed. Exercise Touching through its existing manual attachment so the optional-interpretor contract has concrete coverage.
- Extend `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Raw.cpp` for interpretor-only and data-only repaint without input, timer, resize or DOM changes; retain ID-only/no-op assertions. Check combined updates, empty/unknown names, JSON transport and reconnect replay. Extend `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` for the reflected property and method, and exercise XML construction and the combined setter through Workflow. Use ordinary tests for state/lifetime checks; retain snapshots only where they provide useful rendering coverage.

### Generation and builds

- Follow `<GacUI repo>/Project.md` and the build/execution guidelines under `<GacUI repo>/.github/Guidelines`: generate the protocol first; generate reflection metadata in the prescribed Debug Win32 then x64 order and run `Metadata_Test`; regenerate the changed FullControlTest resource through `GacUI_Compiler`, then repeat its required metadata steps and rebuild before running consumers. Check for compiler error artifacts.
- Run the affected unit-test files through the repository wrapper, respecting the existing test filter and retaining related coverage. Inspect the final summary and appended Debug leak output. Check generated and snapshot diffs; keep manual, generated and snapshot changes in the repository's required separate commits when implementing.

### Windows application checks

| Application | Backends | Focus |
| --- | --- | --- |
| `CppTest` | Default Direct2D and `/Gdi` | `Misc / Raw Clock`, plus the manual Touching painter |
| `RemotingTest_Core` with `RemotingTest_Rendering_Win32` | Default Direct2D and renderer `/Gdi` | The same pages, including renderer replacement |
| `CppTest_Metaonly` | Default Direct2D and `/Gdi` | Existing focused `RawClock` UIA scenario required for the FullControlTest resource change |

- Run `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` with `-Scenario RawClock`, repeating with `-Gdi`; update affected assertions in `<GacUI repo>/Test/UIA_CppTest_Shared.cs`. Use `-SkipBuild` only after building current sources. Keep `CppTest_Reflection` and `GacUI_Host` compiling with early registration, and exercise the interpreted resource path after regeneration.
- For remote checks, use one existing transport, such as Core `/FCT /Pipe /AsPort:8888` with renderer `/Pipe /AsPort:8889`, adding `/Gdi` only to the renderer for the second run. Use the documented launch/execution procedures. Navigate through automation first; use the local showcase for UIA checks because the renderer does not transport a semantic UIA tree.
- Verify actual clock drawing and hand movement from the received data, then resize, switch away/back, refresh the palette and reconnect a replacement renderer. Existing automation checks establish changing data and bounds but do not alone prove that the clock painter draws. Include controlled fixed-data interpretor-only transitions so periodic clock updates cannot hide a missing repaint.
- On Touching, verify the existing rectangle/contact rendering from its payload with `Interpretor` empty, including data changes, resize and remote replacement. Preserve attachment cleanup without duplicate handlers. Limit this to the affected drawing behavior; a full unrelated touch/SOP run is unnecessary.
- Record actual rendering evidence and any unavailable session-dependent checks separately from automation/state assertions. No verification in `wGac`, `iGac` or `GacJS` is required.

## REVIEW COMMENTS

No unresolved review comments.
