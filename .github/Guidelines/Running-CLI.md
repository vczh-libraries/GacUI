# Running a CLI Application Project

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux or macOS.

## Windows Specific

- Only run `copilotExecute.ps1` to run a CLI application project.
- DO NOT call executables or scripts yourself.

### Executing copilotExecute.ps1

`PROJECT-NAME` is the name of the project.
Run the CLI application built from `SOLUTION-ROOT\PROJECT-NAME\PROJECT-NAME.vcxproj`:

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable PROJECT-NAME
```

`-Mode` and `-Executable` are required.

`-Configuration` and `-Platform` arguments are available to specify the target configuration:
- When both arguments are omitted, the last build configuration will be picked up.
- Both arguments should be provided together or omitted together.
- `-Configuration` can be `Debug` (default) or `Release`.
- `-Platform` can be `x64` (default) or `Win32`
- Pick the default option (omit both arguments) when there is no specific requirement.

## Linux Specific

Building only happens in a folder that has a `vmake` file.
- If the repo has only one project, it is in `REPO-ROOT/Test/Linux`.
- If the repo has multiple projects, it is in `REPO-ROOT/Test/Linux/PROJECT-NAME`.
  - The `PROJECT-NAME` name follows `PROJECT-NAME.vcxproj`.
You are required to `cd` to such folder before running the compiled CLI binary, otherwise it will fail.

After a successful build, `Bin/UnitTest` will be generated as the executable.
If you can't find it, first check if the build succeeded, and then read `makefile` to find the correct binary file name.
`Bin/UnitTest` is the default name in any `vmake` if it is not changed, so many CLI applications are still named `UnitTest`.

**IMPORTANT**: Always run it asynchronously, read terminal output and its return code.
Compiled binary might have a bug causing it to trap in a dead loop. DO NOT just wait for it to complete.
If this seems suspicious, you are recommended to kill the process and run it again with the debugger.

Only the "debug x64" configuration is supported on Linux. If you are instructed to build and run other configuration, ignore it.
