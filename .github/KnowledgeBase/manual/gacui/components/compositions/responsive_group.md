# \<ResponsiveGroup\>

**\<ResponsiveGroup/\>** tells its managed **GuiResponsiveCompositionBase** to switch view at the same time when itself is required to do so.

**\<ResponsiveGroup/\>** is a **GuiResponsiveCompositionBase** and a **\<Bounds/\>**.

## Managing child GuiResponsiveCompositionBase

When things inside a **\<ResponsiveGroup/\>** are changed, **\<ResponsiveGroup/\>** will search for any child composition that is a **GuiResponsiveCompositionBase**. If a child composition is not, it will search into this child composition and see if there are any **GuiResponsiveCompositionBase** recursively. When a **GuiResponsiveCompositionBase** is found, it stops searching for child compositions inside this **GuiResponsiveCompositionBase**.

Eventually **\<ResponsiveGroup/\>** knows all child **GuiResponsiveCompositionBase** inside it.

**IMPORTANT**: If a child **GuiResponsiveCompositionBase** shares no axis with the group's **Direction**, it is not a managed **GuiResponsiveCompositionBase**.

## \<ResponsiveGroup\>::Direction

The default value for this property is **Both**.

Valid values for this property are **Horizontal**, **Vertical** and **Both**.

When **Direction** is **Horizontal**, it only reacts to changing to its width. When **Direction** is **Vertical**, it only reacts to changing to its height.

## \<ResponsiveGroup\>::LevelCount

This property represents how many different view it could have. It is the largest **LevelCount** of all managed **GuiResponsiveCompositionBase**.

If there is no managed **GuiResponsiveCompositionBase**, **LevelCount** is 1.

## \<ResponsiveGroup\>::CurrentLevel

This property represents the current view of a **\<ResponsiveGroup/\>**.

The smallest view is 0, the largest view is **LevelCount** - 1.

## \<ResponsiveGroup\>::ViewSizes

**ViewSizes** is an optional integer array of preferred minimum sizes indexed by **CurrentLevel**. Write entries from small to large: entry 0 belongs to level 0. This order is the reverse of the largest-first **Views** collection of **\<ResponsiveView/\>**.

For an array with **N** entries and current level **L**, use **ViewSizes[L]** when **L \< N**. Otherwise use **ViewSizes[N - 1] + (L - N + 1)**. For example, **{64 230}** selects 64, 230 and 231 at levels 0, 1 and 2. Entries beyond the child-derived **LevelCount** are stored but unused; they do not create levels.

**Horizontal** applies the selected value to **PreferredMinSize.x** and preserves its height. **Vertical** applies it to **PreferredMinSize.y** and preserves its width. **Both** applies the same value to both components. The group reapplies the value on assignment, level or direction changes, and minimum-size measurement. Manually setting an affected component of **PreferredMinSize** does not override **ViewSizes**.

The default empty array disables this behavior. Clearing an assigned array stops further overrides and keeps the current stored **PreferredMinSize**; it does not restore the value from before assignment. Normal bounds measurement still applies, so children, elements and margins can require more space than the selected preference. With zero margins, a nonzero level is retained at its effective minimum and lowered below that minimum; growing to the next level's effective minimum admits that level.

Both literal XML forms below are supported without an eval binder. Child content is omitted here. In C++, **SetViewSizes(const collections::Array\<vint\>&)** copies the array; **GetViewSizes()** returns a const reference containing only the configured entries, without extrapolated values. Reflection exposes an assignable typed integer-array property.
```XML
<ResponsiveGroup Direction="Horizontal" ViewSizes="{64 230}">
    <!-- Account rows -->
</ResponsiveGroup>

<ResponsiveGroup Direction="Horizontal">
    <att.ViewSizes><_>120</_><_>334</_></att.ViewSizes>
    <!-- Movie rows -->
</ResponsiveGroup>
```

With virtualized lists, initialize newly realized responsive rows to their group's established level before attaching them. Removing all rows temporarily leaves **LevelCount == 1** and **CurrentLevel == 0**, regardless of the array length. Retain the intended level separately while reloading rows. **ReloadVisibleStyles()** clears rows immediately; replacement rows can appear during the call or during later layout. Suppress nested reloads until replacement rows restore that level. Restoring level zero may change the level count without raising **CurrentLevelChanged**. Keep the arranger's row-height reset when switching between compact and detailed templates.

## Choosing a view

When a **\<ResponsiveGroup/\>** is required to switch to a larger view (increasing **CurrentLevel** by 1), or when a **\<ResponsiveGroup/\>** is required to switch to a smaller view (decreasing **CurrentLevel** by 1), it tells all managed **GuiResponsiveCompositionBase** to synchronize with that new **CurrentLevel**. If any **GuiResponsiveCompositionBase**'s **LevelCount** is not large enough, it stays with its largest view.

