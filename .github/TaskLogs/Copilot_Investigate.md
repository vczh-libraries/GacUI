# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

Extra requirements for making commits:
- `## Release` should be in the last step.
- Before beginning `## Release`, all local changes in affected repos should be committed and pushed first.

## FakeDialogService TUI Dialogs

- Message/Color/Font/File dialog no need to have empty lines, except one above buttons.
- The message in the message box should not have a border, just put a label there it is fine.
- Message box buttons should align to center horizontally. (other dialogs have button aligned left, that is correct)
- Color dialog scroll bars are too big:
  - Both label and scroll bar should align to center vertically in their cell.
  - Scroll bars should have 1 pixel in height.

### DETAILS

- Make layout changes in the authored `Source/Utilities/FakeServices/TuiDialogs/*.xml` resources. Preserve their factories, view-model bindings, localized strings, default actions and accept/cancel behavior; regenerate protected C++ outputs through `GacUI_Compiler`.
- In `MessageBox.xml`, replace the message's bordered `ScrollContainer` with a normal `Label`. Preserve explicit line breaks, the separate icon label and the message text. Center the natural-width button group across the whole dialog, retaining the named `buttonStack` and its default-button lookup. Do not remove borders from the shared scroll-container template.
- Remove blanket `CellPadding`/stack padding and excess initial client height that introduce empty rows. A stretched percentage content row can retain blank space even after padding is removed. Keep exactly one explicit blank row before the final action row, while preserving actual textbox/list/group borders and useful preview/list areas. Content text containing blank lines must remain unchanged.
- The color dialog's "scroll bars" are the three `HTracker` controls in `ColorDialog.xml`. `tuiskin::TuiHTrackerTemplate` in `Test/Resources/App/TuiSkin/Template_Scroll.xml` already has a one-cell preferred minimum height; the containing component currently stretches it beside a bordered textbox and adds padding. Arrange each tracker at an actual height of one cell, centered vertically, and center each RGB label within its cell. A minimum height alone does not prevent stretching.
- The existing non-message action rows are not consistently left aligned: Color, simple/full Font and File place OK/Cancel after a leading percentage column. Apply the requested left alignment when compacting these rows; retain the full-font dialog's `Pick a Color` action. Center message actions only.
- Clarify `GacUILayout.md` with these dialog/form spacing rules and how to center a one-cell control without stretching it. Keep label text left aligned while centering vertically; use local layout/template changes rather than changing every label's defaults. The existing TUI label already preserves explicit newlines.

### VERIFICATION

- These are implementation requirements, not checks performed by this review. Use the repository build/run wrappers with absolute PowerShell paths from `Test/GacUISrc`. Resource changes require `GacUI_Compiler`, which generates both architectures. Check its output and `git status` for `*.UI.errors.txt`; then follow `Project.md`: build Debug Win32, run `Metadata_Generate` Win32, build Debug x64, run `Metadata_Generate` x64 and `Metadata_Test` x64. Rebuild affected consumers before UI checks. Refresh metadata before compilation too if resources need changed reflection declarations.
- Run the required `UnitTest` project for C++ changes, including generated C++, following `.github/Guidelines/Running-UnitTest.md`. Respect existing file filters while including affected tests, and inspect completed results and Debug leak reports. Keep unrelated GUI snapshot expectations unchanged.
- Follow `.github/Jobs/DebugTuiControlTestSop.md` in Windows Terminal using `copilotExecute.ps1 -Mode CLI -Executable CppTest_Tui -Configuration Debug -Platform x64 -Interactive`. Inspect Message, Color, simple Font, full Font, Open File and Save File at 120x40, 80x25 and after restoring the larger viewport. Require compact rows, exactly one blank row before final actions, centered message buttons and left-aligned other actions.
- Check one-, two- and three-button messages in English and Chinese, short/multiline/CJK text, and cases with/without icons. Require a borderless message, complete readable text, centered actions despite localized widths, correct default activation/results, and normal dismissal/focus return. Include multiline file-validation prompts.
- Count each RGB tracker's rendered height as one row. Test mouse dragging, keyboard movement, 0/255 endpoints and typed RGB 12,34,56; accept/reopen and change/cancel must retain the existing color semantics. Recheck font effects and the nested color picker, file enumeration/selection and nested validation prompts after compacting their layouts.

## TuiControlTest

- After startup the main window does not resized to fit the CLI window.
  - This should also be added to `DebugTuiControlTestSop.md`.
- Although TUI's `INativeWindow::Hide` is currently no-op, but it should:
  - Query window close and give a chance for the app to prepare. You can check out how Windows native GUI implementation does that to invoke necessary callbacks.
  - Call directly or behave like current `GetTuiApplication()->Stop` so that to quit `GetCurrentController()->Run`.
  - Update the `Exit` tab in `TuiControlTest` according to this.
- In `Misc/Dialogs/*`, there are tables where the first column are labels and the second columns are text boxes.
  - Those label should align to center vertically inside their cell.
  - No need to keep empty lines, except one above buttons in the last line.

### DETAILS

#### Startup geometry

- Trace `Test/Resources/App/TuiControlTest/Resource.xml`, `Test/GacUISrc/CppTest_Tui/Main.cpp`, `GuiMainHostedWindowProxy::CheckAndSyncProperties` in `Source/PlatformProviders/Hosted/GuiHostedWindowProxy_Main.cpp`, and `TuiControllerBase::Run` in `Source/PlatformProviders/TUI/TuiController.cpp`. The physical `TuiWindow` constructor reads the terminal size, but hosted setup subsequently copies the showcase's explicit 120x40 size into it.
- Synchronize the physical and hosted main bounds with the current TUI buffer after hosted properties have been applied and before the first visible frame. Do not wait for the initial `BufferSizeChanged`: VlppOS sends it after `Starting`, which is already occupied by the nested application loop. Reuse the existing size/Moved propagation; do not resize Windows Terminal or add a second TUI loop.
- Preserve `SystemFrameWindow` and the main bounds' `MinSizeLimitation="NoLimit"`, so hidden pages cannot enlarge the physical viewport. Keep later terminal-resize handling and the existing programmatic stored-size behavior unless the startup fix requires a documented change to that contract.

#### Main-window closing

- Implement the physical-window behavior in `TuiWindow::Hide(bool closeWindow)` in `Source/PlatformProviders/TUI/TuiWindow.cpp`. Both `Hide(false)` and `Hide(true)` must request closing. The Windows implementation in `Source/PlatformProviders/Windows/WinNativeWindow.cpp` also sends both through its close-query path.
- Deliver `INativeWindowListener::BeforeClosing` with cancellation, and leave the application running when canceled. For an accepted request, deliver `AfterClosing`, update visibility and the corresponding close notification, then request the existing TUI stop so the nested native/hosted/application loops unwind normally. These callbacks reach `WindowClosing` and `WindowReadyToClose` in `Source/Application/Controls/GuiWindowControls.cpp` through `GuiHostedController`; do not duplicate them at each layer.
- Keep physical and hosted `Closed` delivery distinct. `GuiHostedController` does not currently forward the physical `Closed` callback; normal hosted teardown stops the window manager and emits hosted close notifications. Preserve that route rather than adding a second forwarder.
- Respect hosted modal-close handling in `GuiWindow::BeforeClosing`: an open modal can cancel main-window termination while it is focused or dismissed. Hiding/closing a hosted child, popup or fake dialog must keep the physical application running.
- Keep callbacks and stopping on the existing owner thread. `TuiWindow::Dispatch` suppresses ordinary callbacks once stop is requested, so do not set that flag before delivering required close notifications. Preserve listener/lifetime handling when a callback closes again or detaches a listener, and leave destruction to the existing teardown path; `Hide` must not destroy its own object synchronously.
- Keep the four existing direct/queued `self.Hide()` and `self.Close()` buttons in `Resource.xml`, but replace the no-op explanation and expectations with accepted/canceled close behavior. Retain the explicit Stop controls as demonstrations of direct stopping, without making them the required exit route. The actual loop boundary is `GetCurrentController()->WindowService()->Run`, reached through `GuiApplication::Run`.

#### Dialog demonstration forms

- Edit `Test/Resources/App/TuiControlTest/LocalizedDialogsTabPage.xml`. The Message and File form tables already have zero `CellPadding`; center their first-column labels within the actual row heights while retaining left text alignment. Include all fields, not only the first Title row.
- Remove blanket vertical spacing between the file form, `Selected Files` label and results, and retain one explicit gap before its final Open/Save buttons. Add the corresponding single gap before Message/Color/Font launch actions. Preserve the shared locale selector, options, stored values, results, scrolling and keyboard access.

### VERIFICATION

- Apply the generation, metadata, build and unit-test requirements above. Extend `Test/GacUISrc/UnitTest/TestTuiProvider.cpp` for first-frame geometry and accepted/canceled physical Hide/Close. Its current geometry/events test calls `Hide(true)`, asserts visibility and continues input; separate those event checks from accepted shutdown instead of retaining the obsolete no-op assumption.
- Test startup at a viewport different from 120x40, including 100x30 and 80x25. Before any manual resize, require main bounds and first rendered layout to match the terminal. Then shrink/grow and restore the viewport, switch to pages with larger minimum sizes, and verify clipping, scrolling, popup placement and mouse hit positions.
- Test each direct/queued Hide/Close path on a fresh run. Assert close-query and ready-to-close callbacks reach the hosted main window once per accepted request, vetoed requests do not request stop or emit accepted-close notifications, and accepted requests return from the application loop normally. Include a canceled request followed by a successful retry and listener removal/reentrant close during notification, checking no duplicate delivery or use-after-free.
- Exercise main-window closing with a hosted modal open and ordinary child/dialog dismissal. Require existing modal interception and continued responsiveness; closing a child must not terminate TUI. Compare ordinary `CppTest` if shared hosted/window code changes.
- For accepted Hide/Close and both explicit Stop paths, verify process completion and restored terminal contents, cursor, colors and input mode; type into the shell afterward. Repeat after dialog use. Forced termination or closing the terminal tab is not evidence of normal teardown.
- Inspect all Misc/Dialogs forms in both locales at both standard terminal sizes. Labels beside bordered textboxes must align with their content row; one-row combos must remain compact. Verify spacing around results/actions and unchanged values, returned outputs, scrolling, keyboard navigation and focus after dialog dismissal.

## Documentation

- Scan `TODO_Task_TUI_FollowUp_[123].md` and see if any knowledge base page or sop need to udpate.

### DETAILS

- Audit the three follow-ups against current code and guidance, rather than copying historical task descriptions as current behavior. Update GacUI-owned pages in this repository; any VlppOS-specific correction belongs in the owning repository under the mono-repo rules.
- Review `GacUILayout.md`, `.github/KnowledgeBase/KB_GacUI_Design_TuiPlatformProvider.md`, `.github/KnowledgeBase/Index_GacUI.md` and `.github/Jobs/DebugTuiControlTestSop.md`. The provider page and SOP currently describe main Hide/Close as no-ops and startup sizing that can differ from the viewport; update both. Add compact fake-dialog/form checks and all four ordinary Exit paths to the SOP.
- Check that prior follow-up guidance remains accurate for popup/combo/focus styles, grid separators and editor navigation, TAB stops, Windows service ownership, cached labels and local/global shortcuts. Preserve the distinction between actual terminal input evidence and injected provider tests.
- Existing SOP verification records describe earlier runs. Keep their historical meaning and add fresh results for this follow-up; do not relabel an old no-op/Stop-only pass as verification of the new closing behavior. Website and aggregate-release guidance changes belong to the final Release stage below.

### VERIFICATION

- Cross-check each changed behavior in follow-ups 1/2/3 against the owning guidance and an explicit SOP check. Search for stale Hide/Close/no-op, Stop-only, startup-size and dialog-spacing claims, and resolve contradictory current instructions without erasing historical results.
- Validate local links, paths, API names and commands. Record new build/runtime evidence separately from procedures and preserve any unverified terminal appearance/input limitations. Documentation changes alone do not require executing the C++ suite.

## Release

- Refer to `REPO-ROOT/../AGENTS.md` for understanding the mono repo.
- Just like how `DarkSkin` and `FullControlTest` is done, `TuiSkin` should also be published to `Release` repo, and `TestControlTest` should become its tutorial.
  - Check out how `Build.ps1 -Project Release` currently handle `DarkSkin` and `FullControlTest`.
  - `TuiSkin` should go to `Release/Import/Skins/TuiSkin`.
  - `TuiControlTest` should go to `Release/Tutorial/GacUI_ControlTemplate/TuiSkin.vcxproj` with a new project `TuiSkin`.
  - `TuiControlTest` resource files will be put in its `UI/TuiControlTest` sub folder, the same structure to the sibling `BlackSkin` for `FullControlTest.
- To implement the new `TuiControlTest` tutorial, necessary new files should go to `GacUILite` library.
  - In `GacUILite` there is a `WinMain.cpp` to start the Direct2D renderer.
  - I would like you to add a `TuiMain.cpp`, but both files will be in every projects.
  - In order not to break existing apps, you should declare a global macro in `TuiSkin.vcxproj` say `GACUILITE_TUI_APPLICATION`.
    - `WinMain.cpp` will `#ifndef GACUILITE_TUI_APPLICATION` the whole file.
    - `TuiMain.cpp` will `#ifdef GACUILITE_TUI_APPLICATION` the whole file.
    - Therefore no other projects need to change, since not defining `GACUILITE_TUI_APPLICATION` causes `WinMain.cpp` to use and ignore `TuiMain.cpp`.
- Update the website to:
  - Introduce GacUI on TUI and how to start.
  - Remember that TUI does not work with remote protocol, so no need to mention TUI in remote protocol specific pages or sections.
  - Scan the demo list and see if any tutorials are missing, fix the list.
  - DO NOT publish the website.

### DETAILS

- Keep this as the final execution stage. Before beginning it, commit and push all local changes in affected repositories as requested. After completing this stage, commit and push the release/tooling/website-source changes too; the website remains unpublished.
- Follow `../AGENTS.md` and the owning repository instructions. `TestControlTest` above refers to `TuiControlTest`. Following the requested BlackSkin sibling structure, use `../Release/Tutorial/GacUI_ControlTemplate/TuiSkin/TuiSkin.vcxproj`, with `Main.cpp`, `.vcxproj.filters` and `UI/TuiControlTest` in that project directory. Register it in `GacUI_ControlTemplate.sln` for Debug/Release and Win32/x64. This also matches the release verifier's project-directory-relative executable lookup.
- Extend `../Tools/Tools/BuildRelease.ps1`, which currently copies `DarkSkin*` and the FullControlTest resources. Copy upstream `GacUI/Release/TuiSkin*` into `Release/Import/Skins/TuiSkin` and authored TuiControlTest resources into the tutorial. `../Tools/Tools/ProjectGacUI.ps1` already mirrors/generates TuiSkin, and `Release/CodegenConfig.xml` already emits separate TuiSkin/TuiSkinReflection pairs. Refresh the upstream GacUI release before the aggregate `Build.ps1 -Project Release`; that command does not itself regenerate GacUI's upstream release.
- Account for the startup macro's compilation scope: `../Release/Tutorial/Lib/GacUILite/GacUILite.vcxproj` is a separately compiled static library. Defining `GACUILITE_TUI_APPLICATION` only in the new executable does not change how that library compiles `WinMain.cpp`. Inventory both whole-file-guarded startup sources in GacUILite, and compile both sources directly in the new TuiSkin project under its macro. Existing projects continue using the shared library's GUI startup; no existing application project needs editing and no project-wide macro is propagated into the shared library build.
- Make the new executable a Console application whose `wmain` calls `SetupTuiWindowsRenderer()`. Put its reusable TUI startup and skin-registration plugin in GacUILite's `TuiMain.cpp`. Depend on `GacGen_TuiSkinResourceLoader`, install `tuiskin::CreateDefaultColorPackage()` through `tuiskin::SetColorPackage`, then register `tuiskin::TuiTheme` before constructing controls. Add the packaged `TuiSkin.cpp` to GacUILite with `/bigobj`, matching DarkSkin. The GUI startup's whole-file guard must cover its DarkSkin plugin as well as `WinMain`; verify the console link does not pull in that archive member.
- Package the tutorial's non-code resources as well as generated C++. Its authored `Test/Resources/App/TuiControlTest/Resource.xml` currently has no `Cpp/Resource` or embedded-resource setting. Follow BlackSkin with a repeatable `Cpp/Resource` setting of `../../../UIRes/TuiSkin.bin`, and load `../UIRes/TuiSkin.bin` from the tutorial's working directory before constructing `tuidemo::TuiMainWindow`. Supply `Skins/TuiSkin/TuiSkin.h` as a generated ordinary include where needed. Put settings in the authored resource or a repeatable deployment transformation, not a one-off edit to the copied XML that the next update overwrites.
- Website changes belong in `../WebsiteSource`: read its instructions and content-format guidance, add a GacUI TUI startup topic/navigation under `packages/website-doc2/src/articles`, and update relevant introduction/download guidance and `packages/website/src/articles/demo.xml`. Explain Windows Terminal, the Console startup/macro, resource compilation/loading, TuiSkin initialization, cell geometry and supported-control limitations. Keep TUI additions outside remote-protocol pages/sections.
- Audit demo entries against actual Release tutorials. Current candidates include missing `GacUI_Xml/Member_Field`, stale `Member_State` and `GacUI_Controls/TextEditor` links, and the omitted Console_Workflow group; resolve each against the current tutorial inventory and add TuiSkin. Use local build/preview/export only, without running website publishing steps or copying output into the published-site repository.

### VERIFICATION

- After upstream generation, run the prescribed aggregate pipeline with the absolute `../Tools/Tools/Build.ps1` path and `-Project Release`. Inspect the packaged TuiSkin/TuiSkinReflection pairs, copied resources and tutorial inventory. Require tutorial resource compilation for both architectures, generated C++ and `UIRes/TuiSkin.bin` without compiler errors. Repeat update/generation to confirm the new tutorial/settings survive regeneration.
- The aggregate C++ verifier currently builds tutorial solutions in Win32 Debug/Release only. Also build the affected ControlTemplate solution in x64 Debug/Release using the prescribed wrapper. Build GUI and TUI projects together and alternate individual builds; require one correct entry point per app, no macro/output contamination, and correct skin initialization. Check BlackSkin and WindowSkin startup too.
- Launch the packaged TuiSkin tutorial through the documented interactive CLI path in Windows Terminal, with the working directory expected by resource loading. Repeat first-frame viewport fit, resize, localized dialogs/document content, accepted/canceled closing and terminal-restoration checks against the packaged files, rather than using development-tree resources.
- Follow WebsiteSource's local build/test and sequential documentation-generation checks. Inspect the rendered topic, navigation, demo/source links and exported markdown. Record failures or observation limits; do not publish the website or distribute generated manuals as part of local verification.

## REVIEW COMMENTS

# UPDATES

# TEST [CONFIRMED]

Reproduce startup bounds and physical close notifications with the injected TUI backend, then verify compact generated dialogs and forms with real TUI compositions. Follow all acceptance criteria above, distinguishing native terminal input and appearance from deterministic provider tests. Release packaging and website checks run only after the GacUI changes are committed and pushed.

The baseline Debug x64 solution build passed with zero warnings/errors. The new startup regression fails at `window->GetClientSize() == NativeSize(16, 8)` after the application stores 120x40 and enters the native Run loop. The existing no-op Hide expectation has been removed from the independent input/geometry test. A separate close regression requires cancellation, ordered notifications, reentrant calls, listener detachment and eventual stop for both Hide flags. Source inspection confirms the current Hide body is empty.

# PROPOSALS

- No.1 Synchronize startup bounds, implement cancellable physical closing and compact TUI layouts

## No.1 Synchronize startup bounds, implement cancellable physical closing and compact TUI layouts

At the native Run boundary, reapply the actual buffer size through the existing BufferSizeChanged path after hosted properties have been copied and before Show. Keep later programmatic stored sizing unchanged. Physical Hide requests BeforeClosing, honors cancellation, sends AfterClosing then Closed after clearing visibility, and only then calls the existing owner-thread Stop. Guard reentrant closing while retaining the existing snapshot/listener-membership dispatch and normal destruction path. Hosted child/modal handling stays in its existing owner.

Use content-sized dialog rows, zero blanket padding, one explicit action gap, a natural-width centered message action stack, and left-aligned other actions. Replace only the message content scroll container with a label. Center labels and a one-cell RGB tracker with local three-row tables (percentage/minimum/percentage), retaining text alignment and bindings. Preserve useful file lists and previews. Extend the Exit demo with veto and callback counters, and update layout/provider/SOP guidance while preserving historical evidence.

### CODE CHANGE

Planned changes: TuiController.cpp, TuiWindow.h/.cpp, TestTuiProvider.cpp; authored TuiDialogs and TuiControlTest XML followed by resource/metadata regeneration; GacUILayout.md, provider KB/index and DebugTuiControlTestSop.md. Validate generated resources and actual hosted first-frame/close behavior, both metadata architectures, the full selected unit suite and terminal checks. Release/tooling/website implementation remains the final stage after committing and pushing this stage.
