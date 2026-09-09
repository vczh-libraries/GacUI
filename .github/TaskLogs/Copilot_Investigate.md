# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

## TuiSkin

- Normal popup template doesn't need to have a border.
  - But menu popup should have that border.
  - GacUILayout.md requests this but this does not look good, fix this document.
- The first pixel of either vertical or horizontal scroll's dragging handler has a different color than the rest, they should have consistent color.
  - When the scroll bar is disabled, the first pixel is visible, which is confusing. These two issues might be connected.
- Combo box need to only take one line, make it a button style with dropdown arrow, instead to have a border making it 3 pixels height.
- In ListView's header, the sorting triangle should be put before the column text using the text color.
  - Currently it is put at the very last, which is confusing when the column also has a dropdown triangle.
- `ALT` label should use white background and black text so that when it it put on top of a control it looks obvious instead of missing the shortcut key sequenceand the control text together.
- When a button or similar looking controls is focused, changing the text to bold doesn't seem obvious enough, change that to use bold+underline instead.

### DETAILS

- Author skin changes in `Test/Resources/App/TuiSkin` and palette defaults in `Test/GacUISrc/Generated_TuiSkin/TuiSkinConfig.cpp`. Follow `Project.md` for generation; do not hand-edit generated outputs or the release mirror in `Source/Skins/TuiSkin`.
- Ordinary combo dropdowns currently reuse the bordered `tuiskin::TuiToolstripMenuTemplate` through `SubMenuTemplate` in `Template_List.xml`, around content that may already have a border. Use a separate borderless `MenuTemplate` for content dropdowns and retain the bordered template for actual menus. Generic `GuiPopup` already disables frame features in `Source/Application/Controls/GuiWindowControls.cpp`; inspect the separately bordered tooltip in `Template_Window.xml` as another ordinary popup. Preserve opaque popup backgrounds and the content controls' own borders.
- `tuiskin::TuiHandleScrollButtonTemplate` in `Template_Scroll.xml` overlays a single `█` on a button background. Replace this with uniform handle painting: enabled states remain distinguishable from the track, and disabled handles blend into it. Both scrollbar orientations and trackers share this template.
- `tuiskin::TuiComboBoxTemplate` in `Template_List.xml` serves both ordinary and date combo boxes. Remove vertical border/padding while preserving `ContainerComposition`, `TextVisible`, selected-item templates, the arrow, button states and date-picker support. This also affects calendar selectors, filters, dialogs and embedded controls.
- Put the sorting indicator before the header title, with the title's state-dependent text color. Reserve separate space for the right-side submenu button, title ellipsis and the resize hit target described under Control Bugs.
- Give `tuiskin::TuiShortcutKeyTemplate` in `Template_Misc.xml` an opaque background. Supply white/black defaults through the palette instead of hardcoding XML colors. Include button, check/radio, combo, menu and tab-header text in the focus-style change; tab headers obtain focus from `OwnerTab.Focused`. Preserve existing hover/pressed underlining.
- Update `GacUILayout.md` during execution to describe borderless ordinary popups, bordered menus, one-row combos, opaque ALT labels and bold+underline focus. Clarify redundant vertical spacing based on the TuiControlTest fixes below.

### VERIFICATION

- These are execution requirements, not checks completed by this review. Use the repository build/run wrappers from `Test/GacUISrc`, with absolute PowerShell script paths. Resource-content changes require `GacUI_Compiler`; its existing entry point generates both architectures. After generation, check compiler output and `git status` for `*.UI.errors.txt`, then perform the `Project.md` sequence: build Debug Win32, run `Metadata_Generate` Win32, build Debug x64, run `Metadata_Generate` x64, and run `Metadata_Test` x64. Rebuild affected consumers before UI verification. Refresh metadata before compilation too if changed reflection declarations are needed by the resources.
- Run the required `UnitTest` project for any C++ changes, including generated C++. Follow `.github/Guidelines/Running-UnitTest.md` for existing file filters and check the completed log for failures and memory leaks. Add behavior regressions in the relevant existing test files for shared-control/provider changes; preserve existing GUI snapshot expectations unless a confirmed shared bug requires a change.
- Follow `.github/Jobs/DebugTuiControlTestSop.md` in a real Windows Terminal at 120 by 40 cells, then 80 by 25 and back. Launch `CppTest_Tui` with CLI mode and `-Interactive`; it has no HTTP automation endpoint. Inspect actual bold, underline and colors as well as cell geometry.
- Check ordinary combo/date/filter popups and tooltips, plus nested File/Edit menus. Content popups must have no extra frame, menus must retain their thin border, and dismissed popups must leave correctly repainted cells. Measure closed plain-text, custom-item and date combos as one row and verify their displayed selection and disabled states.
- Check horizontal/vertical handles of one and multiple cells at both ends, during hover/drag, explicitly disabled and when content fits the viewport. Require uniform color, no isolated disabled glyph, and working arrow/paging/drag behavior. Include trackers.
- Cycle sorting states on ordinary headers and Birthday's filter header at wide and narrow widths. Sorting must precede the title and remain separate from the working submenu arrow. Show ALT sequences over text and colored controls. Move the pointer away while transferring keyboard focus across buttons, check/radio controls, combos, menus and tabs to verify focus alone produces bold+underline.

## TuiControlTest

- TextList/ListView/TreeView/BindableDataGrid
  - Extra lines around top/bottom of the list is unnecessary, as list boxes have a border, they would be clear.
  - TextBox/TextBox, Misc/Localization has a similar issue, remove all unnecessary empty lines between controls.
- Empty line between data cell bottom border and content, should remove the gap.
  - I think this was designed like this because text editor has 3 pixel height. So a different look of text editor should be used.
  - In TUI's `DataGridComponents.xml` make a dedicated control template for `demo::TextEditor`'s text box.

### DETAILS

- Inspect `TextListTabPage.xml`, `ListViewTabPage.xml`, `TreeViewTabPage.xml`, `DataGridTabPage.xml`, `TextBoxTabPage.xml` and `LocalizedStringsTabPage.xml` under `Test/Resources/App/TuiControlTest`. Their `CellPadding="1"` settings can add both outer and inter-cell spacing. Remove redundant blank rows around bordered controls while retaining necessary insets inside borders and the existing page content/grouping.
- The text editor is currently named `tuidemo::TuiTextEditor` in `DataGridComponents.xml`, used as `demo:TuiTextEditor`. Define and explicitly assign its dedicated borderless, one-row textbox template there. Preserve caret, selection and document-baseline colors; keep the ordinary standalone textbox template's appearance.
- A local editor template alone cannot remove the grid gap. `DefaultDataGridItemTemplate::OnInitialize` in `Source/Controls/ListControlPackage/GuiDataGridControls.cpp` explicitly reserves three TUI rows. Remove that fixed reservation and derive the height from the actual visualizer/editor content. Retain the bottom/right separator space owned by `CellBorderVisualizerTemplate` in `GuiDataGridExtensions.cpp`; one content row should meet its bottom separator without an empty row between them.
- Coordinate compact rows with the shared combo-template change and selected-value fix below. The same text editor is used by `RefreshListTabPages.xml`. Update the obsolete three-row-editor explanation in `.github/KnowledgeBase/KB_GacUI_Design_TuiPlatformProvider.md` after the change.

### VERIFICATION

- Inspect every named page and all TextBox child pages at both terminal sizes, after shrinking/growing and after leaving/returning. Require compact spacing, intact borders, accessible controls and unchanged demo content. Switch localization between en-US and zh-CN and retain all twelve formatted entries.
- In BindableDataGrid, inspect rows before, during and after editing Name, Gender, Category, Birthday and Website. The content must meet its bottom separator without a blank row; caret, selection and dropdown content must remain visible. Verify accepted values after reopening cells and scrolling rows out of view and back.
- Repeat the File editor on Refresh List / BindableDataGrid. Verify standalone textboxes still retain their intended appearance, independent editing, scrolling and Tab/No Tab behavior.

## TUI Platform Provider

- `TuiController` accepting an `INativeController` is not a good design:
  - e.g. in Windows implementation, it should make a new class inheriting from `TuiController`, and fill Windows specific services and other stuff.
  - `INativeInputService` and `INativeResourceService` does not need to be its base class, the Windows implementation could just make two service classesinheriting from original Windows ones.
- Verify how clipper applies when drawing elements.
- `TuiLabelRenderer` should cache its paragraph to avoid unnecessary paragraph creation.
  - Reuse the cached paragraph during rendering.
- `TuiElementRenderer` is better to split into 3 different classes instead of having 3 `if constexpr` branches.

### DETAILS

- The portable class is `TuiControllerBase` in `Source/PlatformProviders/TUI/TuiController.h`. `TuiWindowsController` already derives from it in `Source/PlatformProviders/Windows/TUI/TuiWindowsController.Windows.h`, but still forwards an `INativeController*`. Remove that dependency and `nativeServices` from the portable base, and move its input/resource service inheritance and platform forwarding into the Windows implementation.
- Keep shared async/callback services, terminal window/geometry behavior and the owner-thread event pump portable. Windows input/resource subclasses must preserve the TUI timer behavior and size-one `TuiFont` metrics while reusing Windows key, shortcut, cursor and OS-name functionality.
- Include the Windows service-window lifecycle and notification routing. `WindowsInputService::SetOwnerHandle` in `Source/PlatformProviders/Windows/ServicesImpl/WindowsInputService.cpp` supplies the HWND used for global shortcuts; clipboard handling also needs its native owner. `GodProc` in `Source/PlatformProviders/Windows/WinNativeWindow.cpp` currently routes clipboard and hotkey messages through `windowsController`. Adapt or share that plumbing with explicit ownership, listener teardown and global-controller restoration; constructing service subclasses alone is insufficient. Never pass a `TuiWindow` to Windows code expecting a `WindowsForm`.
- Adapt `TuiTestController` in `Test/GacUISrc/UnitTest/TestTuiProvider.cpp` to the new service arrangement while keeping its injected backend independent of Windows services and a real terminal.
- Clipping already happens through `TuiGraphicsRenderTarget` drawing methods in `Source/PlatformProviders/TUI/TuiGraphics.cpp`, using the inherited clipper stack intersected with the current viewport. Empty clipper hooks alone are not evidence of a bug. Preserve original border geometry without inventing corners at clip boundaries, reject partial width-two glyphs, and retain the backend's repair of existing wide-character pairs.
- Split `TuiElementRenderer` in `Source/PlatformProviders/TUI/TuiGraphicsRenderers.cpp` into concrete renderers for `TuiBorderElement`, `GuiSolidBorderElement` and `GuiSolidBackgroundElement`, preserving registrations and drawing behavior.
- Keep `TuiLabelRenderer` using a cached `IGuiGraphicsParagraph`. It already retains the paragraph between unchanged redraws, but currently recreates it on every element-state or render-width change. Reuse the existing paragraph through its width, wrapping, alignment, style and color setters. The implementation in `Source/PlatformProviders/TUI/TuiTextLayout.h/.cpp` has no text or render-target setter, so recreate it when the effective display text or render target changes, including text changes caused by single-line normalization or ellipsis. Cache natural unwrapped/unellipsized metrics separately and refresh them when normalized source text changes, even if the displayed ellipsis text remains identical. Preserve document/caret/inline-object paragraph behavior.
- Update `.github/KnowledgeBase/KB_GacUI_Design_TuiPlatformProvider.md` during execution because its ownership guidance currently describes the design being replaced.

### VERIFICATION

- Extend `TestTuiProvider.cpp` for nested/disjoint clippers, restoration after popping an empty clipper, partially clipped fills/borders/labels/carets, width-two text at both edges, and drawing after a buffer resize. Assert border geometry and wide-pair integrity. Exercise all three concrete renderers through element creation and render-target attachment.
- Cover the actual label renderer, beyond the existing paragraph tests: empty text, single/multiline CRLF handling, tabs, CJK/supplementary characters, horizontal/vertical alignment, wrapping, ellipsis, natural minimum size and `WrapLineHeightCalculation`. Verify unchanged redraws reuse both the paragraph and its layout; width/wrapping/alignment changes that keep effective text unchanged reuse the paragraph while updating layout, and style/color changes repaint without recreating it. Check replacement when effective text or render target changes, ellipsis-width changes that produce either identical or different display text, and source-text changes that preserve displayed ellipsis but change natural metrics.
- Preserve existing deterministic window/input/async/timer tests. In Windows Terminal, check startup, titles, resize, clipboard notifications, global shortcut callbacks, queued/delayed work and caret blinking, including inside modal dialogs. Use both Stop buttons on separate runs and verify terminal input, cursor and colors restore normally. Confirm ordinary `CppTest` startup and services still work after the Windows refactor.

## Control Bugs

For every bug, you need to verify if this is a TUI only issue or a general GacUI issue that happens in both `CppTest_Tui` and `CppTest`.

- Starting `CppTest_Tui` and `CppTest` together always crash the later one. `CppTest_Tui` does not start the automation service so there might be a different reason.
  - Starting other GUI test apps together is expected to fail when the http port is taken by the started automation service though.
- ListView/DataGrid's non-last column can't drag to resize. The first pixel of the column is reserved for dragging to resize the previous column.
- DataGrid:
  - When a cell is selected, pressing [LEFT]/[RIGHT] can't move the selected cell. Need to verify if this is a TUI only issue or a general GacUI issue.
  - When the text editor is opened, pressing [LEFT]/[RIGHT] select another cell instead of moving the caret.
  - Data grid combo box editor does not render the select value, even though the cell value is still updated. This is probably a TUI only issue.
- Shortcut key `ctrl+win+alt+q` not working, this is a TUI only issue, make sure it is the limitation of CLI preventing you from using `ALT`:
  - If so change it to `ctrl+win+shift+q`.
  - Otherwise this would be a bug.

### DETAILS

- Record each issue's reproduction in both applications and distinguish source-based hypotheses from confirmed runtime causes. Use `.github/Guidelines/Debugging.md` for crashes and `.github/Jobs/DebugRemoteProtocolSop.md` for the ordinary FullControlTest comparison.
- Both showcase `Resource.xml` files register `global:Ctrl+Shift+Alt+Command+Q` and read `commandGlobalShortcut.Shortcut.Name`. `WindowsInputService::RegisterGlobalShortcutKey` in `Source/PlatformProviders/Windows/ServicesImpl/WindowsInputService.cpp` reports an occupied chord, which can leave the command without a shortcut. Investigate this potential startup collision with the crash stack. If confirmed, assign the TUI demonstration a distinct global chord and update its displayed instructions consistently; preserve visibility of unexpected registration failures.
- Inspect `ListViewColumnItemArranger` in `Source/Controls/ListControlPackage/GuiListViewControls.h/.cpp`: its splitter width is eight, including in terminal cell units, and a non-last column's splitter is attached inside the next header's container. The TUI container in `Template_List.xml` starts one cell inside the header. Make the TUI hit region one cell at the actual first cell of the following header while retaining appropriate GUI metrics. Also check the drag handler's immediate read of cached bounds after changing expected bounds under deferred layout.
- Separate selected-cell state, grid keyboard focus and editor focus. `TestControls_List_DataGrid_Properties.cpp` currently tests arrow navigation after explicit `SetFocused()`, which misses the mouse-selection path. Trace the list and DataGrid key handlers in `GuiListControls.cpp` and `GuiDataGridControls.cpp`, including handled-event propagation while an editor owns focus.
- `GuiVirtualDataGrid::OnKeyDown` in `GuiDataGridControls.cpp` currently clamps the right boundary to the column count, an invalid index that clears selection. Keep navigation within valid columns when fixing the reported behavior.
- Gender/category editors in TUI `DataGridComponents.xml` supply custom item templates. `GuiComboBoxListControl` in `Source/Controls/ListControlPackage/GuiComboControls.cpp` hides ordinary text for that mode and inserts the selected-item composition instead. Inspect its bounds, minimum size and colors together with the compact combo/grid changes; forcing ordinary text visible would bypass the intended content path.
- The local `Ctrl+Alt+Win+Q` command and OS global shortcut use different input paths. The imported Windows TUI decoder preserves Alt from console records but does not populate `osSuper`, as documented in `.github/KnowledgeBase/KB_VlppOS_TerminalUserInterface.md`. Changing Alt to Shift while retaining Win is therefore insufficient by itself. Trace real modifier delivery first and use the requested fallback only after proving an Alt limitation. If the decoder needs fixing, change the owning VlppOS source and use the normal dependency-update workflow rather than editing `Import`.

### VERIFICATION

- Reproduce startup in both orders with the second process under the prescribed debugger; record the failure stack and shortcut-registration result. After fixing the cause, both applications must remain interactive. Exercise their distinct global shortcuts, then stop/restart either application and verify registration release. Keep the expected automation-port conflict between other GUI test apps separate.
- In both applications, resize first, middle and last columns wider/narrower, including after horizontal scrolling. In TUI, target the first cell of the following header for each non-last column. Require the intended column to resize, header/cell alignment to agree, and sorting/submenu targets to remain usable.
- Begin grid navigation by clicking a cell, then repeat with Tab focus and programmatic focus. Left/Right must move one column in the same row; the first/last boundaries must retain selection. Check Up/Down too. Extend the existing DataGrid tests to cover mouse focus and boundaries.
- Open the text editor by supported mouse/keyboard paths. Left/Right must move the caret without changing cells, and Shift+Left/Right must select text. Verify subsequent typing at the moved caret, editor dismissal and restored grid navigation. Preserve current Enter/Escape and immediate `CellValue` submission semantics; do not assume Escape rolls back an already submitted value.
- In both gender/category editors, verify the initial selection, changed selection, closed display and saved value agree after popup dismissal, editor reopening, page revisiting and row recycling. Cover both applications even when the eventual fix is TUI-specific.
- Test Ctrl, Alt, Shift and Win separately and in the requested local chord using actual terminal input. Test OS global delivery separately. Synthetic modifier injection verifies forwarding only; record unavailable terminal modifiers explicitly instead of claiming the replacement chord works without observing it.

## MISC

- `TuiSkin` and `TuiControlTest` resource files should be added to `GacUI_Compiler`, just like `DarkSkin` and `FullControlTest`, but it doesn't affect compiling, so no need to run `GacUI_Compiler`.

### DETAILS

- Add each authored XML file under `Test/Resources/App/TuiSkin` and `Test/Resources/App/TuiControlTest` explicitly as an `Xml` item in `Test/GacUISrc/GacUI_Compiler/GacUI_Compiler.vcxproj` and in its `.vcxproj.filters`, grouped under the corresponding resource folders. Follow the existing DarkSkin/FullControlTest pattern without wildcards or generated-output entries.
- `Test/GacUISrc/GacUI_Compiler/Main.cpp` already compiles both resources and generates both architectures; this request concerns Solution Explorer inventory. The instruction not to run the compiler applies to this inventory-only change. The skin/showcase content changes above still require generation and the metadata/build checks described under TuiSkin.

### VERIFICATION

- Compare the authored XML inventories with both project files. Require every resource exactly once, existing include paths and valid filter mappings, including any template resource added while implementing the other sections. Check that both project files parse as XML and that the existing resources remain listed.
- Verify this inventory change does not alter resource compilation order or add another compilation pass. Do not run code generation solely to verify the inventory edit.

## REVIEW COMMENTS

# UPDATES

# TEST [CONFIRMED]

The verification requirements in the problem description are the acceptance criteria; runtime observations and source hypotheses are recorded separately.

- `TestControls_List_DataGrid_Properties.cpp`: new `NavigateAfterMouseSelection` starts with an actual cell click, then checks focus and horizontal/vertical navigation. New `NavigateColumnBoundaries` requires selection to remain in the first/last valid columns. Existing selection cases remain unchanged.
- `TestTuiProvider.cpp`: new nested/disjoint clipping regression covers restoration after popping an empty clipper, fills, original border geometry, clipped carets, width-two glyphs at both edges and drawing after a physical-buffer resize. Existing deterministic paragraph/window/timer tests remain in place.
- Further renderer regressions will check actual registered elements and paragraph/layout caching, with natural metrics checked independently of ellipsized display text. Extend existing editor/column tests for focused caret movement and splitter drag behavior.
- Build and run through repository wrappers, inspect completed raw logs for failures and leaks, regenerate changed resources and perform the required Debug Win32/x64 metadata sequence. Compare existing GUI snapshots and resource inventories.
- Runtime matrix: real Windows Terminal at 120x40, 80x25 and restored size, with ordinary CppTest comparison for shared controls; use CDB for startup collision stacks and report unavailable physical inputs/visual observations explicitly.

Initial source observations (not runtime confirmations): the label renderer recreates its paragraph in every state-change callback; DataGrid clamps Right to the column count instead of its final valid index; the splitter reads cached width immediately after invalidating expected bounds; the TUI handle overlays one block glyph; combos and ordinary content dropdowns have vertical borders.

Baseline Debug x64 solution build completed with zero warnings/errors. The full UnitTest run reached the new DataGrid tests: mouse selection and navigation passed; `NavigateColumnBoundaries` failed after Right on the last column, confirming the invalid-index defect in shared controls. The suite stops at this assertion, so later provider tests have not yet run.

With CppTest_Tui running in Windows Terminal, CppTest as the second process under CDB requested Ctrl+Shift+Alt+Win+Q. `RegisterHotKey` returned zero, GetLastError was 1409 (hot key already registered). The second-chance exception stack ran through `vl::__vwsn::This<IGuiShortcutKeyItem>` (null `thisValue`), the generated MainWindow shortcut-name subscription, `GuiInstanceRootObject::AddSubscription`, MainWindow initialization, and GuiMain. This confirms a shared OS chord collision, independent of the GUI automation port.

Windows Terminal console-buffer inspection confirms baseline three-row combos and redundant blank rows on TextList. The computer-use service does not expose this Terminal window, and desktop capture fails with an invalid handle; physical font/color appearance and actual physical modifier delivery remain unverified. Console input injection is recorded only as synthetic input.

# PROPOSALS

- No.1 [CONFIRMED] Coordinate terminal services, rendering caches and compact control templates

## No.1 [CONFIRMED] Coordinate terminal services, rendering caches and compact control templates

Keep terminal window/event/async behavior in the portable controller, with abstract platform service accessors. The Windows controller will own Windows-derived input/resource services, clipboard/image services and its own explicitly owned hidden service window. Route clipboard and global hotkey messages using the window's controller pointer, detach native owners before destroying the HWND, and restore the previously published controller. Preserve terminal timers and size-one fonts in the Windows service overrides. Adapt the injected test controller with deterministic standalone services.

Replace the three conditional renderer branches with concrete types. Keep the label's display paragraph until effective text or render target changes, and use existing setters for state and width changes. Cache normalized source text and its natural metrics independently of ellipsis. Use the registered graphics layout provider so tests can observe actual paragraph creation and layout reuse without production diagnostic counters. Retain clipper-stack behavior and test drawing at clip boundaries and after resize.

Use borderless opaque ordinary content menus/tooltips, one-row button-like combos, uniform scroll handles and palette-driven ALT badges. Sorting belongs before the header title; reserve its right-side submenu separately. Apply bold plus underline to focused text, retain hover underlining, remove redundant table padding and give embedded grid textboxes a dedicated borderless template. Remove only the obsolete TUI minimum row reservation, retaining cell separators. Inspect selected combo item colors in the intended custom-item path and preserve standalone textbox appearance.

For shared controls, synchronize layout before reading the resized header's effective width; use a one-cell splitter at the next header's outer edge only for TUI, preserving GUI dimensions. Clamp DataGrid navigation to valid columns, preserve editor key handling and restore grid focus on editor dismissal. Extend tests before treating any unconfirmed mouse-focus hypothesis as a defect. Give the TUI showcase a distinct global shortcut after confirming collision, while leaving unexpected registration failures visible. Do not substitute Alt with Shift without evidence from real input; document decoder and physical-modifier limitations separately.

Add all authored TuiSkin/TuiControlTest XML files exactly once to both compiler project inventories. Update layout/provider guidance. Generate resources and metadata in the prescribed order, run the full unit suite, and compare both showcase applications through their available native surfaces.

### CODE CHANGE

- Portable provider: remove the native-controller parameter, forwarding pointer, and input/resource-service inheritance from TuiControllerBase. Keep shared callbacks, async work, terminal windows and the owner-thread pump portable. Adapt the injected test controller to standalone services.
- Windows provider: own Windows-derived input/resource services, clipboard/image services and a message-only service HWND. Route clipboard/hotkey notifications through GWLP_USERDATA. Preserve 16 ms terminal timers and size-one TuiFont metrics; detach native owners, listeners and COM on teardown.
- Controller/resource lifetime: save the actual installed native controller through GetNativeController, restore all published globals on normal/exceptional return, and pair GuiHostedController's constructor listener in its destructor. Do not finalize hosted windows after GuiApplicationMain has already unwound its application. Stop the terminal timer only while the backend exists. Windows image frames/encoders use their owning WIC service.
- Rendering: replace the conditional element renderer with concrete border, solid-border and solid-background renderers. Reuse a label's display paragraph and layout through width, wrapping, alignment, color and style setters; replace it for effective-text or render-target changes. Cache natural source metrics independently. Allocate new graphics element IDs across resource-manager lifetimes because GuiElementBase caches IDs for the process.
- Shared controls: clamp DataGrid Right to the final valid column. Consume document navigation keys after moving the caret, preserving existing singleline Enter and immediate CellValue semantics. Synchronize header layout before reading resized bounds. TUI splitters occupy the following header's first cell; GUI splitters retain their eight-pixel metrics.
- Compact grid editors: remove the fixed three-row TUI reservation. Keep a CellBorderVisualizerTemplate's outer separator visible, hide only its content, and copy that content inset to the editor. Keep editor ownership in the original data cell so visualizer replacement cannot destroy it. Restore the visualizer content on dismissal.
- Authored skin/showcase: separate opaque borderless content popups from bordered menus; compact ordinary/date combos; paint uniform scrollbar/tracker handles; move sorting before the header title; add white/black ALT palette defaults and bold-plus-underline focus. Remove redundant padding on the six named pages. Assign the grid's dedicated borderless textbox template and initialize custom combo selected-item colors. Give the Birthday filter its explicit content-menu template. Prevent hidden-page minimum sizes from enlarging the physical main window with NoLimit on the showcase's outer bounds.
- Startup chord: the TUI global demonstration now uses Ctrl+Shift+Alt+Win+F8; ordinary FullControlTest retains Q. The local Ctrl+Alt+Win+Q command is unchanged because an Alt limitation was not established and replacing Alt while retaining unobservable Win would not solve it.
- Inventory/documentation: list all 33 authored XML files exactly once in both compiler project inventories without changing compilation order. Update GacUILayout.md, the TUI provider knowledge base and shortcut instructions. Regenerate outputs through the existing compiler entry point.

### CONFIRMED

The proposal fixes the reproduced boundary/editor/resize/startup defects and implements the coordinated provider, renderer and resource changes. It is the sole proposal and is retained.

**Automated validation**

- The baseline full suite reproduced the new invalid-right-boundary assertion. The mouse-selection regression passed, so no speculative generic focus change was made.
- The final Debug x64 full UnitTest run returned zero: **90/90 files and 1743/1743 cases passed**. The completed raw Execute.log contains no skipped/failed cases or memory-leak dump. An earlier complete x64 run also passed after the final production C++ editor-placement change.
- All eight provider cases passed, including actual registered renderer creation, paragraph/layout reuse, clipping, wide-character repair, resize, deterministic input/async/timer behavior and explicit stopping.
- Shared regressions cover mouse selection, valid column boundaries, caret navigation/selection/replacement, unchanged singleline Enter, Escape dismissal and restored grid navigation, plus first/middle/last column growth/shrink with horizontal scrolling.
- Final GacUI_Compiler execution generated all existing resources for both architectures and returned zero, with no UI.errors.txt or leak output. Both generated TuiControlTest.cpp files contain F8, the explicit borderless filter and NoLimit main bounds.
- The required post-generation sequence passed: Debug Win32 solution build (zero warnings/errors, 37.20 seconds), Metadata_Generate Win32, Debug x64 solution build (zero warnings/errors, 34.82 seconds), Metadata_Generate x64, Metadata_Test x64. Every command used the repository wrapper.
- Both project XML files parse; all 33 authored XML resources occur exactly once with valid filters. GacUI_Compiler/Main.cpp is unchanged.
- Existing GUI frame/JSON snapshots match HEAD. Ten generated compiler text logs reorder unchanged declaration blocks; new regression snapshots come from the test runner. The authored diff has no whitespace errors. Four new generated compiler text snapshots retain the generator's extra blank line at EOF; they were not hand-edited.

**Runtime comparison**

| Issue | Evidence and result |
|---|---|
| Starting both showcases | CDB confirmed the second application fails in both orders when Q is occupied: RegisterHotKey returns 0, GetLastError=1409, modifiers=0xf, key=81, id=1. The null shortcut is read by the generated shortcut-name subscription through This<IGuiShortcutKeyItem>, AddSubscription and MainWindow initialization. With TUI F8 and GUI Q, both applications remained interactive in both startup orders. |
| Header resizing | GUI first/middle/last columns grew and shrank with their original hit regions; the initial GUI drag already worked. TUI uses the actual first cell of the next header and now grows/shrinks first/middle/last columns. Repeated TUI checks after horizontal scrolling at 80x25 preserved header/cell alignment. |
| Mouse/grid navigation | GUI non-editor mouse selection followed by Right moved the focus rectangle. Updated TUI non-editor selection followed by Right moved the cell indicator. Shared tests protect mouse focus and first/last boundaries. The confirmed invalid-column and editor-event defects are fixed. |
| Text editor navigation | Both applications moved the caret with End/Left and replaced a Shift-selected character without selecting another cell. GUI produced 涼宮 春X日 then 涼宮 春Y日. Enter left the singleline editor open; Escape dismissed it and grid navigation resumed. |
| Custom combo values | GUI Category initially displayed Lime and retained Black after selection/reopening; its selected-item path was already visible. TUI Gender changed Female to Male and Category changed Lime to Black, with visible closed/custom-editor values retained after page revisiting and ItemSource removal/restoration. |
| Local/global shortcuts | Distinct Q/F8 registrations were held while both applications ran and released on exit. A synthetic WM_HOTKEY reached the TUI-owned service HWND and displayed the exact F8 dialog. Physical shortcut delivery is limited as described below. |

**Terminal geometry, rendering and services**

- Used real Windows Terminal and the CLI wrapper with -Interactive. The final executable fit 120x40, shrank to exactly 80x25 with intact outer/inner right and bottom borders, and returned to 120x40.
- Inspected TextList, both ListView panes, independently expanded TreeView, compact BindableDataGrid, all four TextBox leaves and Localization at the small viewport. Standalone textboxes retain their borders; document panes wrap and scroll independently. The Tab-enabled textbox inserted a four-cell tab without changing its sibling.
- Both zh-CN and en-US retained all twelve formatted localization entries, including literal-dollar sentences, after resizing and page changes.
- Grid content meets its separator before/during/after text and custom-combo editing. The Refresh List File editor changed Three to ThreXe at the moved caret and retained it on reopening.
- Plain/custom combo dropdowns retain only their content control's own border. Date popups are borderless. The final Birthday filter is an opaque two-row popup without an extra frame; checking From enables its date text and opens a calendar inside the viewport. Nested File/Save As and Edit menus retain their frames. Dismissal repaints the underlying controls.
- Sorting renders before Birthday's title and separately from its working submenu arrow. Header resizing remains usable after scrolling.
- Read-only CDB inspection confirmed TrueColor mode and a 16 ms timer. Horizontal/vertical multi-cell handles use RGB(128,128,128) throughout against RGB(64,64,64) tracks. One-cell trackers use the same colors. Content-fit disabled handle cells use RGB(64,64,64). Dragging moves handles to the other end without leaving the old glyph. Legacy console attributes map both grays to one palette slot and cannot distinguish these RGB values.
- Synthetic Alt display produced black foreground and white background (native attribute 240). After moving the pointer away, focused button, checkbox, radio, combo and tab-header text had bold=true and underline=true in live TuiPixel styles.
- Copy/paste between independent TUI textboxes produced ArcherArcher. CDB confirmed an actual clipboard notification stack: TuiControllerBase::ClipboardUpdated <- TuiWindowsController::ServiceWindowProc <- USER32 dispatch <- PumpPlatformEvents <- RunOneCycle. This notification came from the control copy operation.
- Queued Hide/Close callbacks visibly completed (Invocations completed: 4). The queued Stop path exited under CDB with code 0 and no leak output; its restored shell accepted echo TUISTOPOK.
- A separate final direct Stop run exited under CDB with code 0 and no leak output. The launch shell measured identical state before/after: input mode 503, output mode 7, cursor visible=true and Gray/Black console colors; it printed RESTORED True and then accepted echo TUIFINALOK. The F8 registration was available afterward.

**Verification limits**

The computer-use service exposes no usable capture/input surface for these Terminal/CppTest windows, and desktop capture fails with an invalid handle. Physical font/RGB appearance, real Ctrl/Alt/Shift/Win delivery, native global activation, and the exhaustive physical hover/disabled matrix remain unverified. SendInput returned zero with error 5 (access denied); synthetic console records and service messages establish forwarding only.

The owning VlppOS Windows decoder preserves Ctrl/Shift/Alt from console records but does not populate osSuper. No physical Alt limitation was proven, so the requested Alt-to-Shift fallback was not applied. The local Win chord remains subject to that documented backend limitation.

A synthetic Down probe did not establish a focused menu item; its focus binding was reviewed but that runtime observation is not marked passed. The GUI Gender editor uses an image-only selected composition, whose exact selected value was not exposed by the HTTP tree. TUI item-source replacement/page revisiting was checked, but the small fixed dataset did not exercise every requested scroll-recycling permutation. Tooltip and every explicit disabled/hover state were not exhaustively observed. These limits remain follow-up verification, not claimed passes.

### INVESTIGATION NOTES

- The trial replacement global chord T was also occupied on this machine. CDB and an independent registration probe returned 1409; Q and F8 could register after the showcases stopped. F8 is the final authored/generated chord.
- Debugging showed textbox focus was already correct. ProcessKey moved the caret and then returned false, allowing the grid to process the same navigation key. Trial preview/Enter-propagation approaches were discarded; final Enter behavior is unchanged.
- The registered-renderer test exposed a process-static element-ID collision when a later GUI resource manager began allocating IDs from zero. A test-only forwarding registry would not solve later manager lifetimes; the process-wide allocation fix passed the full GUI suite.
- Exception debugging exposed facade-pointer restoration, StopTimer after backend destruction, and hosted-window finalization after application unwinding. The final ownership changes preserve the original error while detaching listeners and restoring globals.
- At 80x25, the old main bounds preferred 80x25 but inherited a cached child minimum of 98x27 and actual bounds of 98x28. The final authored NoLimit change fixes the physical viewport, confirmed in the regenerated executable.
- An attempted debugger-injected setter call in an earlier probe used an incorrect implicit this argument. That probe was terminated; this was a debugger experiment failure and is not a normal Stop result. Subsequent verification used read-only debugger inspection and actual control input.
- Intermediate builds caught Unicode character-literal encoding, protected listener conversion and container-type cast errors; these were corrected. The first splitter test used an assertion-only idle frame, which the test framework rejects. Its assertions now run in the next real drag/close action. Final builds and the full suite pass.
