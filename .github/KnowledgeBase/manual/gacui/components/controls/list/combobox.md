# \<ComboButton\> and \<ComboBox\>

- **\<ComboButton/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiComboButton* - **Template Tag**: \<ComboBoxTemplate/\> - **Template Name**: ComboBox - **\<ComboBox/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiComboBoxListControl* - **Template Tag**: \<ComboBoxTemplate/\> - **Template Name**: ComboBox

## \<ComboButton\>

**\<ComboButton/\>** is a button with a drop down storing a user-provided control. When an item in the list control is selected, the button displays the selected item.

In GacUI XML Resource, **DropdownControl** is a required property for **\<ComboButton/\>**. **\<ComboButton/\>** owns this control, it will be deleted when **\<ComboButton/\>** is deleted. **DropdownControl** will be added to the drop down.

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/ColorPicker2/UI/Resource.xml) for details about using **ItemTemplate**.

## \<ComboBox\>

**\<ComboBox/\>** is a button with a drop down storing a user-provided list control. When an item in the list control is selected, the button displays the selected item.

In GacUI XML Resource, **ListControl** is a required property for **\<ComboBox/\>**. **\<ComboBox/\>** owns this list control, it will be deleted when **\<ComboBox/\>** is deleted. **ListControl** will be added to the drop down.

In C++, **ListControl** becomes the second argument in the constructor of **GuiComboBoxListControl**.

The **SelectedIndex (SelectedIndexChanged)**, **SelectedItem** and **ItemProvider** properties always sync with **ListControl**.

The **ItemTemplate (ItemTemplateChanged)** property customizes how the selected item looks like in the **\<ComboBox/\>**. The control template of **\<ComboBox/\>** only renders the selected item as text. When **ItemTemplate** is assigned to a non-empty value, the **TextVisible** property in the control template will be set to **false** so that it doesn't render text any more, and an instance created from **ItemTemplate** will appear in the **\<ComboBox/\>**.

As an item template, the item template class assigned to **ItemTemplate** is required to have exactly one argument in its constructor. In GacUI XML Resource, A constructor argument is a **\<ref.Parameter/\>**. The **SelectedItem** property of the **\<ComboBox/\>** will be assigned to this constructor argument. This is very convenient when **ListControl** is a **\<BindableTextList/\>**, when the item template could be shared in both **\<ComboBox/\>** and **\<BindableTextList/\>** to make the selected item look exactly the same as in the list control.

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/ColorPicker/UI/Resource.xml) for details about using **ItemTemplate**.

