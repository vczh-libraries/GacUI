Update `GacUI_Windows/Direct2DClock` tutorial to use raw element like the new `Misc/RawClock` page in FullControlTest. This tutorial needs to accept an optional /Gdi option to switch to GDI, and the rendering needs to support both Direct2D and Gdi. You need to run `Build.ps1 -Project Release` to build the release repo. Unlike `GacUI_ControlTemplates/DarkSkin`, `Direct2DClock` is completely detached from `GacUI` source code so you can just copy necessary source code without worrying about code sharing. And rename this tutorial to `RawClock`.
commit and push all local changes.

- Update KB pages and `GacJS/doc` + `wGac` + `iGac` + `iGac/doc` documents to mention the new changes applied to `<Raw/>`.
  - Especially `GacJS/doc` + `iGac/doc` to scan if there is any stale fact.
  - `TODO_TASK.md`, `TODO_TASK2.md`, `TODO_TASK3.md`, `TODO_TASK4.md` are all completed tasks.
  - KB pages should be updated in `Tools` repo.
  - `GacUIXmlResource.md` needs to mention how to use `<Raw/>` for ownerdraw, including simple introductions to these 4 elements:
    - `GuiDirect2DElement`
    - `GuiGDIElement`
    - `GuiWaylandElement`
    - `GuiCocoaElement`
  - Also mention that `<Raw/>` will be the standard and best way to do ownerdraw with GacUI, no matter if remote protocol is in use, TUI does not support `<Raw/>`.
  - Update the website to introduce or update stale `<Raw/>`.
  - Update the website to mention replace the intruduction for `Direct2DClick` tutorial to `RawClock`, saying that this is a demo for `<Raw/>` with native rendering API.
  - Publish the website.
