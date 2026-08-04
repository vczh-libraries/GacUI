# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Move files from `Test\RemotingHelpers\AutomationService` to `Source\Utilities`, from `Source_RemotingHelpers` to `Source_GacUI_Core`, so that the following test apps:
- CppTest
- CppTest_Metaonly
- CppTest_Reflection
- GacUI_Host
- Playground
No more need to use reference `Source_RemotingHelpers` and `Generated_RemoteViewModelHost`. That should be more reasonable.

For RemotingTest_Rendering_Win32, add `/port:xxxx` so that 8889 for automation service will not be hardcoded anymore ---- meanwhile when `/port:` is not specified we still use 8889 as a default value. Update any markdown files that saying the 8889 port for automation service for `RemotingTest_Rendering_*`.

Run all 9 test apps, just to make sure their automation service is working. No unit test running is needed.

commit and push all local changes.

# UPDATES

# TEST [CONFIRMED]

- Build `Test/GacUISrc/GacUISrc.sln` in the default Debug x64 configuration through `copilotBuild.ps1`; require zero build errors.
- Confirm the automation sources are owned by `Source_GacUI_Core`, no longer by `Source_RemotingHelpers`, and all include paths resolve to `Source/Utilities/AutomationService`.
- Confirm `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, and `Playground` no longer import `Source_RemotingHelpers` or `Generated_RemoteViewModelTest` (the existing project name corresponding to the requested `Generated_RemoteViewModelHost`).
- Run all nine Windows test applications in Debug x64 through `copilotExecute.ps1`, without running `UnitTest`:
  - Require a nonempty automation response for `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `CppTest_Rvm`, `GacUI_Host`, and `Playground`, then exit each application through its automation endpoint.
  - Run `RemotingTest_Core` with `RemotingTest_Rendering_Win32`; require a nonempty Core `Controls` response on port 8888 and a nonempty renderer `Dom` response on an explicitly selected non-default port, then exit cleanly through automation.
  - Run `RemotingTest_RvmHost` as the companion service for `CppTest_Rvm` and confirm the RVM application becomes automation-ready. The host itself has no UI automation endpoint.
- Run `RemotingTest_Rendering_Win32` once without `/port:` and require the renderer automation endpoint to remain available on the default port 8889.
- Do not run unit tests, as explicitly requested.

# PROPOSALS

- No.1 Promote automation endpoint utilities into GacUI Core and parameterize renderer automation [CONFIRMED]

## No.1 Promote automation endpoint utilities into GacUI Core and parameterize renderer automation

Move the MiniHTTP and Windows HTTP automation endpoint implementations to `Source/Utilities/AutomationService`, list them in `Source_GacUI_Core` under matching `Utilities/AutomationService` filters, and remove them from `Source_RemotingHelpers`. Update all consumers and platform build inventories to use the production source path. Route the Windows-specific automation source to the Windows CodePack category while leaving MiniHTTP in the cross-platform GacUI category.

Remove the obsolete `Source_RemotingHelpers` and `Generated_RemoteViewModelTest` imports, plus their solution shared-item mappings, from the five named standalone test applications. Those applications already receive Core source through their linked GacUI libraries, and they do not use remote-view-model generated code.

Parse an optional `/port:<number>` argument in `RemotingTest_Rendering_Win32`, retain 8889 as the default, validate the supplied TCP port, and pass the selected port into both Windows HTTP and MiniHTTP automation startup. Document the optional port everywhere the renderer's 8889 automation port is described.

### CODE CHANGE

- Moved `MiniHttpAutomationService.*` and `WindowsAutomationService.Windows.*` to `Source/Utilities/AutomationService`, corrected their source-relative includes, added them to `Source_GacUI_Core` under `Utilities/AutomationService` filters, and removed their former `Source_RemotingHelpers` inventory/filter entries.
- Updated all Windows and portable test consumers, and excluded the Windows-only implementation in every Linux `vmake` that imports `Source_GacUI_Core`.
- Removed `Source_RemotingHelpers` and `Generated_RemoteViewModelTest` imports and solution shared-item mappings from `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, and `Playground`.
- Added validated `/port:<port>` parsing to `RemotingTest_Rendering_Win32`, retained 8889 as the default, and passed the selected value to both Windows HTTP and MiniHTTP renderer automation startup.
- Updated the renderer port documentation, automation-service knowledge, project ownership notes, and Linux/source inventories. Made Playground resolve its XML input from the executable location so the supported execution wrapper can run it from the solution directory.
- Updated `Release/CodegenConfig.xml` so MiniHTTP is part of the `gacui` category and the Windows endpoint is part of the `windows` category. Regenerated the tracked combined outputs: MiniHTTP declarations/implementation appear in `Release/GacUI.h` and `Release/GacUI.cpp`; Windows declarations/implementation appear in `Release/GacUI.Windows.h` and `Release/GacUI.Windows.cpp`. No separate automation-service CodePack pair was created.

### CONFIRMED

The final Debug x64 solution build completed through `copilotBuild.ps1` with 0 warnings and 0 errors. The shared-item and CodePack XML files parse successfully, stale automation source paths are absent, and the five named applications have neither obsolete import.

All nine requested applications were run through `copilotExecute.ps1` without running `UnitTest`:
- `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, `GacUI_Host`, and `Playground` returned nonempty `Controls` payloads (84,955 to 163,359 bytes), accepted `!Exit`, and terminated with exit code 0.
- `CppTest_Rvm` returned a 16,511-byte `Controls` payload after `RemotingTest_RvmHost` supplied its service, accepted `!Exit`, and terminated with exit code 0. The host then took its intentional exit-code-1 requester-disconnect path.
- `RemotingTest_Core /MiniHttp /FCT` and `RemotingTest_Rendering_Win32 /MiniHttp /port:8890` returned nonempty Core `Controls` and renderer `Dom` payloads (163,124 and 1,490,872 bytes) on ports 8888 and 8890.
- A second `/MiniHttp` pair omitted `/port:` and returned nonempty Core `Controls` and renderer `Dom` payloads (162,998 and 1,490,872 bytes) on ports 8888 and the default 8889. Core automation accepted `!Exit`; both wrappers terminated with exit code 0.

The combined generated release files were inspected after CodePack regeneration. MiniHTTP symbols occur in `GacUI.h`/`GacUI.cpp`, Windows automation symbols occur in `GacUI.Windows.h`/`GacUI.Windows.cpp`, and neither implementation leaked into the opposite pair.
