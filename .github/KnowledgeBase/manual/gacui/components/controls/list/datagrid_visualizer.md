# Visualizer

The**VisualizerFactory**property is a**list::IDataVisualizerFactory**. Please check out[this page](../../../.././gacui/xmlres/instance/properties.md)for using this property in GacUI XML Resource.

To create a**list::IDataVisualizerFactory**in C++, it is recommended to use**list::DataVisualizerFactory**as the implementation for the interface. The constructor has two argument. The first argument is an[item template property value](../../../.././gacui/xmlres/instance/properties.md)to create a**GuiGridVisualizerTemplate**. The second argument is a decorated visualizer factory, which is optional.

If the decorated visualizer factory is not**null**, the created visualizer template from the first argument will contain the one created from the decorated visualizer factory.

The default visualizer factory for cells in the first column is:**MainColumnVisualizerTemplate;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate**

This means that, for these cells, a text will be displayed in a focus rectangle (only rendered when a cell has a focus), and the focus rectangle is contained in a border. There is only right and bottom border lines for a cell, but when all cells are rendered together, they form a table.

The default visualizer factory for cells in the other columns is:**SubColumnVisualizerTemplate;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate**

For example, to create the above default visualizer factories:
```
auto mainProperty = [](const Value&) { return new MainColumnVisualizerTemplate; };
auto subProperty = [](const Value&) { return new SubColumnVisualizerTemplate; };
auto focusRectangleProperty = [](const Value&) { return new FocusRectangleVisualizerTemplate; };
auto cellBorderProperty = [](const Value&) { return new CellBorderVisualizerTemplate; };


defaultMainColumnVisualizerFactory = 
    Ptr(new DataVisualizerFactory(cellBorderProperty,
        Ptr(new DataVisualizerFactory(focusRectangleProperty,
            Ptr(new DataVisualizerFactory(mainProperty)
    )))));
defaultSubColumnVisualizerFactory = 
    Ptr(new DataVisualizerFactory(cellBorderProperty,
        Ptr(new DataVisualizerFactory(focusRectangleProperty,
            Ptr(new DataVisualizerFactory(subProperty)
    )))));
```


We create item template property values for all predefined visualizer templates:
- MainColumnVisualizerTemplate
- SubColumnVisualizerTemplate
- FocusRectangleVisualizerTemplate
- CellBorderVisualizerTemplateAnd then combine them together using nested instances of**DataVisualizerFactory**.

**NOTE**: The visualizer order in C++ is the reverse of in GacUI XML Resource.

Please check out[Item Templates](../../../.././gacui/components/itemplates/home.md)for details about**GuiGridVisualizerTemplate**.

## Step-by-Step Sample

In this demo, cell visualizers are applied to the**Author**column and the**Summary**column:
```
<att.AuthorColumn-set>
  <att.VisualizerFactory>HyperlinkVisualizerTemplate;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate</att.VisualizerFactory>
</att.AuthorColumn-set>
<att.SummaryColumn-set>
  <att.VisualizerFactory>sample:SummaryVisualizer;FocusRectangleVisualizerTemplate;CellBorderVisualizerTemplate</att.VisualizerFactory>
</att.SummaryColumn-set>
```


**HyperlinkVisualizerTemplate**is a predefined visualizer to convert a cell to a hyperlink. It only changes the look and the mouse cursor for all cells under a column.

**sample:SummaryVisualizer**is a visualizer defined in the same resource file. It adds an icon to the text, when the mouse hovers above the icon, a tooltip is displayed to show the whole summary.

In GacUI XML Resource,[multiple templates](../../../.././gacui/xmlres/instance/properties.md)could be assigned to the**VisualizerFactory**property to make multiple visualizers nested together. Here**FocusRectangleVisualizerTemplate**and**CellBorderVisualizerTemplate**are used to keep a cell looks like a default one, which could be focused and has a border.


- Source code:[control_datagrid_visualizer](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_visualizer/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_datagrid_visualizer.gif)

