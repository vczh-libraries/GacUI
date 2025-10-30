# \<Bounds\>

**\<Bounds/\>** is able to set a relative position directly. But usually this composition is used when you just need to have a composition.

Three properties are provided by **\<Bounds/\>**:

## ExpectedBounds

To set a relative position, the default value is 0 for all its components.

**IMPORTANT**: The relative position of a composition does not always listen to the value of **ExpectedBounds**.

**ExpectedBounds** is only a suggestion. If any component of **AlignmentToParent** is not **-1**, the result will be affected accordingly.

The size of **ExpectedBounds** limits the composition's minimum size.

Please check out [this page](../../.././gacui/components/compositions/home.md) for more information.

## AlignmentToParent

**AlignmentToParent** is added for all compositions that can control the relative position by themselves. The position of **\<StackItem/\>** and **\<FlowItem/\>** are completely decided by **\<Stack/\>** and **\<Flow/\>**, **AlignmentToParent** does not exist in these compositions.

Please check out [this page](../../.././gacui/components/compositions/home.md) for more information.

## Sample

- Source code: [layout_bounds](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/layout_bounds/Resource.xml) - ![](https://gaclib.net/doc/gacui/layout_bounds.gif)

