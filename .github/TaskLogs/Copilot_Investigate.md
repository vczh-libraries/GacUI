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

Implementation pending. The reproduction, SOP, targeted UIA checks and website explanation have been committed and pushed.
