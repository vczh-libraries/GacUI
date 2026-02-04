# \<ResponsiveFixed\>

**\<ResponsiveFixed/\>**only has one view. If there are some**GuiResponsiveCompositionBase**added as child compositions in**\<ResponsiveFixed/\>**, they are ignored and their views will not change.

**\<ResponsiveFixed/\>**is a**GuiResponsiveCompositionBase**and a**\<Bounds/\>**.

Sometimes you want to just add a static view to**\<ResponsiveView/\>**::**View**, but**Views**requires**GuiResponsiveCompositionBase**.**\<ResponsiveFixed/\>**is designed for this scenario.

## \<ResponsiveFixed\>::Direction

Since**\<ResponsiveFixed/\>**only has one view, this property is ignored.

## \<ResponsiveFixed\>::LevelCount

This property is always 1.

## \<ResponsiveFixed\>::CurrentLevel

This property is always 0.

