Details are in [TODO_Touch.md](./TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests. Here are completed works:
- `TODO_Touch_Step1.md`
- `TODO_Touch_Step2.md`
- `TODO_Touch_Step3.md`

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

This request is about refactoring point experience implementation.

1) `GuiPointActionManager`

I would like to convert fallback click to a more general gesture system. Here I proposed a extensible gesture state machine system, but the extensibility is only opened for `GuiPointActionManager`, it is not yet opened to any end users yet. Such gesture is totally based on pure calculation, instead of any OS supported gestures. And it should solve the wired relationship between `GuiScrollView` and `GuiPointActionManager`, making `GuiGraphicsHost::GetPointActionManager` no longer necessary, `GuiPointActionManager` becomes a pure private helper class for `GuiGraphicsHost`.

First of all we need a new interface
```C++
class IGuiPointGesture : public virtual Interface
{
public:
  virtual void OnPointSequenceStarted(...) = 0;
  virtual void OnPointAction(...) = 0;
  virtual void OnPointSequenceEnded(...) = 0;
  virtual bool IsGestureSelected() = 0;
  virtual Nullable<NativeMouseButton> GetFallbackClick() = 0;
};
```

Fixed types of gestures will be defined in `GuiGraphicsHost_Point.cpp`, forming a `List<Ptr<IGuiPointGesture>>` stored inside `GuiPointActionManager` for iterating.
The order is important because different gesture might have priority or implementation dependencies. There is a complex state machine inside `GuiPointActionManager`, this design will be gone forever, becoming multiple small state machines with priority in each `IGuiPointGesture` implementation.

**OnPointSequenceStarted**: `GuiPointActionManager::Initialize` will calls every `OnPointSequenceStarted`, until any `IsGestureSelected` returns true, this function reset any state in a gesture. Here I would like `pointActionManager->Initialize(firstPoint);` only called after accepted, as if it is not accepted, all subsequence `GuiGraphicsHost::Point*` will not be called anyway.

**OnPointAction**: `GuiPointActionManager::OnPointInput` will call every `OnPointAction`, until any `IsGestureSelected` returns true. Or when there is already a selected gesture, only its `OnPointAction` will be called. The selection will be reset in `Initialize`.

**OnPointSequenceEnded**: `GuiPointActionManager::PointSequenceEnded` will call every `OnPointSequenceEnded`, until any `IsGestureSelected` returns true. Or when there is already a selected gesture, only its `OnPointSequenceEnded` will be called.

**IsGestureSelected**: Returns false, until any `pointGesture` events is selected by the event handler, returns true. This value will be reset in `OnPointSequenceStarted`.

**GetFallbackClick**: `GuiPointActionManager::GetFallbackClick` will return the first `GetFallbackClick` that does not return empty, or when there is already a selected gesture, returns its `GetCallbackClick` directly.

By the way, in `GuiPointActionManager`, `Initialize`, `OnPointInput` and `PointSequenceEnded` will be renamed to `OnPointSequenceStarted`, `OnPointAction`, `OnPointSequenceEnded` to match the naming convention, and these 3 functions need to be placed together in h and cpp, followed by capturing functions, followed by the rest. `GetAxis` and `GetDisplacement` is no longer needed`.

Secondly here are all predefined gestures.

There is a `64` magic number in `GuiGraphicsHost_Point.cpp`, I have added a `StaticPointRange` constant, use `StaticPointRange * StaticPointRange` to replace it, scan for any other usages. Below are all suggested class names of `IGuiPointGesture` implementation, putting in `namespace predefined_gestures`.

**LeftClickGesture**: This one does not invoke `pointGesture` event. The state machine is simple, only when there is only one point through the whole sequence, and the point does not drag far away than `StaticPointRange` in anymoment in the sequence, its `GetFallbackClick` returns left button.

**RightClickGesture**: This one does not invoke `pointGesture` event. It is the same to `LeftClickGesture`, the only difference is that, only when the sequence lasts >= 0.5 seconds, its `GetFallbackClick` returns right button.

**ScrollingGesture**: This one invoke `pointGesture` event. The logic is already implemented, but maybe with a little difference.
If the whole sequence has only one point, when the first point moves longer than `StaticPointRange`, it locks down the scrolling direction and triggers the first `pointGesture`. Later any `OnPointAction` will calls `pointGesture` with the same direction until the second point happens, and no more `pointGesture` after that.

**DraggingAndRotationGesture**: This one invoke `pointGesture` event in both `OnPointSequenceStarted` and `OnPointAction`, the gesture starts immediately. But if `pointGesture` in `OnPointSequenceStarted` is not selected, `OnPointAction` will no longer invoke `pointGesture` in this sequence. The logic is already implemented in `Touching` test page but now we can move them here instead of doing math in XML files.

Here is their priority, aka the order added to the gesture list. We need to also keep class definitions in this order too:
- RightClickGesture
- LeftClickGesture
- ScrollingGesture
- DraggingAndRotationGesture

When a gesture is selected, the `selected` value will be ignored in this sequence, but to simplify control handling any control could just keep selecting this gesture forever.

Thirdly we need to have a new `pointGesture` event added to `GuiGraphicsEventReceiver`. The event definition looks like this:

```C++
struct GuiPointGesture_Scrolling
{
  bool isVertical; // true means up/down, false means left/right.
  vint distance; // in unit of non-native pixels. right/down will be positive.
};

struct GuiPointGesture_DraggingAndRotation
{
  double rotation; // 2*pi a whole circle clockwise (because rignt/down is positive)
  double scale;
  Point distance;
  // the calculation is in the definition order, an object is rotated by `rotation`, and the scaled by `scale`, and then move by `distance`.
};

struct GuiPointGestureEventArgs : GuiEventArgs
{
  Variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation> gesture;
  bool selected = false; // a control should not select a gesture when it is already handled, this keeps selected and handled different.
};
```

2) `GuiScrollView`

A new empty `OnPointGesture` will be added to `GuiControl` for overriding.
Now `GuiScrollView` only needs to handle `OnPointGesture`, to select this gesture, and keep scrolling when `OnPointGesture` is called.

3) `Touching` tab in `FullControlTest`

The logic is moved so now this test page only need to handle `pointGesture` events.

4) New `TouchingList` tab in `FullControlTest` besides `Touching`.

This is a text list will big item (item template needed). On top there is "Clear" button to delete everyting, and "Add 5 Items" button to add 5 text items with some lorem ipsum make up content.

Dragging up and down scrolls the list items, this should be automatically happens.

Dragging right nothing happens.

Dragging left will move the item towards left, with the right space filled by red. The red zone will have right aligned label called "DELETE IT". The label will have a reasonable margin, and it defines the maximum space of the red zone. The item should not be dragged more than the maximum red zone. When the sequence ends and the red zone is extended to its maximum space, the item will be deleted in `InvokeInMainThread`, immediate deletion will crash GacUI because following messages still need the alive composition.

The dragging will need to be implement in the list itself, as gesture does not have a "to parent fallback" like `pointSequenceStarted` event.

5) Verification and Documentation

Native renderer, remote protocol with native renderer, GacJS, should all verify, but to keep the verification fast, only verify `Touching`, `TouchingList` and list control scrolling, additionally make sure touching on text is fallbacked to mouse so a simple dragging should select some text.

`KB_GacUI_Design_PointInput.md` needs to update to introduce this new architure, emphasis that the extensibility only opens to GacUI maintainers. A complete rewrite is welcome to organize the document in this way and remember this is a page to introduce API and its design:
- `INativeWindowListener` events, how to implement and how to handle. Mentioning only Windows and remote protocol supports touch experience.
- Remote protocol, how to send point events in a correct way. Mentioning only Windows and GacJS supports touch experience.
- How to handle point events in `GuiGraphicsEventReceiver`.
- List predefined gestures and click fallbacks.
- Controls that already use gestures.
