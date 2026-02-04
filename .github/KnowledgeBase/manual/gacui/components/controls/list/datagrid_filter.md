# Filter

A filter is an instance of interface**list::IDataFilter**.

When a filter is installed to the data grid,**SetCallback**will be called.**GetItemProvider**in the callback object provides access to the**ItemProvider**of the data grid.**OnProcessorChanged**in the callback object is called by the filter, when the filter definition is changed (e.g. a the date range is changed in a filter that only allow items in that range).

The**Filter**function accepts an item from**ItemSource**, it returns**true**when the item satisfies the filter.

## Implementing a Filter

Typically you need to implement the interface directly.

When a filter is semantically designed for a column, a filter could be implemented by a column dropdown. Usually this is done by creating a**\<CustomControl/\>**or a**\<Popup/\>**for the UI in the column dropdown. The control has a**list::IDataFilter**property, whose the implementation talks to the control for filter arguments. In the end, install the filter to both**Filter**and**Popup**in**list::DataColumn**.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/DataGrid/UI/Resource.xml)for details.

## Predefined Filter Combinators

**list::DataAndFilter**: Combine multiple filters to one filter. An item satisfies this filter when it satisfies all sub filters.

**list::DataOrFilter**: Combine multiple filters to one filter. An item satisfies this filter when it satisfies at least one sub filters.

**list::DataNotFilter**: Convert one filter to another. An item satisfies this filter when it unsatisfies the sub filter.

## Installing Filters

Multiple filters could be assigned to each column. A filter could be assigned to the control using the**AdditionalFilter**property. An item must satisfies**AdditionalFilter**add every filter assigned to columns in order to appear in the control.

## Step-by-Step Sample

In this demo, a control is created for editing a date range using two**\<DateComboBox/\>**, each with a**\<CheckBox/\>**for controlling the condition availability.

In the constructor of this control, a filter is created. It reads the date range from the control to filter a row object (which is**BookItem**).

In the derived main window, the following configuration is added to the**DatePublishedColumn**. Please check out the base class**sample::MainWindow**for details about how a**\<DateColumn/\>**is exposed as a property.
```
<att.DatePublishedColumn-set>
  <att.Filter-eval>dateFilter.Filter</att.Filter-eval>
  <att.Popup>
    <ToolstripMenu>
      <sample:DateFilter ref.Name="dateFilter">
        <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
      </sample:DateFilter>
    </ToolstripMenu>
  </att.Popup>
</att.DatePublishedColumn-set>
```
Setting the**Popup**property doesn't automatically apply the filter, the**Filter**property is still needed. Using**-eval**on the**Filter**property means an**IDataFilter^**is assigned to the property. Setting**Filter**without any binder means a[filter function](../../../.././gacui/xmlres/instance/properties.md)is assigned to the property, in this case an**IDataFilter^**is automatically created from this assigned function.


- Source code:[control_datagrid_filter](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_filter/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_datagrid_filter.gif)

