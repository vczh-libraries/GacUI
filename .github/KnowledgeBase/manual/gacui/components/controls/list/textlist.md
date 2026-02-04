# \<TextList\>


- **\<TextList/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiTextList*
  - **Template Tag**: \<TextListTemplate/\>
  - **Item Template Tag**: \<TextListItemTemplate/\>
  - **Template Name**: TextList
  - **Default Item Arranger**: list::FixedHeightItemArranger
  - **Predefined Item Template**:
    - **SetView(Text)**(default): list::DefaultTextListItemTemplate
    - **SetView(Check)**: list::DefaultCheckTextListItemTemplate
    - **SetView(Radio)**: list::DefaultRadioTextListItemTemplate

The**CheckTextListItem**and**RadioTextListItem**themes are for check or radio bullets, which are**\<SelectableButtonTemplate/\>**. But they will be overrides by**CheckBulletTemplate**and**RadioBulletTemplate**in**\<TextListTheme/\>**.

**\<TextList/\>**stores a list of**list::TextItem**and display them. In**list::TextItem**there are**Text**and**Checked**properties, changing these properties also immediately causes changing in the list control.

You can add, remove or change items in the**Items**property to change content in the list control.

The**SelectedItem**property returns the currently selected item in**list::TextItem**. If no item is selected, or multiple items are selected, this property returns**null**.

The**View**property defines all predefined view in**\<TextList/\>**. Valid values are listed at the beginning of this page. You could ignore this property and change**ItemTemplate**and**Arranger**by assigning new values to these properties or calling the protected method**SetStyleAndArranger**to set them at the same time. Doing this makes**View**become**Unknown**.

## Sample


- Source code:[control_list_textlist](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_list_textlist/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_list_textlist.gif)

