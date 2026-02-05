# Building a Solution

- Only run `copilotBuild.ps1` to build a solution.
- DO NOT use msbuild by yourself.
- The script builds all projects in a solution.

## Executing copilotBuild.ps1

Before building, ensure the debugger has stopped.
If there is any error message, it means the debugger is not alive, it is good.

```
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
```

And then run this script to build the solution:

```
cd SOLUTION-ROOT
& REPO-ROOT\.github\Scripts\copilotBuild.ps1
```

## The Correct Way to Read Compiler Result

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
