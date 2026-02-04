# \<Xml\>

[Here](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/XmlPage.xml)is the sample used in this topic.

**\<Xml/\>**works like**\<Text/\>**, but there are a few differences:
- It can be assigned to properties that accept**system::xml::XmlDocument^**.

There is no property in predefined GacUI objects to accept an XML, it is designed to store static structured data. Another choice is to use**\<Script\>**and write them in Workflow script.

In the sample, XML is read by a piece of Workflow script, and data are loaded to a list view control. XML helper functions and Linq are not introduced to Workflow, so this piece of code will be much simpler in C++.

First we define an XML resource item in**res://MainWindow/XmlPage/IDE**:
```
<Xml name="IDE">
  <IDEs>
    <IDE name="Visual Studio" dev="Microsoft" />
    <IDE name="Visual Studio Code" dev="Microsoft" />
    <IDE name="XCode" dev="Apple" />
    <IDE name="CLion" dev="JetBrains" />
    <IDE name="C++ Builder" dev="Embarcadero" />
    <IDE name="Eclipse" dev="Eclipse Foundation" />
  </IDEs>
</Xml>
```


And then we define a class to store the data, with a function to read the XML and fill to an observable list.
```
module sharedscript;

using system::*;
using system::xml::*;

namespace sample
{
  class IDE
  {
    prop Name: string = "" {}
    prop Developer: string = "" {}
  }

  func Fill(doc: XmlDocument^, ides: observe IDE^[]): void
  {
    for (nodeIde in doc.rootElement.subNodes)
    {
      var xIde = cast (XmlElement^) nodeIde;
      var oIde = new IDE^();
      for (att in xIde.attributes)
      {
        switch (att.name)
        {
          case "name": { oIde.Name = att.value; }
          case "dev": { oIde.Developer = att.value; }
        }
      }
      ides.Add(oIde);
    }
  }
}
```


In the sample, a variable**var ides: observe IDE^[] = {};**is added to the tab page using**\<ref.Members/\>**. You can write as many class members as you want in this tag. If you need to write something in the constructor or the destructor, you can also use**\<ref.Ctor/\>**or**\<ref.Dtor/\>**, they accept one Workflow statement or a pair of brace with multiple statements.

Now we need to bind this observable list to a**BindableListView**:
```
<BindableListView View="Detail" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false" env.ItemType="sample::IDE^">
  <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
  <att.ItemSource-bind>self.ides</att.ItemSource-bind>

  <att.Columns>
    <ListViewColumn Text="IDE" TextProperty="Name" Size="180"/>
    <ListViewColumn Text="Developer" TextProperty="Developer" Size="240"/>
  </att.Columns>
</BindableListView>
```
There are three important things here:
- **ItemSource**: This property accept any collection type, and list view items will be created in a one-to-one mapping to items in the collection. If it is an observable collection just like this sample, when items are added to or removed from this observable collection, list view items will change automatically.
- **env.ItemType**: The type name of items in**ItemSource**.
- **TextProperty**in**ListViewColumn**: One of a property in**env.ItemType**. For**BindableDataGrid**, if you specify editor templates or visualizer templates to columns, you can represent data in any way you want so that this property does not have to be a string property.

Finally, we create a button. By clicking it, it loads all data to the**ides**observable collection, so that the list view will sense the change and create list view items:
```
<Button ref.Name="buttonLoad" Text="Click Me To Load Data">
  <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
  <ev.Clicked-eval><![CDATA[
    {
      var doc = cast (system::XmlDocument^) self.ResolveResource('res', 'MainWindow/XmlPage/IDE', true);
      Fill(doc, self.ides);
      buttonLoad.Enabled = false;
    }
  ]]></ev.Clicked-eval>
</Button>
```


Here is how it looks like:


- Source code:[kb_xmlres_data](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/XmlPage.xml)
- ![](https://gaclib.net/doc/gacui/kb_xmlres_tag_xml.gif)

