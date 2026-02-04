# Cross XML Resource References

A**GacUI XML Resource**can import types from other resource files. These resource files should satisfy the following conditions:
- They should be able to be discovered under one**GacUI.xml**.
- The resource file being imported must have a name in its metadata.
- The resource file importing others must specify dependencies in its metadata.

These items will be removed from the resource binary and cannot be imported:
- \<InstanceStyle/\>These items can be imported by URI like**import-res://RESOURCE-NAME/path/to/the/resource/item**:
- \<Text/\>
- \<Image/\>
- \<Xml/\>
- \<Doc/\>These items will be removed from the resource binary, but their generated types can be used in resource files that import them:
- \<Script/\>
- \<Instance/\>
- \<Animation/\>
- \<LocalizedStrings/\>

Note: in \<Script/\> only types can be exported, global variables and functions are invisible to other resource files. Names in resource files that are eventually loaded to the same GacUI application process must be unique. All created Workflow types must be unique too.

Here are some examples:
- [DocumentEditor](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditor): A custom control which provides functionalities of rich text document editing and shared UIs.
- [DocumentEditorToolstrip](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorToolstrip): A document editor using toolstrip, with the above custom control being imported.
- [DocumentEditorRibbon](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/DocumentEditorRibbon): A document editor using ribbon, with the above custom control being imported.

