# Solution and Project File Structure

- A solution file (`*.sln` or `*.slnx`) contains multiple project files.
- Typical C++ project files are XML files in `*.vcxproj` or `*.vcxitems` naming.
- The XML file `*.vcxitems.filters` or `*.vcxproj.filters` organizes source files in solution explorer folders (virtual folders) that could be different from physical file system, which creates a human friendly view.
- The XML file `*.vcxproj.user` contains some temporary local configuration for a project. This file is not tracked by git, but it contains arguments for running the project.
- When adding a source file to a specific solution explorer folder:
  - It must be also added to one or multiple project files.
  - Find the `*.vcxitems.filters` or `*.vcxproj.filters` file with the same name.
  - Each file must be attached to a solution explorer folder, described in this XPath: `/Project/ItemGroup/ClCompile@Include="PhysicalFile"/Filter`.
  - Inside the `Filter` tag there is the solution explorer folder.
  - Edit that `*.vcxitems.filters` or `*.vcxproj.filters` file to include the source file.

## Renaming and Removing Source Files

- All affected `*.vcxitems`, `*.vcxproj`, `*.vcxitems.filters` and `*.vcxproj.filters` must be updated.
