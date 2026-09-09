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

## UPDATE

the overall work is wonderful! but I found two more items to fix:

- For sub windows, the window border should appear between the title and \_OX buttons (for example in a message dialog there is only X button, between title and X there are currently nothing, but the border should be there. if \_OX buttons shows up together they should not have gap in between \_OX).
- Color/Font/File dialogs should align buttons in the last row to the right.
  - This is actually a mistake in my original tast request, so please also fix GacUILayout.md and other documents if needed.

# TEST [CONFIRMED]

Reproduce startup bounds and physical close notifications with the injected TUI backend, then verify compact generated dialogs and forms with real TUI compositions. Follow all acceptance criteria above, distinguishing native terminal input and appearance from deterministic provider tests. Release packaging and website checks run only after the GacUI changes are committed and pushed.

The baseline Debug x64 solution build passed with zero warnings/errors. The new startup regression fails at `window->GetClientSize() == NativeSize(16, 8)` after the application stores 120x40 and enters the native Run loop. The existing no-op Hide expectation has been removed from the independent input/geometry test. A separate close regression requires cancellation, ordered notifications, reentrant calls, listener detachment and eventual stop for both Hide flags. Source inspection confirms the current Hide body is empty.

After applying only startup synchronization, the next unit run reaches the new close case and exits unsuccessfully before a summary. The live pre-layout/pre-close Debug x64 showcase was launched through the interactive wrapper in Windows Terminal at 100x30 (CppTest_Tui PID 11728). Console-event input opens the Chinese message dialog: its message is bordered, contains extra empty rows, and its single action is at the left. The color dialog has five-row RGB components with labels above textbox content and actions at the right. On Exit, direct Hide advances the invocation count to one but leaves the process running. These are live console-buffer and injected-console-input observations. OpenInputDesktop returns null/error 5 and GetForegroundWindow returns null; physical native input and displayed font/cursor/RGB appearance are not verified.

CDB with source lines enabled confirms that the captured/rethrown `UnitTestAssertError::message` is `Assertion failure: events.Count() == 1 && events[0] == L"BeforeClosing"`. Stop TUI exits the baseline process normally; the restored PowerShell accepts a command and prints `TUI_EXIT: 0`.

# PROPOSALS

- No.1 Synchronize startup bounds, implement cancellable physical closing and compact TUI layouts [DENIED]
- No.2 Package the TUI tutorial and document local startup [CONFIRMED]
- No.3 Preserve the child title border and right-align non-message dialog actions

## No.1 Synchronize startup bounds, implement cancellable physical closing and compact TUI layouts [DENIED]

At the native Run boundary, reapply the actual buffer size through the existing BufferSizeChanged path after hosted properties have been copied and before Show. Keep later programmatic stored sizing unchanged. Physical Hide requests BeforeClosing, honors cancellation, sends AfterClosing then Closed after clearing visibility, and only then calls the existing owner-thread Stop. Guard reentrant closing while retaining the existing snapshot/listener-membership dispatch and normal destruction path. Hosted child/modal handling stays in its existing owner.

Use content-sized dialog rows, zero blanket padding, one explicit action gap, a natural-width centered message action stack, and left-aligned other actions. Replace only the message content scroll container with a label. Center labels and a one-cell RGB tracker with local three-row tables (percentage/minimum/percentage), retaining text alignment and bindings. Preserve useful file lists and previews. Extend the Exit demo with veto and callback counters, and update layout/provider/SOP guidance while preserving historical evidence.

### CODE CHANGE

Implemented the native Run synchronization and guarded physical closing in TuiController.cpp and TuiWindow.h/.cpp. TestTuiProvider.cpp separates geometry/input from shutdown, verifies both physical Hide flags with cancellation/reentrancy/detachment, tests all direct/queued hosted paths at 100x30 and 80x25, and exercises real GuiWindow modal interception and first-layout bounds with TUI graphics. Hosted tests call protected controller operations through WindowService(), matching the public API.

Both final resource architectures generated without UI errors. Debug Win32/x64 builds and their metadata generation, plus x64 metadata validation, passed. Compilation corrected the new composition assertion to GetCachedBounds(). The first unit run passed the physical and eight hosted closing combinations, then exposed a fixture error: SetHostedApplication requires clearing the existing unit-test hosted application before installing the nested TUI host. The fixture now clears/restores that registration explicitly; production hosted code is unchanged.

The next fixture assertion incorrectly used GuiControl::GetVisible() (composition visibility) to check window dismissal. CDB confirms main closing reaches GuiWindow::BeforeClosing through the single physical/hosted path. The test now uses GuiControlHost::GetOpening(), which reads native-window visibility, for both modal and main windows.

Authored TuiDialogs and TuiControlTest XML now use compact content rows, centered message actions, left-aligned other actions, centered field labels and one-cell RGB tracker rows. Exit adds a veto checkbox and close-query/ready counters. Resource generation precedes both metadata architectures and the final full unit suite. GacUILayout.md, the provider KB/index and the SOP describe the new contracts and retain historical records. Release/tooling/website implementation remains the final stage after committing and pushing this stage.

### DENIED BY USER

The continuation corrects only this proposal's non-message action alignment: Color, both Font variants and File actions must be right aligned. Its original left-alignment requirement and the evidence below remain historical. Retain the working startup, closing, compact-row, centered-message and one-cell-tracker changes; No.3 supersedes the rejected alignment and adds the missing child title-border behavior.

The final Debug x64 unit run passed 90/90 files and 1,748/1,748 cases with no memory-leak dump. Both final Debug architectures build with zero warnings/errors; Metadata_Generate passed in Win32/x64 and Metadata_Test passed in x64. GacUI_Compiler completed both resource architectures with exit 0 and no UI error files. Existing GUI snapshots are unchanged; the new Tui/Closing test contributes its generated record.

Live Windows Terminal checks used rebuilt Win32 PID 13292 and x64 fresh processes at 100x30/80x25, plus a 120x40 → 80x25 → 120x40 dialog run (PID 17588). Initial layouts fill both non-resource viewport sizes. All seven message sets returned the expected first default in Chinese at 120x40 and English at 80x25; second/third defaults returned SelectTryAgain/SelectContinue. All four icon choices retained CJK text. The Chinese overwrite prompt kept its question and filename on separate unbordered rows, with one gap and centered actions.

RGB labels align with textbox contents. CDB measured each actual tracker as 38x1 cells; live attributes show its strip on only that content row. Arrow keys move one, Home/End reach 0/255, and dragging changes/clamps values at both ends. Entering 12,34,56 yields #0C2238, reopening retains it, and canceling later edits preserves it. Both font variants retain their previews and compact left actions; full font retains effects and its nested Pick a Color. File forms scroll to adjacent result labels/lists and their one-gap actions. The picker retains useful lists and accepts the relative filename Code\VczhLibraries\GacUI\Project.md under C:\ as the exact existing path; an earlier absolute filename input was discarded because that field is combined with its current directory. Empty selection and Chinese overwrite prompts dismiss back to their owner; cancel preserves prior results.

All four vetoed Exit requests advanced invocation/query counters once to four while ready stayed zero. Accepted direct Hide (13292), queued Hide (9868), queued Close (19348), direct Close (16812), direct Stop (12944) and queued Stop (9276) each completed normally with exit 0. Direct Close after the final dialog run also returned 0. Each restored the shell's input/output modes 484/7, attributes 7 and visible 25-percent cursor, and accepted a command in the restored shell. Provider tests establish the once-only query/ready/closed ordering, reentrancy, detachment and modal interception.

These live checks use console event replay and buffer/debugger inspection. Input-desktop access fails with error 5, so physical keyboard/mouse input and final displayed font styles/cursor/RGB fidelity remain unverified. Prior follow-up records remain historical; current procedures were audited against follow-ups 1/2/3. The release/tooling/website stage follows this commit.

## No.2 Package the TUI tutorial and document local startup [CONFIRMED]

GacUI implementation commit 062d670ef is pushed; Tools, Release and WebsiteSource are clean and aligned with their remotes before this stage. The parent AGENTS.md matches Tools/MonoRepo.md. Owning instructions, release scripts, project inventories and WebsiteSource article/navigation/export guidance have been read.

Refresh GacUI through the existing Release-GacUI operation, including GacGen, both skin architectures and the final CodePack. Extend BuildRelease.ps1 to copy TuiSkin pairs and authored TuiControlTest resources. Apply repeatable deployment settings for GacUI.h plus Skins/TuiSkin/TuiSkin.h, and Cpp/Resource ../../../UIRes/TuiSkin.bin. Add the TuiSkin project beside BlackSkin for all four configurations. Inventory guarded WinMain.cpp/TuiMain.cpp and packaged TuiSkin.cpp in GacUILite, and compile both startup files directly in the console project under GACUILITE_TUI_APPLICATION. The TUI plugin depends on GacGen_TuiSkinResourceLoader and initializes its palette before the theme. Main loads the non-code resource before constructing TuiMainWindow.

Add a GacUI TUI startup article/navigation, update introduction/download guidance and audit the demo list against actual tutorials. Keep terminal additions outside remote-protocol sections. Run Build.ps1 -Project Release, repeat resource deployment/generation, build ControlTemplate x64 Debug/Release and alternate individual GUI/TUI builds through supplied wrappers. Check packaged startup/dialog/exit behavior. Run WebsiteSource build/tests and sequential main/doc downloads plus markdown export; inspect rendered navigation/topic/demo and exported markdown locally. Do not publish or distribute manuals.

### CODE CHANGE

Implemented the repeatable resource deployment in Tools/Tools/BuildRelease.ps1, both guarded GacUILite startup sources, packaged TuiSkin compilation and the new console tutorial with all four solution configurations. The generated UI implementation is TuiControlTestPartialClasses.cpp. A preliminary CppCompressed setting was removed after checking GacBuild's contract: it embeds binary resources, whereas this tutorial deliberately loads its separate UIRes/TuiSkin.bin. The first aggregate run had already loaded the earlier deployment function; the repeat deployment/generation results below verify the final settings.

Release-GacUI completed both skin architectures and final CodePack with exit 0 and no skin compiler error files. Its generated upstream outputs were committed and pushed as 82bcb46a2 before aggregate import. The aggregate Release pipeline then ran through all resource and C++ tutorials.

WebsiteSource adds the local TUI startup article, reference navigation, introduction/download links and the corrected demo inventory. All 61 tutorial projects outside the shared Lib directories match the list: Member_Field replaces stale Member_State, stale TextEditor is removed, and Console_Workflow plus TuiSkin are included. The ten-package website build and three-package test suite (57 tests) passed. Sequential website/doc downloads and markdown export completed. Browser inspection confirms the guide layout, navigation, code examples and demo links. The new guide's download link now points directly to Release so it also survives markdown export. Export still reports eight references to excluded home/registered and vlppreflection/home pages; none comes from the new TUI guide. No publishing or manual distribution was performed.

Packaged launching exposed a pre-existing wrapper defect: GetSolutionDir starts from Get-Location's PathInfo, whose Parent is unavailable, so launching from a tutorial project directory never reaches the parent solution. The owning Tools/Copilot/Scripts/copilotShared.ps1 now walks DirectoryInfo.FullName/Parent; the GacUI and Release copies are synchronized. The existing GacUI interactive CLI branch is also copied to the Tools template and Release wrapper. Solution discovery now passes from the nested TuiSkin project and from GacUISrc itself. The Release wrapper launches the packaged console executable from its resource-loading working directory.

ControlTemplate Debug/Release x64 builds both passed with zero warnings/errors. Live x64 Debug startup uses the packaged TuiSkin.bin and first renders at 100x30. CDB resolves wmain and DefaultTuiSkinPlugin::Load, with no WinMain or DefaultSkinPlugin::Load in TuiSkin. BlackSkin and WindowSkin both create their expected native windows, resolve WinMain and DefaultSkinPlugin::Load with no TUI entry/plugin, and exit normally through WM_CLOSE. Terminal verification continues with debugger output capture for teardown/leaks; native input-desktop access remains unavailable.

The x64 Debug individual build sequence TuiSkin, BlackSkin, TuiSkin, WindowSkin passed through Build-Sln without rebuilding unrelated solutions. At that point, the aggregate pipeline had passed all Win32 Debug tutorial builds and their executable inventory while its Win32 Release builds continued. The authored changes pass diff whitespace checking. Aggregate import exposes existing whitespace in GacUI.h documentation; its hash exactly matches the protected upstream generated header, which was not hand edited.

Packaged terminal checks use Debug x64 PID 20768 at initial 100x30, 120x40 and 80x25, Win32 Debug PID 11856 at initial 80x25, and fresh x64 Debug/Release exit processes. Chinese three-action messages retain borderless CJK text, centered actions and default SelectCancel. Typed RGB 12/34/56 accepts as #0C2238 and survives reopening/cancel; English simple/full font previews and nested color work. File enumeration and the empty-selection validation prompt return to the owner, with compact labels/actions. Embedded document controls load their text from UIRes/TuiSkin.bin and relayout after restoring the larger viewport.

All four vetoed Hide/Close requests report invocation/query counts 4/4 and ready 0. Accepted direct Close after dialogs (20768), direct Hide (11856), queued Hide (11268), queued Close in Release (21360), direct Stop (11628) and queued Stop in Release (1696) each exit 0. CDB records process 20768 exiting 0 without a leak dump. Settled shell modes are 484/7, attributes 7 and cursor 25/visible; shell commands remain usable. An initial direct-Stop replay clicked column 12 outside the natural-width button; inspection showed no invocation, and clicking its actual column 5 completed normally. Intermediate snapshots under concurrent build load were taken during repaint; stable snapshots are the evidence used here. Physical input and displayed font/cursor/RGB fidelity remain unverified.

The aggregate Build.ps1 -Project Release completed with exit 0 through the final repository check. Its log contains 88 /P32 and 88 /P64 resource compilations, all twelve GacUI tutorial solution builds (six solutions in each Win32 configuration), successful Workflow generation/rebuild and no failure/crash/missing-binary diagnostics. No resource compiler error files were produced. Its final repository check reported only the expected uncommitted Release package. Repeat UpdateRelease and tutorial generation followed as recorded below.

The independently built Debug x64 binaries also pass dumpbin /headers inspection: TuiSkin has AMD64 machine 8664, Console subsystem 3 and wmainCRTStartup; BlackSkin and WindowSkin have GUI subsystem 2 and WinMainCRTStartup. This complements the live entry/plugin checks and confirms the alternating project builds retain the intended startup selection.

### CONFIRMED

Repeat Build.ps1 -Project UpdateRelease completed successfully, followed by Release/Tools/GacBuild.ps1 for Tutorial/GacUI.xml. Both copied BlackSkin and TuiSkin resources rebuilt for /P32 and /P64, merged and deployed without errors. The final TuiControlTest resource retains NormalInclude GacUI.h;Skins/TuiSkin/TuiSkin.h and Resource ../../../UIRes/TuiSkin.bin. CppCompressed and its unused generated TuiControlTest.cpp are absent. The project and all four solution configurations survive the update. All five generated TuiControlTest C++ source/header hashes match the pre-repeat build inputs, so the completed four-configuration builds and alternating individual builds cover the final generated code. TuiSkin.bin was regenerated after removing the preliminary embedded-resource setting and was verified separately at runtime.

All four packaged TuiSkin/TuiSkinReflection files match the final upstream GacUI release hashes. The twenty copied authored XML files other than transformed Resource.xml also match upstream hashes. Whitespace checks pass for the authored startup, project, solution and wrapper changes; the full package check reports inherited whitespace in the upstream generated GacUI.h and copied XML script bodies. Those imported contents are preserved rather than hand edited.

The final Win32 Release packaged process (PID 22076) uses the regenerated binary resource and first renders at 100x30 without resizing. Embedded Controls displays its packaged document text/table. A Chinese message containing 最终发布资源：你好！ is borderless with one gap and a centered 确定 action; Enter returns SelectOK. A canceled direct Close reports invocation/query counts 1/1 and ready 0, and the accepted retry completes normally. The interactive wrapper reports exit 0; shell modes 484/7, attributes 7 and cursor 25/visible are restored, and a shell command runs afterward. The owned shell/terminal then closes normally with exit. No TuiSkin, GUI sibling or CDB process remains. Physical input and displayed font/cursor/RGB fidelity remain outside the available observation surface.

The aggregate build, repeat generation, packaged startup/dialog/closing checks, x64 solution builds, alternating project builds, website build and 57 website tests establish this release stage. Local website/doc download and markdown export checks are complete, with the existing excluded-category references recorded above. The website remains unpublished and generated manuals were not distributed. Release is committed and pushed as 42fe20ae; Tools as 9e1a1dd and WebsiteSource as a97babb. The final GacUI verification record and SOP are committed after these results.

No.1 and No.2 are complementary stages, not competing alternatives: retain No.1's provider/layout fixes and No.2's final packaging/documentation changes together. The complete GacUI suite passed 90/90 files and 1,748/1,748 cases before the release stage, with unchanged GUI snapshots and no leak dump; the release-only changes do not modify those tested provider/layout sources.

## No.3 Preserve the child title border and right-align non-message dialog actions

Keep No.1's startup, closing and compact-layout fixes. In authored TuiSkin Template_Window.xml, remove the opaque background covering the full title hit region. Render the caption with its own leading/trailing space using the existing ellipsized SolidLabel, leaving the already-rendered thick/double border visible up to the close button. Keep an empty title empty. Preserve title dragging, resize hit regions, the compact close glyph and hosted frame capabilities: GuiHostedWindowProxy_NonMain currently disables minimize/maximize, so do not add unsupported window commands or new gaps between caption buttons.

Move the flexible column before the minimum-width action columns in ColorDialog.xml, both FontDialog.xml window variants and FileDialog.xml, shifting the action sites accordingly. Right-align the complete full-font action group including Pick a Color. Preserve one explicit row above actions, message-button centering, all bindings and accept/cancel behavior. Correct current GacUILayout, provider KB and SOP guidance while retaining prior verification records as historical.

Verify generated resources for both architectures, the prescribed Debug Win32/x64 build and metadata sequence, and the full configured unit suite including leak/snapshot checks. Inspect rebuilt Windows Terminal dialogs in both locales at 120x40 and 80x25; check short/long/CJK/empty titles, thick/double borders, title dragging and close hit targets, right-aligned Color/simple Font/full Font/Open/Save actions, nested dialogs, and normal return to the shell. Refresh the existing upstream release and packaged tutorial last, after committing/pushing the GacUI fix; build and smoke-test the affected packaged consumer. The website remains unpublished.

### CODE CHANGE

The baseline authored title strip contains the full-width SolidBackground that erases the border after the caption. The four non-message window tables place their Percentage column after their action cells, confirming the old left alignment. The continuation changes are scoped to these XML layouts and their current guidance; generated files will be updated only through the supplied compilers and release scripts.
