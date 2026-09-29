# Building a Solution

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux/macOS Specific` section if you are on Linux/macOS.

## Windows Specific

- Only run `copilotBuild.ps1` to build a solution.
- DO NOT use MSBuild by yourself.
- The script builds all projects in a solution.

### Executing copilotBuild.ps1

Run this script to build the solution:

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\Scripts\copilotBuild.ps1
```

It is possible that, before running `copilotBuild.ps1`, the binary to compile is still running or still being debugged. This could cause the linking to fail. You need to check the error message, and in case when it happens:
- Follow `### Stop a Debugger` in `REPO-ROOT/.github/Guidelines/Debugging.md` to stop debugging.
- Rebuild, and this issue should be gone.

### Ensure Target Configuration

`-Configuration` and `-Platform` arguments are available to specify the target configuration:
- `-Configuration` can be `Debug` (default) or `Release`.
- `-Platform` can be `x64` (default) or `Win32`
- Pick the default option (omit both arguments) when there is no specific requirement.

### The Correct Way to Read Compiler Result

- The only source of truth is the raw output of the compiler.
- Wait for the script to finish before reading the log file.
  - DO NOT need to read the output from the script.
  - Building takes a long time. DO NOT hurry.
  - When the script finishes, the result is saved to `REPO-ROOT/.github/Scripts/Build.log`.
  - A temporary file `Build.log.unfinished` is created during building. It will be automatically deleted as soon as the building finishes. If you see this file, it means the building is not finished yet.
- When build succeeds, the last several lines of `Build.log` indicate the number of warnings and errors in the following pattern:
  - "Build succeeded."
  - "0 Warning(s)"
  - "0 Error(s)"
- DO NOT delete the log file by yourself.

## Linux/macOS Specific

Building only happens in a folder that has a `vmake` file.
- If the repo has only one project, it is in `REPO-ROOT/Test/Linux`.
- If the repo has multiple projects, it is in `REPO-ROOT/Test/Linux/PROJECT-NAME`.
  - The `PROJECT-NAME` name follows `PROJECT-NAME.vcxproj`.
You are required to `cd` to such folder before running `build.sh`, otherwise it will fail.

Call `REPO-ROOT/.github/Ubuntu/build.sh` for incremental build.
Call `REPO-ROOT/.github/Ubuntu/build.sh -f` for full rebuild.
`build.sh` will read the local `vmake` configuration file and generate a `makefile` in the same folder before building.
`build.sh` will also run other script files in that folder; you may need to run `chmod +x` if any script file is blocked.

Only the "debug x64" configuration is supported on Linux. If you are instructed to build and run other configuration, ignore it.

### WebAssembly Specific

Building only happens in a folder that has a `vmake` and `vbuild` file, and the `vbuild` file should have the quoted JSON key `"WASM=YES"` in it.
Use `REPO-ROOT/.github/Ubuntu/build.sh` with `-bw` (incremental) or `-fbw` (full) to build wasm out of the test project:
Full build is required if the previous target platform is not the current one (native or wasm app).

For unit test projects running with web assembly:
- There will be a `./Bin/app.html` generated, run this web page and the test project will start.
- Run `./Bin/app.sh ./vbuild` with Node.js 22.17 or newer installed and open the printed URL.
- The launcher supplies the COOP/COEP headers required for pthreads.
- The page is running async, all retained tests must pass with exactly one `wasm_main returns 0.` line.

The JSON object under `"WASM=YES"` optionally contains `rootFolder`, `folders`, `includes`, and `excludes`. Paths are relative to the configuration file; selected files and empty folders prefill OPFS after clearing its old contents. Without `rootFolder` no fixtures are loaded. VlppOS supplies the default OPFS filesystem with `/` as its working directory, whole-file memory buffering and writable close persistence. Wasm linking enables Asyncify, and the worker awaits the Embind entry.
