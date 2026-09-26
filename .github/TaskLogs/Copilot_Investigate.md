# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

I think by changing the default property for layouts to `SupportArray`, and disable both Composition and Layouts property to make it Unsupported, we can delete:

- `LoaderStateMap`
- `ResolvingResult::loaderStates`
- `GuiEasyInstanceLoaderState`

And therefore make `AssignParameters` simpler as it reads all values at the same time. perform this refactor, and then run the verification in the original request.

# UPDATES

# TEST [CONFIRMED]

The framework batches SupportArray values for a property and its selected loader in ProcessPropertyOthers. Returning Unsupported for Composition and Layouts stops loader fallback before ordinary assignment, eval/uri/bind and set handling. Verify duplicate mixed default payloads with layout descriptors interspersed, valid ordered sibling descriptors, independent layouts, and source-positioned rejection of both named properties for owners and descriptors. Preserve runtime setters, initialization order, property expressions/bindings and native BuildLayout grammar checks.

Build Debug x64 through .github/Scripts/copilotBuild.ps1 from Test/GacUISrc and run UnitTest through copilotExecute.ps1 -Mode UnitTest -Executable UnitTest. Inspect completed logs and Debug leak output. Run Release-GacUI through the existing Tools pipeline, then Tools/Tools/Build.ps1 -Project UpdateRelease. Rebuild Release/Tools/Executables with the GacUI wrapper in Release Win32 (mapped to x86), deploy with CopyExecutables.ps1, then clear and build Release/Tutorial/GacUI.xml using GacClear.ps1 and GacBuild.ps1. Verify every x32/x64 resource output, deployment and generated-source diff; generated files must not gain runtime duplicate guards. Reflection registration is unchanged, so metadata regeneration is needed only if that changes or GacUI_Compiler is run.

# PROPOSALS

- No.1 Batch default XML content and reject named content properties [CONFIRMED]

## No.1 Batch default XML content and reject named content properties

Change GuiEasyInstanceLoader<T>::GetPropertyType to Array(nullptr) for the default property, retaining the layout/control/composition acceptable types. Return Unsupported() for Composition and Layouts, including descriptor types handled by the shared loader. All initial XML content then arrives in one call under the empty property key. Count composition/control payloads locally in AssignParameters and preserve the current source-positioned duplicate diagnostic. Remove LoaderStateMap, ResolvingResult::loaderStates and GuiEasyInstanceLoaderState. Leave final initialization, reflected native APIs and BuildLayout validation intact.

### CODE CHANGE

Simplified assignment generation to iterate the default property's batch with a local occupied flag. Updated ordinary compiler rejection tests for duplicate batches and unsupported property forms. Converted existing successful XML examples and runtime-grammar tests from named content properties to default children. Kept compiled ordinary replacement coverage using the default property. Updated the authoring guideline, manual and compiler knowledge base to document the new XML restriction and stateless validation. Generated release artifacts through the original tools after Debug verification.

Implemented the proposal and reviewed the source diff. The Debug x64 wrapper build passed with zero warnings/errors in 1:15. Focused CDB verification passed TestResource.cpp and TestControls_EasyLayout.cpp: 2/2 files and 39/39 cases, including 16 duplicate-payload XML combinations and 40 unsupported-property forms. The latter cover both owners and descriptors, each property alone or alongside a default payload, plain attributes/elements, set elements and eval/bind/uri attributes/elements. Successful cases preserve independent instances, ordered descriptors, ordinary runtime replacement, initialization and supported property bindings. No resource snapshots changed during focused testing.

Release-GacUI completed both CodePack passes, the Release Win32 GacGen rebuild and DarkSkin/TuiSkin generation for x32/x64. All four skin results have nonempty assemblies and no Errors.txt; skin sources remain unchanged. The packed release changes only GacUICompiler.cpp and GacUICompiler.h. Reflection registration was not changed and GacUI_Compiler was not run.

UpdateRelease rebuilt and deployed all six Release tools successfully. The additional wrapper build of Release/Tools/Executables resolved Release|Win32 to Release|x86 and passed with zero warnings/errors, followed by CopyExecutables.ps1. Deployed GacGen matches its rebuilt executable (SHA256 8ED3C7AA942C12FB8D6F96342FF83FB46388A4A7E758C839CC46BB83E6DE961F), both compiler imports match the GacUI packed files, and deployed Reflection32.bin/Reflection64.bin match GacUI/Test/Resources/Metadata and Tools/Tools. Cleared tutorial caches before starting GacBuild.ps1 at 2026-09-26 22:14:33 UTC.

The completed unfiltered Debug x64 wrapper run passed 93/93 files and 1813/1813 cases. Execute.log includes TestResource.cpp, TestControls_EasyLayout.cpp, TestCompositions_EasyLayout.cpp and TestCompositions_EazyLayoutFailures.cpp, with no skipped files or appended memory-leak dump. Restored seven unrelated file-dialog scheduling and date-dependent theme snapshots from the same cases identified during the previous investigation. The two changed file-dialog frames have identical content after normalizing object identifiers. No Test/Resources files remain changed.

GacBuild.ps1 completed successfully for all 52 tutorial resources and both architectures. Independently checked all 104 architecture output directories: no Errors.txt, fresh nonempty Workflow.txt and 520 fresh nonempty binary artifacts, 556 generated-source/merged-destination checks, and 104 configured deployment copies with matching SHA256 hashes. Unchanged merged C++ files retain their timestamps because CppMerge does not rewrite identical content.

### CONFIRMED

SupportArray supplies all default child values to one AssignParameters call, including mixed controls, compositions and layout descriptors. Unsupported for both named content properties prevents alternative XML setters or binders from bypassing that batch. The local flag preserves the original source-positioned duplicate-payload compilation error, so LoaderStateMap, ResolvingResult::loaderStates and GuiEasyInstanceLoaderState are no longer needed and have been removed. The expanded rejection tests and full suite confirm this behavior while preserving supported bindings, runtime replacement, initialization and native BuildLayout checks.

The sibling Release tutorial diff contains exactly 17 generated C++ files. Automated comparison confirms all other statements are identical and each parent's Layouts.Add arguments remain in the same order. The only changes move 245 Layouts.Add statements into their batched assignments and remove 490 redundant brace lines. No runtime validation guards are added. Packed compiler sources and imported copies contain the same refactor. Both repositories pass git diff --check. The proposal is confirmed and the implementation is retained.
