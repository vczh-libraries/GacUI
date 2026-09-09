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
