# \<ResponsiveContainer\>

**\<ResponsiveContainer/\>**tells the associated**GuiResponsiveCompositionBase**to switch to another view when**Bounds**is changed.

## \<ResponsiveContainer\>::ResponsiveTarget

When a**GuiResponsiveCompositionBase**is assigned to**ResponsiveTarget**, it is also added as a child composition in \<ResponsiveContainer/\>, with its**AlignmentToParent**set to**left:0 top:0 right:0 bottom:0**, and it switches to the largest view that could fit in the**\<ResponsiveContainer/\>**.

When a**GuiResponsiveCompositionBase**is removed from**ResponsiveTarget**, it is also removed from \<ResponsiveContainer/\>.

## Choosing a view

When a**\<ResponsiveContainer/\>**becomes larger, it will try to tell its**ResponsiveTarget**to switch to a largest view that could fit in the**\<ResponsiveContainer/\>**. If the current view is the largest view, or the next larger view could not fit in the**\<ResponsiveContainer/\>**, it stays with the current view.

When a**\<ResponsiveContainer/\>**becomes smaller, it does the same but try to switch to a smaller view.

If the size of**\<ResponsiveContainer/\>**is decided by the window size, when the window is being resized, it takes effect only when the mouse is released. It doesn't change view when the mouse is still dragging.

