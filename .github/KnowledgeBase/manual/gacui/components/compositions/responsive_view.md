# \<ResponsiveView\>

**\<ResponsiveView/\>** tells its current managed **GuiResponsiveCompositionBase** to switch view, or switch to a new managed **GuiResponsiveCompositionBase**, when itself is required to do so.

**\<ResponsiveView/\>** is a **GuiResponsiveCompositionBase** and a **\<Bounds/\>**.

## \<ResponsiveView\>::Views

**\<ResponsiveView/\>** only manages **GuiResponsiveCompositionBase** that is added to **Views**. The first one in **Views** is the one that is expected to be the largest one. The last one in **Views** is the one that is expected to be the smallest one.

Although only the **CurrentView** is a child composition of **\<ResponsiveView/\>**, but every **GuiResponsiveCompositionBase** in **View** will be deleted when **\<ResponsiveView/\>** is deleted.

**IMPORTANT**: If a **GuiResponsiveCompositionBase** in **View** has a different **Direction** than **\<ResponsiveView/\>**, its **LevelCount** will be treated like 1.

## \<ResponsiveView\>::Direction

The default value for this property is **Both**.

Valid values for this property are **Horizontal**, **Vertical** and **Both**.

When **Direction** is **Horizontal**, it only reacts to changing to its width. When **Direction** is **Vertical**, it only reacts to changing to its height.

## \<ResponsiveView\>::LevelCount

This property represents how many different view it could have. It is the sum of **LevelCount** of all managed **GuiResponsiveCompositionBase**.

If there is no managed **GuiResponsiveCompositionBase**, **LevelCount** is 1.

## \<ResponsiveView\>::CurrentLevel

This property represents the current view of a **\<ResponsiveView/\>**.

The smallest view is 0, the largest view is **LevelCount** - 1.

## \<ResponsiveView\>::CurrentView

Each view in **Views** could have multiple view (**LevelCount** \> 1). **CurrentView** represents a **GuiResponsiveCompositionBase** in **Views** who is selected to display one of its view.

When **Views** is empty, **CurrentView** is **null**.

**IMPORTANT**: **LevelCount IS NOT** always **Views.Count**, so **CurrentView IS NOT** always **Views[CurrentView]**.

## Choosing a view

When a **\<ResponsiveView/\>** is required to switch to a larger view (increasing **CurrentLevel** by 1), **\<ResponsiveView/\>** will tell **CurrentView** to switch to a larger view. If **CurrentView** is already in its largest view, than **\<ResponsiveView/\>** switches **CurrentView** to its previous one in **Views**. The new **CurrentView** will be in its smallest view.

When a **\<ResponsiveView/\>** is required to switch to a smaller view (decreasing **CurrentLevel** by 1), **\<ResponsiveView/\>** will tell **CurrentView** to switch to a smaller view. If **CurrentView** is already in its smallest view, than **\<ResponsiveView/\>** switches **CurrentView** to its next one in **Views**. The new **CurrentView** will be in its largest view.

When **CurrentView** is changed, the old one is removed from **\<ResponsiveView/\>**, the new one is added as a child composition in **\<ResponsiveView/\>** with its **AlignmentToParent** set to **left:0 top:0 right:0 bottom:0**.

