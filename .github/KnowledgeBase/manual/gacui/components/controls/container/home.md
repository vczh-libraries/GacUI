# Container Controls

Unlike compositions that only layout multiple other objects in groups, container controls offer more features in user interaction.

**\<GroupBox/\>** is the simplest container composition which displays its **Text** with its content. Controls serving a same purpose could be grouped in a **\<GroupBox/\>**.

**\<ScrollContainer/\>** is a container with built-in scroll bars. When the size of the container is too small to display all its content, scroll bars will be enabled or shown.

Scroll bars are provided in **GuiScrollView**, which is the base class of **\<ScrollContainer/\>** and also all other list controls and some editor controls. **HorizontalAlwaysVisible** and **VerticalAlwaysVisible** properties also exist in list controls and some editor controls, so that their scroll bars can be optionally hidden when list items are not too many.

**\<Tab/\>** is a container of multiple **\<TabPage/\>**. Differet controls can be organized in different **\<TabPage/\>** in one **\<Tab/\>**. When a **\<TabPage/\>** is activated by the user, only contents in this page are visible, contents in other pages are hidden but still contribute to the minimum size of the **\<Tab/\>**.

