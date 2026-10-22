# Handling Input Events

Raw input events are in compositions. Normally a composition doesn't receive events, until **GuiGraphicsComposition**::**GetEventReceiver** is called. This function create a big object containing all event registrations inside it, and cause **GuiGraphicsComposition**::**HasEventReceiver** to become **true** forever. You are not able to free the space of event registration object once it is associated to a composition.

In the event registration object you will find a lot of fields for events, for example, **mouseDown**. There are 3 methods for subscribing an event:
- composition-\>GetEventReceiver()-\>EVENT_NAME.**AttachMethod**: subscribe an event using an object and a member function pointer.
- composition-\>GetEventReceiver()-\>EVENT_NAME.**AttachFunction**: subscribe an event using a function pointer or a **vl::Func\<...\>** functor.
- composition-\>GetEventReceiver()-\>EVENT_NAME.**AttachLambda**: subscribe an event using a lambda expression or other functors. All 3 methods returns a new handler object per each call. The only purpose for this object is to use in composition-\>GetEventReceiver()-\>EVENT_NAME.**Detach** and cancel the registration that returning that perticular handler object. If **Detach** returns **false**, either this registration has already been canceled, or this registration doesn't happen in this event.

In the event callback function, the first argument is always **GuiGraphicsComposition*** which is the owner of the event registration object, the second argument varies in different events.

In GacUI XML Resource, raw input events are treated like attributes of a composition. You can either specify a method name of the current instance, or specify a piece of code to run. Please check out [this page](../../.././gacui/xmlres/instance/events.md) for details.

## Events and Parent Chain

Most of the events are not only raised on the composition that receives the input, but also all the way to the root in the parent chain, which means after the event is raised on the source composition, it is then raised on its parent, and its parent's parent until the end.

Such process could be detected using members of the second argument:
- **compositionSource**: the composition right under the mouse, which is the original event raiser. When the event raises through the parent chain, this member doesn't change.
- **eventSource**: If the **compositionSource** has an associated event registration object, **eventSource** is **compositionSource**. Otherwise, **eventSource** is the nearest parent composition that has an associated event registration object.
- **handled**: you can set this member in the event callback function to **true** to stop the event from going through more further in the parent chain. Meanwhile, the first argument is the composition who owns the event registration object the callback function is registered to. When the event raises through the parent chain, the first argument becomes the current "parent".

## Mouse Events

### Button and Wheel Events

GacUI offers the following button events:
- mouseDown
- mouseUp
- mouseDoubleClick
- horizontalWheel
- verticalWheel

The three button events share the same event type. Read **button** to distinguish **Left**, **Middle**, **Right**, **Mouse4**, and **Mouse5**.

The type of the second argument is **vl::presentation::compositions::GuiMouseEventArgs&**, or **presentation::composition::GuiMouseEventArgs*** in **Workflow**.

Mouse button and wheel events are raised on captured composition, or the composition right under the mouse when no composition is captured, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

Other members contains the information of the event:
- **ctrl**: **true** if the CTRL key is pressing.
- **shift**: **true** if the SHIFT key is pressing.
- **osSuper**: **true** if the Windows, Command, or Super key is pressing.
- **button**: the button for a down, up, or double-click event.
- **left**: **true** if the mouse left button is pressing.
- **middle**: **true** if the mouse middle button is pressing.
- **right**: **true** if the mouse right button is pressing.
- **x** and **y**: the relative coordinate to **first callback argument**.
- **wheel**: direction and distance of the wheel. A positive number means the wheel is rotating towards right or down. The unit of the rotation is 120 for today's typical mouse.

### Moving Events

GacUI offers the following button events:
- mouseMove
- mouseEnter
- mouseLeave

**mouseEnter** happens when the mouse moves into a composition. **mouseLeave** happens when the mouse moves out of a composition. When a mouse moves from one composition to its child composition, it doesn't count as leaving the original composition, it is still in the original composition and its child composition.

**mouseMove** happens when the mouse moves. It is raised on captured composition, or the composition right under the mouse when no composition is captured, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

### Capturing

When a button down event happens, the **compositionSource** is captured. A captured composition becomes the **compositionSource** of all following mouse events happen in the containing window (or menu, popup, etc...), including button up events.

Capturing is released after every held mouse button is released.

### Moving or deleting controls or compositions during mouse events.

Some times you may want to change the control tree structure in a callback of mouse events, (including **Clicked** in buttons).

This is tricky because, mouse events are not just sent to this composition, it is sent to all its parent compositions. Even when **handled** is set to **true**, there is still something to do after the callback function ends.

When you move or delete controls or compositions that affect the parent chain, **GacUI will stably CRASH**.

In order to solve this issue, **GetApplication()-\>InvokeInMainThread** is your friend. Whenever you call this function, the callback to **InvokeInMainThread** will run right after the current series of events. That is a very safe point to restructure your controls or compositions.

For example, if you customize the tab control to have a close button in each tab page:
```
void MyTabPage::buttonCloseClicked(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
{
    // this is still in the chain of mouse button events,
    // so InvokeInMainThread is required to delete MyTabPage.
    GetApplication()->InvokeInMainThread(this->GetRelatedControlHost(), [=]()
    {
        this->GetOwnerTab()->GetPages().Remove(this);
        SafeDeleteControl(this);
    });
}
```

## Touch and Built-in Gestures

Windows and the GacJS browser renderer support touch input. The native wGac (Wayland) and iGac (macOS) providers do not support touch input. This limitation does not apply to GacJS running in a browser on Linux or macOS, with either WebAssembly or HTTP transport. Physical mouse and pen input continue through the mouse events.

### Accepting a Touch Sequence

A sequence starts with the first contact and ends when all contacts lift or the sequence is canceled. `GuiGraphicsEventReceiver` exposes four point events:
- `pointSequenceStarted` queries the hit composition and its ancestors. Set `acceptedComposition` to a composition in the same graphics host to capture the sequence. The first non-null choice wins.
- `pointAction` reports Down, Update, Up, Enter and Leave through `eventType`.
- `pointGesture` offers a built-in scrolling or manipulation payload.
- `pointSequenceEnded` reports successful completion or cancellation exactly once to the accepted composition.

Only the Started query traverses the parent chain. It continues even when `handled` is true, allowing an explicit handler to accept the sequence. Action, Gesture and End go only to the accepted composition and never bubble. Setting `GuiControl.AcceptPointEvents` to `true` enables default acceptance at its bounds when the Started query is unhandled and has no owner. This property defaults to `false`; `GuiScrollView` enables it by default. Setting it to `false` disables that control's default acceptance, but custom Started handlers or ancestors can still accept the sequence. `GuiControl.PointEventComposition` identifies capture within a control's bounds subtree, including during End, and becomes null after cleanup.

Started, Action and End expose a borrowed `Points` list in the receiving composition's logical coordinates. Slots follow contact Down order; lifted slots are empty and are not reused within the sequence. Copy values needed after the callback. Queue control removal with `GetApplication()->InvokeInMainThread`, just as for mouse handlers; deleting the receiving composition synchronously is unsupported.

### Gesture Selection and Payloads

The graphics host has four built-in recognizers, in priority order:
- **Right click:** a successful single-contact sequence lasting at least 500 milliseconds, with no excursion beyond a radius of eight logical units from its initial position. Recognition occurs on successful release, not while holding.
- **Left click:** the same single-contact movement limit without the duration minimum. Right click wins when its duration requirement is met.
- **Scrolling:** movement beyond that radius locks to the axis with the larger absolute displacement; horizontal wins a tie. `GuiPointGesture_Scrolling` supplies `isVertical` and signed incremental `distance`, positive towards right or down. The first distance includes all movement from Down. A second contact permanently stops scrolling offers for this sequence.
- **Dragging, rotation and scaling:**`GuiPointGesture_DraggingAndRotation` is offered with identity values at Started and must be selected immediately to continue. One contact translates; two translate, rotate and uniformly scale; more than two pause. Contact membership changes establish a new baseline without a jump.

For an unhandled `pointGesture` offer, set `selected` to `true` to choose that recognizer for the rest of the sequence, and set `handled` to `true` when consuming the payload. Selection is sticky and independent of `handled`. Raw Action and End events still arrive. A declined scrolling offer can be offered again; a declined initial manipulation offer cannot. These recognizers are fixed; applications cannot register additional recognizers or select an OS gesture implementation.

`GuiPointGestureEventArgs.gesture` is a writable `Variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation>` in C++, reflected as the corresponding `variant<...>` in Workflow. Read the lowercase `gesture` field; there is no `Gesture` property or `GetGesture()` method. A Workflow handler can extract the active alternative with a nullable weak cast:
```Workflow
if (not arguments.handled)
{
    if (var scrolling : GuiPointGesture_Scrolling? = arguments.gesture as GuiPointGesture_Scrolling?)
    {
        arguments.selected = true;
        arguments.handled = true;
        // Apply scrolling.distance along the scrolling.isVertical axis.
    }
}
```

Extracted payloads are copies. Apply manipulation in the order `p' = scale * R(rotation) * p + distance` in accepted-composition logical coordinates. Positive rotation is clockwise in radians. Payloads are incremental, including centroid-aware translation for two contacts.

### Clicks and Existing Controls

An accepted sequence with an eligible click and no selected scrolling or manipulation gesture automatically produces a deferred mouse click. It uses the original contact position with current layout after point cleanup. Cancellation, any additional contact, or movement outside the click radius prevents this fallback; returning inside the radius does not restore it. End `handled` does not veto the click.

A rejected sequence uses mouse compatibility immediately, so ordinary controls and text selection can keep their existing mouse behavior. For remote input, only the original first contact drives that stream; later contacts never replace it. See [GuiScrollView touch behavior](../../.././gacui/components/controls/container/scrollcontainer.md) for scrolling, nested controls, selectable text and gestures that scroll views do not consume.

The [Touching sample](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Windows/Touching) applies manipulation to a custom-drawn rectangle. [TouchingList](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Windows/TouchingList) adds application-specific horizontal swipe-to-delete behavior while retaining built-in vertical scrolling. Swipe-to-delete is not a default behavior of list controls.

## Focus Events

GacUI offers the following button events:
- gotFocus
- lostFocus
- caretNotify

The type of the second argument is **vl::presentation::compositions::GuiEventArgs&**, or **presentation::composition::GuiEventArgs*** in **Workflow**.

A focus composition is the **FocusableComposition** of a control that has focus. Control like buttons will take focus after it is clicked, you can also call **SetFocus** to move focus to a control.

When a control get its focus, its **FocusableComposition** has the focus. If you are writing a control template, you can set any composition to the **FocusableComposition** property of the control template, and it becomes the **FocusableComposition** of the control.

A window has only one focused control. **gotFocus** raises on a composition when it is focused. **lostFocus** raises on a previous focused composition when it lost the focus.

**caretNotify** is keep raising on the focused control every half second. If a control wants to render an input caret, it needs to subscribe to **caretNotify**, and than changing the input caret between visible and invisible state when this event happens.

COMPOSITION-\>**GetRelatedGraphicsHost**()-\>**SetCaretPoint** is useful to tell the system where to show the little window for the input method. This function is strongly recommended to be called by the focused control to avoid state corruption.

If the window itself gets or loses focus, related events are raised on the window, events described here are not affected.

## Keyboard Events

GacUI offers the following button events:
- previewKey
- keyDown
- keyUp

The type of the second argument is **vl::presentation::compositions::GuiKeyEventArgs&**, or **presentation::composition::GuiKeyEventArgs*** in **Workflow**.

Keyboard events are raised on the focused composition, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

**previewKey** is raised before any other keyboard events. If **handled** is set to **true**, not only the parent chain will stop, the following keyboard event will also be canceled.

**code** member of the second argument maps to an item in **vl::presentation::VKEY** enum.

Other members contains the information of the event:
- **ctrl**: **true** if the CTRL key is pressing.
- **shift**: **true** if the SHIFT key is pressing.
- **alt**: **true** if the ALT button is pressing.
- **osSuper**: **true** if the Windows, Command, or Super key is pressing.
- **capslock**: **true** if the CAPSLOCK button is activated.
- **autoRepeatKeyDown**: **true** if this event is generated because of holding a key.

## Input Events

GacUI offers the following button events:
- previewCharInput
- charInput

The type of the second argument is **vl::presentation::compositions::GuiCharEventArgs&**, or **presentation::composition::GuiCharEventArgs*** in **Workflow**.

Input events are raised on the focused composition, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

**previewCharInput** is raised before any other keyboard events. If **handled** is set to **true**, not only the parent chain will stop, the following keyboard event will also be canceled.

**code** member of the second argument is a **wchar_t** that being typed into the control. In Windows, **wchar_t** is a UTF-16 code point. In other platform, **wchar_t** is a UTF-32 code point.

Other members contains the information of the event:
- **ctrl**: **true** if the CTRL key is pressing.
- **shift**: **true** if the SHIFT key is pressing.
- **alt**: **true** if the ALT button is pressing.
- **osSuper**: **true** if the Windows, Command, or Super key is pressing.
- **capslock**: **true** if the CAPSLOCK button is activated.

## Other Events

GacUI offers the following button events:
- clipboardNotify
- renderTargetChanged

The type of the second argument is **vl::presentation::compositions::GuiEventArgs&**, or **presentation::composition::GuiEventArgs*** in **Workflow**.

When content in the system clipboard changes, **clipboardNotify** raises on every compositions.

When the render target is changed, this may because a composition is added to or remove from a window, or the window itself loses and recreates its render target, **renderTargetChanged** raises on the affected root composition and all direct or indirect child compositions.

If a composition is not added to a window, its render target is **null**.

