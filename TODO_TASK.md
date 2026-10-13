In `EasyLayoutResponsiveWindow.xml` there is a trick

```xml
<ResponsiveGroup Direction="Horizontal">
  <!-- Forward both navigation widths across the scroll viewport. -->
  <ResponsiveView ref.Name="accountWidth" Direction="Horizontal">
    <att.Views>
      <ResponsiveFixed PreferredMinSize="x:230"/>
      <ResponsiveFixed PreferredMinSize="x:64"/>
    </att.Views>
  </ResponsiveView>
  <BindableTextList ref.Name="accountList" Text="Accounts" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false" env.ItemType="demo::EasyResponsiveAccount^">
    ...
  </BindableTextList>
</ResponsiveGroup>
```

to configure a responsive group. Maybe this could be implemented as a feature into responsive group to be an additional layer of triggering, instead of just relying on the minimum size.
We could assign a size (int) array optionally, and the responsive group update its own `PreferredMinSize`.
`ViewSizes` is ordered from small to large: index 0 corresponds to `CurrentLevel == 0`, the smallest view.
The corresponding component of `PreferredMinSize` according to `Direction` will be overrided, no matter how the user specify the value.
When the size array has more elements than levels, extra numbers will be ignored.
When the size array has less elements than levels, the array will not be physically changed but any extra level will have `PreferredMinSize` increased by 1 then the previous one.
The above rules should implicitly infer that: when the size array is empty, its own `PreferredMinSize` will not be touched.

I would expect the syntax like this:

```xml
<ResponsiveGroup Direction="Horizontal" ViewSizes="{64 230}">
  <!-- or alternatively, there are two samples in the easy layout responsive window, use both syntax -->
  <!-- In this example, horizontal means x, view sizes mean y with vertical -->
  <att.ViewSizes>
    <_>64</_>
    <_>230</_>
  </att.ViewSizes>
  <BindableTextList ref.Name="accountList" Text="Accounts" HorizontalAlwaysVisible="false" VerticalAlwaysVisible="false" env.ItemType="demo::EasyResponsiveAccount^">
    ...
  </BindableTextList>
</ResponsiveGroup>
```

In `TestCompositions_Responsive.cpp` add 4 test cases to cover 0, less, equal, more size array elements than levels, each one covers horizontal and vertical cases separatedly, no need to add other unit tests:
- Resize the root composition to exactly preferred size and 1 number less than the preferred size to make sure it does not change and change when shrinking, all the way from the largest to the smallest level.
- Growing should be tested.
- In the 0 and less scenario, actual min size should larger than calculated preferred min size, that's how unspecified sizes are not effecting the layout determining, in this case the testing boundary will be actual min sizes instead of calculated preferred min size, when covering levels that do not have view size specified.

Commit and push once finishing, and then update the KB pages in `Tools` repo, and website to reflect the new array size feature of responsive group, publish the website.

## DETAILS

- Add `ViewSizes` to `GuiResponsiveGroupComposition` in `<GacUI repo>/Source/GraphicsComposition/GuiGraphicsResponsiveComposition.h` and its matching implementation. Unlike the largest-first `GuiResponsiveViewComposition::Views` collection, this array follows numeric levels from smallest to largest, as clarified during review. For a nonempty array with count `N` and current level `L`, use `ViewSizes[L]` when `L < N`; otherwise use `ViewSizes[N - 1] + (L - N + 1)`. For example, `{64 230}` supplies 64, 230 and 231 to levels 0, 1 and 2. Neither extrapolation nor ignoring surplus entries changes the stored array.
- Keep `LevelCount` derived from participating responsive children. Array length does not create levels; a group without participating children still has one level. Recompute the active size when the level, child-derived level count, direction or assigned sizes change, and invalidate layout through the existing composition mechanism.
- `Direction` is a mask: Horizontal overrides x, Vertical overrides y, and Both overrides both components with the same scalar. Preserve any unselected component of `PreferredMinSize`. A nonempty array takes precedence over a user assignment to the selected component. An empty array performs no override; clearing a previously nonempty array leaves the currently stored `PreferredMinSize` unchanged rather than restoring an earlier value.
- This is an additional preferred minimum, not a replacement for measured element/child minima. Preserve the ordinary measurement and margin rules in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsComposition_Layout.cpp`. Where actual content requires more space than the calculated preference, responsive decisions use that larger effective minimum.
- Expose a reflected, assignable integer-array property and register it in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionCompositions.cpp`. Own the assigned values so later caller mutation cannot silently bypass layout invalidation. The two XML spellings above are alternatives: use one on each showcase group, not both assignments on the same group. The existing array-property path in `<GacUI repo>/Source/Compiler/GuiInstanceLoader.cpp` supports child values; the literal `ViewSizes="{64 230}"` also needs compiler support because ordinary integer text parsing does not interpret it as an array. Preserve the requested spelling and verify both generated C++ and Workflow execution.
- In `<GacUI repo>/Test/Resources/App/FullControlTest/EasyLayoutResponsiveWindow.xml`, replace `accountWidth` with account-group sizes `{64 230}` and `movieWidth` with movie-group sizes `{120 334}`. Name the account group and update `InitializeItemTemplate` and both item factories to use the owning group's established level before attaching newly realized rows. Preserve the separate account/movie responsive boundaries, shared movie control, selection and independent scrolling.
- Removing the placeholder views also removes the permanent two-level children. During movie-row reload, all realized responsive children can disappear, temporarily changing the group to `LevelCount == 1` and `CurrentLevel == 0`. Preserve the intended movie level across this interval and prevent nested reloads from teardown/restoration notifications. `ReloadVisibleStyles()` clears immediately but recreates rows during later layout, so a guard confined to that function call is insufficient. Initialize replacement rows from the retained level until restoration completes. Keep the row-height reset: the default fixed-height arranger otherwise retains the larger detailed-row height after shrinking. Do not make array length determine levels to hide this lifecycle issue.
- After implementation and verification, commit and push GacUI, separating manually maintained files from generated artifacts as required by `<GacUI repo>/Project.md`. Then update the authored responsive-layout guidance in `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_LayoutAndGuiGraphicsComposition.md` and the public article in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/compositions/responsive_group.xml`. Follow the website repository's publishing workflow, refresh exported manual content and synchronize the knowledge base through the existing tools, then commit and push affected repositories. Keep the frozen `website-doc1` package unchanged. These are implementation follow-ups; this review only updates the task document.

## VERIFICATION

1. Add exactly four ordinary unit-test cases in `<GacUI repo>/Test/GacUISrc/UnitTest/TestCompositions_Responsive.cpp`: empty, shorter, equal-length and longer arrays relative to the child-derived level count. Each case exercises Horizontal and Vertical separately, using an actual group inside a responsive container. Keep these geometry tests outside the remote snapshot framework. Assert current level, the selected preferred-size component, the unaffected component, and unchanged array contents/count.
2. Use distinguishable minima for every level. Explicit entries should exceed actual content minima so their thresholds control the test. In the empty case and the unspecified levels of the shorter-array case, actual minima must exceed the unchanged or extrapolated preference. With zero margins, shrinking to the effective minimum retains a nonzero level and one unit less descends; level zero remains zero below its minimum. Growing to one unit below the next effective minimum stays at the smaller level, and equality admits the larger level. Traverse all levels in both directions and repeat. Include assignment/clearing and direction-update checks within the same four cases where needed; do not add further unit-test cases. Surplus values must neither change existing thresholds nor create levels.
3. Build and run the required unit tests through the repository wrappers, respecting existing test filters and checking the final results and leak diagnostics. Because reflection and resource XML change, perform the Debug Win32/x64 metadata generation and x64 `Metadata_Test` sequence from `<GacUI repo>/Project.md`, compile FullControlTest through `GacUI_Compiler`, and repeat the required metadata steps after compilation. Inspect diagnostics, both generated architectures and any resource error files, then rebuild consumers. Do not hand-edit generated source or metadata.
4. Run both `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` with `-Scenario ResponsiveWindow` using their prescribed workflow and distinct automation ports. Reuse the existing four-appearance checks, repeated compact/detailed row-height checks, short/tall account scrolling and selection assertions. Also resize while scrolled near the end, realize previously unseen rows, and reopen the window. Require stable convergence, no recursive reload or oscillation, synchronized row appearances and valid scroll positions; exact pixel offsets need not survive row-height changes. Run the existing `Layout` scenario for palette and neighboring-page regressions, inspect through UiaList as prescribed, and verify the resources in `GacUI_Host` as a Workflow-binary consumer. The two real groups provide compilation/runtime coverage of both XML spellings without additional unit-test cases.
5. Document numeric-level ordering, extrapolation, ignored surplus entries, empty-array behavior, direction handling and measured-minimum precedence consistently in the KB and public manual. Verify rendered examples and exported Markdown, then complete the website workflow's publication and live-page checks. Report actual build/runtime/publication results separately from static review; none of these implementation checks has been executed merely by reviewing this document.

## REVIEW COMMENTS

No unresolved review comments. The user confirmed that `ViewSizes` runs from small to large; the examples and level mapping above reflect that decision.
