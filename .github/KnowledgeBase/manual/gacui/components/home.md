# GacUI Components

**GacUI**has a rich set of predefined objects in**GacUI XML Resource**.

## Elements

[Elements](../.././gacui/kb/elements.md)offer rendering of geometry shapes and texts.

[Here](../.././gacui/components/elements/home.md)contains detailed introduction for:
- \<FocusRectangle\>
- \<SolidBorder\>
- \<SolidBackground\>
- \<Gui3DBorderElement\>
- \<Gui3DSplitterElement\>
- \<GradientBackground\>
- \<InnerShadow\>
- \<SolidLabel\>
- \<ImageFrame\>
- \<Polygon\>

**presentation::elements::GuiColorizedTextElement**and**presentation::elements::GuiDocumentElement**are not expected to be used directly. Editor controls will create and update these elements in the specified area provided by the control template.

## Compositions

[Composition](../.././gacui/kb/compositions.md)offer layout algorithms based on constraints.

[Here](../.././gacui/components/compositions/home.md)contains detailed introduction for:
- \<Bounds\>
- \<Stack\>, \<RepeatStack\>, \<StackItem\>
- \<Flow\>, \<RepeatFlow\>, \<FlowItem\>
- \<Table\>, \<Cell\>, \<RowSplitter\>, \<ColumnSplitter\>
- \<SharedSizeRoot\>, \<SharedSizeItem\>
- \<SideAligned\>
- \<PartialView\>
- Responsive Design series:
  - \<ResponsiveShared\>
  - \<ResponsiveView\>
  - \<ResponsiveFixed\>
  - \<ResponsiveStack\>
  - \<ResponsiveGroup\>
  - \<ResponsiveContainer\>

## Controls

[Controls](../.././gacui/kb/controls.md)offer a rich set of predefined UI functionalities.

[Here](../.././gacui/components/controls/home.md)contains detailed introduction for:
- Basic Controls
- Container Controls
- Editor Controls
- List Controls
- Toolstrip Controls
- Ribbon Controls

Generally controls are in singly rooted inheritance based on their appearances. Like every controls with scroll bars inherit from**GuiScrollView**.

But there are also**GuiDocumentCommonInterface**for editor controls, offering rendering and editing features.

## Control Templates

[Control Templates](../.././gacui/kb/ctemplates.md)offer customization of control appearances.

[Here](../.././gacui/components/ctemplates/home.md)contains detailed introduction for:
- \<ControlTemplate/\>
  - \<LabelTemplate/\>
  - \<DocumentLabelTemplate/\>
  - \<WindowTemplate/\>
    - \<MenuTemplate/\>
      - \<RibbonToolstripMenuTemplate/\>
  - \<ButtonTemplate/\>
    - \<SelectableButtonTemplate/\>
      - \<ToolstripButtonTemplate/\>
        - \<ListViewColumnHeaderTemplate/\>
        - \<ComboBoxTemplate/\>
          - \<DateComboBoxTemplate/\>
  - \<ScrollTemplate/\>
  - \<ScrollViewTemplate/\>
    - \<DocumentViewerTemplate/\>
    - \<ListControlTemplate/\>
      - \<TextListTemplate/\>
      - \<ListViewTemplate/\>
      - \<TreeViewTemplate/\>
  - \<TabTemplate/\>
    - \<RibbonTabTemplate/\>
  - \<DatePickerTemplate/\>
  - \<RibbonGroupTemplate/\>
  - \<RibbonIconLabelTemplate/\>
  - \<RibbonButtonsTemplate/\>
  - \<RibbonToolstripsTemplate/\>
  - \<RibbonGalleryTemplate/\>
    - \<RibbonGalleryListTemplate/\>

## Item Templates

[Item Templates](../.././gacui/kb/itemplates.md)offer customization of list item appearances.

[Here](../.././gacui/components/itemplates/home.md)contains detailed introduction for:
- \<ListItemTemplate\>
  - \<TextListItemTemplate\>
    - \<TreeItemTemplate\>
- \<GridCellTemplate\>
  - \<GuiGridVisualizerTemplate\>
  - \<GuiGridEditorTemplate\>

## Components

Components offer other features that are not involved in rendering.

[Here](../.././gacui/components/components/home.md)contains detailed introduction for:
- \<MessageDialog\>
- \<ColorDialog\>
- \<FontDialog\>
- \<OpenFileDialog\>
- \<SaveFileDialog\>
- GuiSelectableButton::\<MutexGroupController\>
- \<ToolstripCommand\>

