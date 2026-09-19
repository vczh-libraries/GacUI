# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

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
- All easy-layout configuration is applied only by an explicit `BuildLayout` call. Setters and permitted XML bindings update stored configuration without changing generated geometry or rebuilding automatically. This includes `Padding`, `Border`, `Direction`, `CellSpan`, and C++ changes to `CellOption`/`Percentage`; the existing XML binding prohibition on the latter two remains in force. Ordinary parent resizing and payload minimum-size changes still use the previously built composition tree normally.
- Each `BuildLayout` uses the current descriptor tree and stored property values, including on subsequent calls. Rebuilding must preserve reused payload/control instances, bindings, handlers, and control state. Detach retained payload roots before deleting obsolete generated containers; do not duplicate attachments or delete payloads that the next built tree still uses. An unchanged repeated build must preserve the same observable layout and ownership.
- Validate the easy-layout grammar and shared-track option rules before lowering the descriptor tree. Cell placement/range conflicts are delegated to the existing table behavior described below. Other runtime rule violations fail immediately as requested; no recovery or defensive cleanup of a process-ending failed build is required.

### Arrangement, spacing, and values

- Treat `Padding` as a nonnegative distance. `Border=true` applies that distance once between the root layout and its parent; `Border=false` applies zero. The constructor's zero alignment is the initial state, and building applies the effective border setting. There is no extra inset between the root and its top-level generated layout.
- Generated tables normally use `BorderVisible=false`, and generated stacks have zero `ExtraMargin`. Nested implementation containers must not multiply visible gutters. A separately declared nested `ez:Layout` has its own padding/border settings; choose them explicitly when preserving existing margins.
- The synthetic percentage spacer in a no-fill docking group must not introduce a second minimum gutter. For example, Top plus Bottom should have one `Padding` gap at minimum size, with the remaining free space opening between them when enlarged. Compare the stack optimization against this same geometry.
- Partition vertical groups into Top, a middle Fill or Row group (without mixing Fill and Row), and Bottom groups; use the corresponding Left, middle Fill or Column, and Right groups horizontally, without mixing Fill and Column. Preserve declaration order within each group in top-to-bottom or left-to-right visual order. Compensate for reversed stack insertion when optimizing Bottom/Right groups so the optimization does not reverse their visible order.
- In a fills-only sibling group, use the single explicit direction if present; otherwise inherit the nearest enclosing resolved arrangement axis, falling back to Horizontal at the root, consistent with the existing stack default. Docking or Row/Column sibling groups establish their own axis; reject an explicitly conflicting Fill direction. The vertical and horizontal forms must be transposes of the same rules.
- Fill percentages are relative weights, not values that must sum to 1 or 100. Reject non-finite/nonpositive weights and negative absolute sizes. Account for `GuiTableComposition::SetRowOption`/`SetColumnOption` in `Source/GraphicsComposition/GuiGraphicsTableComposition.cpp`: percentage values below `0.001` are currently coerced to zero, while measurement divides by percentage weights. An accepted positive weight must not silently become an unusable zero; normalize safely or reject unsupported magnitudes explicitly. Apply the same policy to percentage `CellOption` values. Span/range validity is delegated to the table as described below.
- For a row-major grid, determine each inner Column's starting column by summing preceding inner spans. The number of columns is the maximum occupied width; shorter rows leave trailing cells empty. Transpose this rule for a column-major grid. Every track still needs at least one single-span declaration somewhere, as required by the original specification.
- Infer each shared track option from single-span declarations only. All-MinSize declarations yield MinSize; otherwise ignore MinSize and require all remaining declarations to agree on the option kind and its relevant numeric value. Ignore the option of a spanning cell.
- In a row-major grid, each outer Row supplies its starting row index and `rowSpan=CellSpan`; each inner Column supplies its accumulated starting column and `columnSpan=CellSpan`. Outer Rows start at consecutive row indices in declaration order, with the appropriate offset for leading Top rows; an outer span does not skip later row declarations. Transpose this mapping for a column-major grid. Pass the resulting ranges to `GuiCellComposition` in declaration order without occupancy-based relocation or overlap repair.
- Delegate overlapping, out-of-range, and otherwise invalid cell sites to `GuiCellComposition::SetSite` and `GuiTableComposition` in `Source/GraphicsComposition/GuiGraphicsTableComposition.cpp`. Do not add easy-layout runtime checks or XML compiler errors for those range conflicts, and do not turn native site rejection into an exception. Use fresh generated cells for each build so a rejected range remains unsited with empty bounds; failed `SetSite` on an already-sited cell can otherwise retain its old placement. The existing easy-layout grammar and single-span option-inference rules still apply.

### XML compiler and reflection integration

- Namespace mappings use semicolon-separated wildcard patterns, not regular-expression alternatives. The predefined value is `presentation::compositions::eazy_layout::GuiEasy*Composition;presentation::compositions::eazy_layout::GuiEasy*Layout`. Keep the requested `eazy_layout` spelling.
- Add the implicit `ez` mapping independently of the default namespace, and only when the instance does not explicitly declare `xmlns:ez`. Follow `GuiInstanceContext::LoadFromXml` in `Source/Compiler/GuiInstanceRepresentation.cpp`; preserve explicit namespace overrides and serialization behavior.
- Add dedicated instance-loader handling for descriptor child collections and composition/control payloads. Do not pass descriptors through the ordinary composition-child attachment path. Reuse the reflection and Workflow AST generation mechanisms rather than generating source text.
- Reject every binding form on `CellOption` and `Percentage`, including one-time `-eval`, `-ref`, and `-uri`, in attribute and property-element forms. Merely setting `GuiInstancePropertyInfo::NotBindable` is insufficient today: `Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowCollectReferences.cpp` enforces that flag only for constructor arguments.
- Validate that `CellOption` fields are compile-time constants as well. Non-binding struct assignments normally accept Workflow expressions in their fields (`Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowParser.cpp`), so the absence of a binding suffix alone does not establish this requirement. Reject runtime-dependent expressions with source-position diagnostics.
- Emit the initial `BuildLayout` after the descriptor tree, payloads, ordinary assignments, and initial evaluation of permitted bindings are ready, including empty layouts. Ordinary assignment and binding installation are separate phases in `Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.cpp`; building during child attachment can happen too early. Keep bindings on `Padding`, `Border`, `Direction`, and `CellSpan` available: later binding updates only change stored values, and application code must explicitly call `BuildLayout` to apply them. Validate runtime-dependent configuration at that call rather than assuming its compile-time value.
- Register the real composition in `GUIREFLECTIONCOMPOSITION_CLASS_TYPELIST` and descriptor classes/the direction enum in the appropriate extra type list in `Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h`, with registrations in `GuiReflectionCompositions.cpp`. `Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` requires every composition class-list entry to derive from `GuiGraphicsComposition`.
- Update public composition includes, explicit project source inventories, loader registration, and generated metadata through the repository's existing mechanisms. Descriptor construction and `BuildLayout` must work in ordinary C++ without reflection; XML generation must also support metadata-only and full-reflection configurations.

### Release tutorial migration

- `Release` here means the sibling repository `../Release`, not generated files under this repository's `Release` directory. Follow that repository's instructions when executing this part. The inventory is `../Release/Tutorial/GacUI_Controls/GacUI_Controls.sln`.
- Convert only the portions of each application that the easy-layout feature can express while preserving appearance and behavior. Keep specialized arrangements such as interactive splitters, repeated flow, and shared-size groups where they cannot be rewritten; preserve their required parent/subtree structure, either in place or as a single composition payload. Record which portions remain unchanged and why; this task does not require extending easy layout to replace every existing composition feature.
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
- Provide a coverage matrix and enough cases to exercise every valid and invalid sibling-type combination and parent/child category in the specification. Include zero, one, and multiple occurrences where applicable, declaration-order variations, both axes, nested combinations, payload kinds, and property boundary values. Assert expected geometry for valid cases, expected failure for rejected grammar/value combinations, and native unsited-cell behavior for delegated range conflicts; a few representative happy paths or snapshot-only checks are insufficient.
- Cover empty and single-payload roots; every permitted vertical group and its horizontal transpose; Top-only/Bottom-only/Left-only/Right-only stack optimizations; interleaved declarations; unequal Fill weights; and minimum-size versus enlarged-parent behavior.
- Verify explicit/inherited directions through multiple nesting levels, root all-Inherited defaults, and conflicting directions. Check default/zero/nondefault padding, both Border settings, nested containers, and the synthetic spacer's minimum gutter.
- Cover row-major and column-major grids, MinSize/Absolute/Percentage option merging, inner and outer spans, ranges spanning both axes, ragged trailing cells, missing single-span declarations, and conflicting non-MinSize options. For overlapping, out-of-range, and nonpositive-span sites, assert the existing table's placement result without expecting an extra easy-layout exception. Include insertion-order conflicts where the earlier cell remains placed and the later cell is unsited, plus a valid-to-invalid explicit rebuild that must not retain the old cell placement.
- Reject mixed layout/payload content, multiple payloads, illegal sibling combinations, invalid parent-controlled payloads, invalid percentage/absolute-size values, and shared/cyclic ownership. Include very small positive percentage values so native table coercion cannot hide a zero-weight error; test invalid cell sites separately under the delegated table behavior above.
- Verify destruction counters and Debug leak output for unbuilt trees, built trees destroyed through an ordinary composition parent, and trees destroyed through an owning control. Include nested control payloads and descriptors that outlive the built owner; no double deletion, leaked pending payload, or access to a destroyed attached payload is acceptable.
- After an initial build, change each easy-layout property and the descriptor structure, and verify that generated layout remains unchanged until the next explicit `BuildLayout`. Then rebuild and verify that current values take effect while reused controls retain identity, bindings, handlers, and state. Cover unchanged repeated builds, replacing generated table/stack arrangements, and destruction after rebuilding. Normal parent resizing and payload minimum-size changes must continue to use the existing built configuration through the layout engine.

### XML and generated-code tests

- Use the remote protocol based GacUI unit test framework for GacUI XML Compiler tests. For valid resources, follow the resource-compilation and frame-assertion pattern documented in `.github/KnowledgeBase/manual/unittest/gacui.md`; verify that compiled XML constructs and initializes the intended layout and payload objects. Keep exhaustive layout-algorithm verification in the direct composition tests above.
- Test implicit `ez`, an explicit `xmlns:ez`, a custom default namespace, and namespace serialization round-trips. Instantiate every descriptor type with defaults and explicit constant values using both attributes and `<att.*>` forms.
- Add negative XML cases in the remote protocol based framework for prohibited bindings, runtime-dependent `CellOption` fields, and statically detectable invalid child structures. Run compilation in the framework's test context and retain the collected compiler errors to assert rejection and useful source-position diagnostics before attempting to instantiate a window; do not pass expected failures through a convenience helper that treats all compilation errors as unexpected fatal failures.
- Verify automatic payload alignment and named-object/event/binding preservation. Check that generated initialization calls `BuildLayout` after all required initial values and permitted bindings' initial evaluations are ready, including empty and nested layouts. Change bound `Padding`, `Border`, `Direction`, and `CellSpan` values after initialization: stored values must change while generated layout retains its previous configuration until an explicit rebuild applies them.
- Exercise Workflow-loaded resources with full reflection, and generated C++ in the supported full, metadata-only, and no-reflection configurations. Follow `Project.md`: build Debug Win32 and run `Metadata_Generate`; build Debug x64, run `Metadata_Generate`, then `Metadata_Test`. Run required code generation and the relevant UnitTest coverage with the prescribed build/execute wrappers, respecting the existing test filter. Inspect final logs and leak reports; ensure no new `*.UI.errors.txt` remains.

### Tutorial equivalence and automation

- Before migration, capture durable visual images and automation geometry for every runnable GacUI_Controls tutorial. Record the revision, renderer, skin, DPI, font settings, client size, and interaction sequence so the same states can be reproduced after migration.
- Compare rendered appearance and control/element geometry at the original size and at smaller/larger sizes. Allow generated composition topology and transient IDs to differ; require the same visible spacing, alignment, content, clipping, and useful resize behavior.
- Exercise all tabs and relevant popups/dialogs, list/grid item templates, editor variants, localization and animation states present in each tutorial. Verify interaction after migration, including splitters and repeated/wrapping content; an unchanged initial screenshot is insufficient.
- Specifically verify CalculatorAndStateMachine's display spanning four columns and its plus button spanning two rows in the same table, as well as AddressBook's draggable splitter and ColorPicker2's repeated shared-size palette.
- For every partially converted tutorial, verify both the rewritten portion and the retained specialized layouts together. Retaining an unsupported arrangement is an expected migration outcome; its existing appearance and interaction must still match the baseline.
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

# TEST [CONFIRMED]

The initial Debug x64 namespace regression reproduced the missing feature. Direct composition tests cover grammar, geometry, spans, explicit rebuilds, and ownership. Remote-protocol XML tests cover namespace resolution, loader validation, initial bindings, and initialization order. Release tutorial baselines and migration comparisons must use the same renderer, client sizes, and interactions. Required builds, metadata generation, both architecture test runs, and final leak checks are acceptance criteria. Reproduction is confirmed; proposal acceptance remains pending the results recorded below.

# PROPOSALS

- No.1 Lower validated descriptors into the existing composition engine

## No.1 Lower validated descriptors into the existing composition engine

Use ordinary bounds, stack, table and fresh cell compositions. A validation/planning pass resolves axes, partitions docking groups, infers shared grid options and records native cell sites before any generated tree changes. Normalize percentage tracks with an exact power-of-two scale so the largest weight is in [1,2); reject ratios below the table's supported 0.001 threshold. A no-fill docking table uses explicit cell insets and zero table padding so its synthetic spacer adds no duplicate gutter.

Track payload lifetime using a composition internal-property sentinel and a shared non-owning state. Pending payloads belong to their descriptor/root; attached payloads belong to the normal composition/control tree. The sentinel clears retained state when the payload dies. Rebuild detaches retained payloads before deleting obsolete containers and then attaches the same instances to the new tree.

Add reflection registrations, implicit ez namespace resolution, dedicated descriptor/payload loaders, constant-value and child-grammar diagnostics, and a final initialization phase after bindings. Keep stored-property changes separate from generated geometry. After runtime/compiler acceptance, update the normal release pipeline, capture automated tutorial baselines, and migrate expressible authored XML while preserving specialized subtrees.

### CODE CHANGE

The first baseline regression requires the implicit ez namespace in a remote-protocol test context with an independently specified default namespace. [CONFIRMED] Debug x64 built with zero warnings/errors; the unfiltered UnitTest run failed at the expected missing-namespace assertion in TestControls_EasyLayout.cpp. The completed Execute.log contains that assertion; this is a reproduction run, not final acceptance.

Added the public descriptor/root declarations and a two-phase runtime builder, the composition inventory entries, and the independent implicit namespace insertion. Direct composition tests are being added before validation of these changes.

Added direct C++ tests for all 128 sibling subsets with one/two occurrences in both declaration orders, exact minimum rectangles, docking gutters, weighted fills, axis inheritance, grids and native site rejection, deferred mutations/rebuilds, and destruction counters. Added reflection registration and dedicated child loaders. The compiler checks constant-only binding restrictions during reference collection, validates static child structure, validates numeric CellOption field expressions, and emits nested-first BuildLayout calls after the binding phase. Remote-protocol tests cover namespace round trips, binding/structure diagnostics and initial/later bound property behavior. Initial build API mistakes were corrected; test execution remains pending.

The first direct geometry run exposed the no-fill Top+Bottom gutter: Bounds alignment is measured from the parent bounds, so a zero-aligned wrapper cancels the cell's internal margin. Apply the gutter to the wrapper alignment instead. Preserve the already-built border alignment of separately declared nested easy-layout roots when attaching their payloads.

CDB located the ownership-test crash in GuiControl::RebuildControlTemplate, called by the fixture without an active theme. The standalone test controls now explicitly install a minimal GuiControlTemplate, matching TestCompositions_Responsive.cpp; the production ownership code was not implicated by that stack.

Expanded XML cases to instantiate all seven descriptor kinds with defaults, attribute constants and att.* constants, including explicit ez plus a custom default namespace. Corrected the runtime-dependent numeric cast case to valid Workflow syntax. The direct layout/ownership section now completes; the unfiltered suite is still running. Native HTTP tutorial inspection works, while Graphics Capture fails and PrintWindow returns a black image in the current desktop session; visual baseline capture remains pending.

Debug Win32 also builds cleanly. Added an Easy Layout page to FullControlTest's authored resources to exercise generated code in all three reflection configurations, including bound stored properties, explicit rebuilds, retained text input and shared grid tracks. Generated outputs will be produced only by GacUI_Compiler. Reordered the new XML test inventory entry beside other control tests without changing test filters. Tutorial capture now records geometry/renderer/skin/DPI/font/client-size metadata at initial, smaller, larger and restored sizes; black capture images are not accepted as visual evidence.

The completed x64 suite reached the new XML positive case after all preceding tests and negative diagnostics passed, then failed with Workflow constructor-expression ambiguity. The fixture's binary numeric struct field is now parenthesized to disambiguate the existing struct shorthand parser; the next run will verify this diagnosis. Win32 Metadata_Generate completed successfully. The legacy Release automation library uses wtou for window IDs, so explicit x64 window-ID requests fail; current GacUI already contains the u64 conversion fix that the required release pipeline will publish. Main-window requests work without the optional ID.

Added an independent grid test proving CellOption and CellSpan mutations are deferred, parent resizing still uses the built percentage tracks, and a later overlapping span rebuild leaves newly created conflicting cells unsited. Nine malformed/duplicate /AsPort forms all exited with status 1 before a tutorial window/listener was created.

Added direct empty-root/single-payload coverage, including live minimum-size propagation without rebuilding and exact default-border rectangles. Coverage matrix: sibling grammar = 128 subsets x two multiplicities x two orders; docking = both axes/minimum/enlarged; fill = unequal/tiny/invalid weights and nested inheritance; grids = both axes/shared options/ragged/spanning/unsited/deferred changes; ownership = pending/built/composition-parent/control-parent/nested controls/outliving descriptors; XML = namespace, all descriptor defaults and both syntaxes, all prohibited binder forms, positioned grammar/constant errors, initial binding order and explicit rebuilds.

Automatic approval review rejected the command that would launch QueryService without /AsPort and verify its default 8888 endpoint beside ContainersAndButtons on 8901, reporting only 'blocked by policy'. No default-port verification is claimed and the rejected action was not retried.

The rebuilt Debug x64 solution passed with zero warnings/errors. Metadata_Generate succeeded for Win32 then x64, and Metadata_Test completed with exit 0 and no appended leak output. The full x64 UnitTest suite is being rerun with expanded fixtures and the XML test in its control-test inventory position.

The expanded grid test exposed a precision bug in maximum-based normalization: a 1:3 split of 200 pixels becomes 49.999999999999993 for its first track and the native table truncates it to 49. Normalize with an exact power-of-two scale (largest weight in [1,2)) instead, preserving binary ratios while keeping all accepted weights above native coercion. The relative minimum ratio policy remains 0.001. This production correction is pending verification.

The showcase generator rejected the fixture's nonexistent TextBox tag. It now uses the established SinglelineTextBox control; generated outputs will be regenerated before rebuilding and testing the rounding correction.

The documented CopyFromScreen fallback also fails with an invalid screen handle. The required visual baseline remains unavailable, so tutorial XML migration remains pending. The Release build wrapper now resolves a requested Win32 configuration to an existing x86 solution platform when needed, matching the established release pipeline's platform resolution and allowing tool builds through the required wrapper.

Release x64 tutorial builds are running to verify that the shared launcher compiles without Debug automation. Native desktop capture remains unavailable; compilation and listener behavior are independent checks.

Added explicit zero/negative inner-span coverage in both grid orientations. Existing single-span declarations establish the shared tracks, so these cases verify native unsited-cell behavior independently of option-inference errors.

A CDB attachment during the quiet compiler phase found active WorkflowScriptPositionVisitor traversal and insertion into the script-position dictionary during Workflow_GenerateRpcMetadata. There was no native crash dialog or deadlock; the debugger was detached and compilation continues.

The Release x64 tutorial solution completed in 21m58s with zero warnings/errors. All 18 runnable tutorials opened native windows, had no listeners on their explicit test ports 9051-9068, and exited 0 after WM_CLOSE. The results are stored in Release/Tutorial/EasyLayoutVerification/release-launch-checks.json. This validates the shared launcher's Release configuration before updating packaged libraries.

Inspection of the generated x86 showcase C++ confirms that all subscriptions are installed before the nested-first BuildLayout calls. GuiInstanceRootObject::AddSubscription opens and immediately updates each subscription, so those calls follow the initial bound values as intended. Runtime confirmation is still pending.

A separate concurrent Debug run verified ContainersAndButtons on port 9091 and Animation on 9092. Both HTTP control trees were available simultaneously; both !Exit requests completed with process status 0 and no remaining listeners on those ports. Results are in Release/Tutorial/EasyLayoutVerification/debug-shutdown-checks.json.

The value-boundary tests now also exercise invalid percentage CellOptions in both shared-track orientations, an invalid direction enum, and actual measurement of accepted tiny equal Fill weights. These close coverage gaps beyond testing Fill validation alone.

GacUI_Compiler completed with exit 0 for x86 and x64, including shared-source merging. No generated .UI.error.txt or .UI.errors.txt is present. The expanded showcase C++ is now ready for all reflection configurations.

The post-generation Debug Win32 build completed with zero warnings/errors (6m11s), including no-reflection, metadata-only and full-reflection showcase consumers. Win32 Metadata_Generate completed with exit 0. The complete Win32 UnitTest suite is now running with the expanded cases.
The post-generation Debug x64 build completed with zero warnings/errors (1m44s); x64 Metadata_Generate and Metadata_Test exited 0. The generated no-reflection showcase accepts typed text and updates its bound label. Toggling the stored direction leaves label bounds (230,157)-(631,444) unchanged until Rebuild; after the asynchronous UI frame it becomes (25,259)-(631,444), retaining the edited text. The initial 300ms read was premature, so live checks must poll for the resulting UI frame. Ctrl+A did not select the existing text in this hosted session; the test correctly verifies the actual inserted text. UIA returned accessibility:null for this showcase in the current desktop session.
The metadata-only generated showcase passed typed-text binding, deferred direction changes, and explicit vertical rebuilding: label bounds (225,157)-(615,441) stayed unchanged after toggling and became (25,258)-(615,441) after Rebuild, preserving edited text. The established Release-GacUI/Build-Release-Update pipeline will refresh packed source, skins, compiler executables and architecture metadata, using the mandated repository build wrappers for every solution build.
The full-reflection generated showcase passed the same text/binding/deferred-rebuild checks as metadata-only with identical rectangles. Both configurations terminated after !Exit and had no native runtime-error dialog; exit codes were unavailable from the separately reacquired Process handles. UIA returned accessibility:null for both; accessibility interactions could not be completed. All three generated C++ reflection configurations have now been exercised through HTTP automation.

Preflight found that GacCommon.ps1 scans every linked XML file with Select-Xml, whose namespace-aware reader rejects implicit ez prefixes before invoking GacUI. The canonical Tools/Tools/GacCommon.ps1 scanner will use XmlTextReader with Namespaces=false for resource discovery; GacUI remains responsible for its own namespace mappings. This also preserves discovery under custom default mappings. The scanner change will be redeployed through Build-Release-Update and verified against all tutorial resources before compilation.

The scanner fix passes implicit-prefix/custom-default-namespace/resource-versus-linked-folder checks, discovers all 52 tutorial resources, and the deployed Release copy matches the canonical Tools script byte-for-byte. The tool solution remained clean (zero warnings/errors). GacBuild is now regenerating the complete Tutorial/GacUI.xml driver for both architectures.

Tutorial/GacUI.xml regeneration finished with exit 0. All 52 resources have fresh x32 and x64 output directories, and no architecture Errors.txt exists. The GacUI_Controls generated files are byte-identical, as expected before migration; the copied FullControlTest outputs include the new showcase. The tutorial solution will now be built against the updated packages in Debug Win32, Debug x64 and Release x64.

Updated-package tutorial build Debug Win32 completed successfully.     0 Warning(s);     0 Error(s); Time Elapsed 00:01:53.89

The canonical Tools scanner fix is committed and pushed as b514f7b. Every declared output in all 52 resource metadata dumps exists and has a timestamp from this generation run; generation-checks.json records this audit.

The updated-package tutorial Debug Win32 build passed with zero warnings/errors. Debug x64 failed with C1128 in packed GacUI.Windows.cpp because the newly published existing UI Automation implementation exceeds MSVC's object-section limit. Add /bigobj to that source entry in GacUILite, matching the project's other packed translation units, then rerun the affected tutorial builds.

Updated-package tutorial build with /bigobj, Debug Win32, completed successfully.     0 Warning(s);     0 Error(s); Time Elapsed 00:00:23.16

Applied the same per-file /bigobj setting to the full GacUI tutorial library, the only other packaged Windows-source consumer, to prevent the identical object-section limit there. The GacUI_HelloWorlds Debug x64 solution will verify that full library, and GacUI_ControlTemplate will verify the pipeline-copied easy-layout showcase, after the controls build finishes.

The complete Debug Win32 UnitTest run finished successfully: 92/92 files and 1784/1784 cases. Its final Execute.log ends at the passing summary with no appended leak dump. All direct easy-layout cases and positive/negative XML cases, including initial binding order and explicit rebuilding, passed. The complete x64 suite is now running with the same tests and unchanged filters.

Updated-package tutorial build with /bigobj, Debug x64, completed successfully.     0 Warning(s);     0 Error(s); Time Elapsed 00:01:02.02

Post-package HTTP checks now successfully address explicit 64-bit main and popup IDs: ColorPicker selected Green from its popup and the ribbon editor opened/dismissed its HOME menu. Both initial element/text/rectangle sequences exactly match their stored baseline. Windows screen capture remains unavailable. Sky still returns accessibility:null, but a windowless MTA UIA client successfully retrieved ColorPicker's child controls and ComboBox Selection/ExpandCollapse patterns; UIA interaction verification is continuing through that supported path.

Updated-package tutorial build with /bigobj, Release x64, completed successfully.     0 Warning(s);     0 Error(s); Time Elapsed 00:13:19.94

Direct Windows UI Automation from an MTA client passed: ColorPicker expanded its combo, selected Maroon, collapsed it and returned Maroon from Selection.Current.GetSelection. The full-reflection easy-layout showcase selected both tabs, changed editor text through ValuePattern, toggled the stored direction through TogglePattern and invoked Rebuild. Bounds stayed unchanged until the explicit build, then switched orientation; the editor retained both runtime identity and text. These results supersede the earlier inability to interact through Sky's empty accessibility response. Durable UIA results are recorded in Release/Tutorial/EasyLayoutVerification/uia-*-checks.json.

All 18 updated-package Release x64 tutorials opened native windows, had neither requested-port nor process-owned listening sockets, and exited 0 after WM_CLOSE. ColorPicker, DocumentEditorRibbon and CppTest_Reflection also completed their Debug interaction checks and exited 0 after correctly formatted !Exit commands, releasing their ports. Durable results are recorded in packaged-release-launch-checks.json and packaged-debug-shutdown-checks.json. An initial shutdown probe incorrectly JSON-quoted the text-protocol command and had no effect; native inspection showed no crash dialog, and the corrected text command passed.

Updated-package GacUI_HelloWorlds Debug x64 built successfully. ; Build succeeded.;     0 Warning(s);     0 Error(s); ; Time Elapsed 00:05:23.35

### CURRENT VERIFICATION AND REMAINING WORK

No.1 remains unconfirmed as a complete task because the requested tutorial XML migration and visual equivalence acceptance have not been performed. The implementation and independent checks are retained for backup and review.

| Area | Evidence and result |
| --- | --- |
| Direct layout grammar | TestCompositions_EasyLayout covers all 128 sibling-kind subsets with one/two occurrences and both declaration orders (512 combinations), plus empty and single-payload roots. Valid cases assert independently specified geometry; invalid cases assert errors. |
| Geometry and mutable configuration | Both axes, one-sided stacks, stable docking order, minimum gutters, weighted fills, inherited/conflicting directions, shared grid options, ragged/spanning/unsited cells, resizing, deferred properties/structure and explicit rebuilding are covered directly. |
| Ownership | Destruction counters cover pending/built payloads, composition/control parents, nested controls and descriptors outliving their attached payloads. Rebuild preserves reused instances and releases obsolete content. |
| XML compiler | TestControls_EasyLayout uses the remote unit framework for namespace round trips, all descriptor kinds and constant syntaxes, 30 positioned rejection cases, initial binding order, later stored values, events and explicit rebuilding. |
| GacUI builds and metadata | Debug Win32/x64 builds passed. Metadata_Generate passed for Win32 then x64; Metadata_Test passed for x64 with no appended leak dump. |
| Full suites | Debug Win32 and x64 each passed 92/92 files and 1784/1784 cases with unchanged filters. Both final Execute.log files ended at their passing summaries with no appended leak dump. |
| Generated C++ | GacUI_Compiler completed both architectures with no UI error files. Full, metadata-only and no-reflection showcase configurations passed HTTP editing/binding/deferred rebuilding. Full reflection also passed UIA Value/Toggle/Invoke with retained editor identity and text. |
| Release pipeline | Established packing/tool/skin/metadata pipeline completed. All 52 tutorial resources regenerated for both architectures; all 520 declared outputs are fresh and present. Tools scanner fix b514f7b accepts implicit ez prefixes and is deployed byte-for-byte. |
| Tutorial builds and automation | GacUI_Controls Debug Win32/x64 and Release x64 passed with zero warnings/errors. All 18 rebuilt Release apps opened, had no listening sockets and exited 0. Debug selected-port concurrency, invalid arguments, popup IDs, UIA selection and clean shutdown passed. Default-port verification was blocked by automatic approval review. |
| Durable migration evidence | HTTP geometry exists for all 18 runnable apps at four sizes, with partial additional interactions and capture metadata. No valid visual image was available. Authored GacUI_Controls XML remains unchanged; no migration/equivalence claim is made. |

To continue: obtain an available Windows capture surface, complete durable before-images and the remaining interaction states, migrate expressible authored XML while retaining the documented specialized subtrees and spacing, regenerate through Tutorial/GacUI.xml, and compare visual/geometry/interaction states at all required sizes. Complete the default-port check only when the approval restriction is resolved.

Source/project/script whitespace checks passed. The CodePack-generated GacUI.h and GacUICompiler.cpp each retain an extra blank line at EOF in both GacUI/Release and Release/Import; the 14 newly generated easy-layout Workflow text snapshots also end with generator-produced blank lines. Generated files were not manually edited to silence these warnings.

Updated-package GacUI_ControlTemplate Debug x64 built successfully. ; Build succeeded.;     0 Warning(s);     0 Error(s); ; Time Elapsed 00:01:09.82

Automatic approval review also rejected an additional command to launch the rebuilt Debug Win32 ColorPicker on selected port 8991, read its controls and close it, reporting only 'blocked by policy'. The command did not run and was not retried. Existing successful Win32 builds and the full Win32 unit run remain valid; no Win32 tutorial live-automation result is claimed.

Release package/automation/baseline changes are committed and pushed as e179f7e7. GacUI_HelloWorlds and GacUI_ControlTemplate both completed Debug x64 with zero warnings/errors, covering the full packaged library and copied generated showcase. The tracked file-dialog snapshot differences produced by asynchronous scheduling were inspected and restored; the new easy-layout snapshots are retained.

The full Debug x64 UnitTest process exited 0. Its authoritative final Execute.log reports 92/92 files and 1784/1784 cases and ends exactly at the passing summary, with no appended leak output. Both required architecture suites now pass. No native tutorial process or debugger from this work remains running. The only outstanding acceptance work is the visual-baseline/tutorial-migration work and the explicitly blocked automation checks above.
