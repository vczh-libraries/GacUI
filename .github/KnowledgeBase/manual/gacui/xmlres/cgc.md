# Code Generation Configuration

The configuration is written in the XML as follow:
```
<Resource>
  <Folder name="GacGenConfig">
    <Folder name="Cpp">
      <Text name="SourceFolder">Source</Text>
      <Text name="NormalInclude">GacUI.h</Text>
      <Text name="Name">MyApp</Text>
      <Text name="Resource">..\MyApp.bin</Text>
      ...
    </Folder>
    <Folder name="ResX86">...</Folder>
    <Folder name="ResX64">...</Folder>
    <Xml name="Metadata">...</Xml>
  </Folder>
</Resource>
```


Here are all available options:
- **Cpp**
  - **Name**: Base name of C++ files and contents, could be any valid C++ identifier.
  - **SourceFolder**: Relative folder to this resource file to store generated C++ files.
  - **NormalInclude**: Files to include in headers, typically**GacUI.h**. Multiple files are separated by ";".
  - **ReflectionInclude**(optional): Files to include in headers when reflection is turned on, typically**GacUIReflection.h**. Multiple files are separated by ";".
  - **Resource**(optional): Output path for a neutral resource binary.
  - **Compressed**(optional,**not recommended**): Like**Resource**but it is compressed.
  - **CppResource**(optional): A C++ file to get rid of loading the resource binary at runtime.
  - **CppCompressed**(optional): Like**CppResource**but the binary is compressed to reduce file size.
- **ResX86**or**ResX64**(optional): Control output paths for x86 or x64 resource binary. These options are necessary only when you don't want generated C++ code and decide to load resource binaries at runtime with reflection enabled.
  - **Assembly**: Resource binary without any Workflow scripts.**Reflection**is not required but you need to generate C++ code.
  - **Resource**(optional): Resource binary with UI as Workflow scripts.**Reflection**is required to load this binary file.
  - **Compressed**(optional): Like**Resource**but it is compressed.
- **Metadata**(optional): An xml describing the name of the resource file and its dependencies. Only resources with a metadata can be depended by others. See[Cross XML Resource References](../.././gacui/xmlres/cxrr.md)for details.
```
<Resource>
  <Folder name="GacGenConfig">
    ...
    <Xml name="Metadata">
      <ResourceMetadata Name="MyApp" Version="1.0">
        <Dependencies>
          <Resource Name="OtherLibrary1"/>
          <Resource Name="OtherLibrary2"/>
          ...
        </Dependencies>
      </ResourceMetadata>
    </Xml>
  </Folder>
</Resource>
```


To compile**GacUI XML Resource**files:
- `empty list item`
  put**GacUI.xml**with the following content in the root folder: ``` <GacUI/> ```
- `empty list item`
  put all resource files in their own sub folders.
- `empty list item`
  Open**PowerShell**, navigate to the root folder and run ``` GacBuild.ps1 -FileName GacUI.xml ``` You must use the correct path for**GacBuild.ps1**which is in[here](https://github.com/vczh-libraries/Release/tree/master/Tools).
- `empty list item`
  Some**.xml.log**folders will be create to containe intermediate files and log files, do not submit them to your source control. If there are errors, you can find details there. Otherwise you will get generated files ready in your specified folders.

If you specify**Cpp/CppResource**or**Cpp/CppCompressed**, you don't need to load resource binary before creating the main window. Instead, you add that generated C++ code (cannot add both) to your project, and it will handle the rest for you.

Otherwise, use the following code to load a resource binary before creating any windows:
```
void GuiMain()
{
    {
        FileStream fileStream(L"MyApp.bin", FileStream::ReadOnly);
        GetResourceManager()->LoadResourceOrPending(fileStream);
    }
    // create the main window and launch ...
}
```
**GetApplication()-\>GetExecutableFolder()**is highly recommended to calculate an absolute path to the resource binary, instead of specifying a relative path in C++.

When the resource binary is compressed:
```
void GuiMain()
{
    {
        FileStream fileStream(L"MyApp.bin", FileStream::ReadOnly);
        LzwDecoder decoder;
        DecoderStream decoderStream(fileStream, decoder);
        GetResourceManager()->LoadResourceOrPending(decoderStream);
    }
    // create the main window and launch ...
}
```


