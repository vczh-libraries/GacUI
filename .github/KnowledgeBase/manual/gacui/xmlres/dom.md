# Folders and Resources

A**GacUI XML Resource**begins with a**\<Resource/\>**tag as a root folder. In this root folder, there can be multiple folders and resource items.

Folders and resource items can be stored in other files.

The[TextEditor](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/TextEditor/UI)tutorial is an exmaple of how to create folders in different files:
```
<?xml version="1.0" encoding="utf-8"?>
<Resource>
  <Folder name="GacGenConfig">
    <Folder name="Cpp">
      <Text name="SourceFolder">Source</Text>
      <Text name="Resource">..\..\UIRes\TextEditor.bin</Text>
      <Text name="NormalInclude">..\..\Utilities.h</Text>
      <Text name="Name">Demo</Text>
    </Folder>
  </Folder>
  <Folder name="Images" content="Link">Images\Folder.xml</Folder>
  <Folder name="AboutWindow" content="Link">AboutWindow.xml</Folder>
  <Folder name="FindWindow" content="Link">FindWindow.xml</Folder>
  <Folder name="MainWindow" content="Link">MainWindow.xml</Folder>
</Resource>
```


For creating a folder, use**\<Folder name="NAME-OF-FOLDER"\>\</Folder\>**and put everything inside the tag.

For creating a folder but you want to put the content in another file, use**\<Folder name="NAME-OF-FOLDER" content="Link"\>FOLDER-XML-FILE\</Folder\>**. In this**FOLDER-XML-FILE**, use**\<Folder/\>**as the root element and put everything in it:
```
<?xml version="1.0" encoding="utf-8"?>
<Folder>
  <Image content="File">New.png</Image>
  <Image content="File">NewText.png</Image>
  <Image content="File">NewXml.png</Image>
  <Image content="File">Open.png</Image>
  <Image content="File">Save.png</Image>
  <Image content="File">SaveAs.png</Image>
  <Image content="File">EditCopy.png</Image>
  <Image content="File">EditCut.png</Image>
  <Image content="File">EditDelete.png</Image>
  <Image content="File">EditFind.png</Image>
  <Image content="File">EditPaste.png</Image>
  <Image content="File">EditRedo.png</Image>
  <Image content="File">EditUndo.png</Image>
</Folder>
```


Writing a resource item in another file is just like folders, but you need**content="File"**instead of**content="Link"**.

The[ListControls](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/ListControls/UI)tutorial is another exmaple of how to create resource items in different files:
```
<Resource>
  ...

  <Instance name="MainWindowResource">
    <Instance ref.CodeBehind="false" ref.Class="demo::MainWindow" xmlns:demo="demo::*">
      <Window ref.Name="self" Text="ListControls" ClientSize="x:640 y:480">
        <att.BoundsComposition-set PreferredMinSize="x:640 y:480"/>
        <Tab>
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
          <att.Pages>
            <demo:TextListTabPage Alt="T"/>
            <demo:ListViewTabPage Alt="L"/>
            <demo:TreeViewTabPage Alt="R"/>
          </att.Pages>
        </Tab>
      </Window>
    </Instance>
  </Instance>

  <Instance name="TextListTabPageResource" content="File">TextListTabPage.xml</Instance>
  <Instance name="ListViewTabPageResource" content="File">ListViewTabPage.xml</Instance>
  <Instance name="TreeViewTabPageResource" content="File">TreeViewTabPage.xml</Instance>

  ...
</Resource>
```


There is one important thing, to create an**\<Instance/\>**, there are two nested**\<Instance/\>**tags.

The outside one is to declare a resource item. The**name**attribute is required, and the**content**attribute is optional. When you just want to write everything in the same file, you don't need to**content**attribute. When you want to write**\<Instance/\>**in another file, you must specify**content="File"**.

The inside one is the content of the resource item. When you want to write**\<Instance/\>**in another file, you only need to have to inside**\<Instance/\>**in that file. For example, the content of**TextListTabPage.xml**is:
```
<Instance ref.CodeBehind="false" ref.Class="demo::TextListTabPage">
  <ref.Members>
    <![CDATA[
      var counter : int = 0;
      var itemsToBind : observe MyTextItem^[] = {};
    ]]>
  </ref.Members>
  <TabPage ref.Name="self" Text="TextList">
    ...
  </TabPage>
</Instance>
```


**\<Image/\>**is required to write the content in another file, because they are binary files.

**\<Text/\>**supports both, but since the content of**\<Text/\>**is plain text, so when you specify**content="File"**, the external file will be treated as a text file and everything in that file will become the content.

**\<Xml/\>**supports both, but since the content of**\<Xml/\>**is also Xml, so when you specify**content="File"**, the external file will be treated as an Xml file and everything in that file will become the content.

For any other kinds of resource items, they all have a**resource item tag**and a**resource content tag**. These two tags may or may not have the same name:
```
<Doc name="NAME">
    <Doc">...</Doc>
</Doc>

<Scriρt name="NAME">
    <Workflow">...</Workflow>
</Scriρt>

<Instance name="NAME">
    <Instance>...</Instance>
</Instance>

<InstanceStyle name="NAME">
    <Styles>...</Styles>
</InstanceStyle>

<Animation name="NAME">
    <Gradient>...</Gradient>
</Animation>

<LocalizedStrings name="NAME">
    <LocalizedStrings>...</LocalizedStrings>
</LocalizedStrings>
```


