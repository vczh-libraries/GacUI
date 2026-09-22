In `FullControlHost` there is a `EazyLayout` page, there was multiple ez:Layout nested together. I made a change to keep it "theoretically" equivalent, except that the splitter is moved to another place.
In order to test nested layout, I removed nested ez:Layout and merge everything together, besides of the moved splitter (this will not be mentioned below) everything else should be working exactly the same.
Your work is to rebuild `FullControlTest`, verify it, and anything is not working, firstly check if there is any xml mistakes and fix them for me, otherwise it is the implementation of ez:Layout which should be fixed.
Basically the idea is that, nested layout doesn't need nested `<ez:Layout/>`, for example:
- Line 15: direction of that `Fill` only affect the one below it, it does not affect any other `Fill` because they are in different container.
- Content in container Line 14 will generated different table with container Line 42 because those `Row` and `Fill` are in different container, so they don't share.
Pay attention to the `Rebuild` button, if clicking it crashes, try to run `BuildLayout` with `GuiApplication::InvokeInMainThread`.

In the same file `EazyLayoutTabPage.xml` I would like you to create another tab page for complex table generation test (but still do not put nested `<ez:Layout/>`). So they should cover things that is uncovered due to my change. My change (see the last commit) is covering nested layout objects. I would call it `EazyLayoutTablePage` with title "Eazy Layout (Table)".

## DETAILS

### Scope and baseline

- The referenced edit is commit `a426672d5`; compare it with its parent `83fc7100b`. The actual resource file is `Test/Resources/App/FullControlTest/EasyLayoutTabPage.xml`, and the existing class/title are `demo::EasyLayoutTabPage` / `Easy Layout`. `demo::FullControlHost` and its Layout tab list are in `Test/Resources/App/FullControlTest/Resource.xml`.
- Keep the latest descriptor nesting and the relocated splitter. Each page should have one owning `ez:Layout`, with nested descriptor groups instead of nested `ez:Layout` objects. Preserve editor/label binding, the two Fill weights 1:2, direction selection, the five mixed docking controls, and their interactive state.
- The author confirmed that removing dynamic padding is intentional: generated nested cells, stack items and bounds must preserve the visible gap of the single owning layout. Keep the default padding of 5 throughout the descriptor tree. Generated nesting must neither multiply the gap nor introduce extra insets at container boundaries; the owning layout applies its border only once. Do not restore the old padding binding or add a padding selector to compensate for a spacing defect.
- The latest edit also removes the shared 1:2 percentage-column grid and its spanning row. Restore those table demonstrations on the new page while retaining the latest descriptor structure on the existing page.
- Use `.github/Guidelines/GacUIEazyLayout.md` for descriptor grammar, direction, spacing, spans and splitter behavior; use `.github/KnowledgeBase/KB_GacUI_Design_LayoutAndGuiGraphicsComposition.md` for measurement, arrangement and settling behavior.

### Existing page and implementation boundaries

- Fix the authored first column's `CellOption="composeType:Absolute percentage:120"` to `CellOption="composeType:Absolute absolute:120"`, as confirmed by the author. `GuiCellOption` in `Source/GraphicsComposition/GuiGraphicsTableComposition.h` defaults its absolute field to 20, so setting the percentage field does not request a 120-pixel absolute track. Compilation success alone cannot detect this mistake.
- The explicit Direction on the inner Fill applies to that Fill's immediate sibling group: the editor and bound label. It must not rotate the root Top/Fill/Bottom arrangement or the separate docking/grid groups. Groups without an explicit direction inherit their enclosing descriptor arrangement as documented; isolation does not mean disabling inheritance.
- Track options, spans and splitter boundaries belong to the grid generated for one descriptor group. The inner Fill group, the mixed Left/Right group and the Row/Column group must remain separate. The current `Prepare` implementation in `Source/GraphicsComposition/EazyLayout/GuiEasyLayout.cpp` already creates a plan per container; reproduce a failing case before changing it. Investigate XML first, then the loader in `Source/Compiler/InstanceLoaders/GuiInstanceLoader_EasyLayout.cpp` or the layout implementation as the evidence requires.
- Rebuild now targets the owning layout, which also contains its triggering button. Exercise an actual mouse click as well as UIA Invoke. If rebuilding during input dispatch invalidates the active composition route, defer the call through `GuiApplication::InvokeInMainThread` in the XML/Workflow handler so it runs after dispatch. Preserve control identity, bindings, event handlers and state; do not suppress the failure with exception recovery or reconstruct the controls.

### New table page

- Add a second resource instance in the existing `EasyLayoutTabPage.xml` Folder with class `demo::EazyLayoutTablePage` and title `Eazy Layout (Table)`, preserving the requested spelling. Add its instance beside the existing page in `Resource.xml`, with a distinct ALT key. The existing Folder link already loads this file; no separate XML file or manually edited generated C++ is needed.
- Demonstrate both rows containing columns and columns containing rows in independent descriptor groups under the page's single owning layout. Use readable labels and at least one editable/bound payload so the results can be measured and state retention exercised.
- Cover the following positive cases without making the showcase a collection of intentional failures:

| Case | Expected result |
| --- | --- |
| Shared tracks across several rows, and the transposed arrangement | Single-span declarations supply shared options within their grid; a nondefault declaration overrides other MinSize declarations for that track. Independent groups do not share options. |
| MinSize, Absolute and Percentage options | Absolute tracks have known initial sizes, minimum tracks fit their contents, and two percentage tracks divide the remaining usable space in a 1:2 ratio. |
| Spanning and ragged content | Include the removed two-column span, shorter rows and an empty single-span declaration where useful. Every shared track has a single-span declaration. Outer Row/Column spans change cell coverage without adding outer tracks; inner spans advance placement along the shared axis. Use valid nonoverlapping sites for visible examples. |
| Both splitter axes and repeated shared boundaries | Include draggable boundaries adjacent to Absolute tracks. Repeated markers for one shared boundary produce one splitter; dragging changes only adjacent Absolute options in that generated table. |
| Nested spacing and rebuilding | Generated cells, stack items and bounds preserve one visible padding gap without extra nested insets. Provide a Rebuild action to restore descriptor-defined track sizes after dragging and verify retained payload state. Ordinary window resizing does not require Rebuild. |

## VERIFICATION

### Generation and builds

1. Record the authored XML defects separately from any reproduced implementation defect. Keep this review's source observations separate from later runtime results.
2. Follow `.github/Guidelines/Building.md` and `Running-CLI.md`: build from `Test/GacUISrc` with the repository `copilotBuild.ps1` wrapper and run `GacUI_Compiler` through `copilotExecute.ps1 -Mode CLI`. Invoke PowerShell scripts with `&` and their absolute paths. Check the compiler exit/output and any `*.UI.errors.txt`; do not edit generated files by hand.
3. After resource generation, perform the required `Project.md` sequence: build Debug Win32 and run `Metadata_Generate`, then build Debug x64 and run `Metadata_Generate` and `Metadata_Test`. If a reflected API changes, regenerate the input metadata before compiling resources that consume that change as well. Verify that both generated architecture variants contain the new page and rebuild the applications before running them.

### Existing page behavior

1. Use automation Controls/Dom geometry at a fixed client size and wait for layout to settle before comparing bounds. Check the editor and bound label in the initial horizontal arrangement, the mixed docking row and its unused middle space, the 24-pixel grid row, the corrected 120-pixel first column, and the separate Shared tracks label. Measure visible gaps across descriptor-container boundaries as well as within each group: neighboring occupied regions have one 5-pixel gap at minimum size, generated nesting adds no inset, and the root border is applied once. Distinguish deliberately unused docking space from padding at enlarged sizes. Account for minimum sizes and coordinate conversion when measuring tracks versus label bounds.
2. Enter distinct editor text, select Second choice and toggle Right check. Toggle the arrangement checkbox and verify geometry remains unchanged before Rebuild. Click Rebuild, verify the editor/label arrangement becomes vertical with weights 1:2, and confirm the other groups retain their orientation and ordering. Return to horizontal and repeat with UIA Invoke. Check editor text, live label updates, combo selection, checkbox state and functioning handlers after each rebuild.
3. Require Left one, Left two, Left three, the combo and Right check to have matching outer top/bottom bounds, positive nonoverlapping bounds and visible content. Verify combo expansion, all three choices, selection and dismissal before and after resizing, rebuilding and palette refresh.
4. Drag the relocated splitter using automation IO. Verify its hit area is in the existing padding gap, only the adjacent Absolute track changes, the other descriptor groups are unaffected, resizing retains the adjustment, and Rebuild restores the configured width. A marker's mere presence does not prove dragging works.

### New page and automated regressions

1. Verify the new tab is reachable and has the exact requested title. Measure its shared track edges, spans, gaps, absolute sizes and percentage ratios at initial and enlarged sizes. Check both grid orientations and demonstrate that changing or dragging one group does not alter the other group's track options.
2. Exercise both splitter axes, repeated markers for a shared boundary, payload editing, repeated rebuilds, page switching and palette refresh. Require retained state, current bindings, visible unclipped content and normal shutdown. Check that nested spacing remains correct in both orientations and after each operation; a splitter occupies the existing gap rather than adding another one. Record expected geometry for each example rather than relying on visual inspection alone.
3. Extend the existing tests only where the reproduced defect or new integration behavior lacks coverage. Use `TestCompositions_EasyLayout.cpp` for direct geometry/group isolation and `TestControls_EasyLayout.cpp` for instantiated XML, event-driven rebuilding and retained controls. Keep direct rejection tests in `TestCompositions_EazyLayoutFailures.cpp` and compiler-only tests in `TestResource.cpp`; retain existing nested-`ez:Layout` tests because that remains a supported feature.
4. Run the required UnitTest coverage through `copilotExecute.ps1 -Mode UnitTest`, following `Running-UnitTest.md` and preserving relevant `.vcxproj.user` filters. Ensure the affected files actually execute, inspect the final case counts and appended leak report, and review generated snapshots for intended changes. Frame titles describe the state produced by the preceding action.
5. Extend `Test/UIA_CppTest_Shared.cs` with meaningful geometry and state assertions for the changed and new pages. The current rebuild check confirms retained text but does not prove the direction changed. Keep the entry wrappers synchronized and run both `Test/UIA_CppTest.ps1 -AsPort 8888` and `Test/UIA_CppTest_Metaonly.ps1 -AsPort 8890` sequentially against rebuilt Debug x64 binaries. Cover roles, names, available patterns, selection/toggle/invoke behavior and bounds in hosted and ordinary native modes.
6. Follow `Project.md` and `Tools/UiaList/Verification/README.md` to inspect both showcase pages with UiaList's preview and Nodes view. Prefer application automation geometry, then UIA; a locked desktop alone is not grounds to skip these checks. Record any unavailable focus/DPI coverage explicitly and check for blocking runtime-error dialogs after timeouts.

## REVIEW COMMENTS
