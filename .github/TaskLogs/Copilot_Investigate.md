# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

## Task 1: Native GacBuild tool

- Rewrite `GacBuild.ps1` and `GacGen.ps1` in C++:
  - The new tool will be `GacBuild`:
    - Copy the same configuration from `GacGen` project.
  - It accepts required arguments:
    - `-mode:(GacBuild|GacGen)` to replace either script.
      - `GacBuild.ps1` calls `GacGen.ps1`, but GacBuild mode does not launch new `GacBuild`, instead it calls the `RunGacGen` function.
    - `-pathGacGen:"absolute path"`
    - `-pathCppMerge:"absolute path"`
    - These paths should be absolute paths and will be checked, it doesn't assume where the tool is.
  - It also accepts arguments passed to the original `GacBuild.ps1` keeping their original semantics.
  - `GacBuild.ps1` and `GacGen.ps1` remain and become wrappers to call `GacBuild`
    - Arguments should not change.
    - `GacBuild` should produce exactly the same log files otherwise `GacGen` won't work.
      - But get rid of `Deploy.bat`, instead it prints a list of files and `GacBuild` will do the copy by itself.
      - Just like how CLI workflow rpc test is doing, running processes and copy files etc should be implemented per different OS in their native ways.
  - Update necessary documents on `GacUI` repo:
    - Knowledge base pages change should be done on `Tools`.
  - Commit and push before doing the rest.

### DETAILS

- Keep the existing `GacGen` executable as the one-architecture compiler. The new orchestration tool belongs under `<GacUI repo>/Tools/GacBuild`, with its own project identities and output names. Adapt the Debug/Release, Win32/x64, C++20, runtime and platform settings from `<GacUI repo>/Tools/GacGen`; also provide the native Unix `vmake` configuration. GacBuild mode calls the same `RunGacGen` implementation used by GacGen mode, while launching the existing GacGen and CppMerge executables as children.
- Preserve both wrapper interfaces: `<Tools repo>/Tools/GacBuild.ps1 -FileName <driver-xml> [-Dump]` and `<Tools repo>/Tools/GacGen.ps1 -FileName <resource-xml> [-MappingFileName <mapping>]`. The native tool must accept the corresponding mode-specific arguments, including the optional mapping. Both tool-path arguments identify existing executable files and remain required in both modes; reject missing, duplicate, empty, relative or invalid tool paths before altering logs. Resolve relative resource/mapping paths from the caller's working directory. Keep each path as one argument, including spaces and non-ASCII characters.
- Port the planning logic from `<Tools repo>/Tools/GacCommon.ps1` as well as the two entry scripts. Preserve namespace-tolerant resource discovery, slash-normalized case-sensitive substring exclusions, `/D32` metadata inspection, anonymous resources before dependency-ordered named resources, and propagation from outdated resources to all named transitive dependents. Reject duplicate names, missing dependencies and cycles with useful diagnostics. `-Dump` still produces planning artifacts without compiling resources.
- Preserve the driver artifacts under `<application repo>/<driver-xml>.log`: `ResourceFiles.txt`, `BuildCandidates.txt`, `ResourceAnonymousFiles.txt`, `ResourceNamedFiles.txt`, `ResourceNamedMapping.txt`, and flattened metadata XML dumps. Preserve record formats and Windows path behavior; normalize native separators when flattening Unix paths. Keep the driver XML distinct from resource XML. Retain `<Tools repo>/Tools/GacCommon.ps1` functionality still used by `<Tools repo>/Tools/GacClear.ps1`.
- Preserve existing log encodings and use matching decoders, including the current MBCS/no-BOM `CppOutput.txt` contract. Its Windows paths remain limited to characters representable in the active code page; do not claim arbitrary Unicode path support. The new deployment list independently uses UTF-8.
- Preserve per-resource caches under `<application repo>/<resource-xml>.log/x32` and `x64` (the former is named `x32`, not `x86`). GacGen continues to produce compiler, binary, C++, and RPC artifacts; GacBuild preserves and consumes their contracts. Incremental freshness still compares input UTC timestamps with the five standard binary caches for each architecture. It does not start tracking production C++, deployed binaries, RPC files or tool binaries; explicit cache clearing is required when those alone change.
- `RunGacGen` clears the resource cache once, runs `/P32` and `/P64` with the same mapping, and validates both architecture results before merging. When C++ is configured, require matching staged filename sets and production destinations from `CppOutput.txt`, create the destination directory, and invoke CppMerge once per pair. Preserve existing destination files so `USER_CONTENT` regions and unchanged-file timestamps survive. Resources without C++ configuration still compile and deploy their configured binaries.
- Replace each architecture's `Deploy.bat` with a structured UTF-8 deployment list, for example `<application repo>/<resource-xml>.log/<architecture>/Deploy.xml` containing escaped source/destination pairs. Keep ordinary diagnostics separate from this machine-readable list. Currently `/P` both writes configured binaries and emits the batch recipe; move `/P` deployment to GacBuild so it stages first and copies only after both compilations and merging succeed. Consume both architecture lists, preserve the final x32 choice for shared neutral outputs, and retain direct `/C32` and `/C64` publication behavior. Validate the complete copy inventory before copying and use native file operations, without running a shell recipe.
- Keep the native process/copy boundary small, with Windows and POSIX implementations in platform-specific source files. Follow the existing CLI Workflow RPC process-launching examples without depending on their test-only transport. Pass explicit executable paths and argument lists, wait for children, surface their output and failures, and avoid introducing parallel resource builds or background polling.
- Make failure handling explicit: the old build script catches per-resource failures and continues, while some GacGen errors and CppMerge I/O failures can return zero. The new tool and wrappers should stop and return nonzero for invalid input, failed children, compiler errors, missing required artifacts, merge failures or copy failures. Validate semantic success as well as exit codes; do not accept pre-existing output as proof that an attempted write succeeded. This intentionally improves failure reporting while preserving successful invocation semantics. Keep error handling limited to reporting and exiting.
- If generation, merging or deployment fails after the standard binary caches have been written, invalidate at least one required cache file for that resource before reporting failure. Otherwise the unchanged ten-file freshness check would skip the failed resource on the next run. Retain diagnostic logs; this is build-state correctness, not recovery or a new cache format.
- Update `<GacUI repo>/Project.md` and relevant tool documentation. As requested, author KB changes in `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_GacGenAndGacBuild.md` and related index/CppMerge guidance, then propagate them in the second stage. Document the deployment-list schema and changed `/P` and failure behavior. Commit and push the first-stage changes in every repository touched before starting release integration.

### VERIFICATION

- Use the repository build/run wrappers to build the new tool and changed GacGen in Windows Debug and Release configurations, covering Win32 and x64. Verify that each build uses its own executable name. Tool-only changes may skip the GacUI unit suite under `<GacUI repo>/Project.md`; any shared compiler/library changes trigger their normal verification requirements.
- Compare old and new planning/output behavior on equivalent clean fixtures: anonymous resources, a named dependency chain and independent resources, exclusions, and `-Dump`. Check manifest formats, dependency order, mapping contents and both architecture caches. A second unchanged build must skip all resources; changing a base input must rebuild it and its transitive dependents; deleting one standard cache output must select that resource again. Confirm production-output-only deletion retains the existing skip behavior and cache clearing restores regeneration.
- Exercise direct native modes and the unchanged PowerShell interfaces, including relative resource paths, an unrelated working directory, explicit tool locations, spaces and non-ASCII paths representable by the existing log encodings, optional mapping, and resources with no C++ output. Compare generated C++, binary payloads and logs, allowing only the documented deployment-list replacement and platform-specific path representation.
- Check complete x32/x64 C++ pairs, native-width `vint`/`vuint` merging, preserved `USER_CONTENT`, unchanged-output timestamps, embedded resources, and RPC C++ plus metadata for both ABIs. Verify each deployed binary against its selected cache payload and verify that direct `/C` generation still publishes its outputs.
- Exercise invalid tool arguments, malformed input/mapping, missing dependencies, cycles, compiler errors with and without `Errors.txt`, missing architecture artifacts, mismatched C++ pairs, and unwritable merge/copy destinations. Require diagnostics and nonzero status through the native executable and wrappers; no merge or deployment should follow a failed prerequisite. Verify a subsequent corrected invocation succeeds without stale caches masking the failure.
- Compare against the documented contracts in `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_GacGenAndGacBuild.md` and `<GacUI repo>/.github/KnowledgeBase/KB_Workflow_Design_CppMerge.md`. Record checks actually run separately from native Unix checks unavailable on Windows.

## Task 2: Tools and Release integration

- Update `Tools` and `Release`.
  - In `Release` repo equivalent `GacBuild.sh` and `GacGen.sh` will be created.
  - Powershell versions will be updated from `Tools` like today:
    - Fix `Build.ps1 -Project GacUI` accordingly.
    - Fix `Build.ps1 -Project Release` accordingly.
  - Wrapper scripts know and calculate absolute paths to pass to the GacBuild tool.
  - Commit and push, and then run `job:copilotInitAll` but skip learning, check out `../AGENTS.md`.

### DETAILS

- In `<Tools repo>/Tools/ProjectGacUI.ps1`, preserve the bootstrap order: metadata, initial GacUI CodePack, build/deploy generation tools, regenerate DarkSkin/TuiSkin, then final CodePack. Build and deploy GacBuild before `Update-GacUI-Skins` starts using the rewritten wrappers. Include the new executable in cleanup/deployment performed by `<Tools repo>/Tools/Build.ps1`; avoid a dependency from generator compilation to the skins it generates.
- Extend `<Tools repo>/Tools/BuildRelease.ps1` to copy the GacBuild sources, including native platform files, from their GacUI owner. Add its project/filter entries and configurations to `<Release repo>/Tools/Executables/Executables.sln`, plus deployment inventory in `<Release repo>/Tools/CopyExecutables.ps1`. Release packaging configurations are maintained in Release; copied implementation files remain maintained in GacUI.
- Add the native Unix build inventory under `<Release repo>/Tools/Executables/GacBuild` and update `<Release repo>/Tools/BuildExecutables.sh` to build and deploy the new tool. This pipeline uses per-tool makefiles, distinct from GacUI's `vmake` setup. Add `<Release repo>/Tools/GacBuild.sh` and `<Release repo>/Tools/GacGen.sh` as thin wrappers with equivalent public operations, quoting and exit-status propagation.
- All wrappers locate executables relative to their own script directory and supply absolute tool paths; the native GacBuild executable must not infer those locations. Preserve GacGen metadata lookup: released GacGen uses adjacent `Reflection32.bin`/`Reflection64.bin`, while the development `Metadata.txt` selects core-only metadata. Ensure release cleanup and subsequent updates retain the Bash wrappers and all required executables/metadata.
- Complete and commit/push Tools and Release integration before the synchronization job. For `job:copilotInitAll` with learning skipped, begin at its **Sync Back Knowledge Base and Instructions** section: use `<Tools repo>/Copilot/copilotInitAll.ps1`, then `<Tools repo>/Tools/CheckRepo.ps1` with `CheckAll`, following the job's parameters and review instructions. Skip the earlier preparation, learning and `-UpdateKB` stages. Review propagated files and commit/push every affected repository afterward; initialization replaces target KB directories, so the canonical Tools changes must be present first.

### VERIFICATION

- Run `<Tools repo>/Tools/Build.ps1 -Project GacUI` and then `-Project Release` through the prescribed workflow. Confirm the freshly built/deployed GacBuild is actually invoked, both skins regenerate, and the Release build's Workflow, XML-generation and tutorial C++ checks succeed.
- Clear relevant resource caches before generation checks because tool binaries are not timestamp inputs. Inspect every tutorial's x32/x64 logs for errors and required nonempty artifacts, compare deployed binaries to their caches, and explain generated source changes outside imported snapshots. Do not infer success solely from the build script's final message.
- Check Windows wrapper compatibility from another working directory and paths containing spaces. Syntax-check Bash wrappers and audit Unix build/source inventories on this host; explicitly distinguish these static checks from unperformed Linux/macOS execution.
- Check source-copy/deployment inventories for both native implementations, metadata availability, and persistence of the Bash wrappers across release updates. After instruction synchronization, verify the canonical KB changes survived, inspect unexpected deletions/drift, and complete the job's commit/push requirements before the next stage.

## Task 3: wGac and iGac integration

- Update `wGac` and `iGac`.
  - Just update `syncProj.sh`, `AGENTS.md`, `README.md` as well as other necessary files, you are not able to run them on Windows.
  - Currently they only build and call `GacGen` tool to only generate cpp files for x64, you are now going to generate x64/x86 and do the merging, using the new `GacBuild` tool.

### DETAILS

- Update `<wGac repo>/syncProj.sh` and `<iGac repo>/syncProj.sh` to build native GacBuild alongside GacGen and CppMerge using the existing build helper. Both scripts already build CppMerge but currently generate with GacGen `/C64`. Invoke the new tool in GacGen mode for each existing resource, passing absolute paths to the selected GacGen and CppMerge executables; no new driver XML or dependency on the Release checkout is needed.
- Preserve the temporary GacGen symlink and adjacent `Metadata.txt` that select full `Reflection32.bin`/`Reflection64.bin`. Pass the absolute symlink path without resolving it to the underlying executable: GacGen derives metadata lookup from its invocation path, and the development executable's usual metadata selects the core-only types.
- Preserve resource rewriting and seed C++ from `<GacUI repo>/Test/Resources/App`, plus shared entry points, palette handler, RVM initializer and argument header from `<GacUI repo>/Test/GacUISrc`. Generate `/P32` and `/P64`, merge into each existing application source directory, and retain both staging trees until generation and merging are verified. Preserve seed/user content, embedded resources and `RemoteViewModelTestRpc.h/.cpp`; keep generated reflection files and their existing exclusion from no-reflection targets.
- Update `<wGac repo>/AGENTS.md`, `<wGac repo>/README.md` and the equivalent `<wGac repo>/README_CN.md`. Update `<iGac repo>/AGENTS.md` and its actually tracked lowercase `<iGac repo>/readme.md`. Describe the new build dependencies, dual-architecture generation, merging, metadata selection and diagnostic artifacts.
- This stage is limited to script/document changes on Windows as requested. Preserve the repositories' normal native import/sync/build instructions, but record those execution checks as unavailable for this task. Commit and push both repositories when their requested changes are complete.

### VERIFICATION

- Run Bash syntax checks without executing either native synchronization/build script. Check all three tool build/output paths, argument quoting, full metadata selection and error handling for both architecture outputs.
- Audit that existing build definitions consume the merged ordinary, embedded-resource and RPC sources, still exclude reflection where required, and preserve shared seed files. Search changed documentation for obsolete `/C64` or x64-only instructions and keep the wGac English/Chinese descriptions equivalent.
- Record Linux/macOS generator execution, actual regeneration, compilation and application behavior as unverified on this Windows host. Do not substitute Windows-generated platform application outputs or claim static inspection proves native runtime behavior.

## REVIEW COMMENTS

No unresolved review comments. The decisions and verification requirements are recorded under each task above.

# UPDATES

# TEST

Task 1: compare native orchestration against the existing PowerShell discovery and generation contracts using isolated resources, real GacGen/CppMerge children, dependency graphs, both architectures, user-content preservation, incremental timestamps and failure/retry cases. Build all four Windows configurations and run the tool-specific verification suite. Task 2: run the requested Tools GacUI and Release pipelines, validate packaging and downstream tutorial outputs, then synchronize instructions without learning. Task 3: syntax-check and inspect the native platform scripts; Linux/macOS execution is unavailable on this Windows host.

# PROPOSALS

- No.1 Native orchestration with explicit child paths and staged deployment

## No.1 Native orchestration with explicit child paths and staged deployment

Implement the reviewed three tasks in order, committing and pushing each completed stage. Preserve existing discovery, dependency, cache and CppMerge contracts. Use one RunGacGen implementation, a small native process/filesystem boundary, and a UTF-8 deployment manifest emitted by GacGen. Validate complete child outputs and invalidate failed resource freshness. Keep native platform app edits static on Windows.

### CODE CHANGE

Task 1 implementation: added <GacUI repo>/Tools/GacBuild with shared planning/generation orchestration and separate Windows/POSIX process, timestamp and copy implementations. Updated <GacUI repo>/Tools/GacGen to stage `/P` binaries and emit UTF-8 deployment XML while retaining direct `/C` publication. Both <Tools repo>/Tools/GacBuild.ps1 and <Tools repo>/Tools/GacGen.ps1 are thin wrappers. Canonical KB changes are authored in <Tools repo>/Copilot/KnowledgeBase; <GacUI repo>/Project.md describes maintenance and verification.

The final native suite passed 37 invocations plus PowerShell wrapper checks. It compares legacy planning manifests and deployed binary hashes, and covers dependency propagation, both-ABI caches, C++/RPC/embedded outputs, user regions, stable timestamps, malformed mapping rejection, missing dependencies, cycles, compiler artifact faults, direct publication, wrapper path handling, cache clearing, and locked merge/copy failures with successful retries. A binary destination inside a newly created C++ output directory is also covered. Both tools build in Debug/Release and Win32/x64 with zero warnings and errors. POSIX source and inventories are reviewed statically; native execution is unavailable on Windows. Packaging and platform synchronization follow only after the first-stage commit.

The representable non-ASCII path case exposed an upstream Windows MBCS decoder defect: <VlppOS repo>/Source/Encoding/CharFormat/CharFormat.Windows.cpp used the system code page for lead-byte detection but the thread code page for conversion. This host has system code page 1252 and thread code page 936; `CppOutput.txt` correctly contains GBK bytes but the reader split them incorrectly. Lead-byte detection now uses `CP_THREAD_ACP`, with stream round-trip regressions under both locales. The VlppOS Debug x64 solution builds cleanly; all 16 test files and 309 cases pass without a memory-leak report. Its owning release was regenerated and copied to GacUI through the standard tools. GacBuild's MBCS path contract is unchanged. GacGen now uses encoding detection for optional dependency maps; UTF-8/no-BOM maps with non-ASCII resource paths pass the integration suite.

The full GacUI Debug x64 solution builds with zero warnings and errors, and all 93 unit-test files and 1814 cases pass with no memory-leak report. The run's unrelated snapshot drift (current calendar month, Windows absolute paths and UTF-16 caret lengths, and file-dialog scheduling) is retained as an ignored local patch; checked-in snapshots are restored. Task 1 is verified and ready for its required commit before release integration. Native Linux/macOS builds and execution remain unverified.
