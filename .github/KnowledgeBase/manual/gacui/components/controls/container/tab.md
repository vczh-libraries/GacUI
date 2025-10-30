# \<Tab\> and \<TabPage\>

- **\<Tab/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiTab* - **Template Tag**: \<TabTemplate/\> - **Template Name**: Tab

- **\<TabPage/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiTabPage* - **Template Tag**: \<ControlTemplate/\> - **Template Name**: CustomControl

When a **\<Tab/\>** is deleted, all **\<TabPage/\>** in its **Pages** will also be deleted.

When a **\<TabPage/\>** is activated, only content of this tab page is visible in the **\<Tab/\>**. But inactivated pages still limit the minimum size of the **\<Tab/\>**. The only exception is when an inactivated page has never become activated, since its content has never been rendered, the minimum size of that page is never calculated.

## Patterns

**\<Tab/\>** is a container with multiple labeled pages, each page is a **\<TabPage/\>**. Typically they are created in this pattern: ``` <Tab> <att.Pages> <TabPage Text="Title" Alt="X"> ... </TabPage> ... </att.Pages> </Tab> ``` If an **ALT** sequence is assigned to a tab page, the page will be selected and shown to the user when the **ALT** sequence is hit. Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/Animation/UI/Resource.xml) for more details.

**\<TabPage/\>** is also a **GuiInstanceRootObject**, which means it could also be a base class of an **\<Instance/\>**. It is very useful when you want to put a tab page in a single file, or create components that just for this tab page and other part of the code cannot access these components. Please check out [ this tutorial page ](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/ListControls) for more details.

## \<Tab\> Properties

### Pages

The default value for this property is empty.

This property is an observable collection. When tab pages are added to **Pages**, labels displaying their **Text** property will also be added to the tab control.

If a tab page is added to **Pages** when it is empty, it automatically becomes activated, and **SelectedPage** is set to this tab page. If a tab page is removed from **Pages** and cause it to become empty, **SelectedPage** will also become **null**.

### SelectedPage (SelectedPageChanged)

This property is **null** only when **Pages** is empty.

This property represents the current activated tab page. Only tab pages in **Pages** could be assigned to this property.

## \<TabPage\> Properties

### OwnerTab

This property returns the **\<Tab/\>** that owns this tab page.

When a tab page is added to a tab's **Pages**, **OwnerTab** becomes this tab control. When a tab page is removed from a tab's **Pages**, **OwnerTab** becomes **null**.

## Sample

- Source code: [control_basic_window](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_window/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_basic_window.gif)

