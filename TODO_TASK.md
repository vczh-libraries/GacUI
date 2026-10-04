The goal of this task is to create a CLI version of `UiaList`:
- `UiaListCli.exe` will be an interactive CLI application, and it is Windows only.
- New project file will be `Tools/UiaList/UiaListCli/UiaListCli.vcxproj`
- It should be released to `../Release/Tools/Executables/UiaListCli/UiaListCli.vcxproj`:
  - Common code will be moved back from `../Release/Tools/Executables/UiaListApp/UiaList` to be under a new lib project file `../Release/Tools/Executables/UiaList/UiaList.vcxproj`
  - `UiaListApp` source files moved from `../Release/Tools/Executables/UiaListApp/Source` to its parent folder, and the `Source` folder will be deleted.
  - In this way relative position of source files are the same in `Tools/UiaList` so they could be just copied directly.
  - Make sure `../Tools/Tools/Build.ps1 -Project UpdateRelease` copies source files in the same coding format, and `../Release/Tools/CopyExecutables.ps1` copies executable files.
- In the same knowledge base page `KB_GacUI_Design_UiaList.md` in `GacUI` repo, add the input command and output format about `UiaListCli`:
  - Run `../Tools/Jobs/job.Windows.copilotInitAll.prompt.md` but skip learning, now the new KB page is spreaded to all repos.
  - Update `../Release/README.md` to add a new hyperlink of the new tool, link to the same page as `UiaListApp`.

You are going to design the input command format and output JSON format to make the interactive CLI application renders all features as `UiaListApp`, make sure all data could be exposed, all actions could be run from `UiaList` implemented view model. Command names are designed like `Verb-Target`, all result should be in JSON format, here are some examples:

1) Listing Processes

User could type `List-Process`, and the tree will be exposed via JSON, window ID could be used in other commands.

2) Exploring a Window

User could type `Print-Window ID` to list all controls in a tree that exposed via JSON, each control exposes limited information for identify and offer ID for other commands, e.g., node/control type, text, id

3) Others

`Query-Properties ID` could be used to read all values of a node,
`Query-Providers ID` for read all providers, including properties, `Run-ProviderInterface::Method ID` to call a function,
`Help-Provider ProviderInterface` to explain the correct format of all methods, some might need extra parameters,
etc, complete all of them in a similar way.

This tool is offered for coding agents, like you in codex, for better computer use, not for human (but you don't need to say this in the knowledge base page).

To verify, there are already some test apps in `Tools/UiaList/Verification`, make sure everything in `UiaListApp` are accessible in `UiaListCli`.
You are also going to test `UiaListCli` against `CppTest` to make sure `UiaListCli` is actually working on every kinds of control.
