`TODO_Task_TUI.md` was completed but multiple issues are found.
If skin or layout issue happens because `GacUILayout.md` said so or the guidance is not clear enough, fix this document.

## TuiControllerBase renderer clipping

- Currently the TUI render target supports clipper.
- Clipping must be implemented in VlppOS's TUI instead of in render target.
  - In this way we could keep DRY.
  - A clipper will be passed to any drawing functions. It could be a TuiClipper struct with x1,y1,x2,y2 in `vint`.
    - In drawing functions, we can first create a clipper equals to the buffer range, and see if null pointer comes in, if not do intersection.
    - In drawing functions such "normalized" clipper will be first calculated and then draw.
  - Render target could just simply calling them in this way.
