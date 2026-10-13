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
