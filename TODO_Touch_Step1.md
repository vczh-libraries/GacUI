Details are in [TODO_Touch.md](../TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests.

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

- Use point as the term as we dont use legacy touch on both Windows and browser.
- Add all point events to `INativeWindowListener` with Windows implementation.
  - gesture will not be included in this version.
- A point sequence begin and point sequence end event before and after a series of point events, when an event share at least one finger with the next event.
  - Point sequence begin starts with the first finger, it will be used to query id that position supporrs touching, responsing "No" will stop reporting all following events.
  - A sequence has an unique id per native window.
  - Not supported sequence begin also denies gesture.
- Maps events to `GuiGraphicsComposition`'s event receiver:
  - The first point locks the composition receiving point events for the complete sequence.
  - It also handles the point sequence begin event, and the event handler needs to declare if it accepts point events. Such information will be reported back to `INativeWindowListener`, with the same behavior, which is when the composition does not accept point events, all subsequence events will not raise.
- `FullControlTest` has a dedicated page next to `RawClock`, named `Touching`, to demo with single point to drag, with two finger to move, rotate and zoom.
  - It will be a sky blue rectangle to move, rotate and zoom, but since it should be rotated, you are going to draw a technically polygon.
  - All existing points should draw a yellow circle around 32x32.
  - Points are not required to hit the rectangle, but the first point should hit the `<Raw/>`.
  - Detailed behavior:
    - When there is only one point pressing, it moves the rectangle.
      - Relative position will be remembed no matter points increased or decreased to one.
    - When there are two points pressing, they move, rotate and zoom the rectangle as if you are pushing the paper. Points will be relative to the rectangle scaled.
      - Relative position will be remembed no matter points increased or decreased to two.
    - Where there are more than two points, nothing is going to happen.
    - Since point events are taken care of, so dragging by mouse on `<Raw/>` will not respond.
  - Actual rendering will happen in `Test\GacUISrc\Generated_FullControlTest\FullControlTestRawElement.cpp`
- Fix automation service to support touching as IO input:
  - A series of point events are sent, but when the point sequence begin is denied, the first point will be converted to mouse events with left button and other points will be ignored. Automation service doesn't need point sequence start/event, those are generated inside automation service.
- No need to update `DebugRemoteProtocolSop.md` for now.
- No need to update KB pages for now.
- No need to fix remote protocol for now, that means it is fine when the native remote renderer does not respond to touching.

I will verify after you completing everything with a device that has a real touch monitor.
