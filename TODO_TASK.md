# "Layout/Easy Layout (Responsive)" in FullControlTest

The goal is to add a new page to FullControlTest to demo how easy layout could work with responsive layout. Before starting the task there is two sub task needs to do:

1) Some place uses "eazy" instead of "easy", this is incorrect. Please fix FullControlTest.

2) I would like a horizontal layout of the tab control in `Layout` tab to list tab headers vertically. So you have to add a new `VerticalTabTemplate.xml` to store that new control template. It should work with color theme in the `DarkSkin` in use, you can verify that by changing the color theme and go back to this page, see if color actually changes. And instead using `<Tab/>` you are going to switch to `<RibbonTab/>` for this particular tab, so that you will get the `BeforeHeaders` and `AfterHeaders` properties. In the control template, `BeforeHeaders` maps to the space above vertical tab headers, and `AfterHeaders` maps to all space below vertical tab headers. The new `Easy Layout (Responsive)` is not a tab page, you are going to put a button instead to the `AfterHeaders`, making sure that the button has a gap of 5 pixels between the last tab header. The gap should be implemented in the button's layout by `AlignmentToParent="left:0 top:5 right:0 bottom:-1"`, the control template does not insert gaps between `BeforeHeaders` -- tab headers -- `AfterHeaders`.

Now you are going to implmenet the new page as a window, create `EasyLayoutResponsiveWindow.xml` and `EasyLayoutResponsiveTemplates.xml` to store that window. Clicking `Easy Layout (Responsive)` opens it as a model window.

The reason to make a new window is that, `FullControlTest`'s smallest size is too big, it is not possible to shink it small enough to trigger the responsive layout we expect to see. So when opening this new window, set its client size to `x:240 y:320` and make it resizable. By changing the size of the window you can trigger the responsive layout, therefore you can test if it re-layout content expectedly.

## Content of the New Test Window

The main layout will be implemented with `ez:Layout`:

```plaintext
+--------------------------+
| Here is responsive       |
| Buttons below are        |
|   not included           |
+--------------------------+
|            | OK | Cancel |
+--------------------------+
```

The `OK` and `Cancel` button doing the same thing to close the window.

The content will be mainly copied from `../Release/Tutorial/GacUI_Layout/Responsive2` which means:
- The #1 layout will be menu + scrollable content
- The #2 layout will be icon options + scrollable content
- The #3 layout will be icon-name options + scrollable content

But instead of making scrollable content looks like some big icons, you are going to make it like a list organized list item vertically. Use a real list control with responsive list item template. `ResponsiveGroup` will be a good parent for storing the list control, as all list item will "response" at the same time, so that to create the #0 layout:
- In #0 layout, all list items will be just movie names
- In #1..#3 layout, all list items will be organized in an 2x2 table like:
  - The left side will be the movie icon picture
  - The right side will be the movie title, followed by the author.

But this time we are demoing `<ez:Layout>`, so even though the `Responsive2` is using raw compositions, we should use `<ez:Layout>` everywhere appropriately.
Organize the layout to keep it easy to understand.

## File Organization

`EasyLayoutResponsiveTemplates.xml` stores all templates used in this window.
`EasyLayoutResponsiveWindow.xml` stores the window, data structure for the movie list. Initialization of account list and movie list could be done in a workflow function and called in the window's constructor. Everything should be done in the xml file, no C++ should be involved to prepare the data.
All new XML files in FullControlTest should be indexed in `GacUI_Compiler` project, also scan if any xml files are missing in other test apps and skins.
All changes should reflect to `.github/Jobs/DebugRemoteProtocolSop.md` if affected, keep unaffected content unchanged. Testing job should be able to follow `DebugRemoteProtocolSop.md` and see if the color theme for the tab and responsive layout is working.

## DETAILS

- This is one task covering the spelling correction, vertical Layout tab, responsive modal window, resource integration and verification. Keep one shared set of details and acceptance checks; the preliminary changes are not separate implementation tasks.
- Correct the FullControlTest-owned spelling consistently: `EazyLayoutTablePageResource`, `demo::EazyLayoutTablePage` and `Eazy Layout (Table)` in `<GacUI repo>/Test/Resources/App/FullControlTest/EasyLayoutTabPage.xml`, their use in `<GacUI repo>/Test/Resources/App/FullControlTest/Resource.xml`, and matching expectations in `<GacUI repo>/Test/UIA_CppTest_Shared.cs`. Regenerate derived code. This does not rename the library's existing `eazy_layout` namespace or shared guideline files.
- Replace only the nested tab inside the top-level `Layout` page. Retain its existing Repeat, Responsive, Easy Layout and Easy Layout (Table) pages and their order. The responsive launch button belongs to `AfterHeaders`, not the page collection. Existing ordinary `TabPage` subclasses can remain in this `RibbonTab`.
- Create the vertical template by copying and adapting `darkskin::TabTemplate` from `<GacUI repo>/Test/Resources/App/DarkSkin/Template_Tab.xml` into `<GacUI repo>/Test/Resources/App/FullControlTest/VerticalTabTemplate.xml`. It already uses `RibbonTabTemplate` and wires both header containers. Reuse its DarkSkin styles and header-template behavior instead of designing a new skin. As explicitly clarified, this control template is an exception to the easy-layout requirement: ordinary tables, stacks and bounds are appropriate here.
- Arrange a minimum-width header rail on the left and filling page content on the right. Inside the rail, `BeforeHeaders` and the vertical headers take their minimum heights; `AfterHeaders` takes the remaining height. Insert no spacing between these regions. Put the launch button directly in `AfterHeaders`, with `AlignmentToParent="left:0 top:5 right:0 bottom:-1"` on its bounds. Keep that alignment outside easy-layout payload normalization. Adapt the copied selected-header decoration for the vertical edge, set `TabOrder="TopToBottom"`, and preserve page selection, focus, ALT access, `Commands`, and `ContainerComposition` wiring.
- Explicitly assigned control templates survive `GuiControl::RefreshThemes()`, and DarkSkin's captured colors do not automatically react to a palette assignment. Use a small XML-side refresh when returning to the top-level Layout page: defer reinstalling the same explicit vertical-tab factory until after the current input callback. Reassigning its nonempty `ControlTemplate` rebuilds the template and reads the current palette. Preserve the page instances, selected page, and application-owned Before/After contents. This avoids changing the shared DarkSkin registration or native palette handler. See `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_AddingNewControl.md` under “Refreshing Installed Templates”; verify this path with both generated C++ and Workflow execution.
- Interpret “model window” as a modal window. Create a fresh `demo::EasyLayoutResponsiveWindow` per launch, owned by the showcase window, titled `Easy Layout (Responsive)`, with resizing enabled and an actual initial client area of 240 by 320. Follow the XML async modal/cleanup pattern in `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorBase.xml`. Both footer buttons close this window; closing it restores the owner without exiting the showcase. Do not inherit the main window's 640 by 480 minimum.
- Use `<Release repo>/Tutorial/GacUI_Layout/Responsive2/UI/Resource.xml` as the reference for account navigation, shared controls and Workflow data initialization. Put the new window, account/movie data types and initialization in `<GacUI repo>/Test/Resources/App/FullControlTest/EasyLayoutResponsiveWindow.xml`; put its custom templates in `<GacUI repo>/Test/Resources/App/FullControlTest/EasyLayoutResponsiveTemplates.xml`. Use distinct `demo` class/resource names and `ref.CodeBehind="false"` where applicable. Reuse or copy the required local image assets into the FullControlTest resource tree and register their resource links; the running demo must not require the Release checkout. Keep data preparation in Workflow.

The four numbers describe these observable appearances, from narrowest to widest:

| Layout | Account navigation | Movie rows |
| --- | --- | --- |
| #0 | Menu | Movie titles only |
| #1 | Menu | Image, title and author |
| #2 | Icon-only options | Image, title and author |
| #3 | Icon-and-name options | Image, title and author |

- Use one real vertical list, such as `BindableTextList` with a responsive `TextListItemTemplate`, and share that control between navigation views so data and selection survive transitions. Existing custom-list examples are in `<GacUI repo>/Test/Resources/App/FullControlTest/RepeatTabPage.xml` and `<GacUI repo>/Test/Resources/App/FullControlTest/RepeatComponents.xml`. The list supplies scrolling and item virtualization; do not retain the tutorial's `ScrollContainer` plus movie `RepeatFlow` as the movie list.
- Synchronize movie-row views through a horizontal `ResponsiveGroup`. Keep movie-row adaptation separate from account-option adaptation: placing both two-level templates in the tutorial's existing wide group would compact the movies when the account names disappear, contradicting #2. Separate responsive containers with suitable responsive boundaries are a viable arrangement. The four appearances need not be one composition's numeric `CurrentLevel`; they must occur in the stated order. Ensure the movie group's rich/compact minimum widths reach its responsive target even though the list viewport normally isolates item minimum sizes. Newly realized items must adopt the current group state.
- The window's outer `ez:Layout` has a filling responsive area and a bottom footer with right-aligned OK and Cancel buttons. The footer stays outside the responsive target. Use easy-layout descriptors for each view and the detailed movie row: an image spanning both rows on the left, title above author on the right. Retain `ResponsiveContainer`, `ResponsiveView`, `ResponsiveGroup`, `ResponsiveShared` and repeat wrappers where their behavior is needed; easy layout does not replace these. Nested easy layouts at such ownership boundaries are supported by `<GacUI repo>/.github/Guidelines/GacUIEazyLayout.md`.
- Keep compact-mode minimum sizes small enough to reach #0 at the initial size. Do not carry over the tutorial's 192 by 192 movie-image minimum or 420-pixel content minimum into this view. Bound thumbnails, use suitable title wrapping/ellipsis, and allow list scrolling instead of enlarging the whole window to fit every item. Account options must also remain reachable at short heights. Choose thresholds from actual content measurements, then record working client sizes for all four appearances in the SOP; do not assume every renderer/font has identical pixel breakpoints.
- Link all three new XML files from `<GacUI repo>/Test/Resources/App/FullControlTest/Resource.xml` and enumerate them in both `<GacUI repo>/Test/GacUISrc/GacUI_Compiler/GacUI_Compiler.vcxproj` and its matching `.vcxproj.filters`. Audit the other authored test-app/skin XML files at implementation time. The review found all 80 existing tracked XML files under `<GacUI repo>/Test/Resources/App` present in both inventories; there is no known omission there to repair. Update other affected source inventories if generation changes their file lists, and obtain generated source/metadata only through the generators required by `<GacUI repo>/Project.md`.
- Extend only the affected FullControlTest operations in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`, with concrete navigation, palette, resizing and modal-close expectations. Keep its unrelated protocol/RVM operations and its separation from renderer setup/test matrices unchanged. Update the shared UIA checks and both entry wrappers required by `<GacUI repo>/Project.md` as part of the same task.

## VERIFICATION

1. Check resource links, unique class/resource names, project/filter inventories, and the FullControlTest spelling correction. Build and run `GacUI_Compiler` with the repository wrappers and verify both x86/x64 outputs. Inspect exit status, diagnostics and any `*.UI.error.txt`/`*.UI.errors.txt`; generated files must not be hand-edited. Complete the required Debug Win32/x64 metadata generation and x64 `Metadata_Test` sequence from `<GacUI repo>/Project.md`, then rebuild consumers. Test-app-only changes may skip the general GacUI unit suite; shared library or built-in skin changes require their normal checks.
2. In rebuilt Debug x64 `CppTest` and `CppTest_Metaonly`, verify the four existing Layout pages remain reachable and retain their content. Check vertical header order, selection, focused Up/Down navigation and ALT access. Require the new action to be a button outside the tab-page collection. Inspect bounds to confirm AfterHeaders fills the remaining rail and the button's top is exactly five layout pixels below the last header, including after resizing and template refresh.
3. Change the showcase palette through Default, Ember, Aurora and back to Default, returning to Layout each time. Require the copied template's background, border/accent, selected and hovered header colors to use the current palette without needing an extra hover to repair stale paint. Retain the selected Layout subpage and existing editor/selection state. Open the responsive window under the changed palette and verify that its custom content and standard controls remain legible and use the intended current colors.
4. Activate the launch button and require one active modal window with the exact title and a settled 240 by 320 client area. The owner must be disabled while the dialog is open. Resize it, then separately test OK, Cancel and title-bar close using fresh openings; each must dismiss the dialog, restore owner input and leave the showcase running. Reopen to check that handlers/windows do not accumulate and each new window starts at the requested size.
5. Resize from #0 through #1, #2 and #3, then back again, waiting for layout to settle at each state. Require the navigation and movie content in the table above, a visible bottom-right footer throughout, and no overlap, clipping of controls, oscillation or unwanted growth beyond the requested client size. Check width changes and short/tall heights separately. Record actual client dimensions and observable content for each state rather than treating a requested size or level number alone as proof.
6. Select a movie, scroll, and change account selection before resizing. Require data and selected movie/account to survive view changes and the list to retain a valid scroll position. Scroll through items initially outside the viewport in every appearance; all realized rows must agree on compact versus detailed content and provide correct selection/hit targets. Verify the detailed rows' image/title/author arrangement and the absence of image/author content in #0.
7. Extend and run `<GacUI repo>/Test/UIA_CppTest.ps1 -Scenario Layout -AsPort 8888` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1 -Scenario Layout -AsPort 8890` through their prescribed workflow. Add targeted roles, names, selection/invoke patterns, modal state and bounds checks to `<GacUI repo>/Test/UIA_CppTest_Shared.cs`, preserving existing Easy Layout/table regression assertions. Use automation Controls/Dom for rendering and geometry that UIA cannot express, and inspect the result with UiaList as required by `<GacUI repo>/Project.md`. Reacquire controls after template changes.
8. Verify the new resources in `GacUI_Host` as a Workflow-binary consumer as well as the generated C++ applications. Follow the affected FullControlTest SOP operations through a native remote renderer, sending input through the renderer surface, and keep the SOP usable by the GacJS testing job. Report actual runtime/renderer coverage separately from unavailable platforms or static checks. Record crashes, generation failures and any missing UIA coverage; a successful review of this task document does not count as executing these implementation checks.

## REVIEW COMMENTS

No unresolved review comments. The implementation decisions, both tab-template clarifications and verification requirements are recorded above for this single task.
