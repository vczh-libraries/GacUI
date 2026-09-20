# Eazy Layout splitters and tutorial refactoring

## Scope and status

This is a plan for future implementation. This request only creates this plan and [EazyLayout.md](EazyLayout.md); it does not implement splitters or change tutorial XML.

Extend the layout contract in [TODO_Task_EazyLayout.md](TODO_Task_EazyLayout.md) with `<ez:Splitter/>`, then continue the `GacUI_Controls` work described in [the EazyLayout section of TODO_Task_Refactor.md](TODO_Task_Refactor.md#eazylayout). Preserve existing appearance, interaction, and the simplification of redundant properties. The UI Automation refactoring in that task is outside this plan.

The tutorial solution is in the sibling repository at `../Release/Tutorial/GacUI_Controls/GacUI_Controls.sln`. It is not the generated `Release` directory inside this repository.

## Proposed XML contract

`ez:Splitter` is a leaf layout descriptor. It represents a draggable boundary in the table built for its containing layout group. It has no children or composition/control payload, and no `Direction`, `Percentage`, `CellOption`, `CellSpan`, or explicit row/column index property.

### Direction and attachment

- Infer the group's direction using the existing rules, ignoring splitter markers: Top/Bottom/Row establish Vertical, Left/Right/Column establish Horizontal, and fills-only groups use an explicit Fill direction, inherited direction, or the root Horizontal fallback.
- A vertical group creates a native RowSplitter; a horizontal group creates a native ColumnSplitter. The direction describes the arrangement axis: a RowSplitter is a horizontal bar dragged vertically.
- Associate a marker with the immediately preceding non-splitter descriptor in its authored sibling list. Keep that association when the builder partitions children into leading, middle, and trailing groups. The marker's position in the XML is not a native table index.
- Place the splitter after the associated generated row or column. For an outer Row/Column, this is the boundary after that descriptor's starting track; its `CellSpan` changes cell coverage, not how many outer tracks the descriptor declares. For an inner Column/Row, the boundary follows its accumulated span.
- Splitters do not consume tracks, cells, spans, percentage weight, or an extra padding gap. Empty ordinary descriptors still declare their usual tracks.
- A group containing a splitter must use a table even when the existing optimization would use a stack. This affects that group only. Preserve the original edge anchoring, visual order, minimum size, and free-space behavior.

Resolve markers before lowering compositions. After grouping and choosing the table representation below, convert the target boundary to `RowsToTheTop` or `ColumnsToTheLeft` on the generated native splitter.

### Shared grids

Allow markers among outer Rows/Columns and among their opposite-axis inner descriptors. For rows containing columns, an outer marker selects a row boundary and an inner marker selects a shared column boundary. Transpose this rule for columns containing rows.

An inner marker belongs to the shared grid table, not a new table inside one cell. Like a native splitter, it extends across the table's perpendicular cell area. It does not affect shared `CellOption` inference or the requirement for a single-span declaration at every shared track.

Multiple distinct markers selecting the same boundary create one native splitter, keyed by owning table, axis, and boundary. Usually these declarations appear in different rows/columns, but coalescing also applies if unusual cell spans produce the same boundary within one list. This lets a shared boundary be declared once, like its shared option. Reusing the same descriptor object in multiple places remains invalid under the existing ownership rules.

### Invalid placements

Reject a marker with no preceding descriptor, consecutive markers in the same sibling list, a marker-only group, and any marker with children or payload. Reject a resolved boundary unless it is strictly inside the generated table: `0 < boundary < trackCount`.

A trailing marker is therefore valid only if the preceding descriptor has a following generated track after grouping. It is invalid after the final generated track. The marked docking representations below do not add a synthetic track to make an otherwise invalid end marker valid. Do not silently retarget an invalid marker to another boundary.

Continue to delegate invalid cell ranges to the native table. A bad cell span does not itself become a new easy-layout range error; independently validate whether a marker resolves to an interior boundary. Do not repair overlapping cells, move unsited cells, or change their existing rejection behavior.

Report statically detectable grammar errors through the XML compiler with source positions. Validate resolved geometry and mutable configuration in `BuildLayout`, including equivalent trees constructed from C++.

### Spacing and dragging

Use the existing padding gap as the native splitter's hit area. `Padding="0"` remains legal and gives a zero-thickness splitter, matching native behavior. A usable mouse splitter needs positive padding; do not silently increase the layout's padding.

Preserve native resizing behavior: only immediately adjacent tracks with an `Absolute` option change when dragged. At least one adjacent Absolute track is needed for resizing. MinSize/Percentage-only boundaries remain inert; a splitter does not turn those options into Absolute. A boundary can resize the Absolute track on either side even though its placement is attached to the preceding descriptor.

This is sufficient for AddressBook's existing Absolute/Percentage boundary and avoids changing ordinary table behavior. Support the requested table conversion for one-sided docking groups as well, while documenting and testing the same native sizing limitation there.

For groups with splitters, use native table padding equal to the root's `Padding`, with `BorderVisible=false`. Preserve the no-fill docking geometry using these representations:

- For Top-only, Bottom-only, Left-only, or Right-only groups, use a table sized to its content along the arrangement axis and aligned to the corresponding edge. Stretch it across the perpendicular axis. Keep the tracks MinSize; do not add an empty spacer track.
- For mixed Top/Bottom or Left/Right groups without fills or explicit grid tracks, omit the old synthetic spacer. Let the first trailing generated track use Percentage to absorb unused space. Keep that track's content at its minimum size along the arrangement axis, aligned to Bottom/Right, and stretched across the perpendicular axis. All other docking tracks remain MinSize. This changes only generated containers, not descriptor values.
- Leave groups with fills or explicit grid tracks on their ordinary table sizing path. Leave groups without markers on their existing implementation path.

The mixed-group representation has one native gutter per neighboring pair and opens unused space before the trailing content. In particular, Top + Bottom has one minimum gap instead of the two gaps produced by Top + empty spacer + Bottom with positive native padding. Its splitter occupies the gutter immediately after the leading track. Remove the old per-child padding compensation in this representation to avoid adding the same gap again. Verify the geometry at minimum and enlarged sizes on both axes.

### Build lifecycle

The descriptor tree and its properties still take effect only at `BuildLayout`. Native mouse dragging updates the generated table immediately through its existing event handlers; it does not rewrite descriptor `CellOption` values.

An explicit rebuild regenerates from the stored descriptors, so it resets dragged sizes to their configured values. Ordinary window resizing preserves the current generated table's dragged sizes. Preserve reused payload/control identity, state, bindings, and handlers across a rebuild, and delete obsolete generated splitters with their table. Do not retain pointers to a discarded native splitter.

## Example: AddressBook's two panes

The intended shape of the replacement is:

```xml
<ez:Layout>
  <ez:Column CellOption="composeType:Absolute absolute:180">
    <ez:Row CellOption="composeType:Percentage percentage:1">
      <GroupBox Text="Categories">
        <!-- Keep the existing contents, including the 3-pixel inset. -->
      </GroupBox>
    </ez:Row>
  </ez:Column>
  <ez:Splitter/>
  <ez:Column CellOption="composeType:Percentage percentage:1">
    <ez:Row>
      <GroupBox Text="Contacts">
        <!-- Keep the existing contents, commands, and bindings. -->
      </GroupBox>
    </ez:Row>
  </ez:Column>
</ez:Layout>
```

The two outer Columns establish Horizontal, so the splitter needs no direction. The first shared Row supplies its percentage option for both columns. Default padding and border reproduce the original table's 5-pixel gap and outer inset. Retain the 180-pixel absolute column option so dragging continues to work.

## Implementation steps

### 1. Runtime descriptor and layout building

Extend [Source/GraphicsComposition/EazyLayout/GuiEasyLayout.h](Source/GraphicsComposition/EazyLayout/GuiEasyLayout.h) and [GuiEasyLayout.cpp](Source/GraphicsComposition/EazyLayout/GuiEasyLayout.cpp):

- Add `GuiEasySplitterLayout` in `vl::presentation::compositions::eazy_layout`, derived from `GuiEasyLayout`, and add its descriptor kind.
- Extend `GuiEasyLayoutBuilder::Prepare` and its plan to store marker-to-descriptor associations and resolved boundaries separately from track options, cell sites, and payloads.
- Include markers in descriptor registration, lifetime, cycle, reuse, and owner validation. Enforce the leaf rule even though the base class exposes child/payload setters.
- Resolve both ordinary group boundaries and shared-grid boundaries without changing authored track counts or span placement. Generated spacer/container details may change as specified above. Coalesce duplicate boundaries.
- Disable the stack optimization only for groups requiring splitters. Implement the edge-aligned and trailing-content table representations above, preserving Bottom/Right ordering and padding.
- Build native splitters as direct children of their owning table, after its dimensions/options and cell placement are established. Keep their normal cursor, mouse capture, size limits, and invalidation behavior.
- Preserve unchanged layouts without markers and the existing rebuild/ownership rules.

Reuse `GuiRowSplitterComposition` and `GuiColumnSplitterComposition`, declared in [Source/GraphicsComposition/GuiGraphicsTableComposition.h](Source/GraphicsComposition/GuiGraphicsTableComposition.h). Their common behavior is implemented in [GuiGraphicsTableComposition.cpp](Source/GraphicsComposition/GuiGraphicsTableComposition.cpp). Avoid a second drag algorithm.

### 2. XML compilation and reflection

- Extend `EasyKind`, type classification, and `VisitEasyLayouts` in [Source/Compiler/InstanceLoaders/GuiInstanceLoader_EasyLayout.cpp](Source/Compiler/InstanceLoaders/GuiInstanceLoader_EasyLayout.cpp). Permit markers at the appropriate descriptor-list levels, including outer grid descriptors, while retaining all existing mixed-content and axis checks.
- Keep the compiler's grammar checks consistent with C++ validation. Do not count markers as ordinary payloads or tracks, and preserve diagnostics for both default content and explicit property-element content forms.
- Register the new descriptor in the extra reflection type list in [Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h](Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h) and in [GuiReflectionCompositions.cpp](Source/Reflection/TypeDescriptors/GuiReflectionCompositions.cpp). It must not enter the composition-only type list.
- Reuse the predefined `ez` wildcard mapping and base descriptor loader. Preserve explicit namespace overrides and the final initialization order of `BuildLayout`.
- Keep existing constant-property restrictions unchanged. Verify descriptor construction in ordinary C++, Workflow-loaded XML, and generated C++ with full, metadata-only, and no reflection.
- Follow [Project.md](Project.md) for required reflection metadata generation and tests. Update explicit source inventories only if implementation actually introduces files.

### 3. Focused verification

The earlier refactor has already created `TestCompositions_EazyLayoutFailures.cpp` and moved compiler-only cases into `TestResource.cpp`. Extend those locations; do not repeat the relocation work from the old task.

| Coverage | Location and expected result |
| --- | --- |
| Layout geometry | `Test/GacUISrc/UnitTest/TestCompositions_EasyLayout.cpp`: both axes; explicit/inherited/fallback direction; reordering; empty descriptors; multiple boundaries; inner spans; outer spans; shared grids and duplicate boundaries. Assert native splitter type, table parent, boundary index, hit rectangle, and independent expected payload rectangles. |
| Forced tables and spacing | Direct composition tests: Top-only, Bottom-only, Left-only, Right-only, and mixed leading/trailing groups. Compare minimum and enlarged sizes, declaration order, zero/default/nondefault padding, borders, and nested groups. Verify one visible gutter and correct free-space placement. |
| Dragging and rebuilds | Reuse the native splitter event-testing pattern in `TestCompositions_Table.cpp`. Cover Absolute/Percentage, Percentage/Absolute, Absolute/Absolute, and inert pairs; both drag directions and limits; repeated drags; parent resize; explicit rebuild resetting sizes; payload identity and lifetime. |
| Runtime failures | `Test/GacUISrc/UnitTest/TestCompositions_EazyLayoutFailures.cpp`: marker-only/first/consecutive markers, invalid resolved boundaries, payload/children, cycles, reused descriptors, and existing invalid sibling combinations with markers inserted. No remote UI frames for expected rejection. |
| Compiler-only failures | `Test/GacUISrc/UnitTest/TestResource.cpp`: invalid marker content/placement that is statically knowable, diagnostic source positions, property-element forms, and namespace handling. Preserve native cell-site semantics. |
| Visible XML integration | `Test/GacUISrc/UnitTest/TestControls_EasyLayout.cpp`: compile and instantiate both axes, named marker descriptors, nested layouts, shared grids, and the AddressBook-shaped layout; use frames to verify visible resizing and splitter interaction. |

Retain existing no-splitter regression coverage, omission/default behavior, option conflicts, binding timing, and cell-site rejection. Verify destruction without a build, after a build, and after adding/removing splitters and rebuilding; inspect the Debug leak report.

Build and execute through `.github/Scripts/copilotBuild.ps1` and `copilotExecute.ps1`, following the repository guidelines and existing test filters. Ensure all affected tests actually run. Run the required Debug Win32 `Metadata_Generate`, then Debug x64 `Metadata_Generate` and `Metadata_Test` steps after reflection changes. Regenerate owned outputs through the documented tools; never edit protected generated files by hand.

### 4. Publish dependencies and continue tutorial refactoring

Before changing XML, re-read instructions in the sibling Release repository and capture baseline screenshots and HTTP automation bounds at fixed DPI, skin, client size, and application state. Record the source revisions and generated dependency versions. The existing Debug automation startup supports `/AsPort`; this work does not need another automation service.

The current authored XML inventory is:

| Resource | Planned action |
| --- | --- |
| `../Release/Tutorial/GacUI_Controls/AddressBook/UI/Resource.xml` | Replace the remaining two-column Table and ColumnSplitter with the shape above. Preserve Categories/Contacts content, 5-pixel outer/gap spacing, 3-pixel inner insets, inner `Border="false"`, the absolute 180 column, and every command/binding/event. |
| `../Release/Tutorial/GacUI_Controls/CalculatorAndStateMachine/UI/Resource.xml` | Keep the native table, as required by the prior refactor. It has both-axis spans and a `//Cell/Button` style selector; splitter support does not remove those migration constraints. |
| Other controls resources and their linked XML | Re-audit for new splitter tables or redundant `ez:*` properties introduced since this inventory. Convert only equivalent arrangements and retain required specialized subtrees. |

At planning time, AddressBook is the only remaining authored splitter table in the controls tutorials. No RowSplitter was found. CalculatorAndStateMachine is the only other native Table. Toolstrip and ribbon splitters are unrelated controls and remain unchanged. Keep ColorPicker2's repeated/wrapping shared-size palette.

Follow resource links outside `GacUI_Controls` as well. DocumentEditor is shared by DocumentEditorToolstrip and DocumentEditorRibbon; test both if those resources change. The linked BlackSkin FullControlTest resources are copied from this repository during release publication, so respect their upstream ownership instead of editing a copy that will be overwritten.

Publish the changed runtime, reflection metadata, and GacGen through the existing Tools release workflow before compiling tutorials. Tutorial GacUILite consumes the sibling Release repository's imported GacUI sources. Regenerate with its `Tools/GacBuild.ps1` and `Tutorial/GacUI.xml` driver, using absolute script paths. Ensure affected resources rebuild after compiler changes even when XML timestamps alone would reuse cached results. Inspect both architecture outputs, logs, and error artifacts; reaching the end of the driver is not proof of success.

Build `GacUI_Controls.sln` in Debug Win32 and x64 through the documented wrapper, and start every affected runnable application through the execution wrapper. Continue omitting defaults, inferred directions, and duplicate shared options; preserve structural empty descriptors, spans, and independent outer track options.

### 5. Tutorial acceptance and documentation

- Compare AddressBook at its initial 640 by 480 size, smaller/larger sizes, maximized, and restored. Check pane bounds, minimum sizes, clipping, padding, and the full splitter hit area against the baseline.
- Drag in both directions, to both limits, and repeatedly; resize after dragging. Verify category selection, all contact view modes, toolbar actions, and new/edit dialogs still behave correctly.
- Use HTTP automation and screenshots first, followed by the prescribed UIA checks. The current default AddressBook control endpoint is `/Automation/AddressBook.exe/Controls` on its selected automation port.
- For any additional affected resources, exercise their tabs, dialogs, popups, repeated content, and all runnable consumers. Record retained native arrangements and why they remain.
- Inspect generated diffs, resource diagnostics, test results, snapshots, and leak reports. Generate snapshots and generated C++ through their owning tools/tests.
- Update [EazyLayout.md](EazyLayout.md) when the implementation lands: move Splitter from planned to supported, document its native sizing requirements and rebuild behavior, and include a minimal working example.
- Commit and push the completed implementation and tutorial changes in their owning repositories, keeping unrelated concurrent work out of each commit.

## Completion criteria for the future work

`ez:Splitter` compiles and builds in both axes, attaches to the specified predecessor after layout ordering, works in shared grids, and forces local table generation without changing spacing or anchoring. Native dragging, invalid placement, and rebuild behavior match the contract above. AddressBook uses the new syntax and matches its prior visible layout and interaction. Other remaining specialized layouts are retained deliberately, and all required unit, metadata, generated-resource, and affected-application checks pass.
