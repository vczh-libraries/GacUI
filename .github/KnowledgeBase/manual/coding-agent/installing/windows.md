# Installing Required Tools on Windows

The Windows agent scripts build with MSBuild through Visual Studio's developer environment and debug with CDB. Install Visual Studio with the C++ desktop workload, a Windows SDK, and the Windows debugging tools that provide `cdb.exe`. If CDB is not available, install the Windows Driver Kit or the Windows debugging tools component from the Visual Studio Installer.

Define these environment variables before asking the agent to build or debug:
- `VLPP_VSDEVCMD_PATH`: absolute path to `VsDevCmd.bat`, for example `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat`.
- `CDBPATH`: absolute path to `cdb.exe`, for example `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\cdb.exe`.

After copying the Release `.github` folder, the agent should use these scripts instead of calling tools directly:
- `.github\Scripts\copilotBuild.ps1`: builds the solution found from `Project.md` context and writes `Build.log`.
- `.github\Scripts\copilotExecute.ps1`: runs unit-test or CLI projects and writes `Execute.log` for unit tests.
- `.github\Scripts\copilotDebug_Start.ps1` and `copilotDebug_RunCommand.ps1`: start CDB and send debugger commands.

If the application uses the tools shipped by Release, build them from the Release repository:
- Open `Tools\Executables\Executables.sln` in Visual Studio.
- Build `Release` with `x64`.
- Run `Tools\CopyExecutables.ps1`.
- Confirm `CodePack.exe`, `CppMerge.exe`, `GacGen.exe`, and `GlrParserGen.exe` exist in `Release\Tools`.

For debugger readability, copy `Import\vlpp.natvis` from Release to Visual Studio's visualizers folder. The CDB startup script also loads the natvis file for debugger commands such as `dx`.

