# GacUILibs -- Getting Started

## New Feature

## Fixed Bugs

## Milestone

- Think about allowing `GacUI.xml` to publish pure reflection metaonly binary for selected resources and their dependencies.
  - Make sure GacBuild.ps1/GacGen.ps1/GacGen.exe can load specified reflection metaonly binary from `GacUI.xml` and self or dependency `Resource.xml`.
- Begin GacUILibs project and put the wiazrd window and chart control there as anstarting.
  - Expose hosted/tui application functions so that XML could know the current mode.
    - By adding direct helper functions and fix all code using the GetXXXApplication detection to use them.
    - Register them as `GuiApplication` getters so that properties could be used in XML.
  - Redo Win7/Win8/Ubuntu/macOS skin

### GacUILibs Organization

- Requires `Release`, `wGac`, `iGac`, `GacJS`, `Tools` to clone (as sibling folders by default)
  - **BOLD** files are copied from elsewhere, DO NOT update
- File Structures:
  - **.github**
  - **.build**
  - Import <- Each sub folder should keep itself a slice of original repo, no file path renaming allowed
    - Release
    - wGac
    - iGac
    - GacJS
  - Source
    - Shared
    - Windows
    - Wayland
    - Cocoa
    - HTML5
  - Test
    - AppOne
    - AppTwo
    - UnitTest
  - Products
    - AppOne
    - AppTwo
  - Doc
    - Jobs
      - Verify.md <- Standard name for full repo verification
    - **Layout.md** <- new Tools/Copilot/GacUILibs/Layout.md, define a layout convention for any GacUILibs, including 
    - **Chat.md** <- from original Tools/Copilot/AGENTS.md
  - **import.sh** / **Import.ps1** <- Refresh `Import` folder from `Release`, `wGac`, `iGac`, `GacJS`, `Tools`, if a monorepo path is not offered it is the parent folder
  - Project.md
  - AGENTS.md
- CMake generates files to `.build` folder
  - Any other folder should not have cmake/msbuild/make generated files.
  - Unless for generated code that committed.
- Build all platforms in one repo.
- Once finishing, release them back to `Release/Import/GacUILibs`.
