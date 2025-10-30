# Control Templates

Control templates change controls' outfit, and they are working based on data bindings.

Controls have their own control template types. For example, **SelectableButton** needs **SelectableButtonTemplate**. Any derived classes of **SelectableButtonTemplate** is acceptable, but extra members will be ignored.

A control template has input properties and output properties. Input properties are for data binding, when the control's state is changed, these properties will be changed, and the control template reacts to these properties and changes how it looks, for example, some colors. Output properties are configuration to the control's outfit related behavior, almost all of them will only be read once, so changing them at runtime takes no effect.

Here we demo how to write a control template for **SelectableButton**. When the mouse is hovering on the button, it becomes brighter. When the button is selected, it becomes less brighter.

These are properties in **ControlTemplate**: - **Input Properties:** - **Focused**: True when the control is focused. - **VisuallyEnabled**: Trhe when the control is enabled. It requires all parent controls to be enabled. - **Font**: The control's font. If a font is not associated to the control, it will be the font from a parent control. - **Context**: The control's **Context** property. - **Text**: The control's **Text** property. - **Output Properties:** - **ContainerComposition**: A composition for control's children, anything put in \<SelectableButton\>\</SelectableButton\> will be in this composition. - **FocusableComposition**: A composition for receiving keyboard events. It works only when the control is focusable.

These are properties in **ButtonTemplate** and **SelectableButtonTemplate**: - **Input Properties:** - **State**: The button's state, can be **Normal**, **Active** or **Pressed**. - **Selected**: True when the button is selected.

We can use all of them to create a control template for **SelectableButton**: ``` <Instance ref.Class="sample::TabHeaderButtonTemplate"> <SelectableButtonTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren"> <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0"> <SolidBackground> <att.Color-bind><![CDATA[ cast Color ( not self.VisuallyEnabled ? "#00000000" : self.State == ButtonState::Pressed ? "#3D3D3D" : self.State == ButtonState::Active ? "#808080" : self.Selected ? "#3D3D3D" : "#00000000" ) ]]></att.Color-bind> </SolidBackground> </Bounds> <Bounds AlignmentToParent="left:0 top:0 right:0 bottom:0"> <att.Visible-bind>self.Focused</att.Visible-bind> <FocusRectangle/> </Bounds> <Bounds AlignmentToParent="left:1 top:1 right:1 bottom:1" MinSizeLimitation="LimitToElement"> <SolidLabel Text-bind="self.Text" Font-bind="self.Font"> <att.Color-bind><![CDATA[ cast Color ( self.VisuallyEnabled ? "#FFFFFF" : "808080" ) ]]></att.Color-bind> </SolidLabel> </Bounds> </SelectableButtonTemplate> </Instance> ```

And then a window containing three check boxes with this control template: ``` <Instance ref.Class="sample::MainWindow" ref.Styles="res://MainWindow/SharedStyle"> <Window Text="kb_ctemplates" IconVisible="true" ClientSize="x:640 y:480"> <att.BoundsComposition-set PreferredMinSize="x:480 y:320"/> <Stack Direction="Vertical" MinSizeLimitation="LimitToElementAndChildren" AlignmentToParent="left:5 top:5 right:-1 bottom:-1" Padding="5"> <StackItem><CheckBox Text="Button 1"/></StackItem> <StackItem><CheckBox Text="Button 2"/></StackItem> <StackItem><CheckBox Text="Button 3"/></StackItem> </Stack> </Window> </Instance> ```

With a style to assign control templates to all check boxes: ``` <Styles> <Style ref.Path="//CheckBox"> <att.ControlTemplate>sample::TabHeaderButtonTemplate</att.ControlTemplate> <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/> </Style> </Styles> ```

Finally we get:

![](https://gaclib.net/doc/gacui/kb_ctemplates_button.gif)

With this new control template, we can create a more complex control template for tab control.

**Tab** needs **TabTemplate**, and it has two important properties: **Commands** and **TabPages**. These are all input properties. **Commands** is an interface with a method **ShowTab** for activate a tab page. **TabPages** is an observable collection for all tab pages at runtime.

By binding **TabPages** to a **RepeatStack**, we can easily create a list of buttons for each tab pages. When tab pages are inserted to or removed from the tab control, the list of buttons will be automatically refreshed.

Firstly we create a control template for **Tab**: ``` <Instance ref.Class="sample::TabTemplate"> <TabTemplate ref.Name="self" ContainerComposition-ref="container" TabOrder="TopToBottom" MinSizeLimitation="LimitToElementAndChildren"> <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren"> <att.Rows> <CellOption>composeType:Percentage percentage:1.0</CellOption> </att.Rows> <att.Columns> <CellOption>composeType:MinSize</CellOption> <CellOption>composeType:Percentage percentage:1.0</CellOption> </att.Columns> <Cell Site="row:0 column:0"> <RepeatStack ref.Name="header" AlignmentToParent="left:0 top:0 right:0 bottom:0" Padding="2" Direction="Vertical" MinSizeLimitation="LimitToElementAndChildren"> <att.ItemTemplate>sample::TabHeaderTemplate</att.ItemTemplate> <att.ItemSource-bind>self.TabPages</att.ItemSource-bind> <ev.ItemInserted-eval> <![CDATA[ { var stackItem = header.StackItems[arguments.itemIndex]; var item = cast (TabHeaderTemplate*) stackItem.Children[0]; item.Commands = self.Commands; } ]]> </ev.ItemInserted-eval> </RepeatStack> </Cell> <Cell Site="row:0 column:1"> <SolidBackground Color="#3D3D3D"/> <Bounds ref.Name="container" AlignmentToParent="left:1 top:1 right:1 bottom:1"/> </Cell> </Table> </TabTemplate> </Instance> ``` In this control template, we create a list of buttons using **RepeatStack** on the left side, and create a container for tab page contents on the right side.

The item template in **RepeatStack** is named **sample::TabHeaderTemplate**, which is: ``` <Instance ref.Class="sample::TabHeaderTemplate"> <ref.Parameter Name="CurrentTabPage" Class="presentation::controls::GuiTabPage"/> <ref.Members> <![CDATA[ prop Commands : ITabCommandExecutor* = null {} ]]> </ref.Members> <ControlTemplate ref.Name="self" MinSizeLimitation="LimitToElementAndChildren"> <CheckBox AutoFocus="false" AutoSelection="false"> <att.ControlTemplate>sample::TabHeaderButtonTemplate</att.ControlTemplate> <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/> <att.Context-ref>CurrentTabPage</att.Context-ref> <att.Text-bind>CurrentTabPage.Text</att.Text-bind> <att.Alt-bind>CurrentTabPage.Alt</att.Alt-bind> <att.Selected-bind>CurrentTabPage.OwnerTab.SelectedPage == CurrentTabPage</att.Selected-bind> <ev.Clicked-eval> <![CDATA[ { self.Commands.ShowTab(CurrentTabPage.OwnerTab.Pages.IndexOf(CurrentTabPage), true); } ]]> </ev.Clicked-eval> </CheckBox> </ControlTemplate> </Instance> ```

**sample::TabHeaderTemplate** has a **Commands** property, it needs to be assigned in **sample::TabTemplate**, so that when a tab header is clicked, it can tell the tab control to switch to that tab.

**sample::TabTemplate** reacts to **ItemInserted** event in **RepeatStack**, by accessing **arguments.itemIndex** which is provided by the event, it knows a new item template is created, and grab that item template to set the **Commands** property.

This ensures that whenever a new item template for the tab header appears in the tab control, the **Commands** property has already been assigned.

In **RepeatStack** there is **\<att.ItemSource-bind\>self.TabPages\</att.ItemSource-bind\>**. This code binds **TabPages** to **RepeatStack**. And **\<att.ItemTemplate\>sample::TabHeaderTemplate\</att.ItemTemplate\>** tells it to create **sample::TabHeaderTemplate** for each tab page.

The assigned tab page for a particular **sample::TabHeaderTemplate** will be in **ref.Parameter**, which is a readonly property and constructor argument.

Knowledge base for item templates will be introduces in the next chapter.

Finally, we create a window with three tab pages to see how it works: ``` <Folder name="MainWindow"> <InstanceStyle name="SharedStyle"> <Styles> <Style ref.Path="//Stack" Direction="Vertical" MinSizeLimitation="LimitToElementAndChildren"/> <Style ref.Path="//RadioButton" GroupController-ref="groupDirection"/> <Style ref.Path="//TabPage"> <att.ContainerComposition-set InternalMargin="left:10 top:10 right:10 bottom:10"/> </Style> </Styles> </InstanceStyle> <Instance name="MainWindowResource"> <Instance ref.Class="sample::MainWindow" ref.Styles="res://MainWindow/SharedStyle" xmlns:x="presentation::controls::GuiSelectableButton::*"> <Window Text="kb_ctemplates" IconVisible="true" ClientSize="x:640 y:480"> <att.BoundsComposition-set PreferredMinSize="x:480 y:320"/> <x:MutexGroupController ref.Name="groupDirection"/> <Tab> <att.ControlTemplate>sample::TabTemplate</att.ControlTemplate> <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/> <att.Pages> <TabPage Text="First TabPage"> <Button Text="A button in the first page"/> </TabPage> <TabPage Text="Second TabPage"> <Stack> <StackItem><CheckBox Text="A check box in the second page"/></StackItem> <StackItem><CheckBox Text="A check box in the second page"/></StackItem> <StackItem><CheckBox Text="A check box in the second page"/></StackItem> </Stack> </TabPage> <TabPage Text="Third TabPage"> <Stack> <StackItem><RadioButton Text="A radio button in the third page"/></StackItem> <StackItem><RadioButton Text="A radio button in the third page"/></StackItem> <StackItem><RadioButton Text="A radio button in the third page"/></StackItem> </Stack> </TabPage> </att.Pages> </Tab> </Window> </Instance> </Instance> </Folder> ```

Here we have a button for the first tab page, three check boxes for the second tab page, and three radio buttons for the third tab page. Each radio button is assigned the same group controller so that they know they are exclusive to each other. By assigning different group controllers to radio buttons, it is very easy to tell these radio buttons who are their allies.

Finally we get:

- Source code: [kb_ctemplates](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_ctemplates/Resource.xml) - ![](https://gaclib.net/doc/gacui/kb_ctemplates_tab.gif)

