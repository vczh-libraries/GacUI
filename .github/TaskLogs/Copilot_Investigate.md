# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

This task is about refactoring `GuiInstanceLoader_EasyLayout.(h|cpp)`.

`GuiEasyCellOptionDeserializer`:

I think the whole class just should not exist. Table's rows and columns already accept `CellOption` as a collection property of `GuiCellOption`, which means this type is already serializable, I believe that could just be reused.

`IsEasyLayoutConstantProperty`:

I think limit `CellOption` and `Percentage` of an easy layout to only use constant does not help, we should just expose it as a trivial property (which already did via reflection). Since only calling `BuildLayout` makes them being used, there is no reason to make it a constant in XML. So this function should be no longer useful.

`VisitEasyLayouts`:

This function should not exist either. `BuildLayout` will check all of these (if not you should do it). There is no need to verify the layout during compiling XML resources.

`Workflow_BuildEasyLayouts`:

`CreateInstance` runs before properties, children, and bindings are initialized, so it is too early to call `BuildLayout` there. Replace the easy-layout-specific final pass with a general initialization mechanism.

Add a new function to `IGuiInstanceLoader`:
```C++
virtual Ptr<workflow::WfStatement> InitializeInstance(you decide the argument);
```
The default implementation returns null, meaning there is nothing to initialize.
For each object, start from its most specific instance loader and follow the existing `GetParentLoader` chain all the way through the default loader. Call every loader's `InitializeInstance` in that order, regardless of `CanCreate`, and collect every non-null statement. A null result does not stop traversal. Constructor selection and `CreateInstance` remain unchanged.

Visit the object tree in depth-first postorder: initialize all subobjects before their owner, and run the complete loader chain for each object. Generate this final initialization after all creation, assignments, and bindings are ready, and append the collected statements to the generated constructor helper's initialization block before the root instance's `ref.Ctor` body.

Only the EasyLayout owning-layout loader needs to return a statement in this task; all other loaders use the default no-op implementation. Therefore this new dispatch mechanism should leave unrelated generated code unchanged. Minor differences in the generated `BuildLayout` calls are acceptable.

`GuiInstanceLoader_EasyLayout.h` and `LoadEasyLayouts`:

From here I believe everything is already waste except `GuiEasyInstanceLoader<T>` and `LoadEasyLayouts`, and the `BuildLayout` calls is moved to `GuiEasyInstanceLoader<T>`. Then we could move `LoadEasyLayouts` forward declaration to the list in `GuiInstanceLoader_Plugin.cpp`. Complete the table in the top comment in `GuiInstanceLoader_Plugin.cpp`, and then no one needs `GuiInstanceLoader_EasyLayout.h` anymore so this header file could be deleted. We could clean up `GuiInstanceLoader_EasyLayout.cpp` to make it very small, with one additional mechanism `IGuiInstanceLoader::InitializeInstance` added to the compiler.

Document:

Verify if this task makes any fact in GacUI's knowledge base stale, update them.

Verification:

Since we are doing a refactor with no change to the compiler semantic, you need to only:
- Run unit test
- Update release of `GacUI`
- Call `../Tools/Tools/Build.ps1 -Project UpdateRelease`
- Rebuild `../Release/Tools`
- Call `../Release/Tools/Build.ps1` on `GacUI.xml` in `Release/Tutorials`
You should only see minor change in `Release` that out of the `Import` folder

## DETAILS

### Intended behavior changes

- Preserve the behavior of existing valid resources. The requested removal of constant-only restrictions and compile-time layout validation intentionally changes which resources compile and when invalid layouts fail; these are exceptions to the semantic-preservation statement above.
- `GuiCellOption` already has reflected fields in `Source/Reflection/TypeDescriptors/GuiReflectionCompositions.cpp`. Reuse the ordinary struct-expression path in `Workflow_ParseTextValue` (`Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowParser.cpp`), as Table does. Remove the EasyLayout deserializer and its constant-expression helpers without introducing a replacement serializer.
- Let reflection handle `CellOption` and `Percentage` as ordinary properties. Remove both the custom property metadata and the binding prohibition in `GuiInstanceLoader_WorkflowCollectReferences.cpp`. Supported bindings and well-typed runtime expressions become legal; ordinary syntax, type, and binder restrictions still apply. In particular, removing the prohibition does not make every existing negative `-set`, `-ref`, or `-uri` example valid.
- These properties continue to store configuration. Initial values must be available before the automatic build; later updates require an explicit `BuildLayout`, just like `Padding`, `Border`, `Direction`, and `CellSpan`.

### Generic final initialization

- `WorkflowGenerateCreatingVisitor::Visit(GuiConstructorRepr*)` in `GuiInstanceLoader_WorkflowGenerateCreating.cpp` calls `CreateInstance` before assigning ordinary properties and children. `Workflow_PrecompileInstanceContext` currently generates creation, then bindings, then easy-layout builds. Calling `BuildLayout` inside `CreateInstance` is therefore too early.
- Add `IGuiInstanceLoader::InitializeInstance` alongside the existing creation methods in `Source/Compiler/GuiInstanceLoader.h/.cpp`. Use the existing method conventions: `GuiResourcePrecompileContext&`, `types::ResolvingResult&`, `const TypeInfo&`, `GlobalStringKey variableName`, `GuiResourceTextPos tagPosition`, and `GuiResourceError::List&`; return `Ptr<workflow::WfStatement>`. No constructor argument map is needed. The base implementation returns null as a successful no-op.
- Replace the easy-layout-specific final pass with a generic depth-first postorder traversal of resolved reference objects. Visit children in the existing representation order, then their parent. Append non-null statements only after all of `Workflow_GenerateBindings`, including event handlers and localized subscriptions. Record their source positions using the existing code-generation mechanism.
- Keep these statements in the existing constructor helper's initialization block, before the user-written `ref.Ctor` body. Do not emit them while creation unwinds: creation traverses setters in reverse order and has not installed bindings yet.
- For each object, start with `GetLoader(typeInfo.typeName)` and repeatedly call `GetParentLoader` until null, invoking `InitializeInstance` on every loader, including the default loader, in most-specific-to-default order. Use the existing manager chain rather than separately walking reflection base types. Do not filter by `CanCreate` or stop after a non-null result. Each hook contributes only its own initialization; it must not invoke parent-loader hooks itself, because the compiler owns traversal.
- Pass the same resolved object type, variable, and source position through that object's whole loader chain. Preserve the special source/base type selection for the root; do not substitute the generated root class type or each parent loader's registration type. Root final initialization must not depend on having constructor arguments, unlike the existing early `InitializeRootInstance` call.
- Preserve coverage of nested constructors and existing objects configured through `att.*-set`. The latter use `GetParameter`, not `CreateInstance`, so a queue populated only by creation calls would miss them. Existing targets still run their applicable loader chain even if no loader can construct their declared reference type. Text and serialized value expressions are not independently initialized objects. This task does not broaden which types are legal XML roots.

### Easy loader initialization and cleanup

- `GuiEasyInstanceLoader<T>` currently overrides neither `CanCreate` nor `CreateInstance`; construction falls through to the default loader. The new full-chain initialization reaches it independently, so keep that construction behavior and the existing constructor/root selection unchanged.
- Add the hook to the owning-layout specialization so it emits one `BuildLayout` call for that object. Descriptor loaders and all unrelated loaders contribute no statement. Null results must not create empty generated statements or blocks; the added dispatch alone should not change unrelated generated initialization.
- Remove `IsEasyLayoutConstantProperty`, `VisitEasyLayouts`, both easy-layout workflow entry points, and their includes/call sites. Move `LoadEasyLayouts` into the declaration and registration lists in `GuiInstanceLoader_Plugin.cpp`, remove its old invocation from `LoadCompositions`, and update the plugin's summary table.
- Delete the now-unused header and remove its entries from `Source_GacUI_Compiler.vcxitems` and `.vcxitems.filters`, following `.github/Guidelines/SourceFileManagement.md`. Keep the implementation's required includes explicit.

### Runtime validation and payload ownership

- Audit the removed checks against `GuiEasyLayoutBuilder::Register`, `Prepare`, and `ValidateOption` in `Source/GraphicsComposition/EazyLayout/GuiEasyLayout.cpp`. Runtime checks already cover most descriptor grammar, splitter rules, bounds requirements, ownership, cycles, and numeric validity. Preserve the native table's handling of invalid cell sites and the ignored inner track option when `CellSpan` is not 1.
- Multiple declarative payloads need an additional runtime initialization check. The old visitor rejects `<ez:Layout><Button/><Button/></ez:Layout>`, but `GuiEasyLayout::SetComposition` replaces and deletes an unbuilt previous payload. By the time `BuildLayout` runs, the duplicate is invisible and references to the first payload may dangle.
- Distinguish checked insertion of initial XML content from ordinary later property replacement. Reject duplicate initial payload insertion before overwriting the first payload, covering default children, explicit `att.Composition`, and combinations in either processing order. Emit the check as runtime initialization behavior rather than restoring compile-time grammar validation. Preserve intentional later `SetComposition` replacement and binding updates. This ownership check necessarily occurs before `BuildLayout`, as existing payload ownership checks already do.

### Documentation

- Update `.github/Guidelines/GacUIEazyLayout.md` and `.github/KnowledgeBase/manual/gacui/components/compositions/eazylayout.md`: both currently require constant `CellOption` and `Percentage` values. Correct binding support and failure timing while retaining explicit-rebuild and initialization-order guidance.
- Update other knowledge-base descriptions only where the implementation changes their facts. Keep compiler-specific hook documentation with `IGuiInstanceLoader`; do not expose those details in ordinary layout usage instructions.

## VERIFICATION

These steps apply when implementing the task; this review does not execute the refactor or release pipeline.

1. Update the mixed rejection case in `Test/GacUISrc/UnitTest/TestResource.cpp`. Separate supported bindings/runtime field expressions that now compile, ordinary syntax/type/binder failures that still fail compilation, and layout grammar that compiles but fails during initialization or explicit `BuildLayout`. Do not retain source-positioned compiler-error expectations for removed grammar checks.
2. Extend the ordinary resource tests in `TestControls_EasyLayout.cpp`. Cover attribute and property-element forms, shorthand and qualified `ComposeType` values, arithmetic/default struct fields, and actual decoded `CellOption` values. Verify initial `CellOption`/`Percentage` bindings affect the first automatic build, then verify later changes leave generated tracks unchanged until explicit rebuilding.
3. Verify both initialization orders: subobjects before their owners, and each object's loaders from most specific through default. Check that null and non-null results both continue traversal, a loader with `CanCreate == false` still contributes its hook, and the owning-layout loader emits `BuildLayout` only once. Initial bindings/events must be ready and `ref.Ctor` must observe a built layout. Cover existing `att.*-set` targets without requiring constructibility, the root's no-constructor-argument path, and a resolved concrete type different from its loader's registered base. Confirm constructor selection stays unchanged and no-op loader chains leave unrelated generated code unchanged.
4. Retain existing ordinary geometry/ownership/failure coverage and add only missing runtime cases. Test duplicate unnamed payloads and mixed unnamed/explicit `Composition` payloads for both owners and descriptors, while preserving legitimate later replacement. Existing negative cases are in `TestCompositions_EazyLayoutFailures.cpp`; verify that the chosen test build actually compiles and runs that file. Use ordinary tests for non-rendering behavior, following `Project.md`.
5. Build Debug x64 through `.github/Scripts/copilotBuild.ps1` from `Test/GacUISrc`, then run `UnitTest` through `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`. Preserve relevant filters and ensure the resource, EasyLayout control/composition, and failure cases actually execute. Inspect completed `Build.log` and `Execute.log`, including the Debug leak report. If reflection registration changes or `GacUI_Compiler` runs, follow the metadata-generation requirements in `Project.md`.
6. Run the GacUI release pipeline, then `& C:\Code\VczhLibraries\Tools\Tools\Build.ps1 -Project UpdateRelease`. The latter already builds `../Release/Tools/Executables/Executables.sln` in Release x86 and deploys its executables. For the separately requested rebuild, use the repository build wrapper from that solution directory with `-Configuration Release -Platform Win32` (the wrapper maps the solution platform to x86), then invoke `& C:\Code\VczhLibraries\Release\Tools\CopyExecutables.ps1`. Verify the deployed GacGen and metadata come from the updated release.
7. The actual tutorial driver is `../Release/Tutorial/GacUI.xml` and the script is `../Release/Tools/GacBuild.ps1`. Refresh caches with `& C:\Code\VczhLibraries\Release\Tools\GacClear.ps1 -FileName C:\Code\VczhLibraries\Release\Tutorial\GacUI.xml`, then run `& C:\Code\VczhLibraries\Release\Tools\GacBuild.ps1 -FileName C:\Code\VczhLibraries\Release\Tutorial\GacUI.xml`. Compiler binaries are not tracked by the resource timestamp cache, so rebuilding GacGen alone can leave tutorials skipped.
8. Inspect per-resource results for both x32 and x64: `GacBuild.ps1` catches failures and continues, so script completion is insufficient. Check errors and expected outputs, then inspect the sibling Release repository's diff outside `Import`. Existing valid tutorials should retain equivalent initialization/layout behavior with only small explainable generated changes. Do not hand-edit generated outputs or conflate this sibling repository with GacUI's own `Release` directory.

## REVIEW COMMENTS

No unresolved review comments; the implementation decisions and verification corrections are recorded above.

# UPDATES

# TEST [CONFIRMED]

First reproduce the rejected well-typed `Percentage-eval="1.0 + 2.0"` expression in the ordinary resource compiler tests. Extend ordinary resource/control tests to distinguish supported expressions, ordinary compiler failures and runtime grammar failures, inspect decoded options and the first build, and verify explicit rebuilding after binding updates. Exercise final initialization ordering with resolved nested/existing objects and the complete loader chain, preserving constructor selection and no-op output. Retain geometry/ownership tests and add duplicate initial payload rejection without disabling later replacement.

The baseline Debug x64 build succeeded with zero warnings/errors. The unfiltered UnitTest run reached the new TestResource regression and failed `errors.Count() == 0`, reporting `Easy layout: Percentage must be constant and does not accept any binding.` No earlier assertion failed. Baseline execution also rewrote existing file-dialog scrolling snapshots and theme-refresh frame values; these are baseline-generated differences to review separately from the refactor.

Build Debug x64 and run UnitTest through the repository wrappers, checking the completed build/test logs and Debug leak report. Then regenerate GacUI release, run UpdateRelease, rebuild/deploy sibling Release tools and clear/recompile all tutorials for both architectures. Inspect each resource result and explain every sibling Release change outside Import. Follow metadata generation if reflection changes.

# PROPOSALS

- No.1 General postorder initialization and ordinary reflected layout properties [CONFIRMED]

## No.1 General postorder initialization and ordinary reflected layout properties

Add the default no-op `IGuiInstanceLoader::InitializeInstance` hook and a representation visitor after binding generation. Initialize reference subobjects in representation order before their owner, then invoke every loader in the existing manager chain without testing `CanCreate`. Preserve the root's source/base type selection and record each returned statement's source location. Keep constructor selection and the earlier constructor-argument initialization unchanged.

Reduce the EasyLayout loader to child assignment and owning-layout initialization. Remove the custom deserializer, constant-property restrictions, static grammar visitor and obsolete header. Ordinary reflected struct fields already supply the needed parser. Runtime builder validation remains authoritative. Generate a null-payload guard before declarative Composition assignment (both unnamed and explicit), so a duplicate cannot delete the first payload; normal setters and reactive binding updates retain replacement semantics. This needs no new reflected runtime API.

### CODE CHANGE

Added the compiler hook/default implementation and a postorder visitor after bindings. Removed the EasyLayout deserializer, binding prohibition, grammar visitor and header, and moved registration to the plugin. The reduced loader emits checked initial payload assignments and one owning-layout BuildLayout call. Ordinary tests now separate compiler failures from accepted runtime expressions/grammar, exercise the initialization chain and generated payload guards, and check decoded fields and bound tracks before/after rebuilding. Updated the authoring guideline, the layout manual and its composition overview; documented the hook contract beside the interface. Native table placement and explicit-rebuild behavior are unchanged.

The runtime audit found that Register/Prepare/ValidateOption already cover descriptor grammar, splitters, bounds requirements, ownership, cycles and numeric checks. No native builder or reflection registration change is needed. Initial payload guards use Workflow's `is not null` type test and independently owned expression nodes. They run for unnamed and explicit Composition assignment before either can replace a previous payload. Ordinary SetComposition and reactive setters retain their existing behavior.

### CONFIRMED

Focused debugger verification passed both TestControls_EasyLayout.cpp and TestResource.cpp: 2/2 files and 37/37 cases. This includes eight generated duplicate-insertion scenarios (both owner kinds and all unnamed/explicit combinations), retained first payloads and subsequent legitimate replacement; six struct-expression forms including qualified enum values with documented semicolon disambiguation and reflected defaults; initial CellOption/Percentage bindings, later explicit rebuilding, event installation before the automatic build and ref.Ctor observing the built tree. The synthetic resolved-reference test checks postorder, null/non-null hook continuation, nonconstructible existing targets, root source type without constructor arguments, default-loader construction, source positions, no-op output and one owning-layout call.

The completed Debug x64 wrapper build has zero warnings and zero errors. The unfiltered wrapper UnitTest run passed 93/93 files and 1811/1811 cases. Execute.log explicitly includes TestResource.cpp, TestControls_EasyLayout.cpp, TestCompositions_EasyLayout.cpp and TestCompositions_EazyLayoutFailures.cpp, with no skipped files and no appended memory-leak dump. Restored the same nine unrelated file-dialog frame-timing and date-dependent theme snapshots that the baseline run changed.

Ran the existing Release-GacUI pipeline from Tools/Tools/Common.ps1 and ProjectGacUI.ps1. It packed the compiler, rebuilt GacGen in Release Win32, regenerated DarkSkin and TuiSkin for x32 and x64, merged their C++ outputs and packed the final release. Both skins' generated code remained unchanged. GacUI's own Release diff contains only GacUICompiler.cpp, GacUICompiler.h and IncludeOnly/GacUICompiler.h.

Ran Tools/Tools/Build.ps1 -Project UpdateRelease successfully, including its Release x86 rebuild and deployment of all six tools. Then ran GacUI/.github/Scripts/copilotBuild.ps1 -Configuration Release -Platform Win32 from Release/Tools/Executables, verified the completed Build.log reports zero warnings and errors for Release|x86, and invoked Release/Tools/CopyExecutables.ps1. The deployed GacGen matches Executables/Release/GacGen.exe (SHA256 5C64B39EE735A9EB5829D5BC71C7E1294876D21BD62AA4EE032CFE283EE1B314). Both deployed reflection metadata files match GacUI/Test/Resources/Metadata and Tools/Tools, and the sibling compiler imports match GacUI's packed release. Reflection registration was unchanged, so metadata regeneration was not required.

Ran Release/Tools/GacClear.ps1 and GacBuild.ps1 on Release/Tutorial/GacUI.xml. All 52 resources were selected for rebuilding. Verified all 104 architecture results individually: no Errors.txt, nonempty Workflow output, all 520 expected binary outputs freshly generated, 104 configured binary deployment hashes matching their architecture outputs, and all 556 merged C++ source checks present and nonempty.

Reviewed the complete sibling Release diff outside Import. Its only changes are 364 added lines in 17 generated C++ files: exactly 182 two-line guards testing GetComposition() before initial payload insertion. There are no removals or other changes, including no changes to existing BuildLayout calls. Thus valid tutorials retain their previous generated behavior while duplicate initial payloads fail before replacement. Unrelated no-op loader chains produce unchanged code. Both repositories pass git diff --check. The proposal is confirmed and the implementation is retained.
