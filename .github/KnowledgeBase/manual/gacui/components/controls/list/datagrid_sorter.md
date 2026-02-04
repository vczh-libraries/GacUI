# Sorter

A sorter is an instance of interface**list::IDataSorter**.

When a sorter is installed to the data grid,**SetCallback**will be called.**GetItemProvider**in the callback object provides access to the**ItemProvider**of the data grid.**OnProcessorChanged**in the callback object is called by the sorter, when the sorter definition is changed.

The**Compare**function accepts two items in**ItemSource**. It returns a negative number when**rows1**should appear before**rows2**in ascending order. It returns a positive number when**rows1**should appear after**rows2**in ascending order. It returns 0 when the order is not important.

## Implementing a Sorter

Typically you need to implement the interface directly.

## Predefined Sorter Combinators

**list::DataMultipleSorter**: Combine two sorters into one sorter. If the**LeftSorter**decides an order for two items, that's the order. If the**LeftSorter**decides that the order is not important, the**RightSorter**kicks in.

**list::DataReverseSorter**: This sorter defines a revert order based on another sorter.

## Installing Sorters

Multiple sorters could be assigned to each columns, but only one sorter could be activated. To activate or cancel a sorter, just click the column header, or request the**list::IDataGridView**view from the**ItemProvider**and there will be methods for controlling sorting.

## Step-by-Step Sample

In this demo, sorters are applied to the first three columns:
```
<att.TitleColumn-set Sorter="[Sys::Compare($1.Title, $2.Title)]"/>
<att.AuthorColumn-set Sorter="[Sys::Compare($1.Author, $2.Author)]"/>
<att.DatePublishedColumn-set Sorter="[Sys::Compare($1.DatePublished, $2.DatePublished)]"/>
```


Each[sorter function](../../../.././gacui/xmlres/instance/properties.md)is a lambda expression,**$1**and**$2**are the two arguments of this function. argument types are specified by**env.ItemType**, just like filter functions.

Using**-eval**on the**Sorter**property means an**IDataSorter^**is assigned to the property. Setting**Sorter**without any binder means a sorter function is assigned to the property, in this case an**IDataSorter^**is automatically created from this assigned function.


- Source code:[control_datagrid_sorter](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_datagrid_sorter/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_datagrid_sorter.gif)

