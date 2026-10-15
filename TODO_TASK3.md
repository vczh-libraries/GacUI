`TODO_TASK.md` and `TODO_TASK2.md` are two already completed works, you are going to update `GacJS` and `wGac` based on them:
- Make sure `GacJS` itself is working properly with remote protocol.
- Make sure all 3 wasm test pages in `GacJS` is working properly.
- You will need to check if the clock is rendered in `GacJS`'s main `index.html` and `wasm-fct/index.html`.
- In `GacUI` there are `FullControlTestRawElement(Remote)?.(h|cpp)`, you need to make a wayland version in `wGac` instead of copying these files in `syncProj.sh`. You can put them in `WGacShared` folder and update test app's cmake file to use them.
  - Create a `GuiWaylandElement` to expose wayland native rendering device just like `GuiDirect2DElement` or `GuiGDIElement`, according to how device lost is handled.
  - Therefore `<Raw/>` become `GuiWaylandElement` in both native app and remote protocol native renderer.
  - Both `test.sh --app:fct` and the remote native renderer need to render the clock, just like how the Windows version is done.
