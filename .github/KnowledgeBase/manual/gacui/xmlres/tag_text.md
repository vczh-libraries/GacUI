# \<Text\>

[Here](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/TextPage.xml) is the sample used in this topic.

**\<Text/\>** is a resource that contains **string**. In **\<Instance/\>** it can be used in the following ways: - **res://path/to/the/resource/item** - **import-res://RESOURCE-NAME/path/to/the/resource/item****RESOURCE-NAME** should be a resource that is specifies in one of a **/ResourceMetadata/Dependencies/Resource/@Name** in **res://GacGenConfig/Metadata**.

In the sample there are three text resources: ``` <Resource> <Folder name="MainWindow"> <Folder name="TextPage"> <Text name="TextResource1">This is the first text resource.</Text> <Text name="TextResource2">the second</Text> <Text name="TextResource3">the third text resource.</Text> </Folder> </Folder> </Resource> ```

**TextResource1** is used in: ``` <Button Text-uri="res://MainWindow/TextPage/TextResource1"/> ``` **-uri** is one of a [data binding](../.././gacui/advanced/bindings.md) operator, which pull the resource by a path and assign to the property. A **\<Text/\>** resource can be assigned to any property that accepts **presentation::GuiTextData^** or **string**.

Here the button's **Text** property becomes the content of this resource.

**TextResource2** is used in: ``` <Button Text-format="This is $((cast (TextData^) self.ResolveResource('res', 'MainWindow/TextPage/TextResource2', true)).Text) text resource."/> ``` **-format** is one of a [data binding](../.././gacui/advanced/bindings.md) operator, which evaluates all Workflow expressions in **$()** and assign the whole result to the property. If any expression [references observable properties](../.././workflow/lang/bind.md), when those property is changed, **-format** will evaluate the whole thing and update the property automatically.

This example consist of **"This is $()" text resource.** and an expression: ``` (cast (TextData^) self.ResolveResource('res', 'MainWindow/TextPage/TextResource2', true) ).Text ``` **self** is the tab page because there is **ref.Name="self"** in **\<TabPage/\>**. **ResolveResource** uses the resource binary containing this tab page as a reference. If the first argument is **'res'**, it finds the path in the current resource. If the first argument is **'import-res'** and the second argument is **'RESOURCE-NAME/path/to/the/resource/item'** it finds the path in a loaded resource binary named **RESOURCE-NAME**.

This function returns **system::ReferenceType^**, but for **\<Text/\>** it is actually a **presentation::GuiTextData^**. Since by default there is already an implicit **using presentation::*;**, so **TextData^** becomes the type name.

The content of **TextResource2** is **the second**, and there is no observable property in this data binding, so it is equivalent to **-format="This is $('the second') text resource."**.

**TextResource1** is used in: ``` <Button ref.Name="button3" Text="Click Me!"> <ev.Clicked-eval> <![CDATA[ { var textRes = (cast (TextData^) self.ResolveResource('res', 'MainWindow/TextPage/TextResource3', true)).Text; button3.Text = $"This button shows $(textRes)!"; } ]]> </ev.Clicked-eval> </Button> ``` Obviously, by clicking the button, it updates the itself.

Here is how it looks like:

- Source code: [kb_xmlres_data](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/TextPage.xml) - ![](https://gaclib.net/doc/gacui/kb_xmlres_tag_text.gif)

