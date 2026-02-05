# Running a Unit Test Project

- Only run `copilotExecute.ps1` to run a unit test project.
- DO NOT call executables or scripts yourself.

## Executing copilotExecute.ps1

`PROJECT-NAME` is the name of the project.

Before testing, ensure the debugger has stopped.
If there is any error message, it means the debugger is not alive, it is good.

```
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
```

And then run test cases in `SOLUTION-ROOT\PROJECT-NAME\PROJECT-NAME.vcxproj`:

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Executable PROJECT-NAME
```

## Ensure Expected Test Files are Selected

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

## The Correct Way to Read Test Result

- The only source of trust is the raw output of the unit test process.
- Wait for the script to finish before reading the log file.
  - DO NOT need to read the output from the script.
  - Testing takes a long time. DO NOT hurry.
  - When the script finishes, the result is saved to `REPO-ROOT/.github/Scripts/Execute.log`.
  - A temporary file `Execute.log.unfinished` is created during testing. It will be automatically deleted as soon as the testing finishes. If you see this file, it means the testing is not finished yet.
- When all test cases pass, the last several lines of `Execute.log` should be in the following pattern; otherwise it crashed at the last shown test case:
  - "Passed test files: X/X"
  - "Passed test cases: Y/Y"
