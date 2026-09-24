# Release Maintenance

Each sub section is a task, do them one after another, commit and push is required right after any task before executing the next one.

## Knowledge Base of Tools

In `GacUI` there are new tools `UiaList` and `GitView`. We only focus on executable from `UiaListApp.vcxproj` and `GitTui.vcxproj`, other projects are just part of the file organization, they are not our target in this request:
- Create dedicated knowledge page for them, focus one what they do and how to use them, no need to mention technical details.
- Update `GacUI\Project.md`'s `## Maintaining Tools` to mention these two new knowledge page files.
- Run job:copilotInitAll but skip learning, so that new knowledge base pages spread to all repos.
- Update `Release\Project.md`'s "Content of This Project/Tools" to introduce new tools.

## Release GacUI

Run `Tools\Tools\Build.ps1 -Project GacUI` to make sure the repo is properly updated.

## Release of Tools

`UiaListApp` and `GitTui` should be created in `Release\Tools\Executables\Executables.sln`:
- Create `UiaListApp.vcxproj` and `GitTui.vcxproj`.
- Copy all necessary .h and .cpp files to `Release\Tools\Executables\(UiaListApp|GitTui)` folder.
  - The project folder should looks like, for example for `UiaListApp`:
    - `UiaList`: store source files from `UiaList.vcxproj`.
    - `Source`: store source files from `UiaListApp.vcxproj`.
  - So the original include paths will be working.
- The source copying should be added to `Build.ps1 -Project UpdateRelease` in the appropriate place where scripts for copying other tools are. To verify:
  - When preparing new vcxproj files you can prepare everything but do not actually copy source files.
  - Update and run `Build.ps1 -Project UpdateRelease`.
  - Build `Executables.sln`.
  - Update and run `Release\Tools\CopyExecutables.ps1` and make sure all executable files are ready, and `UiaListApp` and `GitTui` actually works.
- Prepare `vmake` for `GitTui`, ignore `UiaListApp` because this is a Windows only tool. `BuildExecutables.sh` cannot be verified in this task, I will do it myself.
- Update `Release\Tools\README.md` to mention new tasks.

## Release

Run `Build.ps1 -Project Release` to make sure the release repo is properly updated.

## Website

[Release 1.4.1.1](https://github.com/vczh-libraries/Release/releases#release-1.4.1.1) has a list of breaking changes. Properly copy them to the website, in GacUI project's "Breaking Changes from 1.0". Exclude remote protocol because it does not exist in `1.0-rc-3`.

Publish the website.
