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

XML resources build the layout automatically after assignments, initial bindings and event handlers are ready. Nested layouts build before their owners, and **ref.Ctor** observes the built layouts. Normal resizing needs no extra work. Call **BuildLayout()** after changing layout descriptions or their properties. Rebuilding keeps existing controls, their state, bindings, and event handlers.

**Padding**, **Border**, **Direction**, **CellSpan**, **Percentage**, and **CellOption** support ordinary well-typed expressions and bindings. **CellOption** uses the same struct expressions as table options, including runtime field expressions and default values for omitted fields. Initial bindings affect the automatic build; later changes take effect only when **BuildLayout()** is called.

Layout grammar, conflicting options and invalid numeric values fail during the automatic initialization build or a later explicit **BuildLayout()**. Ownership checks can fail during assignment. Initial XML content cannot contain multiple payloads, including combinations of unnamed children and **att.Composition**; duplicates fail XML compilation. Later property replacement and binding updates remain supported. Ordinary XML syntax, type and binder restrictions are also checked at compilation.

