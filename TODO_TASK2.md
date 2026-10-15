The semantic of `<Raw/>` has been changed, here is the already completed task `./TODO_TASK.md`.
Your goal is to make sure it works on `GacJS` repo:
- Raw element becomes a `<div>`.
- The drawing source code should be in `index.html` where it create a canvas element to fill the div and draw the clock.
  - `getRawElements`, `rawElement(Installed|Uninstalled|Updated)` needs to be added to the `IGacUIRenderer` interface.
  - These api offers an object `{element, id, data}`, where `element` is the div.
  - `index.html` calls `GacUIHtmlRenderer.runGacUI` to get the renderer, then these API becomes available.
- `wasm-fct/index.html` and `wasmPage.ts` will have to fix but unfortunately they cannot be tested on Windows. I will test it manually on Linux.
  - It is totally OK to duplicate the canvas creating and drawing code across `index.html` and `wasm-fct/index.html`.
commit and push all local changes once finishing.
