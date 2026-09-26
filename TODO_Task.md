This task is about refactoring `GuiInstanceLoader_EasyLayout.(h|cpp)`.

`GuiEasyCellOptionDeserializer`:

I think the whole class just should not exist. Table's rows and columns already accept `CellOption` as a collection property of `GuiCellOption`, which means this type is already serializable, I believe that could just be reused.

`IsEasyLayoutConstantProperty`:

I think limit `CellOption` and `Percentage` of an easy layout to only use constant does not help, we should just expose it as a trivial property (which already did via reflection). Since only calling `BuildLayout` makes them being used, there is no reason to make it a constant in XML. So this function should be no longer useful.

`VisitEasyLayouts`:

This function should not exist either. `BuildLayout` will check all of these (if not you should do it). There is no need to verify the layout during compiling XML resources.

`Workflow_BuildEasyLayouts`:

This function is suspecious. I think implementing `CreateInstance` should add `BuildLayout`. But I think maybe the issue is that `CreateInstance` is calling too early, you should verify if this is the case.

If yes, then we might need to add a new function to `IGuiInstanceLoader`:
```C++
virtual Ptr<workflow::WfStatement> InitializeInstance(you decide the argument);
```
The default implementation returns null, means nothing needs to do.
`InitializeInstance` needs to be called from the actual loader that implements `CanCreate`, just like how `ctorLoader` is decided to call `CreateInstance`. Imagine objects like a tree (where it already is), from the first leaf we call `InitializeInstance`, and when all sub objects of an object is processed it calls `InitializeInstance`, like a deep first search. A series of `InitializeInstance` will be called after everything is done, collect all non-null statements, add to the generated constructor type of the root instance.

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
- Select only the first loader whose `CanCreate` accepts the resolved construction type, as `ctorLoader` does. Preserve the special source/base type selection for the root; do not substitute the generated root class type. A null hook result does not trigger another ancestor hook. Root final initialization must not depend on having constructor arguments, unlike the existing early `InitializeRootInstance` call.
- Preserve coverage of nested constructors and existing objects configured through `att.*-set`. The latter use `GetParameter`, not `CreateInstance`, so a queue populated only by creation calls would miss them. If an existing target's declared reference type has no creation-capable loader, traverse its children and skip its own hook; do not require an already-existing object to be constructible. Text and serialized value expressions are not independently initialized objects. This task does not broaden which types are legal XML roots.

### Easy loader construction and cleanup

- `GuiEasyInstanceLoader<T>` currently overrides neither `CanCreate` nor `CreateInstance`; construction falls through to the default loader. Merely adding `InitializeInstance` to this class would leave it unused under the requested dispatch contract.
- Make the owning-layout specialization participate in constructor selection and preserve the existing reflected construction behavior, including the resolved concrete type and applicable constructor/root paths. Reuse the existing construction machinery where possible; do not accidentally construct the loader's registration type for a derived resource type. Its final hook emits `BuildLayout`; descriptor loaders have no build call.
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
3. Verify finalization order and dispatch: nested owners build before their parents; initial bindings and events are ready; `ref.Ctor` observes a built layout; existing targets configured through `att.*-set` remain covered. Exercise the root's no-constructor-argument path and a resolved concrete type different from the selected loader's registered base. A loader returning null leaves unrelated initialization behavior unchanged.
4. Retain existing ordinary geometry/ownership/failure coverage and add only missing runtime cases. Test duplicate unnamed payloads and mixed unnamed/explicit `Composition` payloads for both owners and descriptors, while preserving legitimate later replacement. Existing negative cases are in `TestCompositions_EazyLayoutFailures.cpp`; verify that the chosen test build actually compiles and runs that file. Use ordinary tests for non-rendering behavior, following `Project.md`.
5. Build Debug x64 through `.github/Scripts/copilotBuild.ps1` from `Test/GacUISrc`, then run `UnitTest` through `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`. Preserve relevant filters and ensure the resource, EasyLayout control/composition, and failure cases actually execute. Inspect completed `Build.log` and `Execute.log`, including the Debug leak report. If reflection registration changes or `GacUI_Compiler` runs, follow the metadata-generation requirements in `Project.md`.
6. Run the GacUI release pipeline, then `& C:\Code\VczhLibraries\Tools\Tools\Build.ps1 -Project UpdateRelease`. The latter already builds `../Release/Tools/Executables/Executables.sln` in Release x86 and deploys its executables. For the separately requested rebuild, use the repository build wrapper from that solution directory with `-Configuration Release -Platform Win32` (the wrapper maps the solution platform to x86), then invoke `& C:\Code\VczhLibraries\Release\Tools\CopyExecutables.ps1`. Verify the deployed GacGen and metadata come from the updated release.
7. The actual tutorial driver is `../Release/Tutorial/GacUI.xml` and the script is `../Release/Tools/GacBuild.ps1`. Refresh caches with `& C:\Code\VczhLibraries\Release\Tools\GacClear.ps1 -FileName C:\Code\VczhLibraries\Release\Tutorial\GacUI.xml`, then run `& C:\Code\VczhLibraries\Release\Tools\GacBuild.ps1 -FileName C:\Code\VczhLibraries\Release\Tutorial\GacUI.xml`. Compiler binaries are not tracked by the resource timestamp cache, so rebuilding GacGen alone can leave tutorials skipped.
8. Inspect per-resource results for both x32 and x64: `GacBuild.ps1` catches failures and continues, so script completion is insufficient. Check errors and expected outputs, then inspect the sibling Release repository's diff outside `Import`. Existing valid tutorials should retain equivalent initialization/layout behavior with only small explainable generated changes. Do not hand-edit generated outputs or conflate this sibling repository with GacUI's own `Release` directory.

## REVIEW COMMENTS

No unresolved review comments; the implementation decisions and verification corrections are recorded above.
