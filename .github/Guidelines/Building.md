# Building a Solution

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux or macOS.

## Windows Specific

- Only run `copilotBuild.ps1` to build a solution.
- DO NOT use msbuild by yourself.
- The script builds all projects in a solution.

### Executing copilotBuild.ps1

Run this script to build the solution:

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\Scripts\copilotBuild.ps1
```

It is possible that, before running `copilotBuild.ps1`, the binary to compile is still running or still being debugged. This could cause the linking to fail. You need to check the error message, and in case when it happens:
- Kill `cdb` process first, if there is any.
  - The cdb path is stored in `$env:CDBPATH`.
  - Avoid running `copilotDebug_Stop.ps1` directly.
- Kill the binary process that is blocked.
- Rebuild, and this issue should gone.

### Ensure Target Configuration

`-Configuration` and `-Platform` arguments are available to specify the target configuration:
- `-Configuration` could be `Debug` (default) or `Release`.
- `-Platform` could be `x64` (default) or `Win32`
- Pick the default option (omit both arguments) when there is no specific requirements.

### The Correct Way to Read Compiler Result

- The only source of trust is the raw output of the compiler.
- Wait for the script to finish before reading the log file.
  - DO NOT need to read the output from the script.
  - Building takes a long time. DO NOT hurry.
  - When the script finishes, the result is saved to `REPO-ROOT/.github/Scripts/Build.log`.
  - A temporary file `Build.log.unfinished` is created during building. It will be automatically deleted as soon as the building finishes. If you see this file, it means the building is not finished yet.
- When build succeeds, the last several lines of `Build.log` indicates the number of warnings and errors in the following pattern:
  - "Build succeeded."
  - "0 Warning(s)"
  - "0 Error(s)"
- DO NOT delete the log file by yourself.

## Linux Specific

Building only happens on a folder that has a `vmake` file.
- If the repo has only one project, it is in `REPO-ROOT/Test/Linux`.
- If the repo has multiple projects, it is in `REPO-ROOT/Test/Linux/PROJECT-NAME`.
  - `PROJECT-NAME` naming is following `PROJECT-NAME.vcxproj`.
You are required to `cd` to such folder before running `build.sh`, otherwise it will fail.

Call `REPO-ROOT/.github/Ubuntu/build.sh` for incremental build.
Call `REPO-ROOT/.github/Ubuntu/build.sh -f` for full rebuild.
`build.sh` will read the local `vmake` configuration file and generate a `makefile` in the same folder before building.
`build.sh` will also run other script files in that folder, run `chmod +x` if any script file is blocked.

Only the "debug x64" configuration is supported on Linux. If you are instructed to build and run other configuration, ignore it.
