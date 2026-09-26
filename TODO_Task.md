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
