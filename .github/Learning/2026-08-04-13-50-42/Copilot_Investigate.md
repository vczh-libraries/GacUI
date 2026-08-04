# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

One small changes, merge Test/Rvmt/ViewModel to Test/RemotingHelpers/Rvmt, and merge Source_Rvmt_ViewModel into Rouce_RemotingHelpers to become one single vcxitems. In Source_RemotingHelpers add a `Rvmt` solution explorer folder for the files in `Test/RemotingHelpers/Rvmt`.

In Release/CodegenConfig.xml, instead of code-packing Test/RemotingHelpers to GacUI.RemotingHelpers.*, just remove this entry and generated files, including files in Release and Release/IncludeOnly.

Build, commit and push. No testing is required.

# UPDATES

## UPDATE

I don't think `ExcludedFromBuild` is needed in Source_RemotingHelpers.vcxitems

# TEST [CONFIRMED]

No unit or runtime testing is required by the task. The current tree confirms the requested structural change is needed: the six RVM helper files are under `Test/Rvmt/ViewModel`, a separate `Source_Rvmt_ViewModel.vcxitems` is imported by the three RVM consumers, and `Release/CodegenConfig.xml` still CodePacks `Test/RemotingHelpers` into `RemotingHelpers*` artifacts in both `Release` and `Release/IncludeOnly`.

Success requires the six files under `Test/RemotingHelpers/Rvmt`, one `Source_RemotingHelpers.vcxitems` inventory with an `Rvmt` Solution Explorer filter, no `Source_Rvmt_ViewModel` project/import/reference, no RemotingHelpers CodePack source/category/codepair configuration, and no generated `RemotingHelpers*` files in either Release output directory. All changed project XML must parse, `git diff --check` must pass, and the requested full solution build must finish with zero warnings and zero errors.

# PROPOSALS

- No.1 Consolidate RVM helpers and remove their Release packaging [CONFIRMED]

## No.1 Consolidate RVM helpers and remove their Release packaging

Move `Test/Rvmt/ViewModel/ViewModel*` to `Test/RemotingHelpers/Rvmt`, adjust their relative includes and the three application includes, and enumerate all six files in `Source_RemotingHelpers.vcxitems`. Keep the files visible under a new `Rvmt` filter and compile the complete inventory in every importing project without `ExcludedFromBuild` metadata.

Remove the separate shared-items project from the three projects, the solution, and the two hand-authored Linux `vmake` files. Remove the `Test/RemotingHelpers` CodePack scan, categories, and output pairs from `Release/CodegenConfig.xml`, delete all eight generated `RemotingHelpers*` files from `Release` and `Release/IncludeOnly`, and align the active project/learning documentation with the new ownership and packaging boundary.

### CODE CHANGE

- Moved `ViewModelHostClient.*`, `ViewModelHostServer.*`, and `ViewModelShared.*` from `Test/Rvmt/ViewModel` to `Test/RemotingHelpers/Rvmt`, fixed the server helper's relative remoting-server include, and updated the three application includes.
- Added the six RVM files exactly once to `Source_RemotingHelpers.vcxitems` and its filters under `Rvmt`, with no `ExcludedFromBuild` metadata. Removed `Source_Rvmt_ViewModel.vcxitems`, its filters, all three imports, its solution project/mappings, and both Linux `vmake` references.
- The first unconditional build proved that the other six shared-helper consumers lacked `RemoteViewModelTestRpc.h`. Kept the helper inventory unconditional, added architecture-specific generated include directories to it, and imported the existing `Generated_RemoteViewModelTest.vcxitems` from those six consumers. Updated the solution shared-items mappings accordingly.
- Removed the `Test/RemotingHelpers` scan, both helper categories, and both helper codepairs from `Release/CodegenConfig.xml`. Deleted `RemotingHelpers.*` and `RemotingHelpers.Windows.*` from both `Release` and `Release/IncludeOnly`.
- Updated `Project.md` and the active coding learning to describe the consolidated helper inventory and removal of Release CodePack output. Archived the completed prior investigation before recording this request.

### CONFIRMED

The consolidation and packaging removal are confirmed. `Source_RemotingHelpers.vcxitems` contains all six RVM files exactly once, its filters assign all six to `Rvmt`, and it contains zero `ExcludedFromBuild` entries. Nine projects import the one helper inventory, the eight projects that need the full generated module import the existing generated shared inventory, and `RemotingTest_RvmHost` retains its targeted generated RPC source configuration. There are no active `Source_Rvmt_ViewModel` references in Windows or hand-authored Linux project metadata.

All changed MSBuild and CodePack XML parses successfully. The old six-file directory, separate shared inventory, eight generated Release files, and all RemotingHelpers references in `Release/CodegenConfig.xml` are absent. `git diff --check` passes.

The final default Debug x64 full-solution build through `copilotBuild.ps1` succeeded with `0 Warning(s)` and `0 Error(s)`. Per the explicit request, no unit or runtime tests were run.
