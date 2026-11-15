# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

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

# UPDATES

## UPDATE

The work is overall good but there is some refactoring need to do:

- Split `RawClockInterpretor` into `RawClockD2DInterpretor` and `RawClockGDIInterpretor`. You can turn functor template arguments into virtual functions in base class `RawClockInterpretorBase`. Move the base class to the header file, therefore future `wGac` and `iGac` port could use it.
- `RawClockInterpretorFactory` split to D2D and GDI versions.
- Turn `RegisterFullControlTestClockInterpretor` into an **`IGuiPlugin`** implementation with `GUI_REGISTER_PLUGIN` to register the plugin, therefore this function will be automatically loaded, no need to call it in `GuiMain`.
- Besides the cross platform `RawClickInterpretorBase`, others about clock interpretors should be guarded by `VCZH_MSVC` therefore the cpp file could just be used without breaking `wGac` and `iGac`.
- Split clock interpretor to `FullControlTestRawClockInterpretor.(h|cpp)`, therefore `RemotingTest_Rendering_Win32` could just avoid loading it.

## UPDATE

When creating `RawClockIntepretorBase` , because it is not a template class anymore, keep only declaration in the header file.

## UPDATE

`FullControlTest` and other test apps are not allowed to be mentioned in KB pages, do some clean up after finishing your work, remember to port KB changes to `Tools` repo and commit push all local changes

# TEST [CONFIRMED]

The first regression extends `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` to require writable string properties `Id`, `Interpretor`, and `Data` without property-change events, plus the two-argument `SetInterpretorAndData` method. It compiles against the original native API and fails at the missing reflected `Interpretor` property.

Implementation coverage will extend the existing ordinary raw Core and Windows tests for atomic state updates, independent factory instances, transition counts, target lifetime, concrete-event destruction order, hosted forwarding, manual Touching handlers, JSON and reconnect replay. The retained XML rendering fixture will exercise property construction and the combined setter through Workflow.

Success also requires protocol generation, ordered Debug Win32/x64 metadata generation and Metadata_Test, FullControlTest resource generation and consumer rebuilds, passing affected unit tests without a Debug leak dump, and the requested Direct2D/GDI local and remote Clock/Touching checks. Actual painting evidence and any unavailable desktop-dependent checks will be recorded separately from state/automation evidence. The excluded sibling repositories are outside this investigation.

# PROPOSALS

Baseline: the Debug x64 solution build succeeded with 0 warnings and 0 errors. Running the compiled reflection regression under CDB with `/C` and a file filter selecting `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` reached the new raw-property test and printed `Assertion failure: property`; CDB stopped on the propagated test exception. The existing Id lookup passed and the new Interpretor lookup was absent. No production source had changed.

- No.1 [DENIED] Select renderer-owned raw interpretors through the resource manager
- No.2 [CONFIRMED] Split portable Clock geometry from Windows painter plugins

## No.1 [DENIED] Select renderer-owned raw interpretors through the resource manager

Add the independent reflected Interpretor property and atomic name/data setter. The common resource manager owns named factories and hosted managers forward registration and lookup. A shared native raw-renderer base caches the selected name, owns one fresh interpretor, and finalizes it before replacement. Concrete Direct2D/GDI destructors finalize while their event objects remain alive; the later base finalization sees no installed instance. Target changes keep the interpretor and use existing native before/after events. The interpretor borrows its element. Unknown names remain stored and do not retry on data-only changes.

Transport all three application strings and explicitly invalidate remote native rendering for name or data changes. Migrate only the Windows Clock painter to early factory registration; keep manual Touching collection attachment and non-Windows entry paths. Add the name to automation state, update targeted assertions, and update canonical raw guidance in the Tools repository before scoped synchronization.

### CODE CHANGE

Implemented the independent raw property and atomic setter, manager-owned factory registry with hosted forwarding, and shared native renderer ownership. Direct2D/GDI concrete destructors finalize before event destruction; a cleared owned pointer makes later base finalization harmless. Clock now borrows its element, releases native resources/tokens explicitly, and is registered in every Windows consuming GuiMain. Manual Touching reconciliation remains intact.

The protocol schema was regenerated through Metadata_UpdateProtocol. Core sends the name and the native receiver applies name/data together, invalidates name/data changes and emits one final-state event. Automation exposes elementInterpretor. Tests cover lifetime counts and late/target-free cleanup, independent instances, hosted lookup, fixed-data native pixel evidence, manual Touching removal, Core scheduling, JSON/reconnect, reflection and XML/Workflow construction/calls. Focused UIA assertions distinguish Clock selection and empty Touching. Ordered metadata/resource generation, consumer builds, execution, canonical guidance updates and scoped synchronization follow.

Initial Debug Win32 build succeeded (0 errors). Review moved the test destructor assertion into an external state check to avoid a noexcept warning, and added Clock brush cleanup coverage without a final target-change event. Two existing code-page warnings came from `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Caret.cpp`.

The early Win32 native debugger run stopped in the test fixture: GuiHostedGraphicsResourceManager requires a non-null controller. The fixture now creates a real scoped GuiHostedController for forwarding checks and releases it before native-target tests. This changes no production behavior.

The corrected Debug x64 focused debugger runs passed: `<GacUI repo>/Test/GacUISrc/UnitTest/TestRawElement_Windows.cpp` (2/2 cases, both backends) and `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Raw.cpp` plus `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` (27/27 cases). Native checks inspected real WIC/GDI pixels for fixed-data Clock hand positions, empty/missing selection and manual Touching rectangle/contact updates, along with lifetime counts and target recreation. Core/Workflow checks exercised atomic initialization, name-only repaint, JSON and replay. The final repository-wrapper run and app checks remain pending.

Review also isolated the remote unknown-name fixture from the lifetime test's deliberately late-registered name, so the remote transition uses a genuinely absent factory during the final wrapper runs. Declaration formatting was aligned without changing behavior.

Generation checkpoint: protocol generation, the first Debug Win32/x64 metadata cycle and Metadata_Test succeeded. GacUI_Compiler completed successfully for both resource architectures with no resource compiler error artifacts. Generated FullControlTest C++ changes are the three-line Interpretor assignment in each architecture; its corresponding Workflow text and resource binaries changed. Canonical guidance was committed/pushed in Tools as 9704e66 and synchronized only into GacUI using `<Tools repo>/Copilot/copilotInit.ps1`. The post-resource build/metadata cycle follows.

Application verification checkpoint: the current Debug x64 CppTest RawClock UIA scenarios passed on Direct2D (122 assertions) and GDI (115); CppTest_Metaonly passed on Direct2D (128) and GDI (124). The interpreted GacUI_Host path passed 254 assertions. Every wrapper confirmed normal shutdown and endpoint release. These checks cover the registered Clock name, time updates, manual Touching with an empty name, contact payload updates, palette refresh, UIA page selection and both raw surfaces resizing.

Native screenshots independently showed the white Clock face and cyan/green/yellow hands on both local backends; successive Direct2D observations showed changed hand positions. Screenshots also showed the manual Touching rectangle/contact markers, and Direct2D marker/rectangle movement matching the changed payload. The remote Direct2D renderer painted Clock before and after palette refresh/resize, and painted Touching contacts and its moved rectangle. A live GDI replacement received the same Core state, retained the moved rectangle, cleared the old contact and terminated the old renderer normally. Its screenshots showed retained Touching pixels, a new contact after resize, and the Clock selected by the replayed factory name.

Verification diagnostics: the first full Win32 run stopped during an automation case without a summary. A debugger rerun passed that case, then hit a timing-sensitive missing-dialog assertion in an existing asynchronous file-dialog case; pausing at that assertion showed the expected dialog present. The unpaused full-wrapper retry has progressed beyond both cases and remains running. During remote inspection, a renderer terminated in the Windows automation HTTP response handler; the inspection client had left an initial DOM response unread. Native Win32 enumeration exposed the runtime dialog omitted by the general window inventory, and CDB captured the response-handler stack. After closing the failed processes and consuming complete HTTP responses, the remote checks above succeeded. A replacement mistakenly launched before its Core was ready produced a separate named-pipe startup error and was also cleaned up. These diagnostics are separate from the passing painter checks.

The remote GDI continuation passed: its contact input reached Core, updated the rectangle, survived resize, and cleaned up on release. Clock continued drawing at the later received time after an Ember palette refresh. Core and the final renderer both exited normally with code 0; all owned GUI/debugger processes were closed and debugger argument files were restored, including the overlapping renderer-launch settings. Post-resource Debug Win32 and x64 solution builds succeeded; the final x64 incremental check reported 0 warnings and 0 errors. Final full-suite summaries and the remaining x64 metadata cycle are pending.

Final Win32 repository-wrapper result: 98/98 test files and 2001/2001 test cases passed, including both native raw backends and the Core/Workflow/reflection coverage. The final log contains no assertion failure and no appended Debug leak dump. The unpaused retry also passed the earlier automation/file-dialog cases without a source change. Its regenerated snapshots differ only in the five expected raw-element baseline files. Final post-resource Metadata_Generate x64 and Metadata_Test both exited successfully and produced no additional metadata differences. The final x64 full-wrapper run is in progress.

### DENIED BY USER

The initial feature passed the verification recorded below, but the continuation supersedes its combined Clock class and explicit GuiMain registration. Retain the core raw-element contract and refactor its test painter according to No.2. The historical test results describe the initial implementation.

The original missing-property regression is fixed. The sole proposal is selected and its implementation is retained. Interpretor selects a fresh renderer-owned native painter independently of Id; the atomic name/data setter exposes the final data during initialization. Tests verify unchanged-name reuse, empty/missing-name removal, no late retry, independent instances, hosted forwarding, exactly-once finalization before concrete event destruction, target recreation and target-free final release. Manual Touching handlers continue rendering after interpretor installation/removal.

Both final repository-wrapper runs passed all 98 test files and 2001 test cases, with no skipped files, assertion failures or appended Debug leak dump. Win32 passed on the unpaused retry and x64 passed its final full run. These runs include the earlier automation/file-dialog stopping points, which did not recur. Fixed-data native tests verify name-only repaint without timer/input/resize/DOM updates, while real WIC/GDI pixel checks verify Clock hand positions and manual Touching rectangle/contact output. Reflection, XML/Workflow construction, the combined setter, JSON transport and reconnect replay all pass.

Protocol generation, both ordered Debug Win32/x64 metadata cycles, Metadata_Test and FullControlTest resource generation completed successfully. Both post-resource solution builds passed. The final metadata cycle made no further changes, and no resource compiler error artifacts remain. The final snapshot diff is limited to three raw descriptor JSON frames and the two architecture-specific Workflow baselines; the latter differ only by the new property assignment and combined setter call.

All requested Windows application checks completed: CppTest and CppTest_Metaonly RawClock UIA scenarios passed on Direct2D and GDI; GacUI_Host passed the interpreted-resource scenario; CppTest_Reflection compiled with early registration. Native screenshots independently confirmed local and remote Clock/Touching painting. The live remote pair passed palette refresh, resize, page switching, received-time hand movement and GDI renderer takeover; manual Touching retained its rectangle, cleared old contacts, accepted new contacts and cleaned up. Final Core/renderer shutdown returned code 0. No session-dependent check remains unavailable. The earlier verification diagnostics remain recorded separately above.

Final review found no further source changes necessary. Manual implementation and generated assets were committed separately as 1b5ef89f6 and 354682bbe. The five canonical guidance files were committed in Tools as 9704e66, synchronized only into GacUI, and verified byte-identical. The snapshot changes are committed separately. No work was performed in wGac, iGac or GacJS.

## No.2 [CONFIRMED] Split portable Clock geometry from Windows painter plugins

Keep the raw-element/library feature from No.1. Extract a non-template RawClockInterpretorBase declaration to <GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawClockInterpretor.h; define its parsing and geometry methods in the matching .cpp. Virtual line/ellipse methods replace functor arguments. Separate Direct2D/GDI interpretors and factories own only their backend state, with all Windows includes, concrete classes and plugin registration guarded by VCZH_MSVC. An IGuiPlugin registers the active backend factory during controller-related loading, before GuiMain. Remove all explicit registration calls.

Compile the new module through the generated-showcase shared inventory and the direct Host/UnitTest inventories, including their filters. Keep it out of RemotingTest_Rendering_Win32 as requested, leaving its manual Touching adapter. Keep the portable base available without Windows includes or plugin effects; do not edit or build sibling platform repositories.

Reuse native lifetime/pixel coverage on both backends, asserting the factory is already present when GuiMain runs; build current consumers and run the repository unit-test wrapper with its existing selection. Exercise local hosted/native and interpreted RawClock scenarios, and check the renderer without the Clock module still handles manual Touching. No protocol, reflection or XML change is planned, so no metadata/resource regeneration is required. After implementation, remove concrete test-application examples and instructions from KB pages, preserve general library contracts, update canonical Tools copies and synchronize only GacUI.

### CODE CHANGE

Extracted the portable Clock base declaration and out-of-line parsing/geometry definitions. Separate Windows D2D/GDI classes and factories now load through a controller-related GUI_REGISTER_PLUGIN. Removed explicit GuiMain/test registrations, added the new source/header to the shared showcase and direct Host/UnitTest inventories and filters, and left the renderer inventory without the Clock module. The existing native pixel/lifetime tests now require the factory to be present automatically at GuiMain entry. Removed named test-application examples and launch/verification details from 24 KB pages while retaining reusable library contracts. Updated canonical Tools pages, copied them into GacUI, and synchronized the new canonical instruction forbidding test-app material in KB pages.

### CONFIRMED

The Debug x64 solution build through the repository wrapper completed with 0 warnings and 0 errors. The full UnitTest run passed all 98 files and 2001 cases, including the automatic factory-presence assertions at GuiMain entry and both native painter pixel/lifetime cases. The final raw log has no appended Debug leak report. No implementation changes were needed after this verification.

The existing RawClock automation and UIA scenario passed for CppTest, CppTest_Metaonly and GacUI_Host on both Direct2D and GDI. These six runs verified timer-driven data, useful bounds, palette refresh, page switching, resize, manual Touching payloads, normal Window.Close and endpoint release. CppTest_Reflection and the remaining shared-inventory consumers compiled in the solution build.

Fresh Pipe Core/renderer pairs passed on both native backends. Core Controls and renderer Dom agreed on the Clock interpretor and time data; the time continued advancing. Native captures confirmed that the Clock surface stays blank because the renderer deliberately excludes the new module. Its independent manual Touching adapter still painted the rectangle and contact marker, translated by the injected movement, matched Core/renderer payloads and removed the contact on Up. Both Core and renderer returned exit code 0 in each run. Remote renderers do not expose a transported semantic UIA tree; the corresponding controls were verified through local UIA above.

One unrelated file-dialog recording changed its scheduling frame numbers. Comparing the index and all affected frame JSON after removing frameId showed identical rendered trees and element descriptions. The seven scheduling-only snapshot artifacts were restored, leaving no snapshot changes in this refactor.

The 24 cleaned KB pages match their canonical Tools copies byte for byte, and a case-insensitive audit found no remaining named test-application references. Tools commits d6e1e15 and 767350a contain the cleanup and are pushed. The GacUI copy also includes the new canonical KB instruction from Tools commit b45bda4. Final review and git diff --check passed. All owned application processes exited and temporary debugger arguments were restored. No work was performed in wGac, iGac or GacJS; non-Windows portability was reviewed through the declaration-only header, out-of-line portable definitions and VCZH_MSVC guards, without claiming a platform build.
