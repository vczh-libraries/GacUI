# Responsive Design Series

Responsive design compositions consist of following compositions: - **\<ResponsiveContainer/\>**: A composition that tells its **ResponsiveTarget** to switch to a different view when the size of the \<ResponsiveContainer/\> is changed. - **GuiResponsiveCompositionBase**: A composition that manages multiple views in order from large to small, it has following sub classes for different way of management: - **\<ResponsiveFixed/\>**: only has one level of view. - **\<ResponsiveView/\>**: allow manually assigned views: - **Views** accepts multiple **GuiResponsiveCompositionBase** in order from large to small as its levels of views. - **SharedControls** accepts multiple controls. When switching views, a **\<ResponsiveShared/\>** could move a referenced shared control from one view to another, keeping all its status (especially for editable controls). - **\<ResponsiveGroup/\>**: the number of its levels of views equals to one direct or indirect child **GuiResponsiveCompositionBase** that has the most levels of views. When it needs to switch to another view, it tells all direct or indirect child **GuiResponsiveCompositionBase** to synchronize to the same level of view. - **\<ResponsiveStack/\>**: the number of its levels of views equals to the sum of levels of views in all direct or indirect child **GuiResponsiveCompositionBase**. When it needs to switch to another view, it tells one indirect child **GuiResponsiveCompositionBase** to switch view, unless all has switched to its largest or the smallest views.

Usually, a **\<ResponsiveContainer/\>** is put inside a window and configured to change its size according to the window size. And then use **\<ResponsiveView/\>**, **\<ResponsiveGroup/\>** and **\<ResponsiveStack/\>** together to control how controls are reorganized to fit in different sizes of the container. finally this **GuiResponsiveCompositionBase** tree will be assigned to **\<ResponsiveContainer/\>**::**ResponsiveTarget** to make the reorganizing automatically happens.

Typically, **\<ResponsiveFixed/\>** will only be put in **\<ResponsiveView/\>**::**Views** when this level of view doesn't have sub levels of views.

## A responsive layout sample

- Source code: [Tutorial/GacUI_Layout/Responsive2/UI/Resource.xml](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Layout/Responsive2/UI/Resource.xml) - ![](https://gaclib.net/doc/gacui/kb_compositions_responsive_2.gif)

There are three level of views: - **View 1**: Buttons with icons and titles on the left side, and video icons on the right side. - **View 2**: Buttons with icons on the left side, and video icons on the right side. - **View 3**: Buttons in a menu on the top side, and video icons on the bottom side.

**View 1** is larger than **View 2** because titles of buttons are hidden.

**View 2** is larger than **View 3** because buttons are hidden. Although the menu of **View 3** increases the height, but video icons are scrollable, the list control could shrink vertically to make spaces for the menu.

## A basic idea to implement this sample

We will easily find that, the difference between **View 1** and **View 2** is button titles. so the button template itself could be a **\<ResponsiveView/\>** which has two **\<ResponsiveFixed/\>** in its **Views** property: - The first (larger) view contains one icon and one title - The second (smaller) view contains only one icon

Now the button becomes responsive, you don't need to create two different view for **View 1** and **View 2** in **\<ResponsiveView/\>**, which is wasting memories and reducing performance. Instead, you could use a **\<ResponsiveGroup/\>** to contain these buttons. Since all buttons have 2 levels of views, so **\<ResponsiveGroup/\>** has 2 levels of views, because all child **GuiResponsiveCompositionBase** will synchronize to the same level of view.

Now **View 1** and **View 2** are represented in one **\<ResponsiveGroup/\>**, we have to create a **\<ResponsiveView/\>** to contain **\<ResponsiveGroup/\>** and **View 3** as its two views. **View 3** is contained in a **\<ResponsiveFixed/\>**, since all compositions in **\<ResponsiveView/\>**::**Views** must be **GuiResponsiveCompositionBase**.

The list control for video icons could be registered in **\<ResponsiveView/\>**::**SharedControls**. When the application switches between views, the list control will be moved to different view, controlled by all **\<ResponsiveShared/\>** in each views.

Finally, the code looks like this (button templates are inlined, which is only for demo, not the way to implement): ``` <Window> <ResponsiveContainer AlignmentToParent="left:0 top:0 right:0 bottom:0"> <att.ResponsiveTarget> <ResponsiveView Direction="Horizontal"> <att.SharedControls> <ScrollContainer ref.Name="videoContainer" ... /> </att.SharedControls> <att.Views> <ResponsiveGroup Direction="Horizontal"> <!-- View 1 and View 2 --> ... <SelectableButtonTemplate> ... <ResponsiveView Direction="Horizontal"> <att.Views> <ResponsiveFixed> <!-- View 1: icon with title    --> </ResponsiveFixed> <ResponsiveFixed> <!-- View 2: icon without title --> </ResponsiveFixed> </att.Views> </ResponsiveView> ... </SelectableButtonTemplate> <SelectableButtonTemplate .../> <SelectableButtonTemplate .../> <SelectableButtonTemplate .../> ... <!-- the place holder for videoContainer in View 1 and View 2 --> <ResponsiveShared Shared-ref="videoContainer"/> ... </ResponsiveGroup> <ResponsiveFixed> <!-- View 3 --> ... <ToolstripMenu .../> ... <!-- the place holder for videoContainer in View 3 --> <ResponsiveShared Shared-ref="videoContainer"/> ... </ResponsiveFixed> </att.Views> </ResponsiveView> </att.ResponsiveTarget> </ResponsiveContainer> </window> ``` - A shared control must appears in **\<ResponsiveView/\>**::**SharedControls** in order to be referenced by multiple **\<ResponsiveShared/\>**. - **Direction="Horizontal"** in the code means the composition responsives to changing of its width.

The code organizes the **GuiResponsiveCompositionBase** as follows: - **\<ResponsiveView/\>** - View[0]: **\<ResponsiveGroup/\>** - **\<ResponsiveView/\>** - View [0]: **\<ResponsiveFixed/\>** (button with icon and title) - View [1]: **\<ResponsiveFixed/\>** (button with icon only) - ... - View[1]: **\<ResponsiveFixed/\>** (button in menu)

When the window is getting smaller, **\<ResponsiveGroup/\>** shrinks all buttons at the same time (because it is a group not a stack) to hide all button titles.

When the window is getting even smaller, **\<ResponsiveGroup/\>** cannot shrinks anymore, so the root **\<ResponsiveView/\>** switches to a second view in **\<ResponsiveFixed/\>** to move all button inside a menu on the top.

