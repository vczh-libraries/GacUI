# \<ResponsiveGroup\>

**\<ResponsiveGroup/\>** tells its managed **GuiResponsiveCompositionBase** to switch view at the same time when itself is required to do so.

**\<ResponsiveGroup/\>** is a **GuiResponsiveCompositionBase** and a **\<Bounds/\>**.

## Managing child GuiResponsiveCompositionBase

When things inside a **\<ResponsiveGroup/\>** are changed, **\<ResponsiveGroup/\>** will search for any child composition that is a **GuiResponsiveCompositionBase**. If a child composition is not, it will search into this child composition and see if there are any **GuiResponsiveCompositionBase** recursively. When a **GuiResponsiveCompositionBase** is found, it stops searching for child compositions inside this **GuiResponsiveCompositionBase**.

Eventually **\<ResponsiveGroup/\>** knows all child **GuiResponsiveCompositionBase** inside it.

**IMPORTANT**: If a child **GuiResponsiveCompositionBase** has a different **Direction** than **\<ResponsiveGroup/\>**, it is not a managed **GuiResponsiveCompositionBase**.

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

## Choosing a view

When a **\<ResponsiveGroup/\>** is required to switch to a larger view (increasing **CurrentLevel** by 1), or when a **\<ResponsiveGroup/\>** is required to switch to a smaller view (decreasing **CurrentLevel** by 1), it tells all managed **GuiResponsiveCompositionBase** to synchronize with that new **CurrentLevel**. If any **GuiResponsiveCompositionBase**'s **LevelCount** is not large enough, it stays with its largest view.

