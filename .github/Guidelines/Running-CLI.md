# Running a CLI Application Project

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
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -CLI PROJECT-NAME
```

`-Configuration` and `-Platform` arguments are available to specify the target configuration:
- When both arguments are omitted, the last build configuration will be picked up.
- Both arguments should be omitted or not omitted at the same time.
- `-Configuration` could be `Debug` (default) or `Release`.
- `-Platform` could be `x64` (default) or `Win32`
- Pick the default option (omit both arguments) when there is no specific requirements.
