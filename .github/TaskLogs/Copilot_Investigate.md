# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- In `GuiControl` add these 2 members:
  - A `ContextMenu` property in type `GuiPopup*`, default to `nullptr`, `GuiControl` does not care about lifetime of the context menu, it only simply tests if it is null before using it.
  - A `DisplayContextMenuOnRightClick` property in type `bool`, default to `true`. When it is true and when its `BoundsComposition` receiving a right click event with `handled` set to `false`, the popup is displayed with the control and the specified relative location.
  - Update the website to mention these 2 properties in the control page, with the title name "ContextMenu and DisplayContextMenuOnRightClick".
- In `FullControlTest` make this change to the ribbon and toolstrip document test page:
  - The change is made in `DocumentEditorBase.xml`.
  - Add a toolstrip menu with following items:
    - `Undo` `Redo` | `Copy` `Cut` `Paste` | `Delete` | `Select All` `Object` | `Set Font ...` `Text Color ...` `Background Color ...` | `Edit Hyperlink ...` `Remove Hyperlink`
    - Reuse toolstrip commands for every menu item. 
    - `Object` has a sub menu like document toolstrip test page's edit menu.
  - Use `ContextMenu` property to attach it to the document viewer.
  - Verify it on `CppTest` to make sure the context menu would show and menu items are working.
  - Update `DebugRemoteProtocolSop.md`:
    - Fill `3.1 Ensure Document Editor Features on "Document Editor (Ribbon)" test page`:
      - Load `TestDocument.bin`. When testing against `GacJS` make sure `remoteFS` option should not turn on otherwise it cannot access the local file system.
      - Based on the loaded document, try every feature on the ribbon and the context menu, including applying styles.
    - Fill `3.2 Ensure Document Editor Features on "Document Editor (Toolstrip)" test page`
      - Simply mention that applying everything in `3.1` on the toolstrip test page (need to skip style application because the UI does not exist in this test page), but text selections and caret location should be done via point events. Since `wGac` and `iGac` native renderers does not support point events, this will be skipped. Since `GacJS` supports point events so this will not be skipped no matter which OS is running. DO NOT repeat test steps.
    - Run `3.1` and `3.2` only with `CppTest` to make sure the test SOP is working.
- Commit and push all local changes so far.
- Publish the website, but when running `job:copilotInitAll`, skip learning.
- Update in `Release` repo `GacUI_Controls/DocumentEditor` with the same change applied to `DocumentEditorBase.xml`.
  - Run `../Tools/Tools/Build.ps1 -Project Release` to make sure the `Release` repo is fully updated.

## DETAILS

### Control properties and input behavior

- Implement the properties and their getters/setters in `<GacUI repo>/Source/Application/Controls/GuiBasicControls.h/.cpp`. Register both writable properties in the `GuiControl` block of `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp`, following the neighboring `CLASS_MEMBER_PROPERTY_FAST` registrations. No additional change events are required.
- Interpret right click as the persistent `BoundsComposition` receiving the unified `mouseUp` event with `GuiMouseEventArgs::button == NativeMouseButton::Right`. Check the automatic-display flag, non-null menu and `handled == false` on entry to the handler. Other buttons and right-button down do not open the menu.
- Call the menu's `GuiPopup::ShowPopup(GuiControl*, Point)` overload, declared in `<GacUI repo>/Source/Application/Controls/GuiWindowControls.h`, with `this` and `Point(arguments.x, arguments.y)`. Mouse dispatch already translates the coordinates into the current receiver's space. This overload handles owner-relative positioning, native conversion and screen-edge adjustment; do not apply another descendant or screen-coordinate conversion.
- Set `handled` when consuming the event to display the menu, so a child and an ancestor cannot both open menus. Leave it unchanged when the menu is null or automatic display is disabled, allowing normal ancestor handling. A descendant can suppress bubbling by handling the event. Custom handling on the same bounds should disable automatic display rather than assume a later handler can cancel a menu already opened.
- Keep menu ownership with the caller. Replacing or clearing `ContextMenu`, and destroying the control, must not delete the popup. The caller must clear or replace the pointer before destroying its target and honor the existing popup owner's lifetime while the popup is shown. Do not copy `TooltipControl`'s owning behavior.
- Store both properties on the control and attach the handler once to its persistent bounds. Replacing control templates or refreshing themes must retain the values without registering another handler.
- Existing eligible long presses already synthesize right-button mouse input. Reuse that route without another gesture handler. Opening the popup must preserve the document's current selection and caret.

### Shared document-editor menu

- In `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorBase.xml`, declare a named `ToolstripMenu` directly under the root `CustomControl`, then assign it to the `document` viewer with `ContextMenu-eval`. The existing root-child control-host loader supplies ownership through `AddControlHostComponent`; each editor instance owns its own menu independently of the borrowed property.
- Reuse `commandUndo`, `commandRedo`, `commandCopy`, `commandCut`, `commandPaste`, `commandDelete`, `commandSelect`, `commandFont`, `commandColor`, `commandBackColor`, `commandEditHyperlink` and `commandRemoveHyperlink` from that resource. Preserve the requested order and separators, and derive item labels, images, shortcuts and enabled states from the commands.
- `Object` is a submenu header, with no executable command of its own. Its children reuse `commandInsertImage`, `commandEditHyperlink` and `commandRemoveHyperlink`, matching `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorToolstrip.xml`. Retain the separately requested top-level hyperlink items too.

### SOP scope and input setup

- Update `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` itself. GacUI job documents are owned there, as clarified in `<GacUI repo>/.github/KnowledgeBase/Learning.md`; shared instruction synchronization does not provide a corresponding Tools-owned SOP.
- Use `<GacUI repo>/Test/TestDocument.bin`. Reload it between destructive test groups and save test output to separate files, preserving the checked-in fixture.
- The instruction to disable `remoteFS` refers to GacJS HTTP with native Core: omit the browser `remoteFS` option and Core `/remoteFS` so file dialogs use Core's local filesystem. GacJS Wasm uses OPFS directly; any future Wasm run of these steps requires provisioning the fixture into that origin's storage. Disabling injection does not expose the host filesystem to Wasm.
- Keep all feature steps in section 3.1. Section 3.2 should reference them once, omit ribbon-only style application, and replace caret/range setup with point input. Short press positions a collapsed caret; holding a different text position for at least 500 ms and releasing extends the range. Dragging document content scrolls. Long pressing with an existing range preserves it and produces the right-click fallback. These press actions complete on release, not merely when the hold threshold elapses.
- Preserve the requested applicability: native wGac/iGac skip the point-input scenario; GacJS runs it on every OS. Execute the completed SOP sections only in `CppTest` for this task, and record synthetic point input separately from physical touch evidence.

### Website, release inputs and publication

- Add the exact heading `ContextMenu and DisplayContextMenuOnRightClick` under Behaviors in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/controls/basic/home.xml`. This is the common-control property page. Describe defaults, borrowed ownership, event consumption, relative placement and disabling automatic display for custom behavior.
- After implementation and required generation/verification, run the established GacUI release pipeline with `<Tools repo>/Tools/Build.ps1 -Project GacUI`. The later `-Project Release` copies existing GacUI exports and reflection binaries; it does not regenerate those inputs from the changed library source.
- Follow `<WebsiteSource repo>/job.publish.prompt.md` for site build/tests, sequential download steps, Markdown export, publication and manual synchronization. Apply the requested learning skip to both `copilotInitAll` invocations, beginning at `Merge Knowledge Base`. Preserve the existing Wasm deployment unless separately requested; publishing documentation alone does not update that demo's runtime.
- Apply the menu and attachment changes to `<Release repo>/Tutorial/GacUI_Controls/DocumentEditor/UI/DocumentEditorBase.xml`, whose resource entry is `<Release repo>/Tutorial/GacUI_Controls/DocumentEditor/UI/ResourceBase.xml`. Preserve its tutorial-specific `@cpp:UserImpl` persistence hooks; do not overwrite the entire file with the FullControlTest version. The shared base feeds both DocumentEditorRibbon and DocumentEditorToolstrip tutorials.
- Run the requested Release build after updating that authored resource and its upstream release inputs. Include publication, synchronized manuals and Release-build changes in a final commit/push sweep after the earlier checkpoint. Keep authored changes, generated outputs and unit-test snapshots in their required separate commits.

## VERIFICATION

These are acceptance checks for implementing the task; this review does not execute them.

### Core regression coverage and generation

- Verify default null/true values, getter/setter round trips, reflected property types and read/write access. With the popup closed, verify replacing/clearing the borrowed menu and destroying its referring control do not delete it.
- Exercise right-button release opening once, other buttons and right-button down not opening the menu, and suppression by prehandled input, a null menu or the automatic-display flag. For nested controls, verify the child menu consumes the event, an unconfigured child permits the ancestor menu, and a handled descendant prevents ancestor display.
- Verify placement for nested child hits, moved owners, scrolled document content and screen edges. Cover the declared generic `GuiPopup*` contract as well as `GuiToolstripMenu`. Reopen after theme refresh and confirm retained settings, one display and unchanged document selection/caret.
- Use ordinary unit tests for properties, reflection and handler-only assertions. Use the snapshot framework only for meaningful popup rendering/placement frames. Include the existing long-press-to-right-click route in input coverage.
- Follow `<GacUI repo>/Project.md` and the build/run guidelines: generate reflection metadata for Debug Win32 and x64, run `Metadata_Test`, run `GacUI_Compiler` for the changed resources and complete the required post-compiler metadata steps. Rebuild affected consumers and run the applicable `UnitTest` cases through the wrappers. Require successful logs, no resource diagnostics and no Debug leak report; obtain all generated changes from the generators.

### CppTest SOP acceptance

- In section 3.1, load the fixture on the ribbon page and record an observable result for each operation. Check the exact context-menu order, separators, Object children, shortcut/enabled-state agreement and repeated opening/dismissal without altering selection.
- Cover Undo/Redo, Copy/Cut/Paste/Delete/Select All, image insertion, font/text/background-color dialogs including cancellation, hyperlink editing/removal/activation, paragraph alignment and Preview/Selectable/Editable modes. Exercise every executable context-menu entry against its existing ribbon/toolstrip equivalent and verify the document result, not just successful invocation.
- Cover ribbon formatting toggles and all eight styles across Headers and Content, including reapplying the selected style, wide/collapsed gallery popups and preserved selection. Check effective formatting after private save/reload, and verify RTF/HTML export outputs. Treat Search, Icon Labels and sample Item A/B controls according to their actual sample behavior; do not invent missing search or formatting commands for them.
- Repeat menu opening and a command after palette refresh. Reload the original fixture as needed so earlier deletions or mode changes do not invalidate later checks.
- In section 3.2, reuse section 3.1 as specified above. Establish caret/ranges through point events and verify a long press over an existing selection opens the menu exactly once on release with the selection intact. Record actual execution of both sections in rebuilt `CppTest` using the automation service first, then attempt the same operations through UIA and record any unavailable coverage.
- Update targeted assertions in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` and run `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1`, as required for FullControlTest changes. Keep this hosted/native UIA regression coverage distinct from the CppTest-only SOP execution. Record any unavailable desktop-dependent coverage explicitly.

### Publication and tutorial verification

- Confirm the new properties are present in fresh GacUI release exports/reflection inputs before building downstream resources. Check successful Release build logs and fresh x32/x64 resource outputs for the shared editor and both ribbon/toolstrip consumers, with no resource compilation diagnostics. Inspect generated changes rather than editing them by hand.
- Complete the publishing job's build/tests and CI checks. Verify the exact heading and ownership/input explanation in rendered HTML, exported Markdown, the live control page and synchronized manuals. Confirm the tutorial menu uses the same command bindings and ownership pattern as FullControlTest while retaining its existing persistence hooks.
- Check repository diffs after generation, publication and synchronization, and commit/push all resulting task changes with the required separation. Report only the platforms and consumers actually exercised.

## REVIEW COMMENTS

No unresolved review comments. The implementation choices and verification requirements are resolved above.

# UPDATES

# TEST [CONFIRMED]

Start with an ordinary reflection regression requiring writable ContextMenu (GuiPopup*) and DisplayContextMenuOnRightClick (bool) properties on GuiControl, with no change events. Build and run it before implementation to confirm the missing API. Extend ordinary control/input coverage for defaults, borrowed lifetime, filtering, bubbling and persistent subscriptions; use rendered popup frames for placement. Follow every acceptance check above, including metadata/resource generation, CppTest automation and UIA, website publication and downstream Release consumers. Record execution evidence and unavailable coverage explicitly.

The initial Debug x64 solution build is a cold build. The added test in `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` uses descriptor lookup, so it compiles before the new native members exist. It checks both getters/setters, no change events, the raw GuiPopup pointer type and bool type. The completed Debug x64 build reports 0 warnings and 0 errors. The completed wrapper-run UnitTest log ends in `TestReflectionTypeList.cpp`, `Controls expose writable context menu properties without change events`, with `Assertion failure: property` (exit 1): the first required descriptor is absent. This confirms the missing API before implementation.

Source analysis confirms that `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp` creates its bounds once, while template replacement preserves it. `GuiPopup::ShowPopup(GuiControl*, Point)` in `<GacUI repo>/Source/Application/Controls/GuiWindowControls.cpp` performs the required coordinate conversion and screen adjustment. Existing point fallback synthesizes right-button input after End; document selection handling already declines long presses when a range exists. The resource root-child control-host ownership pattern is already used by toolstrip-menu tests.

The initial run also regenerated the existing file-dialog InitialName_Open snapshots before reaching the new regression; those outputs are committed separately. The targeted UIA client now compiles under Windows PowerShell and is queued for execution after rebuilding the showcase. Website build and tests pass; publication has not started.

# PROPOSALS

- No.1 Borrowed context menus on persistent control bounds

## No.1 Borrowed context menus on persistent control bounds

Store a nullable GuiPopup pointer and a true-by-default automatic-display flag on GuiControl. Attach one mouseUp handler when constructing BoundsComposition. On an unhandled right-button release with both settings enabled, mark the event handled and call ShowPopup(this, Point(arguments.x, arguments.y)). Leave ownership and all coordinate conversion with the existing caller/popup APIs. Existing point fallback supplies long-press right clicks; no document-selection or gesture behavior changes are needed. Register writable FAST reflection properties without events.

Declare the shared document editor's named ToolstripMenu directly under its root and bind its entries to existing commands. Attach it with ContextMenu-eval so the loader owns the popup separately from the borrowed property. Verify ordinary property/lifetime/input contracts, rendered placement and document selection, regenerate both metadata architectures and resources, then exercise CppTest SOP and targeted hosted/native UIA regressions. Publish the authored documentation and synchronized manuals after the requested checkpoint, regenerate GacUI release inputs, and update/build the Release tutorial without replacing its persistence hooks.

### CODE CHANGE

Implemented the two stored properties, one persistent-bounds mouseUp handler and FAST reflection registrations in GuiBasicControls and GuiReflectionControls. Added the owned root-child documentContextMenu with shared command bindings and the borrowed viewer attachment. Ordinary regressions cover native/reflected round trips, closed-popup lifetime, handler filtering, nested bubbling, moved owners, generic-popup screen clamping, document long-press selection and scrolling, and retained settings after template/theme replacement. A rendered toolstrip-menu test covers actual popup placement, command execution and edge adjustment. The existing UIA harness now has a DocumentContextMenu scenario. Generation and execution are the next verification steps.

The first implementation build caught a patch-placement error: the two FAST registrations had matched ListViewItem.Tag instead of GuiControl.Tag. Moved them into the GuiControl block next to Context/TooltipControl; the native implementation was unchanged. Rebuilding before generation. The expanded CppTest-only SOP client compiles under Windows PowerShell and checks document results for formatting, dialogs, links, images, modes, scrolling and exports.

The CppTest-only DocumentEditorSop scenario reuses the shared UIA harness while driving first-pass commands and fake dialogs through automation IO. It covers formatting toggles/alignment, font/color cancel/apply, private formatting reload, top-level/Object/equivalent hyperlink actions and activation, image cancel/insert/undo/redo and private/RTF/HTML output, edit modes/shortcuts, point scrolling and selection, and sample controls. DocumentStyles covers all eight styles, reapplication, popup widths, palette changes, and sample Item A/B entries. These are pending runtime validation; synthetic point checks do not claim physical-touch evidence.

The next build reached the new ordinary mouse test and caught test-call signature errors. Reflection SetValue requires an lvalue Value receiver, and RClick takes Nullable<NativePoint>; the test now keeps the receiver Value and supplies explicit NativePoint arguments. The input-test translation unit also includes GuiReflectionPlugin.h for its typed descriptor lookup. No production behavior changed.

Debug Win32 now builds with 0 warnings and 0 errors. Metadata_Generate completed through the CLI wrapper with exit 0; the generated ReflectionCore32 text contains both writable properties and their accessor signatures.

Debug x64 also builds with 0 warnings and 0 errors. Its Metadata_Generate run completed through the CLI wrapper with exit 0. An early Win32 style smoke run is exercising the existing style UI before resource regeneration; it does not count as the completed SOP. The first launch attempt ended before producing a fresh client log, so no result is claimed for that attempt.

Metadata_Test Debug x64 completed through the CLI wrapper with exit 0. The early Debug Win32 DocumentStyles smoke run passed 1,209 assertions and normal Window.Close/endpoint release, including all eight styles, three popup widths, two palettes, sample A/B entries and private style reload. It used the pre-regeneration showcase resources; the full SOP remains pending. UIA popup backgrounds expose unnamed Toggle controls without Invoke, so their application uses automation IO after UIA lookup/scrolling.

Website preparation: sequential website download, documentation download and Markdown export all exited 0. The local browser rendered the exact new heading under Behaviors and all ownership, input, placement and refresh paragraphs; the exported manual contains the same explanation. Markdown export reported existing missing article URLs for registered-type/VlppReflection overview pages; none are introduced by this property section. Publication and manual synchronization remain pending.

SOP client review added separate context/equivalent-command/shortcut clipboard checks with exact text comparisons, effective paragraph alignment, formatted RTF/HTML color/family/size checks, and hyperlink private reload. Point selection resets through another paragraph before starting a new range, matching the existing short-press-in-selection behavior. The updated client compiles under Windows PowerShell; these additions still require runtime validation.

Publication staging used a nondeleting copy after automatic approval review rejected recursive folder replacement with no specific reason. The source and destination documentation inventories both contain 365 files with no extras or missing files, so the staged result is equivalent. Removed 296 generated newline-only changes and regenerated the changed page from LF XML. Only the intended property section remains in the website diff; the existing 19-file wasm-fct folder has no changes.

The generated x86 FullControlTest source registers documentContextMenu with the root through AddControlHostComponent and then calls document.SetContextMenu with that same popup. The separate UiaList inspector solution built Debug x64 through the wrapper with 0 warnings and 0 errors, ready for the required preview/Nodes review after rebuilding the showcase.

The Debug Win32 run passed the new rendered toolstrip-menu case, then failed in the new ordinary lifetime test because it expected SafeDeleteControl to destroy GuiPopup immediately. GuiControlHost deletion is queued until events finish. The test now retains its disposal flags across frames and checks them after queued destruction; popup cleanup also gets a frame before closing the main test window. The borrowed-lifetime assertions before cleanup all passed. This is a test timing correction, with no production change.

GacUI_Compiler Debug x64 completed through the CLI wrapper with exit 0 after regenerating both architectures and merging shared dialogs/viewer code. Both FullControlTest source variants contain the owned AddControlHostComponent registration and borrowed SetContextMenu attachment. The post-compiler Debug Win32/x64 rebuild and metadata sequence follows.

The post-compiler Debug Win32 solution rebuild passed with 0 warnings and 0 errors (1:09), followed by Metadata_Generate exit 0. No resource diagnostics were found after generation. The corrected lifetime test is included in this rebuild. Completed build/test evidence is copied under `<GacUI repo>/Test/Output/ContextMenu` before later wrappers or instruction synchronization can replace the standard logs.

The post-compiler Debug x64 rebuild passed with 0 warnings and 0 errors (1:25), then Metadata_Generate and Metadata_Test both exited 0. This completes the required post-compiler metadata sequence. The rebuilt x64 ordinary suite and CppTest-only DocumentEditorSop wrapper are now running; that SOP scenario includes the targeted context-menu assertions and all-style coverage.

The first rebuilt CppTest SOP attempt reached menu inspection but rejected the client's Escape key alias; the automation API uses Esc. After correcting the alias, synthetic Esc through the hosted input route did not dismiss the nonactivating popup. The SOP/client now record that limitation and use an outside click, explicitly checking unchanged selection. Native-window checks address the popup IO endpoint when available. These are input-driver corrections; no production keyboard behavior was changed.

The completed Debug x64 unit log reports 96/96 test files and 1,966/1,966 cases, exit 0, with no appended leak dump or memory-leak file. The focused hyperlink probe preserved selection and serialized the correct link. A second probe reproduced stale selection reads: immediate reads alternated the previous P/G selection, while ControlsSnapshot returned the new G/P selection. EditorSelect now synchronizes after scrolling and input; the probe confirms immediate and settled reads agree. The original hyperlink selection assertion remains.

The rebuilt UiaList inspector selected CppTest and loaded 125 UIA nodes. Its UI preview explicitly reports "Preview unavailable. The window must be visible and capture must be supported." This session therefore supplies no captured-pixel review; automation geometry shows the 13-item menu and shortcuts within the host bounds. Nodes inspection continues separately.

UiaList Nodes review confirmed Menu role, Invoke providers for Undo/Redo/Copy/Cut/Paste/Select All, and ExpandCollapse without Invoke for Object. Opening Copy's detailed inspector property window then caused UiaList itself to display Microsoft Visual C++ Runtime Library: "Debug Error! ... abort() has been called". Captured the owned dialog and deliberately pressed its Abort button; CppTest remained responsive. Detailed inspector property review and captured pixels are unavailable in this session; the independent CppTest UIA menu assertions already verified item roles, names, states, shortcuts and operations.

The synchronized SOP passed formatted private reload/export and its original hyperlink-selection assertion, then exposed a harness role mismatch: RibbonLargeDropdownButton with a submenu is a UIA MenuItem, not Button. Corrected the Edit Mode lookup using the provider contract, added synchronous completion before read-only mutation assertions, and strengthened shortcut checks to require the actual read-only/selection-support state. Production behavior is unchanged; a focused mode probe precedes the next full SOP run. Public parameter/return comments were completed during code review.

Observed SOP progress under the required concurrent Release link takes over ten minutes to reach the first editor's hyperlink stage. The full scenario repeats both editors and both command routes, then all style checks; its dedicated outer deadline is now 90 minutes. Other scenarios retain their existing 30-minute deadline.

Focused verification confirms all Preview/Selectable/Editable menu commands in both editors via automation IO and UIA, and all ribbon mode shortcuts. Toolstrip Ctrl+Shift+P fails because FullControlTest statically registers duplicate window-wide shortcuts; GuiShortcutKeyManager::CreateShortcutIfNotExist leaves the second editor without those registrations. The existing TuiControlTest resource binds all nine shortcuts to document.Focused. Apply that established pattern to FullControlTest, retain assertions for actual mode/text results, regenerate resources and rebuild before rerunning. The in-progress Release pipeline was deliberately canceled before changing generated inputs; its partial logs are archived and do not count as a successful pipeline.

Readback after the failing toolstrip shortcut confirmed Toolstrip stayed Editable while the hidden Ribbon changed to Preview. This distinguishes the existing shortcut collision from UIA focus or input timing. The focused probe passed all 12 menu-driven mode combinations and the three ribbon shortcuts before exposing it. The diagnostic CppTest exited normally, its temporary user arguments were removed, and GacUI_Compiler Debug x64 is regenerating the focus-bound resources.

Reviewing the remaining command lookups found that the toolstrip Insert Image equivalent is a ToolstripSplitButton. Its lookup now requires UIA SplitButton, matching WindowsUIAutomationSemantic and the authored resource, while using its existing Invoke/main-click action. This is another harness role correction before the full rerun.

The focused-menu assertions now require all six exact clipboard/edit AcceleratorKey strings, including Select All, in addition to agreement with equivalent controls. This checks that focus-bound commands retain the expected shortcut presentation when the document menu opens.

The focus-fix GacUI_Compiler Debug x64 run completed with exit 0. Both generated FullControlTest variants contain nine Focused subscriptions, owned AddControlHostComponent registration and the borrowed SetContextMenu attachment. No resource diagnostics were found; unchanged dialog/viewer/TUI resources returned to their original generated contents. The standard Debug Win32/x64 rebuild and metadata validation sequence is running.

The remaining sample-field checks read values immediately after injected typing. Apply the already-proven ControlsSnapshot completion barrier in EditorSetValue too, so these assertions inspect completed input rather than a previous queued value. This changes only the test driver and retains all result assertions.

The freshly rebuilt CppTest focused mode probe passed all 12 menu-command combinations (Preview/Selectable/Editable, both editors, automation IO and UIA) and all six mode-shortcut checks. The toolstrip Ctrl+Shift+P/S/E checks now change the visible editor correctly. No native runtime-error dialog was present; automation Exit and its CLI wrapper both completed normally with exit 0, and owned temporary user arguments were removed.

The focus-fix post-compiler solution builds passed Debug Win32 (9:37) and Debug x64 (10:39), each with zero warnings/errors. Metadata_Generate passed both architectures and Metadata_Test Debug x64 passed; the combined runner exited 0. The fresh CppTest-only SOP run is in progress and has passed both editors through automation IO with the exact accelerator assertions.

The fresh CppTest SOP passed all four DocumentContextMenu combinations, exact shortcut strings and palette refresh, then all ribbon clipboard routes, formatting toggles/alignment, font/color cancellation/application, formatted private reload/RTF/HTML checks, and all three hyperlink routes. The broader SOP remains in progress. The required GacUI Release pipeline was restarted after the focused fix passed; it is rebuilding Release Win32. Save the requested authored/generated/snapshot checkpoint with this status explicitly pending.

The fresh full SOP passed both ribbon image routes (cancel/insert/undo/redo/private reload/RTF/HTML), then failed text preservation after a typed phrase and one Undo. Source inspection shows automation Type sends individual characters and GuiDocumentCommonInterface records one undo step per character. A focused readback confirmed one Ctrl+Z removes only the trailing space and exactly 13 Ctrl+Z operations restore the original fixture. Correct the driver to undo all typed characters and require exact fixture equality before testing mode shortcuts; retain all later text/state assertions. The underlying editor behavior is unchanged.

The focused remainder probe passed ribbon mode/text/cancellation and Search/Icon Labels/group-expansion checks. Toolstrip persistence then exposed an existing harness lookup that searches the page for Save as, although the authored item belongs to the separate File popup. Locate the item through the existing hosted/native Popup helper, wait for the popup item, and retain all save/cancel/output assertions before rerunning the remainder and full SOP.

The corrected focused remainder probe passed all four editor/automation-IO-or-UIA combinations: exact typed-text Undo restoration, all mode shortcuts and states, canceled load/save, ribbon Search/Icon Labels/group expansion, and toolstrip synthetic point scrolling/selection-preserving long-press opening. The owned diagnostic CppTest had no runtime-error dialog and exited 0 normally. A fresh complete SOP run follows.

Website publication commit d902082638536332777b032f37f3bf602f55f927 passed Pages build/deployment run 41690010506. Browser verification opened the homepage and live basic-control page (redirecting to gaclib.net) and confirmed the exact new heading under Behaviors plus defaults, borrowed ownership, event consumption, relative placement, custom handling and refresh/long-press explanation. Wasm files remain unchanged. The first copilotInitAll job started at Merge Knowledge Base, verified all seven 286-file inputs identical, synchronized eight repos and completed CheckAll. No learning was consumed or changed; only the active GacUI task was reported dirty.

The next full SOP passed all four context-menu passes but reached Select All before the queued context command had completed. EditorCommand’s direct context branch returned before the existing completion barrier used by every other branch. Add the same ControlsSnapshot barrier before that return and retain the exact complete-document selection assertion; rerun the full scenario. This is a test-driver synchronization correction only.

Both publication manual-sync passes completed from Merge Knowledge Base with learning skipped. The 214-file exported manual inventory matched the canonical target exactly, and only the new basic-control section changed. Canonical Tools commit e90391c and all eight propagated manual commits were pushed. Final normalized hashes match all 286 knowledge-base files in all eight repositories; no learning was consumed or updated.

Review before the style phase found that DocumentStyles reads the entire current document’s font size as its baseline. A read-only UIA probe on the populated fixture returns MixedAttributeValue, so this fresh full-SOP path would fail before testing styles. Capture the empty editor’s default font size before the SOP loads any fixture and pass it into DocumentStyles; retain all dedicated style attribute expectations. The current run has passed all four context-menu passes, all three ribbon clipboard routes and the four formatting toggles; cancel it deliberately after preserving logs and restart with the corrected baseline.
