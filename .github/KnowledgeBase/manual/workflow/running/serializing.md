# Serializing and Loading Assemblies

Involved files: - [W05_Lib/W05_Lib.h](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W05_Lib/W05_Lib.h) - [W05_Lib/W05_Lib.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W05_Lib/W05_Lib.cpp) - [W05_Compile/Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W05_Compile/Main.cpp) - [W05_DynamicRun/Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W05_DynamicRun/Main.cpp)

Saving an assembly to a file is very simple: ``` FilePath assemblyPath = FilePath(argv[0]).GetFolder() / L"WorkflowAssembly.bin"; Console::WriteLine(L"Writing the assembly to: " + assemblyPath.GetFullPath()); FileStream fileStream(assemblyPath.GetFullPath(), FileStream::WriteOnly); assembly->Serialize(fileStream); ``` In this piece of code, the compiled Workflow binary is put in the same folder that containing the executable file.

Loading an assembly from a file is as simple as saving: ``` FilePath assemblyPath = FilePath(argv[0]).GetFolder() / L"WorkflowAssembly.bin"; FileStream fileStream(assemblyPath.GetFullPath(), FileStream::ReadOnly); WfAssemblyLoadErrors errors; auto assembly = WfAssembly::Deserialize(fileStream, errors); ``` After you get the assembly, you can load and call functions or classes in the script as usual.

You don't need to call **WfLoadTable** before loading an assembly, so the launching is very fast.

And by not having to run the compiler, you can get rid of the following files: - **VlppParser.h** - **VlppParser.cpp** - **VlppWorkflowCompiler.h** - **VlppWorkflowCompiler.cpp** which makes the executable file even smaller.

