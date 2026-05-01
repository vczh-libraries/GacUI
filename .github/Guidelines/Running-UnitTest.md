# Running a Unit Test Project

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux or macOS.

## Windows Specific

- Only run `copilotExecute.ps1` to run a unit test project.
- DO NOT call executables or scripts yourself.

### Executing copilotExecute.ps1

`PROJECT-NAME` is the name of the project.
Run test cases in `SOLUTION-ROOT\PROJECT-NAME\PROJECT-NAME.vcxproj`:

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable PROJECT-NAME
```

`-Mode` and `-Executable` are required.

`-Configuration` and `-Platform` arguments are available to specify the target configuration:
- When both arguments are omitted, the last build configuration will be picked up.
- Both arguments should be omitted or not omitted at the same time.
- `-Configuration` could be `Debug` (default) or `Release`.
- `-Platform` could be `x64` (default) or `Win32`
- Pick the default option (omit both arguments) when there is no specific requirements.

### Ensure Expected Test Files are Selected

Test cases are organized in multiple test files.
In `PROJECT-NAME\PROJECT-NAME.vcxproj.user` there is a filter, when it is effective, you will see filtered test files marked with `[SKIPPED]` in `Execute.log`.
The filter is defined in this XPath: `/Project/PropertyGroup@Condition="'$(Configuration)|$(Platform)'=='Debug|x64'"/LocalDebuggerCommandArguments`.
The filter is effective only when the file exists and the element exists with one or multiple `/F:FILE-NAME.cpp`, listing all test files to execute, unlisted files are skipped.
If the element exists but there is no `/F:FILE-NAME.cpp`, it executes all test files, none is skipped.

**IMPORTANT**:

ONLY WHEN test files you want to run is skipped, you can edit `PROJECT-NAME\PROJECT-NAME.vcxproj.user` to activate your filter.
- This would typically happen when:
  - A new test file is added.
  - A test file is renamed.

You can clean up the filter to remove unrelated files that are either not existing or are totally unrelated to the current task you are working on.
If the current task does not work on that test file, but it tests a closely related topic, you should better keep it in the list.

DO NOT delete this `*.vcxproj.user` file.
DO NOT clean the filter (aka delete all `/FILE-NAME.cpp`) by yourself. I put a filter there because running everything is slow and unnecessary for the current task.
Ignore `LocalDebuggerCommandArgumentsHistory` in `*.vcxproj.user`.

### The Correct Way to Read Test Result

- The only source of trust is the raw output of the unit test process.
- Wait for the script to finish before reading the log file.
  - DO NOT need to read the output from the script.
  - Testing takes a long time. DO NOT hurry.
  - When the script finishes, the result is saved to `REPO-ROOT/.github/Scripts/Execute.log`.
  - A temporary file `Execute.log.unfinished` is created during testing. It will be automatically deleted as soon as the testing finishes. If you see this file, it means the testing is not finished yet.
    - Read `Execute.log.unfinished` every 5 minutes.
    - If the log is not updated, it is possible that the unit test raised a fatal error and blocked by the system message dialog. You need to:
      - Kill the unit test process.
      - You are required to run the unit test with debugger attached. The fatal error will report to the debugger and you can see what happened.
- When all test cases pass, the last several lines of `Execute.log` should be in the following pattern; otherwise it crashed at the last shown test case:
  - "Passed test files: X/X"
  - "Passed test cases: Y/Y"
- DO NOT delete the log file by yourself.

## Linux Specific

Building only happens on a folder that has a `vmake` file.
- If the repo has only one project, it is in `REPO-ROOT/Test/Linux`.
- If the repo has multiple projects, it is in `REPO-ROOT/Test/Linux/PROJECT-NAME`.
  - `PROJECT-NAME` naming is following `PROJECT-NAME.vcxproj`.
You are required to `cd` to such folder before running the compiled unit test binary, otherwise it will fail.

After a successful build, `Bin/UnitTest` will be generated as the executable.
If you can'f find it, first check if the build succeeded, and then read `makefile` to find the correct binary file name.
The unit test project supports following command line options:
- `/D`: crash at the first failure, the error message is not printed. This is recommended for debugging.
- `/C`: crash at the first failure and print the error message if possible. This is recommended for usual running.
- `/R`: print all failures without crashing. Be careful to use it because failure cases usually affect following cases, making everything not stable after the first failure.
- `/F:FILENAME.cpp`: file filter.
  - If no `/F` appears, all test cpp files will run.
  - If one or multiple `/F` appear, only specified cpp files will run.
  - `FILENAME.cpp` do not contain file path, and it is case sensitive.

**IMPORTANT**: Always run it async, read terminal output and its return code.
Compiled binary might have bug causing it to trap in a dead looping. DO NOT just wait for it to complete.
If you feel suspicious, you are recommended to kill the process and run it again with the debugger.
When `/D` or `/C` is specified, the unit test binary stops at the first failure, causing it not able to summary how many test cases pass or fail at the end. This would be an obvious signal to tell that it fails.

Only the "debug x64" configuration is supported on Linux. If you are instructed to build and run other configuration, ignore it.
