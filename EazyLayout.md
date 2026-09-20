# Eazy Layout

`<ez:Layout>` arranges controls using docking, proportional space, or rows and columns. Layout descriptors specify the arrangement, and the layout creates the required tables, cells, stacks, and bounds wrappers. Splitters let users resize adjacent parts by dragging their shared boundary.

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
| `ez:Splitter` | Marks a resizing boundary after the preceding row or column. | No additional layout properties. |

`ez:Layout` is a bounds composition. It starts with `MinSizeLimitation="LimitToElementAndChildren"` and zero `AlignmentToParent`; building applies its border setting. The other elements are layout descriptions, not extra controls or compositions.

## Content

A layout or descriptor can be empty, contain a list of layout descriptors, or contain one control or independently positioned composition. `ez:Splitter` is always empty. Descriptors and a control/composition cannot be mixed at the same level, and a descriptor cannot contain two controls directly. Multiple controls require child descriptors or a single container.

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

Each arrangement also accepts Splitter descriptors at its internal boundaries, as described under [Splitters](#splitters). Vertical and horizontal docking cannot be mixed at the same level, and Fill cannot be mixed with Row or Column siblings. A nested descriptor group can use a different arrangement.

The visual order is Top, middle, Bottom, or Left, middle, Right. Declaration order is preserved within each of those groups, including Bottom and Right. Thus a Bottom written before a Top still appears below that Top.

Docking without fills or explicit rows/columns leaves unused space between the leading and trailing groups. If there is only one docking side, it stays against that side. One-sided groups without splitters may be built as stacks; the others use tables. Adding a splitter forces its containing group to use a table while preserving edge alignment, visual order, and spacing.

`Percentage` is a relative weight. Two default fills divide their space equally; weights `1` and `3` divide it in a 1:3 ratio. The weights do not need to add up to 1 or 100.

## Direction

Top, Bottom, and Row siblings establish a vertical arrangement. Left, Right, and Column siblings establish a horizontal arrangement. Splitters follow that direction and do not participate in direction inference.

For groups whose non-splitter children are all Fills:

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

Grids consist of rows containing columns, or columns containing rows. A Row that is not directly inside a Column accepts Column descriptors and optional Splitters; a Column that is not directly inside a Row accepts Row descriptors and optional Splitters. The inner Row or Column descriptors can contain ordinary content or another layout group.

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

## Splitters

`<ez:Splitter/>` attaches to the immediately preceding descriptor in its sibling list and marks the boundary after that descriptor's row or column. The association is preserved when docking descriptors are arranged into leading, middle, and trailing groups. Splitters do not add tracks or cells, change spans, or contribute sizing options.

In a vertical arrangement, a splitter forms a horizontal bar that moves up and down. In a horizontal arrangement, it forms a vertical bar that moves left and right. A splitter has no separate direction, size, span, or row/column index property.

```xml
<ez:Layout>
  <ez:Column CellOption="composeType:Absolute absolute:180">
    <ez:Row CellOption="composeType:Percentage percentage:1">
      <MultilineTextBox/>
    </ez:Row>
  </ez:Column>
  <ez:Splitter/>
  <ez:Column CellOption="composeType:Percentage percentage:1">
    <ez:Row>
      <MultilineTextBox/>
    </ez:Row>
  </ez:Column>
</ez:Layout>
```

The first column starts at 180 pixels, and the second takes the remaining width. The Columns establish the horizontal direction. Dragging the splitter changes the first column's absolute width.

At least one immediately adjacent track must have an `Absolute` option for dragging to resize the layout. Only adjacent Absolute options change; boundaries between MinSize or Percentage tracks do not resize. A splitter can resize an Absolute track on either side of its boundary.

### Grid boundaries

In rows containing columns, a splitter among the outer Rows selects a row boundary. A splitter among inner Columns selects a shared column boundary and extends across the table's cell area. Reverse these rules for columns containing rows.

After an outer Row or Column, the boundary follows its starting track; an outer `CellSpan` changes cell coverage without declaring extra tracks. After an inner Column or Row, the boundary follows its accumulated span. Multiple distinct Splitter descriptors resolving to the same table boundary produce one splitter. They do not affect shared track options or the requirement for single-span declarations.

### Placement rules

A splitter requires a preceding descriptor and an internal table boundary. It cannot be the first child, follow another splitter directly, or contain children or a payload. A group containing only splitters is invalid.

A splitter written last in a sibling list is valid only when its preceding descriptor has a following track after layout ordering. A boundary after the final track is invalid. Invalid cell ranges retain the table behavior described above; a splitter's boundary is checked independently.

## Spacing

`Padding` is a nonnegative gap between neighboring parts in one layout tree. Nested generated tables and stacks do not add extra copies of that gap.

`Border="true"` applies the same gap between the layout and its parent. `Border="false"` removes that outside gap. Border controls spacing, not a drawn line. There is no extra inset between the root and its first generated container.

A no-fill docking arrangement has one minimum gap between neighboring visible parts, including across its automatic empty space. Enlarging the container can open that space further.

A splitter occupies the existing padding gap and adds no extra gap. Its thickness is `Padding`; with `Padding="0"`, it has no mouse hit area. Docking groups retain one minimum gap between neighboring visible parts when splitters are present.

A separately written nested `ez:Layout` has its own padding and border. `Border="false"` removes its outside gap when the enclosing layout already supplies the required spacing.

### Removing redundant Bounds wrappers

`ez:Layout` is itself a bounds composition. A Bounds whose only child is an easy layout usually adds nothing when it fills its parent with `AlignmentToParent="left:0 top:0 right:0 bottom:0"`. Remove that Bounds and put the layout directly in its parent. The same applies when a descriptor supplies the wrapper's zero alignment during `BuildLayout`.

Do not copy `AlignmentToParent` to the layout: `BuildLayout` replaces it with the outside inset selected by `Padding` and `Border`. A uniform wrapper inset equal to the layout's padding can replace `Border="false"` with `Border="true"`. Preserve the inner gaps when making this conversion. For example, an outside inset of 3 and inner gaps of 5 still need a wrapper; changing Padding to 3 would also change the inner gaps.

Keep a wrapper for asymmetric insets, one-sided anchoring, or unanchored scroll content that these settings cannot reproduce. There is no separate Margin property. Preserve any other meaningful bounds properties, and check minimum-size propagation as well as the visible layout at different window sizes.

## Values, bindings, and rebuilding

Fill weights and percentage cell options used to size tracks must be finite and positive. Absolute track sizes and padding must be nonnegative. An inner descriptor's `CellOption` is ignored when its span is not 1. Within each generated row or column axis, the smallest percentage weight must be at least `0.001` times the largest; more extreme ratios are rejected.

`CellOption` and `Percentage` must be constants in XML. No binding form is allowed, including `-bind`, `-eval`, `-ref`, or `-uri`, in either attributes or property elements. Numeric constant expressions are allowed in `CellOption` fields, but runtime expressions are not.

Bindings on `Padding`, `Border`, `Direction`, and `CellSpan` are allowed. These properties store configuration. Changing them does not immediately rebuild or change the generated layout.

XML initialization calls `BuildLayout` after the descriptor tree, content, assignments, and initial binding values are ready. In C++, call it explicitly. Call it again to apply later property or structural changes. Rebuilding preserves reused controls and their state, bindings, and handlers, while replacing generated containers as needed. Normal parent resizing and content minimum-size changes work without rebuilding.

Dragging a splitter updates the generated table immediately without changing descriptor values. Window resizing retains the adjusted track sizes. An explicit `BuildLayout` restores the sizes specified by the descriptors.

The layout owns supplied content even before the first build. Destroying it safely deletes that content once, whether it is still pending or already attached to the built composition tree.

Invalid layout grammar, conflicting options, unsupported values, and invalid ownership fail at compilation where detectable, or at `BuildLayout`. They are programming errors. Native cell-site rejection follows the separate rule above.
