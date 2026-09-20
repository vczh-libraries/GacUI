# Eazy Layout

`<ez:Layout>` arranges controls using docking, proportional space, or rows and columns. It builds the ordinary GacUI compositions needed for the arrangement. You normally do not need to write their tables, cells, stacks, or bounds wrappers yourself.

This document describes the existing feature, extracted from [the original task](TODO_Task_EazyLayout.md) and [the EazyLayout refactoring notes](TODO_Task_Refactor.md#eazylayout). The proposed `<ez:Splitter>` extension is described separately at the end; it is not implemented yet.

## Getting started

The `ez` namespace is predefined. An explicit `xmlns:ez` overrides that default.

```xml
<ez:Layout>
  <ez:Top>
    <Label Text="Title"/>
  </ez:Top>
  <ez:Fill>
    <MultilineTextBox/>
  </ez:Fill>
  <ez:Bottom>
    <Button Text="Close"/>
  </ez:Bottom>
</ez:Layout>
```

The title and button use their minimum heights. The text box takes the remaining height. There are five pixels between neighboring parts and five pixels around the layout. `Top` and `Bottom` already establish the vertical direction, so the fill needs no `Direction`.

## Elements and defaults

| Element | Purpose | Properties and defaults |
| --- | --- | --- |
| `ez:Layout` | Owns the layout and builds its compositions. | `Padding="5"`, `Border="true"` |
| `ez:Top`, `ez:Bottom` | Put minimum-height parts above or below the middle. | No additional layout properties. |
| `ez:Left`, `ez:Right` | Put minimum-width parts before or after the middle. | No additional layout properties. |
| `ez:Fill` | Shares the remaining space with other fills. | `Percentage="1"`, `Direction="Inherited"` |
| `ez:Row`, `ez:Column` | Describe table tracks and cells. | `CellOption="composeType:MinSize"`, `CellSpan="1"` |

`ez:Layout` is a bounds composition. It starts with `MinSizeLimitation="LimitToElementAndChildren"` and zero `AlignmentToParent`; building applies its border setting. The other elements are layout descriptions, not extra controls or compositions.

## What can go inside

A layout or descriptor can be empty, contain a list of layout descriptors, or contain one control or independently positioned composition. Do not mix descriptors and a control/composition at the same level, or put two controls directly in one descriptor. Use child descriptors or a single container when you need multiple controls.

An outer `Row` or `Column` has the additional grid rules below. A nested `ez:Layout` counts as one composition, with its own settings.

A control contributes its existing `BoundsComposition`. The layout stretches ordinary bounds payloads to their assigned area by setting all four alignment values to zero. A directly nested `ez:Layout` keeps its own border alignment. Existing control names, bindings, events, and child content are preserved.

Parent-controlled compositions such as `Cell` and `StackItem` cannot be standalone payloads. Keep their required `Table` or `Stack` as the single payload instead. Each descriptor and payload must belong to one place in the tree; sharing them or creating a cycle is invalid.

## Docking and fills

Each sibling group can use one of these arrangements:

| Arrangement | Allowed siblings |
| --- | --- |
| Proportional parts | Fills only. |
| Vertical docking | Tops and/or Bottoms, optionally with Fills in the middle. |
| Horizontal docking | Lefts and/or Rights, optionally with Fills in the middle. |
| Rows | Rows, optionally with Tops and/or Bottoms. |
| Columns | Columns, optionally with Lefts and/or Rights. |

Do not mix vertical and horizontal docking at the same level. Do not mix Fill with Row or Column siblings. Nest another descriptor group to change the arrangement.

The visual order is Top, middle, Bottom, or Left, middle, Right. Declaration order is preserved within each of those groups, including Bottom and Right. Thus a Bottom written before a Top still appears below that Top.

Docking without fills or explicit rows/columns leaves unused space between the leading and trailing groups. If there is only one docking side, it stays against that side. These one-sided groups may be built as stacks; the others use tables. The choice of generated composition must preserve the same visible spacing.

`Percentage` is a relative weight. Two default fills divide their space equally; weights `1` and `3` divide it in a 1:3 ratio. The weights do not need to add up to 1 or 100.

## Direction

Top, Bottom, and Row siblings establish a vertical arrangement. Left, Right, and Column siblings establish a horizontal arrangement.

For fills-only siblings:

1. Use an explicit `Direction="Horizontal"` or `Direction="Vertical"` if one is present. One declaration is enough for the whole sibling group.
2. Otherwise inherit the enclosing descriptor arrangement's direction.
3. At the root, use Horizontal if no direction was established.

Two different explicit directions in one group are invalid. An explicit Fill direction must also agree with docking siblings.

```xml
<ez:Layout>
  <ez:Fill Direction="Vertical">
    <MultilineTextBox/>
  </ez:Fill>
  <ez:Fill Percentage="2">
    <MultilineTextBox/>
  </ez:Fill>
</ez:Layout>
```

These text boxes are above and below each other, with relative heights 1:2. A separately declared `ez:Layout` starts a new root; it does not inherit direction through its surrounding control or composition.

## Rows and columns

Write rows containing columns, or columns containing rows. A Row that is not directly inside a Column accepts only Column descriptors; a Column that is not directly inside a Row accepts only Row descriptors. Those inner descriptors can then contain ordinary content or another layout group.

```xml
<ez:Layout>
  <ez:Row>
    <ez:Column>
      <Label Text="Name"/>
    </ez:Column>
    <ez:Column CellOption="composeType:Percentage percentage:1">
      <SinglelineTextBox/>
    </ez:Column>
  </ez:Row>
  <ez:Row>
    <ez:Column>
      <Label Text="Email"/>
    </ez:Column>
    <ez:Column>
      <SinglelineTextBox/>
    </ez:Column>
  </ez:Row>
</ez:Layout>
```

Both rows use their minimum heights. Both labels share a minimum-width column. Both text boxes share the column that takes the remaining width. Only one declaration is needed to give that shared column its percentage option.

`CellOption` has three forms:

| Value | Meaning |
| --- | --- |
| `composeType:MinSize` | Size the track from its content. This is the default. |
| `composeType:Absolute absolute:200` | Set the track's absolute size to 200, using the native table's sizing behavior. |
| `composeType:Percentage percentage:1` | Give the track a relative share of the space left after other tracks. |

For rows containing columns, each outer Row defines a separate row option. Inner Columns at the same column position share one column option. Reverse this rule for columns containing rows.

To find a shared option, consider only descriptors whose `CellSpan` is 1. Ignore `MinSize` when another declaration supplies a nondefault option. All remaining declarations must agree in kind and value. If all declarations use `MinSize`, the track uses `MinSize`. Every shared track needs at least one single-span declaration somewhere, even if that descriptor has no content.

### Spans and placement

`CellSpan` says how many tracks a descriptor covers along its own axis. In rows containing columns:

- Each outer Row starts at the next row, after any leading Tops. Its `CellSpan` becomes the row span of its cells; it does not skip later Row declarations.
- Inner Columns start after the preceding Columns' spans. Their `CellSpan` becomes the column span.
- The widest row determines the number of columns. Shorter rows leave empty positions at the end.
- An inner Column with a span other than 1 does not supply a shared column option.

Transpose those rules for columns containing rows. Cell ranges go to the ordinary table in declaration order. Overlapping, out-of-range, or nonpositive-span sites follow the native table's rejection behavior: a rejected cell remains unsited and does not appear. Easy layout does not move it elsewhere or add a separate range-conflict exception. The shared-track declaration rules still apply.

## Spacing

`Padding` is a nonnegative gap between neighboring parts in one layout tree. Nested generated tables and stacks do not add extra copies of that gap.

`Border="true"` applies the same gap between the layout and its parent. `Border="false"` removes that outside gap. Border controls spacing, not a drawn line. There is no extra inset between the root and its first generated container.

A no-fill docking arrangement has one minimum gap between neighboring visible parts, including across its automatic empty space. Enlarging the container can open that space further.

A separately written nested `ez:Layout` has its own padding and border. Use `Border="false"` there when the enclosing layout already supplies the required gap. Preserve deliberate values such as 3 or 10 when converting an existing UI.

## Values, bindings, and rebuilding

Fill weights and percentage cell options used to size tracks must be finite and positive. Used absolute sizes and padding must be nonnegative. An inner descriptor's `CellOption` is ignored when its span is not 1. Within each generated row or column axis, the smallest percentage weight must be at least `0.001` times the largest; more extreme ratios are rejected.

`CellOption` and `Percentage` must be constants in XML. No binding form is allowed, including `-bind`, `-eval`, `-ref`, or `-uri`, in either attributes or property elements. Numeric constant expressions are allowed in `CellOption` fields, but runtime expressions are not.

Bindings on `Padding`, `Border`, `Direction`, and `CellSpan` are allowed. These properties store configuration. Changing them does not immediately rebuild or change the generated layout.

XML initialization calls `BuildLayout` after the descriptor tree, content, assignments, and initial binding values are ready. In C++, call it explicitly. Call it again to apply later property or structural changes. Rebuilding preserves reused controls and their state, bindings, and handlers, while replacing generated containers as needed. Normal parent resizing and content minimum-size changes work without rebuilding.

The layout owns supplied content even before the first build. Destroying it must safely delete that content once, whether it is still pending or already attached to the built composition tree.

Invalid layout grammar, conflicting options, unsupported values, and invalid ownership fail at compilation where detectable, or at `BuildLayout`. They are programming errors. Native cell-site rejection follows the separate rule above.

## Keep tutorial XML simple

Omit default properties. Omit Fill directions that docking or inheritance already determines, and keep only one necessary explicit direction in a fills-only group. Equal fill weights may all be omitted together; changing only one weight can change the ratio.

State a shared nondefault track option only once. Do not remove an outer row's or column's option merely because another outer track has the same value: those are separate tracks. Keep empty structural descriptors and meaningful spans.

Specialized layouts such as wrapping/repeated content, shared-size groups, and tables with placement that cannot be expressed equivalently may remain as ordinary composition subtrees.

## Planned splitter extension

`<ez:Splitter/>` is planned as a descriptor between layout parts. Its direction will follow the existing inference rules, and it will attach to the preceding row or column. A splitter will force its containing arrangement to use a table when it would otherwise use a stack.

See [the splitter implementation and tutorial migration plan](TODO_Task_EazyLayoutSplitter.md) for the proposed placement, spacing, interaction, and verification rules. Until that work is implemented, keep native `RowSplitter` and `ColumnSplitter` compositions with their owning tables.

## Implementation references

- [GuiEasyLayout.h](Source/GraphicsComposition/EazyLayout/GuiEasyLayout.h) declares the types in `vl::presentation::compositions::eazy_layout`, including `GuiEasyLayoutComposition` and its `BuildLayout` method.
- [GuiEasyLayout.cpp](Source/GraphicsComposition/EazyLayout/GuiEasyLayout.cpp) validates and builds the arrangements.
- [GuiInstanceLoader_EasyLayout.cpp](Source/Compiler/InstanceLoaders/GuiInstanceLoader_EasyLayout.cpp) handles XML content and initialization.
