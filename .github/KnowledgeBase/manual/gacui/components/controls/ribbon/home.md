# Ribbon Controls

Ribbon controls are enhanced toolbar controls organized in tab pages.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/DocumentEditorRibbon.xml)for using ribbon controls in GacUI XML Resource.

## Ribbon Application Organizations

An typical ribbon application has a**\<RibbonTab/\>**at the top of the window.**BeforeHeaders**and**AfterHeaders**are two containers in**\<RibbonTab/\>**for extra controls. Usually a button will be put in**BeforeHeaders**to show a home menu.

Inside**\<RibbonTab/\>::Pages**there are multiple**\<RibbonTagPage/\>**.

Inside**\<RibbonTagPage/\>::Groups**there are multiple**\<RibbonGroup/\>**.

Inside**\<RibbonGroup/\>::Items**there could be multiple following controls:
- **\<RibbonSplitter/\>**
- **\<RibbonLargeButton/\>**
- **\<RibbonLargeDropdownButton/\>**
- **\<RibbonLargeSplitButton/\>**
- **\<RibbonButtons/\>**: A group of resizable buttons. The following controls are acceptble in the**Buttons**property:
  - **\<ToolstripButton/\>**
  - **\<ToolstripDropdownButton/\>**
  - **\<ToolstripSplitButton/\>**
  - **\<RibbonIconLabel/\>**: A label with an image. An extra control could be put inside it, which will be displayed after the label. When the space is not enough, the label will disappear, leaving only the image.When the space is far away from enough to display toolstrip buttons, they will be automatically switched to the following themes accordingly:
  - **\<RibbonSmallButton/\>**
  - **\<RibbonLargeButton/\>**
  - **\<RibbonSmallDropdownButton/\>**
  - **\<RibbonLargeDropdownButton/\>**
  - **\<RibbonSmallSplitButton/\>**
  - **\<RibbonLargeSplitButton/\>**
- **\<RibbonToolstrips/\>**: Inside**\<RibbonToolstrips/\>::Groups**there could be multiple**\<ToolstripGroup/\>**containing multiple:
  - **\<ToolstripButton/\>**
  - **\<ToolstripDropdownButton/\>**
  - **\<ToolstripSplitButton/\>**Each group renders a toolbar. When there is no enough space for organizing toolbars in two lines, they will be organizing in three lines.
- `empty list item`
  **\<BindableRibbonGalleryList/\>**: A bindable list control (in theme**RibbonGalleryItemList**) displaying custom items. Items will be organized in groups if**GroupTitleProperty**and**GroupChildrenProperty**are both non-empty.
  The gallery list also has a menu. On the top of the menu there is a larger gallery list. Both**\<RibbonToolstripHeader/\>**and**\<MenuItemButton/\>**could appear in**SubMenu**of the gallery list.
  The**ItemTemplate**in this control must be assigned, or there will be no item but only group headers in the gallery list.

## Sample

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorRibbon/UI)for details about ribbon controls.

