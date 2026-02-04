# Controls

[Controls](../../.././gacui/kb/controls.md)offer a rich set of predefined UI functionalities. Controls organize and visualize data by data binding. Controls define their skins by control templates and item templates. Controls have**BoundsComposition**and**ContainerComposition**to connect with compositions.

Here are predefined controls by category:

## Basic Controls


- GuiControl
  - GuiControlHost
    - **\<Window\>**
      - **GuiPopup**
        - GuiTooltip
  - **\<CustomControl\>**
  - **\<Label\>**
  - **\<Button\>**
    - GuiSelectableButton
      - **\<CheckBox\>**(virtual)
      - **\<RadioButton\>**(virtual)
      - GuiMenuButton
        - GuiComboBoxBase
          - GuiComboBoxListControl
            - **\<DateComboBox\>**
  - GuiScroll
    - **\<HScroll\>**(virtual)
    - **\<VScroll\>**(virtual)
    - **\<HTracker\>**(virtual)
    - **\<VTracker\>**(virtual)
    - **\<ProgressBar\>**(virtual)
  - **\<DatePicker\>**

## Container Controls


- GuiControl
  - \<CustomControl\>
    - **\<TabPage\>**
  - **\<Tab\>**
  - GuiScrollView
    - **\<ScrollContainer\>**
  - **\<GroupBox\>**(virtual)

## Editor Controls


- GuiControl
  - GuiScrollView
    - \<ScrollContainer\>
      - **\<DocumentViewer\>**
        - **\<MultilineTextBox\>**
  - **\<DocumentLabel\>**
    - **\<DocumentTextBox\>**(virtual)
    - **\<SinglelineTextBox\>**

## List Controls


- GuiControl
  - \<Button\>
    - GuiSelectableButton
      - GuiMenuButton
        - GuiComboBoxBase
          - GuiComboBoxListControl
            - **\<ComboBox\>**(virtual)
  - GuiScrollView
    - GuiListControl
      - GuiSelectableListControl
        - GuiVirtialTextList
          - **\<TextList\>**
          - **\<BindableTextList\>**
        - GuiListViewBase
          - GuiVirtualListView
            - **\<ListView\>**
            - **\<BindableListView\>**
            - GuiVirtualDataGrid
              - **\<BindableDataGrid\>**
        - GuiVirtualTreeListControl
          - GuiVirtualTreeView
            - **\<TreeView\>**
            - **\<BindableTreeView\>**

## Toolstrip Controls


- GuiControl
  - GuiControlHost
    - \<Window\>
      - GuiPopup
        - GuiMenu
          - **\<ToolstripMenu\>**
  - GuiMenuBar
    - **\<ToolstripMenuBar\>**
  - **\<ToolstripToolBar\>**
    - **\<ToolstripToolBarInMenu\>**(Virtual)
  - \<Button\>
    - GuiSelectableButton
      - GuiMenuButton
        - **\<ToolstripButton\>**
          - **\<MenuBarButton\>**(virtual)
          - **\<MenuItemButton\>**(virtual)
          - **\<ToolstripDropdownButton\>**(virtual)
          - **\<ToolstripSplitButton\>**(virtual)
  - **\<MenuSplitter\>**(virtual)
  - **\<ToolstripSplitter\>**(virtual)
  - **\<ToolstripSplitterInMenu\>**(virtual)
  - GuiToolstripNestedContainer
    - **\<ToolstripGroupContainer\>**
    - **\<ToolstripGroup\>**

## Ribbon Controls


- GuiControl
  - GuiControlHost
    - \<Window\>
      - GuiPopup
        - GuiMenu
          - \<ToolstripMenu\>
            - **\<RibbonToolstripMenu\>**
  - \<Button\>
    - GuiSelectableButton
      - GuiMenuButton
        - \<ToolstripButton\>
          - **\<RibbonSmallButton\>**(virtual)
          - **\<RibbonSmallDropdownButton\>**(virtual)
          - **\<RibbonSmallSplitButton\>**(virtual)
          - **\<RibbonLargeButton\>**(virtual)
          - **\<RibbonLargeDropdownButton\>**(virtual)
          - **\<RibbonLargeSplitButton\>**(virtual)
  - \<CustomControl\>
    - \<TabPage\>
      - **\<RibbonTabPage\>**
  - \<Tab\>
    - **\<RibbonTab\>**
  - **\<RibbonGroup\>**
  - **\<RibbonIconLabel\>**
    - **\<RibbonSmallIconLabel\>**(virtual)
  - **\<RibbonButtons\>**
  - **\<RibbonToolstrips\>**
  - **\<RibbonGallery\>**
    - **\<BindableRibbonGalleryList\>**
  - **\<RibbonSplitter\>**(virtual)
  - **\<RibbonToolstripHeader\>**(virtual)

