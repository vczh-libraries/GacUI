# Handling Input Events

Raw input events are in compositions. Normally a composition doesn't receive events, until **GuiGraphicsComposition**::**GetEventReceiver** is called. This function create a big object containing all event registrations inside it, and cause **GuiGraphicsComposition**::**HasEventReceiver** to become **true** forever. You are not able to free the space of event registration object once it is associated to a composition.

In the event registration object you will find a lot of fields for events, for example, **leftButtonDown**. There are 3 methods for subscribing an event: - composition-\>GetEventReceiver()-\>EVENT_NAME.**AttachMethod**: subscribe an event using an object and a member function pointer. - composition-\>GetEventReceiver()-\>EVENT_NAME.**AttachFunction**: subscribe an event using a function pointer or a **vl::Func\<...\>** functor. - composition-\>GetEventReceiver()-\>EVENT_NAME.**AttachLambda**: subscribe an event using a lambda expression or other functors. All 3 methods returns a new handler object per each call. The only purpose for this object is to use in composition-\>GetEventReceiver()-\>EVENT_NAME.**Detach** and cancel the registration that returning that perticular handler object. If **Detach** returns **false**, either this registration has already been canceled, or this registration doesn't happen in this event.

In the event callback function, the first argument is always **GuiGraphicsComposition*** which is the owner of the event registration object, the second argument varies in different events.

In GacUI XML Resource, raw input events are treated like attributes of a composition. You can either specify a method name of the current instance, or specify a piece of code to run. Please check out [this page](../../.././gacui/xmlres/instance/events.md) for details.

## Events and Parent Chain

Most of the events are not only raised on the composition that receives the input, but also all the way to the root in the parent chain, which means after the event is raised on the source composition, it is then raised on its parent, and its parent's parent until the end.

Such process could be detected using members of the second argument: - **compositionSource**: the composition right under the mouse, which is the original event raiser. When the event raises through the parent chain, this member doesn't change. - **eventSource**: If the **compositionSource** has an associated event registration object, **eventSource** is **compositionSource**. Otherwise, **eventSource** is the nearest parent composition that has an associated event registration object. - **handled**: you can set this member in the event callback function to **true** to stop the event from going through more further in the parent chain. Meanwhile, the first argument is the composition who owns the event registration object the callback function is registered to. When the event raises through the parent chain, the first argument becomes the current "parent".

## Mouse Events

### Button and Wheel Events

GacUI offers the following button events: - leftButtonDown - leftButtonUp - leftButtonDoubleClick - middleButtonDown - middleButtonUp - middleButtonDoubleClick - rightButtonDown - rightButtonUp - rightButtonDoubleClick - horizontalWheel - verticalWheel mapping to the 5 standard buttons of a mouse. But a typical mouse today, middle button and vertical wheel are merged together, horizontal wheel are usually missing.

The type of the second argument is **vl::presentation::compositions::GuiMouseEventArgs&**, or **presentation::composition::GuiMouseEventArgs*** in **Workflow**.

Mouse button and wheel events are raised on captured composition, or the composition right under the mouse when no composition is captured, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

Other members contains the information of the event: - **ctrl**: **true** if the CTRL key is pressing. - **shift**: **true** if the SHIFT key is pressing. - **left**: **true** if the mouse left button is pressing. - **middle**: **true** if the mouse middle button is pressing. - **right**: **true** if the mouse right button is pressing. - **x** and **y**: the relative coordinate to **first callback argument**. - **wheel**: direction and distance of the wheel. A positive number means the wheel is rotating towards right or down. The unit of the rotation is 120 for today's typical mouse.

### Moving Events

GacUI offers the following button events: - mouseMove - mouseEnter - mouseLeave mapping to the 5 standard buttons of a mouse. But a typical mouse today, middle button and vertical wheel are merged together, horizontal wheel are usually missing.

**mouseEnter** happens when the mouse moves into a composition. **mouseLeave** happens when the mouse moves out of a composition. When a mouse moves from one composition to its child composition, it doesn't count as leaving the original composition, it is still in the original composition and its child composition.

**mouseMove** happens when the mouse moves. It is raised on captured composition, or the composition right under the mouse when no composition is captured, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

### Capturing

When a button down event happens, the **compositionSource** is captured. A captured composition becomes the **compositionSource** of all following mouse events happen in the containing window (or menu, popup, etc...), including button up events.

After the button up event happens, the capturing is released.

### Moving or deleting controls or compositions during mouse events.

Some times you may want to change the control tree structure in a callback of mouse events, (including **Clicked** in buttons).

This is tricky because, mouse events are not just sent to this composition, it is sent to all its parent compositions. Even when **handled** is set to **true**, there is still something to do after the callback function ends.

When you move or delete controls or compositions that affect the parent chain, **GacUI will stably CRASH**.

In order to solve this issue, **GetApplication()-\>InvokeInMainThread** is your friend. Whenever you call this function, the callback to **InvokeInMainThread** will run right after the current series of events. That is a very safe point to restructure your controls or compositions.

For example, if you customize the tab control to have a close button in each tab page: ``` void MyTabPage::buttonCloseClicked(GuiGraphicsComposition* sender, GuiEventArgs& arguments) { // this is still in the chain of mouse button events, // so InvokeInMainThread is required to delete MyTabPage. GetApplication()->InvokeInMainThread(this->GetRelatedControlHost(), [=]() { this->GetOwnerTab()->GetPages().Remove(this); SafeDeleteControl(this); }); } ```

## Focus Events

GacUI offers the following button events: - gotFocus - lostFocus - caretNotify

The type of the second argument is **vl::presentation::compositions::GuiEventArgs&**, or **presentation::composition::GuiEventArgs*** in **Workflow**.

A focus composition is the **FocusableComposition** of a control that has focus. Control like buttons will take focus after it is clicked, you can also call **SetFocus** to move focus to a control.

When a control get its focus, its **FocusableComposition** has the focus. If you are writing a control template, you can set any composition to the **FocusableComposition** property of the control template, and it becomes the **FocusableComposition** of the control.

A window has only one focused control. **gotFocus** raises on a composition when it is focused. **lostFocus** raises on a previous focused composition when it lost the focus.

**caretNotify** is keep raising on the focused control every half second. If a control wants to render an input caret, it needs to subscribe to **caretNotify**, and than changing the input caret between visible and invisible state when this event happens.

COMPOSITION-\>**GetRelatedGraphicsHost**()-\>**SetCaretPoint** is useful to tell the system where to show the little window for the input method. This function is strongly recommended to be called by the focused control to avoid state corruption.

If the window itself gets or loses focus, related events are raised on the window, events described here are not affected.

## Keyboard Events

GacUI offers the following button events: - previewKey - keyDown - keyUp

The type of the second argument is **vl::presentation::compositions::GuiKeyEventArgs&**, or **presentation::composition::GuiKeyEventArgs*** in **Workflow**.

Keyboard events are raised on the focused composition, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

**previewKey** is raised before any other keyboard events. If **handled** is set to **true**, not only the parent chain will stop, the following keyboard event will also be canceled.

**code** member of the second argument maps to an item in **vl::presentation::VKEY** enum.

Other members contains the information of the event: - **ctrl**: **true** if the CTRL key is pressing. - **shift**: **true** if the SHIFT key is pressing. - **alt**: **true** if the ALT button is pressing. - **capslock**: **true** if the CAPSLOCK button is activated. - **autoRepeatKeyDown**: **true** if this event is generated because of holding a key.

## Input Events

GacUI offers the following button events: - previewCharInput - charInput

The type of the second argument is **vl::presentation::compositions::GuiCharEventArgs&**, or **presentation::composition::GuiCharEventArgs*** in **Workflow**.

Input events are raised on the focused composition, and then go through the parent chain, which is stoppable by setting **handled** to **true**.

**previewCharInput** is raised before any other keyboard events. If **handled** is set to **true**, not only the parent chain will stop, the following keyboard event will also be canceled.

**code** member of the second argument is a **wchar_t** that being typed into the control. In Windows, **wchar_t** is a UTF-16 code point. In other platform, **wchar_t** is a UTF-32 code point.

Other members contains the information of the event: - **ctrl**: **true** if the CTRL key is pressing. - **shift**: **true** if the SHIFT key is pressing. - **alt**: **true** if the ALT button is pressing. - **capslock**: **true** if the CAPSLOCK button is activated.

## Other Events

GacUI offers the following button events: - clipboardNotify - renderTargetChanged

The type of the second argument is **vl::presentation::compositions::GuiEventArgs&**, or **presentation::composition::GuiEventArgs*** in **Workflow**.

When content in the system clipboard changes, **clipboardNotify** raises on every compositions.

When the render target is changed, this may because a composition is added to or remove from a window, or the window itself loses and recreates its render target, **renderTargetChanged** raises on the affected root composition and all direct or indirect child compositions.

If a composition is not added to a window, its render target is **null**.

