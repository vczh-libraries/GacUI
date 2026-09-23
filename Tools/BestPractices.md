# Best Practice for Creating and Maintaining GacUI based Tools

You are not recommended to change GacUI itself unless you find any bug that the root cause is actually in GacUI.

## Practice complete Model-View-ViewModel pattern

- GacUI XML Resource defines view model interfaces, UI, and how UI react to view models by data binding.
- All non-rendering features should be represented by view model interfaces.
- Implementation of view model interfaces therefore no need to know anything about the actual UI, although it inevitably depends on GacUI source code.
- A root interface will be offered to the main window, all sub view models are accessible from it.
- The design of view model interfaces and data structure offered by the view model interfaces should already be in the shape of the UI, to reduce calculation needed in GacUI XML Resource.
- View model implementations are required to be in C++.
- All events are required to handle with the `-eval` binding in GacUI XML Resource.
- It is forbidden to modify any generated files.

## Important vcxproj Settings

- `Windows SDK Version`: `Windows 10.0 SDK (latest)`
- `Platform Toolset`: `v145 for Microsoft C++ Build Tools`
- `C++ Language Standard`: `ISO C++ 20 Standard`
- `Include Directories`: add the `REPO-ROOT/Import` folder
- `Use Library Dependency Inputs`: `Yes`
- `Preprocessor`:
  - `VCZH_DEBUG_NO_REFLECTION`: all reflection code will be eliminated during compiling
  - debug profile
    - `VCZH_CHECK_MEMORY_LEAKS`
    - memory leaks checking should only be applied on debug profile.

Settings are expected to be aligned with other projects.

## Basic File Organization

- `README.md`: introduction to this project, but do not maintain it like a specification, it is prepared for developers that use this tool instead of maintainers. You could put what does this tool do, how to build it, how to utilize source files of it, and whatever you think users should know.
- `<TOOL-NAME>.sln`.
- `Gaclib/Gaclib.vcxproj`: a library to add all necessary files from `REPO-ROOT/Import`.
- `<TOOL-NAME>/<TOOL-NAME>.vcxproj`: a library to compile GacUI XML Resource with view model implementation.
  - `UI`: a folder for GacUI XMl Resource.
    - All texts on UI are required to be localizable, default to `en-US` in the XML, but GacUI should pick up the language based on OS language automatically.
    - Offer English, Chinese, Japanese translations and keep them in sync.
  - `Source`: a folder for generated C++ code from `UI`:
    - Binary resource should be printed in C++ so that the app does not need to load an extra binary resource. This is done by adding `CppCompressed`  to `GacGenConfig`.
    - `REPO-ROOT/../Tools/Tools/GacBuild.ps1` should be used to generate this folder.
  - `ViewModel`: View model implementation.
- `<GUI-NAME>/<GUI-NAME>.vcxproj`: an interactive app running `<TOOL-NAME>`, based on GacUI with native renderer.
- `<TUI-NAME>/<TUI-NAME>.vcxproj`: an interactive app running `<TOOL-NAME>`, based on GacUI with TUI renderer.
