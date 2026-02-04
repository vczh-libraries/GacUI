# \<InstanceStyle\>

After a**GacUI XML Resource**is converted to a resource binary:
- This resource does not exist in the resource binary.
- **\<Instance/\>**using this**\<InstanceStyle/\>**will copy everything from styles to instances. Styles itself doesn't generate Workflow scripts.

**\<InstanceStyle/\>**is used to make the code looks clean. It adds attributes to instances by matching instance tags using a query language that looks similar to XPath.

**\<InstanceStyle/\>**works on XML code, it doesn't know the semantic about instances.

To apply styles on instances, put the path of the style in the**ref.Styles**property of those instances. Since styles are not included in the resource binary, it means it cannot be imported, so**res://**is the only valid way to use styles.

Multiple paths are separated by ";" in**ref.Styles**. If they are conflicts between styles, the later win. If some properties in instances conflict with styles, instances win. Unless for collection properties, all values are preserved.

## Path

A path represents a query which returns a collection of GacUI object tags. Sometimes properties and events are also written as tags, but they will not be queried, only objects will.

**/NAME**means one level of direct child tag. A query begins from the root tag. For an instance that is a**\<Window/\>**,**/Window**contains the root instance, but**/CustomControl**returns nothing.

**/Window/Stack/StackItem**is also a valid path, which first find the root instance which is a**\<Window/\>**, and then find all direct children which are**\<Stack/\>**, and then find all direct children in**\<Stack/\>**which are**\<StackItem/\>**.

If you use**//**instead of**/**, any indirect or indirect children of that name will be found.

**NAME**could be in these formats:
- **Button**: a tag that looks like**\<Button/\>**or**\<anything:Button/\>**, the namespace doesn't matter.
- *****: any tag that is an object.
- **Button.buttonSubmit**: a tag that looks like**\<Button ref.Name="buttonSubmit"/\>**or**\<anything:Button ref.Name="buttonSubmit"/\>**, the namespace doesn't matter, but the variable name must match.
- ***.buttonSubmit**: a tag with**ref.Name**set to**buttonSubmit**.You can also add**@ATTRIBUTE:**as a prefix of**NAME**, which means this object must be assigned to a property called**ATTRIBUTE**. For example,**//Tab/@Pages:TabPage**only returns all**\<TabPage/\>**that are assigned to the**Pages**property in any**\<Tab/\>**.

The following binary operators are also valid in a path, and you can use**()**to change the precedences:
- **A + B**: Union of A and B.
- **A * B**: Intersection of A and B.
- **A - B**: Objects in A but not in B.
- **A ^ B**: Objects that only in A or only in B, equivalents to**(A + B) - (A * B)*******and**^**has a higher precedence than**+**and**-**, but parentheses are always recommended if you feel confused.

## Sample

In[this sample](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/TextPage.xml), there is an**\<InstanceStyle/\>**:
```
<InstanceStyle name="SharedStyle">
  <Styles>
    <Style ref.Path="//Stack" AlignmentToParent="left:5 top:5 right:5 bottom:5" Padding="5" Direction="Vertical"/>
    <Style ref.Path="//Stack + //StackItem" MinSizeLimitation="LimitToElementAndChildren"/>
    <Style ref.Path="//StackItem/*">
      <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
    </Style>
  </Styles>
</InstanceStyle>
```
A**\<Styles/\>**consists of multiple**\<Style/\>**, any**\<Style/\>**has its own path and properties. In these styles:
- **//Stack**: Any**\<Stack/\>**.
- **//Stack + //StackItem**: Any**\<Stack/\>**and**\<StackItem/\>**.
- **//StackItem/***: Any direct child in**\<StackItem/\>**.Specified properties will be assigned to all objects that described by the path. In this sample, anything that is a direct child of**\<StackItem/\>**, its**BoundsComposition**object will be assigned a property**AlignmentToParent**. So you don't need to write it three times for all three**\<Button/\>**in all three**\<StackItem/\>**

