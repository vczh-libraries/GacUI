# Cross XML Resource References

A GacUI XML Resource could[use some resources in another XML](../.././gacui/xmlres/cxrr.md). There are 3 types of resources:
- Resource files: They could be accessed using**import-res://RESOURCE-NAME/...**in the**-uri binding**.
- Objects that compiled to Workflow: They could be accessed using their generated class or interface names.
- Everything else: They could not be accessed from another XML.

## XML Resource Header

To make a GacUI XML Resource be able to depend on or be depended by another resource, a header is required in**GacGenConfig/Metadata**like this:
```
<Resource>
  <Folder name="GacGenConfig">
    <Xml name="Metadata">
      <ResourceMetadata Name="EditorRibbon" Version="1.0">
        <Dependencies>
          <Resource Name="EditorBase"/>
        </Dependencies>
      </ResourceMetadata>
    </Xml>
    ...
  </Folder>
  ...
</Resource>
```

- **/ResourceMetadata/@Name**: The name of the resource. All resources that loaded into an application should either have a unique name or have no name. If a resource want to depend on or to be depended by another resource, it should have a name.
- **/ResourceMetadata/@Version**: The only legal value is**1.0**. It is bounded to the GacUI XML Resource Compiler (GacGen.exe). To ensure that all depended resources are compiled using a compatible version of compiler, an resource could only depend on another resource that has exactly the same version.
- **/ResourceMetadata/Dependencies/Resource/@Name**: Here are all resources that the current resource depends on. The name must match their**/ResourceMetadata/@Name**in their metadata.

## Compile Resource with Dependencies

[GacBuild.ps1](https://github.com/vczh-libraries/Release/tree/master/Tools)is the only tool to properly compile resources with dependencies.

You need to have a file called**GacUI.xml**:
```
<GacUI/>
```
A GacUI XML Resource and all its dependeices must be put inside the folder containing**GacUI.xml**. Resource files could be put in any level of sub folders.

Then you could call**GacBuild.ps1 -FileName path/to/GacUI.xml**in**PowerShell**. All resource files in the folder containing**GacUI.xml**will be found and compiled in the dependency order.

**GacBuild.ps1**forces incremental build. If a resource is not changed, and all its dependencies are not changed, then the resource will not be compiled again. You could use**GacClear.ps1 -FileName path/to/GacUI.xml**to clear all cache, the next time**GacBuild.ps1**will recompile every resource files.

Generated resource binaries, workflow binaries and C++ code will be put in places[specified in each GacUI XML Resource](../.././gacui/xmlres/cgc.md).

## Loading Resource Dependencies in your C++ Application

**vl::presentation::GetResourceManager()-\>LoadResourceOrPending(fileStream[, errors][, usage]);**is the only function to load compiled resource binaries:
- **fileStream**: It could be any[vl::stream::IStream](../.././vlppos/using-streams.md), you could use**FileStream**here to load a resource from file.
- **errors**: If the resource could not be loaded, errors will be stored in this list. An empty error list after calling this function means the resource is loaded properly. If the**errors**argument is not use, then the function will crash if there is any error.
- **usage**: It could be**InstanceClass**if you include all Workflow binaries into this resource file, and**DataOnly**if not. A resource binary containing Workflow binaries could be produced using[res://GacGenCppConfig/ResX86/Resource and res://GacGenCppConfig/ResX/Resource](../.././gacui/xmlres/cgc.md)**The content of Workflow binaries is different for x86 or x64**, you need to load the correct one.Typically, if you use compiled resource with generated C++ files instead of Workflow binaries, you only need the first argument.

You don't need to worry about the order to load resource files, they will be taken care of in this function, just call**LoadResourceOrPending**for all of them.

Calling**LoadResourceOrPending**without all depended resources prepared results in delay loading. Such resource will be automatically loaded after all depended resources are loaded. You could call**GetResource**to see if a resource has already been loaded or not. If it returns**null**but**LoadResourceOrPending**succeeded, it means some depended resources are not prepared yet, and the loading is delayed.

## Get Rid of Resource Files

By using[CppResource or CppCompressed](../.././gacui/xmlres/cgc.md)in**res://GacGenConfig/Cpp**, you will get multiple cpp files for each GacUI XML Resource. Link all these cpp files to your project, resource binaries will be compiled into your executable file. When your application starts, all resources will be properly loaded before**GuiMain**.

## Using Resource in Dependedcies

If there is a resource object**res://path/to/the/resource**in a GacUI XML Resource called**BaseResource**, another GacUI XML Resource depending on it could use**import-res://BaseResource/path/to/the/resource**. Such resource path could be use in**-uri binding**, or in the**ResolveResource**method in all[root UI instance](../.././gacui/xmlres/instance/root_instance.md).

If a resource is compiled to Workflow classes or instances, just use the class or interface names directly. But if the resource is not in the dependency list (**/ResourceMetadata/Dependencies/Resource/@Name**), the class or interface name will not be found when running**GacBuild.ps1**, errors will be generated and the resource cannot be compiled.

