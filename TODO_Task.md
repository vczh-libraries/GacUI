In `FullControlHost` there is a `EazyLayout` page, there was multiple ez:Layout nested together. I made a change to keep it "theoretically" equivalent, except that the splitter is moved to another place.
In order to test nested layout, I removed nested ez:Layout and merge everything together, besides of the moved splitter (this will not be mentioned below) everything else should be working exactly the same.
Your work is to rebuild `FullControlTest`, verify it, and anything is not working, firstly check if there is any xml mistakes and fix them for me, otherwise it is the implementation of ez:Layout which should be fixed.
Basically the idea is that, nested layout doesn't need nested `<ez:Layout/>`, for example:
- Line 15: direction of that `Fill` only affect the one below it, it does not affect any other `Fill` because they are in different container.
- Content in container Line 14 will generated different table with container Line 42 because those `Row` and `Fill` are in different container, so they don't share.
Pay attention to the `Rebuild` button, if clicking it crashes, try to run `BuildLayout` with `GuiApplication::InvokeInMainThread`.

In the same file `EazyLayoutTabPage.xml` I would like you to create another tab page for complex table generation test (but still do not put nested `<ez:Layout/>`). So they should cover things that is uncovered due to my change. My change (see the last commit) is covering nested layout objects. I would call it `EazyLayoutTablePage` with title "Eazy Layout (Table)".
