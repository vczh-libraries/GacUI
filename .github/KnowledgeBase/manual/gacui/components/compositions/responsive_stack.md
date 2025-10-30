# \<ResponsiveStack\>

**\<ResponsiveStack/\>** tells its managed **GuiResponsiveCompositionBase** to switch view one after another when itself is required to do so.

**\<ResponsiveStack/\>** is a **GuiResponsiveCompositionBase** and a **\<Bounds/\>**.

## Managing child GuiResponsiveCompositionBase

When things inside a **\<ResponsiveStack/\>** are changed, **\<ResponsiveStack/\>** will search for any child composition that is a **GuiResponsiveCompositionBase**. If a child composition is not, it will search into this child composition and see if there are any **GuiResponsiveCompositionBase** recursively. When a **GuiResponsiveCompositionBase** is found, it stops searching for child compositions inside this **GuiResponsiveCompositionBase**.

Eventually **\<ResponsiveStack/\>** knows all child **GuiResponsiveCompositionBase** inside it.

**IMPORTANT**: If a child **GuiResponsiveCompositionBase** has a different **Direction** than **\<ResponsiveStack/\>**, it is not a managed **GuiResponsiveCompositionBase**.

## \<ResponsiveStack\>::Direction

The default value for this property is **Both**.

Valid values for this property are **Horizontal**, **Vertical** and **Both**.

When **Direction** is **Horizontal**, it only reacts to changing to its width. When **Direction** is **Vertical**, it only reacts to changing to its height.

## \<ResponsiveStack\>::LevelCount

This property represents how many different view it could have. It is "sum of **LevelCount** of all managed **GuiResponsiveCompositionBase**" - "the number of all managed **GuiResponsiveCompositionBase**" + 1.

For example, there are 3 managed **GuiResponsiveCompositionBase**, their **LevelCount** are 2, 3 and 4: - **LevelCount** is 2 means that composition could change 1 times from smallest to largest. - **LevelCount** is 3 means that composition could change 2 times from smallest to largest. - **LevelCount** is 4 means that composition could change 3 times from smallest to largest. So in total this **\<ResponsiveStack/\>** could change 1 + 2 + 3 = 6 times from smallest to largest, its **LevelCount** must be 7, which is exactly "2 + 3 + 4 - 3(numbers of managed) + 1".

If there is no managed **GuiResponsiveCompositionBase**, **LevelCount** is 1.

## \<ResponsiveStack\>::CurrentLevel

This property represents the current view of a **\<ResponsiveStack/\>**.

The smallest view is 0, the largest view is **LevelCount** - 1.

## Choosing a view

When a **\<ResponsiveStack/\>** is required to switch to a larger view (increasing **CurrentLevel** by 1), **\<ResponsiveStack/\>** will find a smallest managed **GuiResponsiveCompositionBase** in size to switch to a larger view. If it fails (usually because it has already switched to its largest view), then **\<ResponsiveStack/\>** will find the next smallest one, until one succeeded.

When a **\<ResponsiveStack/\>** is required to switch to a smaller view (decreasing **CurrentLevel** by 1), **\<ResponsiveStack/\>** will find a largest managed **GuiResponsiveCompositionBase** in size to switch to a smaller view. If it fails (usually because it has already switched to its smallest view), then **\<ResponsiveStack/\>** will find the next largest one, until one succeeded.

**\<ResponsiveStack/\>** measures the size of a **GuiResponsiveCompositionBase** according to **Direction**: - **Horizontal**: size equals to its width. - **Vertical**: size equals to its height. - **Both**: size equals to its area.

