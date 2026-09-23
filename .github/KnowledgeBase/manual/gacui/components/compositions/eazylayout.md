# \<ez:Layout\>

**\<ez:Layout/\>** is a bounds composition that arranges controls using docking, proportional space, and grids. The **ez** namespace is predefined in GacUI XML resources. Its child layout descriptions are not controls or compositions.

## Arranging Content

Use **\<ez:Top/\>** and **\<ez:Bottom/\>** for vertical docking, or **\<ez:Left/\>** and **\<ez:Right/\>** for horizontal docking. **\<ez:Fill/\>** shares the remaining space using its **Percentage** weight, which defaults to 1. For a group containing only fills, **Direction="Horizontal"** or **Direction="Vertical"** selects the arrangement; otherwise it inherits the enclosing arrangement, defaulting to horizontal at the root.

For grids, put **\<ez:Column/\>** descriptions inside **\<ez:Row/\>**, or rows inside columns. **CellOption** selects minimum, absolute, or proportional sizing, and **CellSpan** spans tracks. Place an empty **\<ez:Splitter/\>** between tracks to let users resize adjacent absolute-sized tracks.

Each layout or description can contain one control or independently positioned composition, or a group of child descriptions. Do not mix horizontal and vertical docking at the same level, or fills with rows or columns. Nest groups to combine these arrangements. For example, a vertical layout can contain a horizontal button group:
```XML
<ez:Layout>
  <ez:Fill><MultilineTextBox/></ez:Fill>
  <ez:Bottom>
    <ez:Right><Button Text="OK"/></ez:Right>
    <ez:Right><Button Text="Cancel"/></ez:Right>
  </ez:Bottom>
</ez:Layout>
```

Nesting **\<ez:Layout/\>** is supported but strongly discouraged for combining arrangements. Use nested description groups within one owning layout instead; different groups can have different arrangements without an extra layout owner.

## Spacing

**Padding** sets the gap between neighboring parts and defaults to 5. Nested description groups do not multiply that gap. **Border** defaults to **true** and adds the same gap around the layout; set it to **false** to remove that outside gap. It does not draw a border.

## Updating the Layout

XML resources build the layout automatically, and normal resizing needs no extra work. Call **BuildLayout()** after changing layout descriptions or their properties. Rebuilding keeps existing controls, their state, bindings, and event handlers.

**Padding**, **Border**, **Direction**, and **CellSpan** support bindings; later changes take effect when **BuildLayout()** is called. **Percentage** and **CellOption** must be constants in XML.

