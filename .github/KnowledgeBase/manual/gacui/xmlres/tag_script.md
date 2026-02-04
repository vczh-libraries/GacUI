# \<Script\>

After a**GacUI XML Resource**is converted to a resource binary:
- This resource does not exist in the resource binary.
- The equivalent Workflow script will be included in the Workflow assembly part in the resource binary.
- If you use the one specified in**GacGenConfig/Cpp**, the Workflow assembly is also not included in the resource binary, because you are supposed to use the generated C++ code.

The[sample](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_xmlres_data/XmlPage.xml)for**\<Xml/\>**also uses**\<Script/\>**.

Currently only Workflow script is the only supported script language in**GacUI XMl Resource**. To include a[Workflow module](../.././workflow/lang/module.md)in the resource, we need to include the code in a**CDATA**section like this:
```
<Scriρt name="RESOURCE-NAME">
  <Workflow><![CDATA[
    /* put the whole module here *./
  ]]></Workflow>
</Scriρt>
```


