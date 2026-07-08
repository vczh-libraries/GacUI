# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

You are going to revert this small piece, run GacUI_Compiler and figure out why that error happens. I think it might due to Win32 build fails and it does not do the cleaning, try to confirm the cause. If that is the case, the correct fix would be unload all fives before compiling the first one, therefore no ReloadResource is needed.

# TEST [CONFIRMED]

Revert the per-resource `ReloadResource` change in `GacUI_Compiler` back to plain `LoadResource`, then build and run `GacUI_Compiler` through the repository scripts.

The investigation succeeds when:
- the duplicate-resource-name failure is reproduced or disproved with the reverted compiler path;
- the failure location and resource-manager state are identified;
- the suspected cause is confirmed or denied, specifically whether a failed Win32 compile leaves the five generated resources loaded before the next compile attempt;
- the final fix does not use per-resource `ReloadResource`;
- `GacUI_Compiler` succeeds without leaving `*.UI.error.txt` or `*.UI.errors.txt`;
- the solution builds and unit tests pass after the final change.

Confirmed with the reverted compiler path:
- `copilotBuild.ps1` succeeds for Debug|x64 after reverting `ReloadResource`;
- `copilotExecute.ps1 -Mode CLI -Executable GacUI_Compiler` selects the Debug|x64 compiler and fails on the first generated resource, `GuiFakeDialogServiceUI`, with `GuiResourceManager::SetResource(Ptr<GuiResource>, GuiResourceUsage)#A resource with the same name has been loaded.`;
- the failure is not caused by a Win32 compile leaving resources behind, because the failure reproduces in a fresh Debug|x64 process before the first resource can be loaded manually;
- `GacUI_Compiler.vcxproj` imports `Source_GacUI_Utilities_Controls.vcxitems`, which compiles `Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUIResource.cpp`;
- that generated resource loader plugin loads `GuiFakeDialogServiceUI` during plugin initialization before `GacUI_Compiler::GuiMain()` starts compiling resources.

# PROPOSALS

- No.1 Clean preloaded generated resources at compiler entry [CONFIRMED]

## No.1 Clean preloaded generated resources at compiler entry

`GacUI_Compiler` should keep the normal `LoadResource(CompileResources(...))` call shape for each resource. The duplicate-resource failure is caused by generated resource loader plugins linked into the compiler executable, not by repeated loading inside the compile loop.

Before compiling the first resource, `GuiMain()` should unload the five generated resource names owned by the compiler run:
- `GuiFakeDialogServiceUI`;
- `GuiUnitTestSnapshotViewer`;
- `DarkSkin`;
- `FullControlTest`;
- `RemoteProtocolTest`.

This makes the compiler entry boundary responsible for clearing preloaded generated resources once, while preserving `GuiResourceManager::SetResource` as a fail-fast invariant during the actual compile/load sequence.

### CODE CHANGE

- Revert the per-resource `ReloadResource` helper from `GacUI_Compiler`.
- Restore every generated resource load call to `LoadResource(CompileResources(...))`.
- Add a one-time cleanup at the beginning of `GacUI_Compiler::GuiMain()` that unloads the five generated resource names before the first compile.

### CONFIRMED

The suspected Win32-cleanup cause was denied. The duplicate-resource failure happens in a fresh Debug|x64 `GacUI_Compiler` process with `ReloadResource` reverted, before the compiler loop can load `GuiFakeDialogServiceUI` itself. The root cause is that `GacUI_Compiler.vcxproj` links `Source_GacUI_Utilities_Controls.vcxitems`, and `GuiFakeDialogServiceUIResource.cpp` registers a generated loader plugin that preloads `GuiFakeDialogServiceUI` during plugin initialization.

The final code keeps `LoadResource(CompileResources(...))` for every generated resource and clears the five compiler-owned generated resource names once at `GuiMain()` entry.

Verification:
- `copilotBuild.ps1` after the fix: `Build succeeded. 0 Warning(s), 0 Error(s)`;
- `copilotExecute.ps1 -Mode CLI -Executable GacUI_Compiler`: completed successfully;
- generated error check: no `*.UI.error.txt` or `*.UI.errors.txt` files;
- rebuild after generated output update: `Build succeeded. 0 Warning(s), 0 Error(s)`;
- `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`: passed `84/84` test files and `1687/1687` test cases;
- Playground automation `GET http://localhost:8888/Automation/Playground/Controls`: HTTP 200, and the cursor assertion passed for both `MultilineTextBox` and `DocumentViewer`: root cursor omitted, exactly one `IBeam` content cursor, visible `HScroll` and `VScroll`, and zero scroll/button cursor nodes.
