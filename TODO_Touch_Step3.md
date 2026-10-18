Details are in [TODO_Touch.md](./TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests.

`TODO_Touch_Step[2]?.md` are completed works.

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

- Make sure `handled` (default false) is available on `point(Action|Sequence|Ended)` events.
  - This is a hint for events in compositions, in predefined controls, if `handled` is true, it means there is a sub composition to override the behavior, any event handler in predefined controls should skip this event.
  - `accept` in `pointSequenceStarted` should be updated to `acceptedComposition`, and all future point events should be sent to this composition. If it is `nullptr`, it means not accepted. This is only for compositio's event, it does not affect `INativeWindowListener` design. `pointSequenceStarted` does not have `handled`, but assigning `acceptedComposition` has the same hint.
    - Unlike `pointSequenceStarted` which will sent to the hittest deepest composition and its all parent compositions, `pointAction` and `pointSequenceEnded` will only sent to `acceptedComposition`.
- Add `bool fallbackClick` to `pointSequenceEnded`, false by default. 
  - `GuiGraphicsHost::pointsScale` should not exist, it should call `Convert` whenever scaling is needed. We can't assume scale does not change.
  - `GuiGraphicsHost` will also remember the position of the point in `pointSequenceStarted`, and when it is accepted the state machine will run, and `pointSequenceEnded` as a true `fallbackClick`, it will first hittest the position, and do regular mouse up and mouse down with the specified mouse button decided by the state machine. If the state machine decides to none, this won't happen. This should have a lot of shared code to trigger the mouse click.
- Add a `GuiControl::(Get|Set)AcceptPointEvents` property, default to false:
  - No matter the property value, `pointSequenceStarted` of its bounds composition will be listened, and when it is true, `acceptedComposition` will be assigned with the bounds composition when `handled` is false.
  - When `assignedComposition` is assigned, the reasonly property `GetPointEventComposition` will return that composition.
  - In all conditions, `pointSequenceEnded` will clear `GetPointEventComposition`.
  - All three point events will be handled by its virtual protected function `OnPoint(Action|Sequence(Started|Ended))`.
- As an exception, `GuiScrollView` will call `SetAcceptPointEvents(true)`:
  - In `OnPointSequenceStarted`, it will remember the position of the first point.
- Implement a state machine in `GuiGraphicsHost`:
  - When the first point drags more than 8 pixels, it will find the nearest direction of up-down or left/right, by checking the angle:
    - Lock that position. You can use 0/1/2 to represent it.
    - If the corresponding scroll bar is visibled and enabled, dragging means scrolling that much pixels comparing to the start position. Otherwise no scrolling happens.
  - Only the first point will be used for scrolling.
   - On `PointSequenceEnded`, only when there was one point activated and the 8 pixels never exceeds (you can check the state for example if it is 0), it fallbacks to left button, or right button when >= 0.5 second has passed.
- Just like `altActionManager` and `tabActionManager`, a `pointActionManager` should be created to store all point related code, leaving `GuiGraphicsHost` to do:
  - Trigger `pointSequenceStarted` by itself so that the `acceptedComposition` will be stored.
  - The rest of the events will be redirected to `pointActionManager`.
  - When `pointSequenceEnded` is called, the `Nullable<NativeMouseButton> pointActionManager->GetFallbackClick()` will tell `GuiGraphicsHost` if a fallback needs to happen.
  - This refactor is to clean up as many code as possible from `GuiGraphicsHost` so all those complex logic will not mix up.
  - `GuiGraphicsHost_Point.(h|cpp)` will be the file to store the new `pointActionManager`.
- To verify, you can find any scrollable list or text box and try to scroldl them:
  - Make new remote protocol based unit test case, no need to update `DebugRemoteProtocolSop.md`.
  - I will check snapshots later.