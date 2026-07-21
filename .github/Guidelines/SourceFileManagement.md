# Solution and Project File Structure

- A solution file (`*.sln` or `*.slnx`) contains multiple project files.
- Typical C++ project files are XML files in `*.vcxproj` or `*.vcxitems` naming.
- The XML file `*.vcxitems.filters` or `*.vcxproj.filters` organizes source files in solution explorer folders (virtual folders) that can be different from the physical file system, creating a human-friendly view.
- The XML file `*.vcxproj.user` contains some temporary local configuration for a project. This file is not tracked by git, but it contains arguments for running the project.
- When adding a source file to a specific solution explorer folder:
  - It must also be added to one or multiple project files.
  - Find the `*.vcxitems.filters` or `*.vcxproj.filters` file with the same name.
  - Each file must be attached to a solution explorer folder, described in this XPath: `/Project/ItemGroup/ClCompile@Include="PhysicalFile"/Filter`.
  - Inside the `Filter` tag there is the solution explorer folder.
  - Edit that `*.vcxitems.filters` or `*.vcxproj.filters` file to include the source file.
- Wild card is **NOT ALLOWED** in project files, each file should be enumerated one by one.

## Renaming and Removing Source Files

- All affected `*.vcxitems`, `*.vcxproj`, `*.vcxitems.filters` and `*.vcxproj.filters` must be updated.

## Adding new C++ Project

- There are two kinds of projects: `*.vcxproj` and `*.vcxitems`.
  - Usually `*.vcxproj` comes with a `*.vcxproj.filters` because solution explorer organization of source files are always needed.
  - Usually `*.vcxitems` does not come with a `*.vcxitems.filters` because all files are just listed below the project in the solution explorer.
- When adding a new project, find a existing project to copy.
- For a typical `*.vcxproj` project, there will always be solution explorer folders `Header Files`, `Resource Files` and `Source Files`.
  - `Header Files` and `Resource Files` should be kept during copy even when there will be no files adding to them.
  - `Header Files` is never used, all source files, including header files, should be in `Source Files` or custom created folders.
  - If all options are correctly copied from another `*.vcxproj` files, it should be good but you still need to make sure that:
    - `Debug` and `Release` are always needed.
    - `Win32` and `x64` are always needed.
    - All configuration should define `VCZH_DEBUG_NO_REFLECTION` macro, unless there is special requirement to enable C++ reflection.
    - All `Debug` configuration should define `VCZH_CHECK_MEMORY_LEAKS`.
- When `*.vcxproj` is added to the solution, you need to build the solution against all configuration to make sure everything works:
  - `Debug` and `Release`.
  - `Win32` and `x64`.
  - Even if you find any pre-existing issues, you should fix them.
  - You can skip the test, as adding new project should not affect test cases, unless existing projects are modified as part of the refactoring.
- Only add the `/bigobj` compiler option to a `*.cpp` file when needed.

## Cross Platform Naming Convention

- Windows specific source file in `Source` folder should be named after `*.Windows.(h|cpp)`.
- When a source file could be shared between Linux and macOS, use `*.Linux.(h|cpp)`.
- Otherwise macOS specific file should use `*.macOS.(h|cpp)`.

Platform guard macros should be used even when in platform specific source files:
- `#ifdef VCZH_MSVC` for Windows.
- `#ifdef VCZH_GCC` for Linux and macOS shared source.
- `#if defined VCZH_MSVC && !defined VCZH_APPLE` for Linuc only.
- `#if defined VCZH_MSVC && defined VCZH_APPLE` for macOS only.

Be awared that during releasing a repo, Linux and macOS source files will be CodePack-ed into a single source file, which is the reason for the above rule.

## Working on Linux/macOS

`makefile` for each project is generated from MSBuild project files during the build.
When the file list needs to be modified, update MSBuild project files directly.
After building, `vmake` file generates both `vmake.txt` and `makefile`.
If you doubt whether the file is built or not, check out `vmake.txt`, which simply lists all used `cpp` files in `makefile`.

Linux/macOS specific C++ source files also need to be added to MSBuild project files, and they are excluded from the build.
In `vmake`, Windows specific C++ source files need to be excluded.
When the `Main.cpp` from MSBuild projects or similar files does not work on Linux, you are recommended to create one for Linux/macOS.

In `vmake`, these variables are available for configuration, most of them are optional:
- `CPP_VCXPROJS`: Use MSBuild project files, they are typically `*.vcxitems` or `*.vcxproj`.
- `CPP_REMOVES`: Files to exclude, from the file list generated from `CPP_VCXPROJS`.
- `CPP_ADDS`: Files to add.
- `CPP_COMPILE_OPTIONS`: Extra compiler options for `clang++` or `g++`.
- `FOLDERS`: Folders that contain generated files. These folders will be completely removed during a full build.
- `CPP_TARGET`: The compiled binary.
