# Project Specific Instruction

## Solution to Work On

You are working on the solution `REPO-ROOT/Test/GacUISrc/GacUISrc.sln`,
therefore `SOLUTION-ROOT` is `REPO-ROOT/Test/GacUISrc`.

## Files not Allowed to Modify

Files in these folders (recursively) are not allowed to modify.
You can only change them using what is described in the `Code Generation Projects` section.
If you encounter any error that prevent these files from being generated,
always fix the root cause.
- `REPO-ROOT/Test/Resources/Metadata`
- `REPO-ROOT/Test/GacUISrc/Generated_DarkSkin`
- `REPO-ROOT/Test/GacUISrc/Generated_Dialogs`
- `REPO-ROOT/Test/GacUISrc/Generated_FullControlTest`
- `REPO-ROOT/Test/GacUISrc/Generated_RemoteProtocolTest`
- `REPO-ROOT/Test/GacUISrc/Generated_UnitTestViewer`
- `REPO-ROOT/Source/Utilities/FakeServices/Dialogs/Source`
- `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/Source`
- `REPO-ROOT/Source/Compiler/InstanceQuery/Generated`
- `REPO-ROOT/Source/Compiler/RemoteProtocol/Generated`

Files in `REPO-ROOT/Import` and `REPO-ROOT/Release` (recursively) are also not allowed to modify.
These files are prepared for foreign dependencies.

## Reflectable Types

- You must be really careful when changing any interface, especially structs, classes, unions and a few functions.
- Check if the class and the method is registered in reflection.
- Reflection allow registering normal functions into a class, becoming its static functions. It is not easy to determine by the function definition itself.
- You must read the knowledge base about reflection and try to find the pattern in any *.cpp file.
- If the reflection registration is affected, you should always fix the reflection and run necessary code generation projects.

## Projects for Verification

You are required to follow the guideline to run any project in this solution,
do not run the compiled binary directly.

The `REPO-ROOT/Test/GacUISrc/UnitTest/UnitTest.vcxproj` is the unit test project.
When any *.h or *.cpp file is changed, unit test is required to run.
Except for the `GuiRemoteRendererSingle` class which is not covered in the unit test.

When any test case fails, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.

For any GacUI specific unit test that running with the GacUI unit test framework,
when it calls `GacUIUnitTest_StartFast_WithResourceAsText` with path, for example, `Application/Windows/Order`,
running it ends up creating log files in `REPO-ROOT/Test/Resources/UnitTestSnapshots/Application/Windows`:
- Order.json: an entry of snapshots for this test case
- Order[*].*: log files about remote protocol recordings and compiler output
- Order/Frame_*.json: snapshot of the UI DOM tree for each frame.

Each Frame_*.json is captured at each `OnNextIdleFrame` call, recording what the UI look like before running the code in this frame.
This is the reason why the name of the frame should say what the previous frame was done,
so that frame names in snapshot files make sense.

### Code Generation Tools

#### REPO-ROOT/../Tools/Tools/GlrParserGen.exe

This executable needs to run if any file in the following folders are changed:
- `REPO-ROOT/Source/Compiler/InstanceQuery/Syntax`
- `REPO-ROOT/Source/Compiler/RemoteProtocol/Syntax`

There is a `Parser.xml` file in these folder.
You need to offer the absolute path of `Parser.xml` to the tool as a command-line argument.
Only run necessary `Parser.xml` in folders that are changed.

### Code Generation Projects

Code generation projects are CLI projects.
They are required to run when a certain set of files are changed,
in order to generate code paring with them.
Here are a list of projects to run and files that should trigger them:

#### Metadata_Generate and Metadata_Test

These two projects need to run if any reflection code is touched:
- `GuiReflection*.cpp` is updated.
- `GacUI_Compiler` project is executed.

To execute these projects, you should:
- Build the solution with Debug|Win32.
- Run `Metadata_Generate` with Debug|Win32.
- Build the solution with Debug|x64.
- Run `Metadata_Generate` with Debug|x64.
- Run `Metadata_Test` with Debug|x64.

It generates binary metadata files containing type informations from reflection code.
This step cannot be skipped after changing any reflection code,
because GacUI_Compiler and some other test applications consume these binary metadata files.

#### Metadata_UpdateProtocol

This project need to run if `REPO-ROOT/Source/PlatformProviders/Remote/Protocol/*.txt` is updated.

It generates `REPO-ROOT/Source/PlatformProviders/Remote/Generated/*`.

#### GacUI_Compiler

This project need to run if any of the following XML file is updated:
- `REPO-ROOT/Source/Utilities/FakeServices/Dialog/*.xml` -> generates `REPO-ROOT/Source/Utilities/FakeServices/Dialog/Source/*`.
- `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/*.xml` -> generates `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/Source/*`.
- `REPO-ROOT/Test/Resources/App/DarkSkin/*.xml` -> generates `REPO-ROOT/Test/GacUISrc/Generated_DarkSkin/Source_(x86|x64)/*`.
  - **IMPORTANT**: `REPO-ROOT/Source/Skins/DarkSkin` has another copy, this is used by the CI and is not involved in this solution, ignore it.
- `REPO-ROOT/Test/Resources/App/FullControlTest/*.xml` -> generates `REPO-ROOT/Test/GacUISrc/Generated_FullControlTest/Source_(x86|x64)/*`.
- `REPO-ROOT/Test/Resources/App/RemoteProtocolTest/*.xml` -> generates `REPO-ROOT/Test/GacUISrc/Generated_RemoteProtocolTest/Source_(x86|x64)/*`.

After running `GacUI_Compiler`, you should always `git status` to find if there is any untracked `*.UI.errors.txt`.
- Such file means there are compile errors in some xml files, read it to find the detail.
- You don't need to delete the file, if `GacUI_Compiler` succeeds the next time, they will be gone.
- `GacUI_Compiler` may also fail by printing one line of error message or return non-zero exit code. If the `*.UI.errors.txt` file does not exist, you are recommended to debug the project to find out what happened.
- Whenever `GacUI_Compiler` reports any error, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.

## Linux Specific

`REPO-ROOT/Test/Linux` stores linux configurations for:
- `Metadata_Generate`: `Metadata_Generate.vcxproj`.
- `Metadata_Test`: `Metadata_Test.vcxproj`.
- `CppTest`: `CppTest.vcxproj`.
- `CppTest_Metaonly`: `CppTest_Metaonly.vcxproj`.
- `CppTest_Reflection`: `CppTest_Reflection.vcxproj`.
- `GacUI_Compiler`: `GacUI_Compiler.vcxproj`.
- `UnitTest`: `UnitTest.vcxproj`.

`Metadata_UpdateProtocol` is not included. If it is needed, create it and remove this line.

You need to build, test and debug in that specific folder, otherwise the unit test will not function properly.
On Linux, only configuration "debug x64" is available, no need to build or run projects with other configurations.
Unlike Windows, building have to be done in each folder separately.
