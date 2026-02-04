# \<TextListItemTemplate\>


- **\<TextListItemTemplate/\>**
  - **C++/Workflow**: (vl::)presentation::templates::GuiTextListItemTemplate*

**\<TextListItemTemplate/\>**derives from**\<ListItemTemplate/\>**. It has the following properties additional to its base class:

## Input Property

### TextColor

This property is assigned by the list control. It comes from the**TextColor**property of**\<TextListTemplate/\>**that applied to the list control.

The list control does not track the changing of the**TextColor**property of**\<TextListTemplate/\>**. If it is changed,**TextColor**of item templates are not changed automatically. But when a list control is assigned with a new control template,**TextColor**of item templates will be updated as well.

### Checked

This property is assigned by the list control. When the item is checked, this property becomes**true**. When the item is unchecked, this property becomes**false**.

## Sample


- Source code:[Tutorial/GacUI_Controls/ColorPicker/UI/Resource.xml](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/ColorPicker/UI/Resource.xml)
- ![](https://gaclib.net/doc/gacui/kb_itemplates_colorpicker.gif)

