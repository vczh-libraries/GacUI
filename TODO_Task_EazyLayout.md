The goal of this task is to create a `<ez:Layout/>` element in GacUI XML Resource:
- `xmlns:ez` will becomes a predefined namespace, just like how the default namespace is defined, `ez` will be inserted.
  - The namespace will be `(vl::)presentation::compositions::eazy_layout::GuiEasy*(Composition|Layout)`.
- `ez:Layout` is the `GuiEasyLayoutComposition` class inheriting from `GuiBoundsComposition` with properties set in its constructors:
  - `MinSizeLimitation`: `LimitToElementAndChildren`.
  - `AlignmentToParent`: `0, 0, 0, 0`.

Specification:
- Here are a list of `ez:Layout` properties:
  - `Padding`: default to 5, it means the gap between all `GuiEasy*Layout` visually. If they are implemented in different nested levels of stacks and tables, properties of stacks and tables should be carefully adjusted.
  - `Border`: default to true, it means the gap between `ez:Layout` to its container. This could be implemented by `AlignmentToParent`, but there is no need to listen to the changing of `AlignmentToParent`.
    - Top level layouts should not have gap between them and `ez:Layout`.
- Here are a list of `ez:*` type which can be used inside `ez:Layout`, but they are not compositions.
  - `ez:Top`, `ez:Bottom`, `ez:Left`, `ez:Right`.
  - `ez:Row`, `ez:Column`, with `CellOption` set to `MinSize` by default, and `CellSpan` set to `1` by default.
  - `ez:Fill`, with `Percentage` property of `double` default to `1`, `Direction` property of a enum class set to `Inherited` chosen from Inherited/Horizontal/Vertical
  - These are in the `GuiEasy*Layout` category.
  - Layout could have children of layouts, one composition or control, but layouts could not mix with composition or control.
    - It could have a list of `Ptr<GuiEasyLayout>` and a `GuiGraphicsCompositions*`.
    - Layouts, compositions or controls as children, or other constraints, are syntax sugar that could be handled in an instance loader.
- Property binding:
  - Do not allow any binding on `CellOption` and `Percentage`, making them known constant in GacUI XML Resource during compiling.
- Usage:
  - When children is a composition or a control, it is implemented as a `<Bounds/>`. For `ez:Layout` itself it is already a `<Bounds/>`:
    - The composition or `BoundsComposition` of control will have `AlignmentToParent` set to `0, 0, 0, 0` automatically.
  - When children are layouts, mixing of these types could appear in the same level, others are not allowed:
    - 0..x fills, mixing `Direction` properties of optional `Inherited`, multiple `Horizontal` and `Vertical` (but these two cannot mix)
      - When there are only fills, we should go through `Direction` to decide the orientation.
      - This will be implemented using a table with only one row/column.
    - non zero of (0..x tops + 0..x bottoms) + 0..x fills
      - This will be implemented using a table with only one column, all tops/bottoms become first few rows or last few rows with `MinSize`, others become `Percentage`.
    - non zero of (0..x tops + 0..x bottoms)
      - Similar to the second category, but a empty row of `Percentage=1` will be automatically inserted.
      - Optimization: if only tops or bottoms appear, stack could be used to avoid the need of empty row, which is also more optimal than a table.
    - 0..x tops + 0..x bottoms + 0..x rows
      - Similar to the second category, but rows have their on `CellOption`.
    - Horizontal version of the last 3 categories above.
  - Columns not in a row directly only allow to have rows in it, vice versa.
    - Such structure forms a table, and perform the following verification. Assuming rows is the higher level one, the reverse version will be applied when columns is the higher level one:
      - Each row could have different columns, but at least there should be a column in any row with `CellSpan` set to 1 for any column position, and for any column position all columns with `CellSpan` should have the same `CellOption`, and when different, `MinSize` will be ignored.
      - `CellOption` will be ignored when `CellSpan` is not `1`.
- The generated code will calls `GuiEasyLayoutComposition::BuildLayout` to build the layout.
  - When such class is built from code, before calling `BuildLayout` nothing will happen.
  - When it is destroyed, all attached compositions should be deleted using `SafeDeleteComposition`.
    - This ensures that no matter the first `BuildLayout` is called or not, they are deleted safely.
  - `BuildLayout` will throw an exception if the rules in usage does not satisfy, but it doesn't need to be catched, as such scenario the app should just crash.

In `Release` repo.
- Make sure all `GacUI_Controls` tutorials accepts `/AsPort` for a automation service which runs only on debug profile. They can only be compiled on Windows so we could just use the windows http instead of minihttp.
- This allows you to know the original layout of them, remember them.
- Change all layouts to use `ez:Layout`, make sure the visible shape does not change.

## DETAILS

### Runtime representation and ownership

- Put the headers and implementation files defining all easy-layout types, including the root composition, layout descriptors, and direction enum, in `Source/GraphicsComposition/EazyLayout`. Add these files to the public composition includes and explicit project source inventories through the existing mechanisms.
- The existing composition pointer type is `vl::presentation::compositions::GuiGraphicsComposition*`, declared in `Source/Application/GraphicsCompositions/GuiGraphicsComposition.h`.
- Each descriptor has either layout children or one composition/control payload. Reject mixed content, multiple payloads, reused payload ownership, and cyclic/shared descriptor trees. Empty layouts are valid and must not require constructing a zero-row or zero-column table.
- A control payload contributes its existing `BoundsComposition`; preserve the control instance, names, bindings, events, and its own child hierarchy. Descriptors are configuration objects, not extra controls or compositions exposed as user content.
- `AlignmentToParent` is available on `GuiBoundsComposition` (`Source/Application/GraphicsCompositions/GuiGraphicsBoundsComposition.h`), not every composition. Bounds-derived payloads and control bounds can receive the automatic zero alignment. Parent-controlled compositions such as cells and stack items cannot be standalone payloads under a bounds wrapper: retain their required table/stack parent as the payload subtree, or reject the invalid direct use.
- Before building, the easy-layout owner must retain ownership of unattached payload roots. After successful attachment, ordinary composition/control-tree ownership takes over. Do not unconditionally call `SafeDeleteComposition` on remembered attached leaf pointers from the easy-layout destructor: those leaves may already have been destroyed by the enclosing tree's deletion traversal.
- The ownership precedent is `GuiResponsiveViewComposition::~GuiResponsiveViewComposition` in `Source/GraphicsComposition/GuiGraphicsResponsiveComposition.cpp`, which skips the attached current view. Check both `SafeDeleteCompositionInternal` in `Source/Application/GraphicsCompositions/GuiGraphicsComposition_Helpers.cpp` and `GuiControl::~GuiControl` in `Source/Application/Controls/GuiBasicControls.cpp`. Pending unattached roots still require safe deletion when no build has happened; attached payload references must not become a second deleting owner.
- Validate the complete descriptor tree before lowering it. Runtime rule violations fail immediately as requested; no recovery or defensive cleanup of a process-ending failed build is required.

### Arrangement, spacing, and values

- Treat `Padding` as a nonnegative distance. `Border=true` applies that distance once between the root layout and its parent; `Border=false` applies zero. The constructor's zero alignment is the initial state, and building applies the effective border setting. There is no extra inset between the root and its top-level generated layout.
- Generated tables normally use `BorderVisible=false`, and generated stacks have zero `ExtraMargin`. Nested implementation containers must not multiply visible gutters. A separately declared nested `ez:Layout` has its own padding/border settings; choose them explicitly when preserving existing margins.
- The synthetic percentage spacer in a no-fill docking group must not introduce a second minimum gutter. For example, Top plus Bottom should have one `Padding` gap at minimum size, with the remaining free space opening between them when enlarged. Compare the stack optimization against this same geometry.
- Partition vertical groups into Top, a middle Fill or Row group (without mixing Fill and Row), and Bottom groups; use the corresponding Left, middle Fill or Column, and Right groups horizontally, without mixing Fill and Column. Preserve declaration order within each group in top-to-bottom or left-to-right visual order. Compensate for reversed stack insertion when optimizing Bottom/Right groups so the optimization does not reverse their visible order.
- In a fills-only sibling group, use the single explicit direction if present; otherwise inherit the nearest enclosing resolved arrangement axis, falling back to Horizontal at the root, consistent with the existing stack default. Docking or Row/Column sibling groups establish their own axis; reject an explicitly conflicting Fill direction. The vertical and horizontal forms must be transposes of the same rules.
- Fill percentages are relative weights, not values that must sum to 1 or 100. Reject non-finite/nonpositive weights, negative absolute sizes, and nonpositive spans. Account for `GuiTableComposition::SetRowOption`/`SetColumnOption` in `Source/GraphicsComposition/GuiGraphicsTableComposition.cpp`: percentage values below `0.001` are currently coerced to zero, while measurement divides by percentage weights. An accepted positive weight must not silently become an unusable zero; normalize safely or reject unsupported magnitudes explicitly. Apply the same policy to percentage `CellOption` values.
- For a row-major grid, determine each inner Column's starting column by summing preceding inner spans. The number of columns is the maximum occupied width; shorter rows leave trailing cells empty. Transpose this rule for a column-major grid. Every track still needs at least one single-span declaration somewhere, as required by the original specification.
- Infer each shared track option from single-span declarations only. All-MinSize declarations yield MinSize; otherwise ignore MinSize and require all remaining declarations to agree on the option kind and its relevant numeric value. Ignore the option of a spanning cell. The meaning of a span on an outer grouping Row/Column remains a review item below.

### XML compiler and reflection integration

- Namespace mappings use semicolon-separated wildcard patterns, not regular-expression alternatives. The predefined value is `presentation::compositions::eazy_layout::GuiEasy*Composition;presentation::compositions::eazy_layout::GuiEasy*Layout`. Keep the requested `eazy_layout` spelling.
- Add the implicit `ez` mapping independently of the default namespace, and only when the instance does not explicitly declare `xmlns:ez`. Follow `GuiInstanceContext::LoadFromXml` in `Source/Compiler/GuiInstanceRepresentation.cpp`; preserve explicit namespace overrides and serialization behavior.
- Add dedicated instance-loader handling for descriptor child collections and composition/control payloads. Do not pass descriptors through the ordinary composition-child attachment path. Reuse the reflection and Workflow AST generation mechanisms rather than generating source text.
- Reject every binding form on `CellOption` and `Percentage`, including one-time `-eval`, `-ref`, and `-uri`, in attribute and property-element forms. Merely setting `GuiInstancePropertyInfo::NotBindable` is insufficient today: `Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowCollectReferences.cpp` enforces that flag only for constructor arguments.
- Validate that `CellOption` fields are compile-time constants as well. Non-binding struct assignments normally accept Workflow expressions in their fields (`Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowParser.cpp`), so the absence of a binding suffix alone does not establish this requirement. Reject runtime-dependent expressions with source-position diagnostics.
- Emit `BuildLayout` after the descriptor tree, payloads, and all effective initial property values have been assigned, including empty layouts. Ordinary assignment and binding installation are separate phases in `Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.cpp`; building during child attachment can happen too early. Honor the eventual mutation/binding policy from the review comment below.
- Register the real composition in `GUIREFLECTIONCOMPOSITION_CLASS_TYPELIST` and descriptor classes/the direction enum in the appropriate extra type list in `Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h`, with registrations in `GuiReflectionCompositions.cpp`. `Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` requires every composition class-list entry to derive from `GuiGraphicsComposition`.
- Update public composition includes, explicit project source inventories, loader registration, and generated metadata through the repository's existing mechanisms. Descriptor construction and `BuildLayout` must work in ordinary C++ without reflection; XML generation must also support metadata-only and full-reflection configurations.

### Release tutorial migration

- `Release` here means the sibling repository `../Release`, not generated files under this repository's `Release` directory. Follow that repository's instructions when executing this part. The inventory is `../Release/Tutorial/GacUI_Controls/GacUI_Controls.sln`.
- Add and verify automation before capturing migration baselines. Runnable tutorials share `../Release/Tutorial/Lib/GacUILite/WinMain.cpp`; changes there affect other tutorial groups too. Follow the ordinary Windows automation-service lifecycle demonstrated by `Test/GacUISrc/CppTest_Reflection/Main.cpp` and the automation documentation.
- Debug applications accept `/AsPort:<decimal port>` in 1..65535, defaulting to 8888, with malformed/duplicate values rejected before listening. Use independently selected ports and identifiable application endpoint names. Release builds must not start an automation listener.
- Update GacUI release outputs, compiler tools, and architecture metadata through the established release pipeline before generating tutorial code. Regenerate resources through `../Release/Tools/GacBuild.ps1` and the existing `../Release/Tutorial/GacUI.xml` dependency driver; generated `UI/Source` files are outputs, not the migration source.
- `DocumentEditor` is a shared library/resource, so verify it through both `DocumentEditorToolstrip` and `DocumentEditorRibbon`. Preserve authored XML, script references, control names, template behavior, and relevant composition events when changing arrangements.
- Preserve existing spacing explicitly: examples include padding 10 in ColorPicker, flow padding 2 and inner margin 1 in ColorPicker2, and content insets of 3 in AddressBook. Applying the default 5 everywhere will change the tutorials' appearance.

## VERIFICATION

These are execution requirements for the future implementation; this review changes only the task document.

Use direct C++ composition unit tests for the layout implementation itself, and the remote protocol based GacUI unit test framework for GacUI XML Compiler integration. Keep these responsibilities explicit in the test coverage.

### Layout and ownership tests

- Add direct C++ tests following `Test/GacUISrc/UnitTest/TestCompositions_*.cpp`, particularly `TestCompositions_Table.cpp`, `TestCompositions_Stack.cpp`, and `TestCompositions_AllNested.cpp`. Construct descriptors and payloads directly, call `BuildLayout`, calculate layout using the existing composition-test pattern, and compare measured minimum sizes and final child rectangles against independently specified expected results. Do not use the remote protocol based unit test framework or XML compilation to verify the layout algorithm itself.
- Provide a coverage matrix and enough cases to exercise every valid and invalid sibling-type combination and parent/child category in the specification. Include zero, one, and multiple occurrences where applicable, declaration-order variations, both axes, nested combinations, payload kinds, and property boundary values. Assert expected geometry for valid cases and the expected failure for invalid cases; a few representative happy paths or snapshot-only checks are insufficient.
- Cover empty and single-payload roots; every permitted vertical group and its horizontal transpose; Top-only/Bottom-only/Left-only/Right-only stack optimizations; interleaved declarations; unequal Fill weights; and minimum-size versus enlarged-parent behavior.
- Verify explicit/inherited directions through multiple nesting levels, root all-Inherited defaults, and conflicting directions. Check default/zero/nondefault padding, both Border settings, nested containers, and the synthetic spacer's minimum gutter.
- Cover row-major and column-major grids, MinSize/Absolute/Percentage option merging, inner spans, ragged trailing cells, missing single-span declarations, and conflicting non-MinSize options. Add the outer-span and mixed-axis cases after their contract is settled below.
- Reject mixed layout/payload content, multiple payloads, illegal sibling combinations, invalid parent-controlled payloads, invalid spans/numeric values, and shared/cyclic ownership. Include very small positive percentage values so native table coercion cannot hide a zero-weight error.
- Verify destruction counters and Debug leak output for unbuilt trees, built trees destroyed through an ordinary composition parent, and trees destroyed through an owning control. Include nested control payloads and descriptors that outlive the built owner; no double deletion, leaked pending payload, or access to a destroyed attached payload is acceptable.
- Test repeated builds and changes after the first build according to the chosen lifecycle contract, preserving control identity and state wherever rebuilding is supported. Normal parent resizing and payload minimum-size changes must continue to update geometry through the existing layout engine.

### XML and generated-code tests

- Use the remote protocol based GacUI unit test framework for GacUI XML Compiler tests. For valid resources, follow the resource-compilation and frame-assertion pattern documented in `.github/KnowledgeBase/manual/unittest/gacui.md`; verify that compiled XML constructs and initializes the intended layout and payload objects. Keep exhaustive layout-algorithm verification in the direct composition tests above.
- Test implicit `ez`, an explicit `xmlns:ez`, a custom default namespace, and namespace serialization round-trips. Instantiate every descriptor type with defaults and explicit constant values using both attributes and `<att.*>` forms.
- Add negative XML cases in the remote protocol based framework for prohibited bindings, runtime-dependent `CellOption` fields, and statically detectable invalid child structures. Run compilation in the framework's test context and retain the collected compiler errors to assert rejection and useful source-position diagnostics before attempting to instantiate a window; do not pass expected failures through a convenience helper that treats all compilation errors as unexpected fatal failures.
- Verify automatic payload alignment and named-object/event/binding preservation. Check that generated initialization calls `BuildLayout` after all required initial values are ready, including empty and nested layouts and any permitted bound properties.
- Exercise Workflow-loaded resources with full reflection, and generated C++ in the supported full, metadata-only, and no-reflection configurations. Follow `Project.md`: build Debug Win32 and run `Metadata_Generate`; build Debug x64, run `Metadata_Generate`, then `Metadata_Test`. Run required code generation and the relevant UnitTest coverage with the prescribed build/execute wrappers, respecting the existing test filter. Inspect final logs and leak reports; ensure no new `*.UI.errors.txt` remains.

### Tutorial equivalence and automation

- Before migration, capture durable visual images and automation geometry for every runnable GacUI_Controls tutorial. Record the revision, renderer, skin, DPI, font settings, client size, and interaction sequence so the same states can be reproduced after migration.
- Compare rendered appearance and control/element geometry at the original size and at smaller/larger sizes. Allow generated composition topology and transient IDs to differ; require the same visible spacing, alignment, content, clipping, and useful resize behavior.
- Exercise all tabs and relevant popups/dialogs, list/grid item templates, editor variants, localization and animation states present in each tutorial. Verify interaction after migration, including splitters and repeated/wrapping content; an unchanged initial screenshot is insufficient.
- Specifically verify CalculatorAndStateMachine's display spanning four columns and its plus button spanning two rows in the same table, as well as AddressBook's draggable splitter and ColorPicker2's repeated shared-size palette.
- Verify Debug automation on the default and selected ports, invalid argument rejection, simultaneous applications on distinct ports, and clean shutdown. Verify Release applications run without an automation listener. Build the tutorial solution and regenerate/check both x86 and x64 resource outputs through its documented workflow.

## REVIEW COMMENTS

### Build lifecycle and mutable properties

**review comment**: Only `CellOption` and `Percentage` forbid bindings, so `Padding`, `Border`, `Direction`, and `CellSpan` can apparently change after generated initialization. It is not specified whether these changes update the layout immediately, wait for another `BuildLayout`, or are invalid. Rebuilding may replace tables/stacks, and a second build must not delete live controls or attach a payload twice. The constructor/build distinction alone does not settle this observable API behavior.

**suggested solution**: Use a fixed descriptor structure after the first successful build, with `Padding` and `Border` remaining live and updating generated spacing/insets. Treat `Direction` and `CellSpan` as construction-time values and reject ongoing bindings/mutations for them once built; make an unchanged repeated `BuildLayout` a no-op. This extends the original binding restriction and therefore needs an explicit decision. If live structural changes are intended instead, specify how rebuilding preserves payload identity, control state, ownership, and initialization order.

Everything will only take effect when the build function is called, changing those properties on the fly does nothing.

### Outer spans and cells spanning both axes

**review comment**: Inner Column spans in a row-major grid naturally mean column spans, but both Row and Column expose `CellSpan`. A span on an outer Row has no defined placement rule: it could span every cell beneath it across subsequent rows, reserve extra tracks, or be disallowed. Without coordinates or an occupancy-placement rule, later rows may overlap it. The existing `../Release/Tutorial/GacUI_Controls/CalculatorAndStateMachine/UI/Resource.xml` needs a four-column display span and a two-row plus-button span in one table; choosing only one grid orientation does not establish how to represent both.

**suggested solution**: For the initial grammar, require outer grouping spans to be 1 and let only inner descriptors span their group's axis. Preserve tables needing spans on both axes as a single composition payload during migration. If these tables must also be expressed entirely with easy-layout descriptors, define cell placement, both-axis spans, overlap detection, and which single-span declarations determine each track before implementing the grid builder.

No worry about this, just set the range to GuiCellComposition, overlapping will be handled by GuiTableComposition, that is, if anything wrong the cell won't show. No extra check about this is necessary.

### Specialized tutorial layouts versus all-layout migration

**review comment**: `../Release/Tutorial/GacUI_Controls/AddressBook/UI/Resource.xml` contains an interactive `ColumnSplitter`, and `ColorPicker2/UI/Resource.xml` uses `SharedSizeRoot`, `RepeatFlow`, and `SharedSizeItem` for dynamically repeated, wrapping, equally sized palette entries. The proposed descriptor grammar has neither splitter access to its generated table nor repeated-flow/shared-size equivalents. Replacing every existing layout solely with these descriptors cannot preserve those behaviors.

**suggested solution**: Convert ordinary arrangements to `ez:Layout` while retaining necessary specialized composition subtrees as the permitted single-composition payload, and document each exception. If "all layouts" requires eliminating these specialized subtrees too, extend the specification with their missing behavior before starting tutorial conversion.

Since ez:Layout only covers partial of features, for apps that uses above compositions that cannot not be rewritten, you only rewrite part of the app where doable.
