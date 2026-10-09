# 1. Windows touch and gesture capabilities

This is a proposed plan, not an implemented feature. The implementation targets are native Windows and the GacJS browser renderer. Other providers should retain their existing mouse behavior without implementing touch. Platform references were checked on 2026-10-09.

The contact-input paths are Windows `WM_POINTER*` on Windows 8 or newer and browser Pointer Events. The catalogs below cover their application-facing touch, gesture, feedback, and testing facilities. Pen and touchpad facilities are identified separately. Device-driver development is outside this plan.

## 1.1 Choosing an input path

| Windows API | What it provides | Suggested use |
|---|---|---|
| Pointer input, `WM_POINTER*` | Separate contacts, contact data, history, and cancellation information. Windows 8 onward. | Primary Windows input path. |
| Interaction Context | Recognizes gestures from pointer data supplied by the application. | Optional Windows recognition implementation. |
| `WM_GESTURE` | Automatic gestures produced by default pointer processing. | Platform behavior reference; raw contacts still use the pointer path. |
| Direct Manipulation | A larger system for moving/scaling content, inertia, and composition. | Optional integration for native viewports and touchpads. |

`WM_POINTER` is the only Windows contact-input path in this plan. Passing unhandled pointer input to `DefWindowProc` can produce `WM_GESTURE` or compatibility mouse input. Handling the pointer stream consistently lets GacUI provide its own gestures and mouse fallback, or explicitly feed Interaction Context. Recognition is separate from contact acquisition; enabling a recognizer does not promise independent, uninterrupted raw and gesture streams. [Pointer default processing](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown), [Interaction Context](https://learn.microsoft.com/en-us/windows/win32/input_intcontext/interaction-context-portal).

## 1.2 All pointer-related message groups

There are **three basic contact messages**: Down, Update, and Up. Windows has no separate `WM_POINTERCANCEL`; cancellation is described below. The remaining messages provide boundaries, ownership, devices, or specialized routing.

| Message | Simple meaning |
|---|---|
| `WM_POINTERDOWN` | Contact begins. |
| `WM_POINTERUPDATE` | Pointer data changes or is reported again. |
| `WM_POINTERUP` | Contact ends; inspect flags before treating it as successful. |
| `WM_POINTERENTER`, `WM_POINTERLEAVE` | The pointer enters/leaves a window or detection range. These are not substitutes for Down/Up. |
| `WM_POINTERCAPTURECHANGED` | The window loses ownership of a pointer. |
| `WM_POINTERACTIVATE` | Decide whether pointer input should activate an inactive window. |
| `WM_NCPOINTERDOWN`, `WM_NCPOINTERUPDATE`, `WM_NCPOINTERUP` | Input in non-client areas, such as the window frame. |
| `WM_POINTERWHEEL`, `WM_POINTERHWHEEL` | Vertical/horizontal wheel input represented by the pointer system. |
| `WM_POINTERDEVICECHANGE`, `WM_POINTERDEVICEINRANGE`, `WM_POINTERDEVICEOUTOFRANGE` | Device changes and device range notifications. |
| `WM_POINTERROUTEDTO`, `WM_POINTERROUTEDAWAY`, `WM_POINTERROUTEDRELEASED` | Specialized routing between configured content owners. |
| `WM_PARENTNOTIFY` | Can tell a parent HWND about a descendant's pointer down. |
| `WM_TOUCHHITTESTING` | Evaluate which nearby target should receive a touch. |
| `DM_POINTERHITTEST` | Participate in Direct Manipulation hit testing. |

[Windows pointer message catalog](https://learn.microsoft.com/en-us/windows/win32/inputmsg/messages).

**Update does not mean only movement.** Position may stay unchanged while contact data changes. Repeated samples can also have the same position. The injection API explicitly uses stationary updates for press-and-hold. Do not implement a hold timer by waiting for movement events. [Touch injection behavior](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-injecttouchinput).

## 1.3 Contact data and functions

`GET_POINTERID_WPARAM` obtains the contact ID; `GetPointerType` distinguishes touch, pen, and other pointer types. `GetPointerInfo` returns `POINTER_INFO`: identity, frame, time, device, target, coordinates, and status flags. The ID remains stable for that contact's lifetime; it does not permanently identify a particular finger. Coordinates include raw and prediction-adjusted forms. Convert screen coordinates and DPI into GacUI's coordinate system deliberately. [POINTER_INFO](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-pointer_info).

`GetPointerTouchInfo` adds contact bounds, orientation, and pressure. Read `touchMask` to find which values are valid. Devices need not measure all of them. `GetPointerPenInfo` similarly adds pen pressure, rotation, tilt, and pen flags; pen hover, eraser, and barrel-button input need separate semantics. [POINTER_TOUCH_INFO](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-pointer_touch_info), [POINTER_PEN_INFO](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-pointer_pen_info).

| Functions | Feature |
|---|---|
| `GetPointerInfoHistory`, `GetPointerTouchInfoHistory`, `GetPointerPenInfoHistory` | Recover samples combined into a delivered message. |
| `GetPointerFrameInfo`, `GetPointerFrameTouchInfo`, `GetPointerFramePenInfo` | Read contacts reported together in one frame. |
| `GetPointerFrameInfoHistory`, `GetPointerFrameTouchInfoHistory`, `GetPointerFramePenInfoHistory` | Read combined frames, including history. |
| `SkipPointerFrameMessages` | Discard queued messages for a frame already processed in full. This does not cancel any contact. |
| `GetPointerInputTransform` | Obtain transforms associated with pointer coordinates. |
| `GetUnpredictedMessagePos` | Obtain position before touch prediction. |
| `GetPointerCursorId` | Obtain a cursor identity, distinct from the contact lifetime ID. |
| `EnableMouseInPointer`, `IsMouseInPointerEnabled` | Enable/query physical mouse input through pointer messages. This is not touch-to-mouse fallback control. |

[Pointer function catalog](https://learn.microsoft.com/en-us/windows/win32/inputmsg/functions).

Frame retrieval covers relevant contacts belonging to the same HWND. Read the information while processing the message: retrieving the next message can make the previous information unavailable. History is needed when every reported sample matters; one message per hardware sample is not guaranteed. [GetPointerFrameTouchInfo](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getpointerframetouchinfo).

Useful flags include contact/range state, primary pointer, confidence, and cancellation. Confidence is a hint about intentional input, not a reliable palm classifier. Only the primary pointer is normally promoted to mouse. When the first finger lifts, another already-down finger does not become primary; all contacts must lift before a new primary is established. This does not stop delivery for the other contact IDs. [Pointer flags](https://learn.microsoft.com/en-us/windows/win32/inputmsg/pointer-flags-contants).

## 1.4 Capture, cancellation, and mouse fallback

Touch down implicitly captures that pointer to its target HWND. Updates can therefore continue outside the window. `WM_POINTERCAPTURECHANGED` means that ownership was lost; a later Up is not guaranteed. Its associated pointer information is limited, so retain the last valid sample yourself. [WM_POINTERDOWN](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown), [WM_POINTERCAPTURECHANGED](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointercapturechanged).

`POINTER_FLAG_CANCELED` reports abnormal termination. It is a flag to read, not a command to set on received input. A canceled Up must not count as a successful release or click. Windows does not document one universal sequence for every cancellation cause. [Cancellation flag](https://learn.microsoft.com/en-us/windows/win32/inputmsg/pointer-flags-contants).

Passing unhandled pointer input to `DefWindowProc` allows default gestures or mouse promotion. Microsoft warns that consuming some input and default-processing the rest can produce undefined behavior. For selective per-control fallback, the framework should consume its touch stream consistently and generate its own compatibility mouse events. `EnableMouseInPointer` controls the opposite direction and is a process-level choice. [Default pointer processing](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown), [EnableMouseInPointer](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemouseinpointer).

`GetCurrentInputMessageSource` identifies the source category of the current message. `GetMessageExtraInfo` has documented touch/pen signatures for promoted mouse messages. Use source information to prevent duplicate delivery while retaining real mouse input. [Input source](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getcurrentinputmessagesource), [promoted mouse identification](https://learn.microsoft.com/en-us/windows/win32/tablet/system-events-and-mouse-messages).

There is no general desktop receiving-side `CancelPointerInput` or `ReleasePointerCapture` API in this pointer API family. `ReleaseCapture` releases **mouse** capture. `WM_CANCELMODE` cancels modes such as menu/scroll handling, not an arbitrary physical finger. The application can stop its own drag or recognizer and ignore the remaining contact packets. Injection APIs can cancel contacts they inject. [Pointer API catalog](https://learn.microsoft.com/en-us/windows/win32/inputmsg/functions), [ReleaseCapture](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasecapture), [WM_CANCELMODE](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-cancelmode).

`RegisterPointerInputTarget`/`UnregisterPointerInputTarget` redirect a pointer type across the desktop and require UI Access privilege. These are specialized routing tools, not per-control capture APIs. The documented `RegisterPointerInputTargetEx` entry is explicitly unsupported and should not be used for this design. [RegisterPointerInputTarget](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerpointerinputtarget), [unsupported Ex variant](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerpointerinputtargetex).

## 1.5 Devices, touch targeting, and feedback

| Functions or messages | Feature |
|---|---|
| `GetSystemMetrics(SM_DIGITIZER)` / `SM_MAXIMUMTOUCHES` | Detect digitizer features and simultaneous-contact capacity. There is no universal two-finger limit. |
| `GetPointerDevices`, `GetPointerDevice` | Inspect connected devices. |
| `GetPointerDeviceProperties`, `GetRawPointerDeviceData` | Inspect additional device properties and values. |
| `GetPointerDeviceRects`, `GetPointerDeviceCursors` | Read device/display coordinate ranges and cursor IDs. |
| `RegisterPointerDeviceNotifications` | Request device change/range notifications. |
| `RegisterTouchHitTestingWindow` | Enable touch target evaluation. |
| `EvaluateProximityToRect`, `EvaluateProximityToPolygon` | Score nearby targets and suggest an adjusted hit point. |
| `PackTouchHitTestingProximityEvaluation` | Format the result for `WM_TOUCHHITTESTING`. |
| `GetWindowFeedbackSetting`, `SetWindowFeedbackSetting` | Query/control OS contact and gesture visual feedback. |
| `WM_TABLET_QUERYSYSTEMGESTURESTATUS` | Control system behaviors such as press-and-hold right-click and pen flicks. |

[System capabilities](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetrics), [device functions](https://learn.microsoft.com/en-us/windows/win32/input_pointerdevice/functions), [touch hit testing](https://learn.microsoft.com/en-us/windows/win32/input_touchhittest/functions), [feedback settings](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowfeedbacksetting), [system gesture settings](https://learn.microsoft.com/en-us/windows/win32/tablet/wm-tablet-querysystemgesturestatus-message).

## 1.6 Automatic Windows gestures

`WM_GESTURENOTIFY` lets a window prepare recognition settings. `WM_GESTURE` supplies a recognized gesture. `GetGestureInfo` reads `GESTUREINFO`; `GetGestureExtraArgs` reads any extra arguments; `CloseGestureInfoHandle` releases handled data. [Gesture messages](https://learn.microsoft.com/en-us/windows/win32/wintouch/wm-gesture), [GetGestureExtraArgs](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getgestureextraargs).

| Gesture ID | Meaning and useful data |
|---|---|
| `GID_PAN` | Move content; provides a position from which to calculate movement. |
| `GID_ZOOM` | Pinch/stretch; provides center and finger distance, from which to calculate scale. |
| `GID_ROTATE` | Rotate; provides center and an encoded angle. |
| `GID_TWOFINGERTAP` | Two fingers tap together. |
| `GID_PRESSANDTAP` | One finger stays down while another taps. This is not one-finger long press. |
| `GID_BEGIN`, `GID_END` | Generic gesture boundaries, not two additional gesture kinds. |

There are **five gesture kinds**, plus the two boundary IDs. There are no `GID_TAP`, `GID_DOUBLETAP`, or `GID_HOLD` identifiers. [Gesture identifiers and arguments](https://learn.microsoft.com/en-us/windows/win32/wintouch/wm-gesture).

`GF_BEGIN`, `GF_INERTIA`, and `GF_END` describe progress. Pan inertia can continue after fingers lift. There is no `GF_CANCEL`. Forward generic `GID_BEGIN`/`GID_END` to default processing as documented, and use the documented handle ownership rules. [Gesture overview](https://learn.microsoft.com/en-us/windows/win32/wintouch/windows-touch-gestures-overview).

`SetGestureConfig`/`GetGestureConfig` select gestures, one-finger pan axes, directional confinement, and pan inertia. Settings persist for the HWND and can be changed during `WM_GESTURENOTIFY`. Configure explicitly; rotation is not enabled by default. Default processing can translate pan into scroll messages, zoom into Ctrl+wheel, and press-and-hold into right-click behavior. [Gesture configuration](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setgestureconfig), [default behavior and troubleshooting](https://learn.microsoft.com/en-us/windows/win32/wintouch/troubleshooting-applications).

Handling a gesture such as pan suppresses that gesture's default action; it does not itself cancel the contacts. Pointer messages have no flag saying that a gesture was recognized or handled. Track that decision from the gesture notification and the control's response. Interaction Context can recognize supplied frames while GacUI retains pointer input; Direct Manipulation can take ownership as described below. [Gesture processing](https://learn.microsoft.com/en-us/windows/win32/wintouch/wm-gesture), [pointer fields](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-pointer_info).

## 1.7 Explicit gesture recognizers and motion engines

**Interaction Context** is suitable for a UI framework that already owns raw input. It recognizes tap/double-tap, secondary tap, hold, cross-slide, and manipulation. Manipulation combines translation, scale, and rotation. Its named Drag interaction concerns mouse/pen dragging; touch panning is a manipulation. [Interaction kinds](https://learn.microsoft.com/en-us/windows/win32/api/interactioncontext/ne-interactioncontext-interaction_id).

| Function family | Feature |
|---|---|
| `CreateInteractionContext`, `DestroyInteractionContext` | Create/release a recognizer. |
| `RegisterOutputCallbackInteractionContext`, `RegisterOutputCallbackInteractionContext2` | Receive recognized results. |
| `SetInteractionConfigurationInteractionContext`, `GetInteractionConfigurationInteractionContext` | Choose recognized actions and axes. |
| `AddPointerInteractionContext`, `RemovePointerInteractionContext` | Choose contacts when `INTERACTION_CONTEXT_PROPERTY_FILTER_POINTERS` is enabled. |
| `ProcessPointerFramesInteractionContext` | Supply complete pointer frames. |
| `BufferPointerPacketsInteractionContext`, `ProcessBufferedPacketsInteractionContext` | Supply and then process buffered pointer history. |
| `ProcessInertiaInteractionContext` | Advance inertia using a timer. |
| `StopInteractionContext` | Stop current recognition while retaining settings. |
| `ResetInteractionContext` | Reset state and settings; current actions are canceled without notifications. |
| `GetStateInteractionContext`, `GetPropertyInteractionContext`, `SetPropertyInteractionContext` | Inspect state and configure general properties. |
| `Get/SetTapParameterInteractionContext`, `Get/SetHoldParameterInteractionContext` | Adjust tap/hold thresholds. |
| `Get/SetTranslationParameterInteractionContext`, `Get/SetInertiaParameterInteractionContext` | Adjust movement and inertia. |
| `GetCrossSlideParameterInteractionContext`, `SetCrossSlideParametersInteractionContext` | Configure cross-slide behavior. |
| `Get/SetMouseWheelParameterInteractionContext`, `SetPivotInteractionContext` | Map wheel motion and configure a rotation pivot. |

In this table, `Get/Set` abbreviates the two correspondingly named functions. Newer entries have their own Windows-version requirements. [Interaction Context function catalog](https://learn.microsoft.com/en-us/windows/win32/api/_input_intcontext/).

Configuration supports axis constraints and translation/rotation/scaling inertia. Output contains incremental and accumulated transforms and velocity. Flags distinguish begin, end, cancel, and inertia; Cancel also has End set. Stopping this recognizer does not cancel the physical OS contact. [Configuration flags](https://learn.microsoft.com/en-us/windows/win32/api/interactioncontext/ne-interactioncontext-interaction_configuration_flags), [manipulation output](https://learn.microsoft.com/en-us/windows/win32/api/interactioncontext/ns-interactioncontext-interaction_arguments_manipulation), [interaction flags](https://learn.microsoft.com/en-us/windows/win32/api/interactioncontext/ne-interactioncontext-interaction_flags).

**Direct Manipulation** offers `IDirectManipulationManager`, viewports, content transforms, event handlers, update management, and compositor integration. A viewport can accept/release contacts with `SetContact`/`ReleaseContact`/`ReleaseAllContacts`, stop motion, and apply configured pan/zoom behavior, including translation/scaling inertia and axis rails. Once it takes a contact, the application's ordinary raw pointer stream can end with capture loss. Its contact methods are specific to Direct Manipulation, not general HWND capture APIs. [Direct Manipulation overview](https://learn.microsoft.com/en-us/windows/win32/directmanipulation/direct-manipulation-portal), [viewport interface](https://learn.microsoft.com/en-us/windows/win32/api/directmanipulation/nn-directmanipulation-idirectmanipulationviewport), [supported configurations](https://learn.microsoft.com/en-us/windows/win32/api/directmanipulation/ne-directmanipulation-directmanipulation_configuration).

## 1.8 Touchpads and injected input

A touchpad is not a touchscreen. Ordinary applications commonly receive wheel messages or system actions, not one raw contact per touchpad finger. Direct Manipulation and InteractionTracker have touchpad integration. [Precision touchpad overview](https://learn.microsoft.com/en-us/windows/win32/input-precisiontouchpad/precision-touchpad-portal).

Microsoft also documents newer, separately gated precision-touchpad facilities: `RegisterTouchpadCapableWindow`/`RegisterTouchpadCapableThread`, `GetPointerTouchpadInfo` variants, `ReportWindowContentInertia`, `TouchpadGesturesController`, `PhysicalGestureRecognizer`, `ProcessPointerFramesInteractionContext2`/`BufferPointerPacketsInteractionContext2`, and touchpad injection through `CreateSyntheticPointerDevice2`/`InjectTouchpadAction`. The overview currently carries a prerelease notice. Treat these as optional future features after checking SDK/OS availability; do not make them part of the initial Windows compatibility baseline. [Precision touchpad facilities and availability notice](https://learn.microsoft.com/en-us/windows/win32/input-precisiontouchpad/precision-touchpad-portal).

For tests, `InitializeTouchInjection`/`InjectTouchInput` simulate one or more contacts. Injected cancellation permits Canceled with Up or Update; this does not establish a universal sequence for physical hardware. Windows 10 version 1809 onward also offers `CreateSyntheticPointerDevice`, `InjectSyntheticPointerInput`, and `DestroySyntheticPointerDevice` for synthetic touch/pen devices. [Touch injection](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-injecttouchinput), [synthetic pointer devices](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createsyntheticpointerdevice).

## 1.9 Typical Windows event sequences

These are relevant lifecycle events, not a complete message queue trace. Boundary, activation, and non-client messages can accompany them. `Update(A/B)` means updates for either or both IDs, not one pointer representing two fingers.

| Scenario | Typical sequence and consequence |
|---|---|
| Tap | `Down(A) -> optional Update(A) -> Up(A)`. A recognizer decides whether it was a tap. |
| Drag outside the HWND | `Down(A) -> Update(A)... -> Up(A)`, including outside positions while captured. |
| A down, B down, A released first | `Down(A) -> Down(B) -> Update(A/B)... -> Up(A) -> Update(B)... -> Up(B)`. B keeps its ID and continues independently. |
| Stationary hold | `Down(A) -> optional same-position updates -> Up(A)`. Recognition also needs time; movement is not required. |
| Native cancellation | A sample carrying `POINTER_FLAG_CANCELED` ends the interaction unsuccessfully. Even a native Up with this flag must not click. |
| Capture taken away | `Down(A) -> Update(A)... -> WM_POINTERCAPTURECHANGED(A)`. Do not wait for Up. |
| Default pointer processing: pan with inertia | `GID_BEGIN -> GID_PAN/GF_BEGIN -> pan updates -> fingers lift -> pan/GF_INERTIA updates -> pan/GF_END -> GID_END`. Exact packet grouping varies. |
| Interaction Context pinch | Raw contacts feed the context; manipulation begins, reports scale/translation/rotation, then ends or enters configured inertia. |
| Application cancels its recognizer | Recognizer cancellation/cleanup occurs; native updates and final release can still arrive and must be drained without activation. |

The first finger ending does not end the second finger. It also does not transfer primary-pointer mouse promotion to that finger. [Pointer lifetime](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-pointer_info), [capture loss](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointercapturechanged), [gesture progress](https://learn.microsoft.com/en-us/windows/win32/wintouch/windows-touch-gestures-overview).

# 2. Browser touch and gesture capabilities

Use **Pointer Events as the only browser contact-input API**. It covers touch, pen, and mouse. Optional high-frequency sampling, Safari gesture recognition, touchpads, and text input are separate capabilities. [Pointer Events](https://developer.mozilla.org/en-US/docs/Web/API/Pointer_events).

## 2.1 All pointer events

The pointer event family has **11 events**, including the optional `pointerrawupdate` feature:

| Event | Simple meaning |
|---|---|
| `pointerdown` | A contact begins. For a mouse, its buttons enter the pressed state. |
| `pointermove` | Position or another pointer property changes. |
| `pointerup` | A contact ends normally. |
| `pointercancel` | The browser terminates this contact stream unsuccessfully. |
| `pointerover` | Enters an element's hit area; bubbles. |
| `pointerenter` | Enters an element or its descendants; does not bubble. |
| `pointerout` | Leaves an element's hit area; bubbles. |
| `pointerleave` | Leaves an element and its descendants; does not bubble. |
| `gotpointercapture` | An element acquires capture for this pointer. |
| `lostpointercapture` | An element loses capture for this pointer. |
| `pointerrawupdate` | Optional updates delivered with less delay/batching. |

Boundary events can also result from contact termination or layout changes. They do not replace Down/Up. [Pointer event inventory](https://developer.mozilla.org/en-US/docs/Web/API/Pointer_events).

`pointermove` can report pressure, contact size, pen orientation, or button changes without a change in X/Y. It is not a timer and does not promise continuous reports while a finger is still. [pointermove](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointermove_event).

## 2.2 Contact data and available methods

| Property | Meaning |
|---|---|
| `pointerId` | Contact identity during its active lifetime. |
| `pointerType` | Usually `touch`, `pen`, or `mouse`. |
| `isPrimary` | Primary pointer of that input type. Other contacts are still usable. |
| `clientX/Y`, `pageX/Y`, `screenX/Y` | Viewport, document, or screen coordinates. Client coordinates use CSS pixels. |
| `width`, `height`, `pressure` | Contact size and normalized pressure, where meaningful. |
| `tangentialPressure`, `tiltX/Y`, `twist` | Additional measurements, mainly for pens. |
| `altitudeAngle`, `azimuthAngle` | Additional pen orientation, in radians. |
| `button`, `buttons` | Changed button and current button state. |
| `altKey`, `ctrlKey`, `metaKey`, `shiftKey` | Modifiers. |
| `timeStamp`, `target`, `isTrusted`, `cancelable` | Time, DOM routing, origin, and whether default action can be prevented. |

Not every device measures every field; some values are defaults rather than measured pressure or size. [PointerEvent data](https://developer.mozilla.org/en-US/docs/Web/API/PointerEvent).

Maintain a map keyed by `pointerId`. IDs can be reused after a contact ends; they are not permanent finger identities. Optional `persistentDeviceId` concerns the device, not a finger, and should not be required by the design. `navigator.maxTouchPoints` reports simultaneous-contact capacity. [pointerId](https://developer.mozilla.org/en-US/docs/Web/API/PointerEvent/pointerId), [persistentDeviceId](https://developer.mozilla.org/en-US/docs/Web/API/PointerEvent/persistentDeviceId), [maxTouchPoints](https://developer.mozilla.org/en-US/docs/Web/API/Navigator/maxTouchPoints).

Do not filter out non-primary contacts. In particular, if A is primary and B is also down, releasing A does not mean B ends or must become primary. [Primary pointer semantics](https://www.w3.org/TR/pointerevents3/#the-primary-pointer).

## 2.3 Capture and cancellation

| Method | Meaning |
|---|---|
| `element.setPointerCapture(id)` | Route this pointer's subsequent events to the element. |
| `element.hasPointerCapture(id)` | Query capture ownership. |
| `element.releasePointerCapture(id)` | Release that ownership. |

Touchscreen Down normally establishes implicit capture. Capture keeps a drag routed to the same element outside its bounds, but does not prevent browser gesture takeover. It also suppresses ordinary boundary transitions over underlying elements while active. [setPointerCapture](https://developer.mozilla.org/en-US/docs/Web/API/Element/setPointerCapture), [pointerdown](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointerdown_event).

`pointerup` and `pointercancel` are different terminal outcomes. The browser may cancel when it starts scrolling/zooming, rejects a palm, or loses the interaction to system UI. **Do not expect Up after Cancel for that stream**, even if the physical finger is still down. Cleanup includes Out/Leave and capture release; do not depend on the exact placement of capture notifications among those boundary events. [pointercancel](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointercancel_event).

`lostpointercapture` alone does not mean cancellation or finger release. It also occurs during normal capture cleanup after Up. If capture is released while still down, input can continue with different DOM routing. [releasePointerCapture](https://developer.mozilla.org/en-US/docs/Web/API/Element/releasePointerCapture), [lostpointercapture](https://developer.mozilla.org/en-US/docs/Web/API/Element/lostpointercapture_event).

There is no standard `cancelPointer(id)` method to terminate a physical contact. Cancel the library's own interaction instead. `preventDefault()` suppresses a cancelable default action; `stopPropagation()` changes DOM propagation; neither means finger cancellation. Dispatching a synthetic `pointercancel` does not stop the native hardware stream. [preventDefault](https://developer.mozilla.org/en-US/docs/Web/API/Event/preventDefault), [dispatchEvent](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/dispatchEvent), [isTrusted](https://developer.mozilla.org/en-US/docs/Web/API/Event/isTrusted).

## 2.4 History, prediction, and input timing

| Feature | Use and limitation |
|---|---|
| `pointerrawupdate` | Lower-delay updates where supported; noncancelable and still possibly coalesced. |
| `getCoalescedEvents()` | Actual historical samples combined into one event, useful for drawing. |
| `getPredictedEvents()` | Estimated future samples, useful for temporary visual prediction. |

Feature-detect these APIs and their secure-context requirements. Ordinary `pointermove` remains the baseline. Do not apply overlapping raw/move samples twice. Replace predictions when actual input arrives; never commit a click, selection, or edit from prediction alone. [Raw updates](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointerrawupdate_event), [coalesced events](https://developer.mozilla.org/en-US/docs/Web/API/PointerEvent/getCoalescedEvents), [predicted events](https://developer.mozilla.org/en-US/docs/Web/API/PointerEvent/getPredictedEvents).

Browser events do not expose the same hardware-frame grouping as Windows frame APIs. A browser adapter can forward one changed contact at a time and let GacUI maintain the complete active-contact set.

## 2.5 Mouse fallback and activation

Browsers can generate compatibility mouse input from primary touch. Preventing the default of a cancelable primary `pointerdown` suppresses its compatibility mouse stream, but does not suppress mouse boundary events. `click`, `auxclick`, and `contextmenu` are separately dispatched higher-level events; preventing a pointer event does not prevent their dispatch. Handle duplicate activation separately when GacUI performs its own fallback. [Compatibility mouse mapping](https://www.w3.org/TR/pointerevents3/#compatibility-mapping-with-mouse-events).

| Higher-level event | Use |
|---|---|
| `click` | Activation, including touch, mouse, keyboard, and accessibility activation. |
| `auxclick` | Activation using a non-primary pointing-device button. |
| `dblclick` | Double-click recognition; a touchscreen double-tap is not guaranteed to produce it. |
| `contextmenu` | Context-menu request; long-press behavior varies. |

Do not drop keyboard/accessibility activation when filtering touch duplicates. [click](https://developer.mozilla.org/en-US/docs/Web/API/Element/click_event), [dblclick](https://developer.mozilla.org/en-US/docs/Web/API/Element/dblclick_event), [contextmenu](https://developer.mozilla.org/en-US/docs/Web/API/Element/contextmenu_event).

Preventing a DOM default action must happen synchronously in the browser handler. Passive listeners cannot prevent default actions. A response from a worker or remote GacUI process arrives too late to reliably decide the original DOM event's default action. Establish browser ownership policy locally before forwarding input. [preventDefault and passive listeners](https://developer.mozilla.org/en-US/docs/Web/API/Event/preventDefault).

## 2.6 Browser gestures and default-action policy

Browsers internally recognize pan, pinch zoom, and other actions, but there is **no portable general set of pan/pinch/rotate/swipe events for application controls**. Implement those recognizers from contacts when GacUI owns the interaction. For example, distance between two pointers provides pinch scale. [Pointer-based pinch example](https://developer.mozilla.org/en-US/docs/Web/API/Pointer_events/Pinch_zoom_gestures).

CSS `touch-action` declares which browser pan/zoom behaviors are allowed:

| Value | Allowed browser action |
|---|---|
| `auto` | Browser chooses normal panning/zooming behavior. |
| `none` | No browser panning/zooming on this surface. |
| `pan-x`, `pan-y` | One-finger panning on the selected axis. |
| `pan-left`, `pan-right`, `pan-up`, `pan-down` | Panning that starts in the selected scroll direction. Reversal can follow. |
| `pinch-zoom` | Multi-finger page panning/zooming. |
| `manipulation` | Panning and pinch zoom, excluding extra actions such as double-tap zoom. |

Directional names refer to scrolling: for example, `pan-up` permits an initial downward finger movement. Set the property before contact begins. The touched element and relevant ancestors jointly constrain the policy; changing it mid-gesture does not change that gesture. Browser takeover can cancel the application's pointer stream. `touch-action` does not control every default, such as selection or context menus. [touch-action](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/touch-action).

`overscroll-behavior` controls scroll boundaries: `auto` permits ordinary behavior, `contain` prevents chaining/navigation while retaining local effects, and `none` also removes local overscroll effects. Axis-specific variants are available. It applies to scroll containers; it does not generate gestures or automatically govern custom-rendered GacUI scrolling. [overscroll-behavior](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/overscroll-behavior).

If the browser owns a DOM scroll container, observe resulting position through `scroll`, and completion through `scrollend` where supported. These are viewport/content state notifications, not individual contact events. Browser inertia does not supply continued touch points after release. [scroll](https://developer.mozilla.org/en-US/docs/Web/API/Element/scroll_event), [scrollend](https://developer.mozilla.org/en-US/docs/Web/API/Element/scrollend_event).

## 2.7 Touchpads and Safari gestures

Trackpads generally produce cursor/wheel input rather than individually exposed fingers. `wheel` supplies `deltaX/Y/Z` and `deltaMode`; units can be pixels, lines, or pages. A wheel event need not cause scrolling. Trackpad pinch can arrive with `ctrlKey == true`, but actual Ctrl+wheel can do the same, so this does not identify a touchscreen pinch. [wheel](https://developer.mozilla.org/en-US/docs/Web/API/Element/wheel_event), [ctrlKey](https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/ctrlKey).

Safari/WebKit additionally provides proprietary `gesturestart`, `gesturechange`, and `gestureend`. `GestureEvent.scale` is relative to the initial distance and `rotation` is relative to the initial orientation. This is useful optional integration, not a cross-browser gesture API. Do not process both these gestures and a shared recognizer for the same interaction. [Apple GestureEvent](https://developer.apple.com/documentation/webkitjs/gestureevent), [GestureEvent reference](https://developer.mozilla.org/en-US/docs/Web/API/GestureEvent).

## 2.8 Typical browser event sequences

These examples omit variable sample counts and exact mouse/capture interleaving. They are not promises about every browser's complete event order.

| Scenario | Typical sequence and consequence |
|---|---|
| Tap | `pointerover -> pointerenter -> pointerdown -> pointerup -> pointerout -> pointerleave`; capture notifications can accompany it. `click` is separate. |
| Captured drag | `down(A) -> capture -> move(A)... -> up(A) -> capture released`. Leaving the element does not end A. |
| A down, B down, A released first | `down(A) -> down(B) -> up(A) -> move(B)... -> up(B)`. Remove only A at its Up. |
| Browser starts scrolling | `down -> possible moves -> pointercancel -> cleanup`. No matching Up is required later. |
| Custom pinch with browser pan/zoom disabled | `down(A) -> down(B) -> moves(A/B)... -> up(A) -> moves(B)... -> up(B)`. GacUI decides how the two-finger gesture changes when one finger remains. |
| Capture released while finger remains | `down -> gotpointercapture -> releasePointerCapture -> lostpointercapture -> further pointer events with normal routing`. This is not cancellation. |
| Library cancels its own action | Internal Cancel occurs; native move/up can still arrive. Ignore them for activation while completing bookkeeping. |
| Browser scroll inertia | Pointer contact ends or is canceled; `scroll` can continue as the browser moves content; eventual `scrollend` where supported. |
| Safari two-finger gesture | First touch begins; adding the second can start `gesturestart`; movement produces `gesturechange`; dropping below two ends the gesture even if one touch remains. This is Safari-specific. |

[Multi-touch handling](https://developer.mozilla.org/en-US/docs/Web/API/Pointer_events/Multi-touch_interaction), [pointer cancellation](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointercancel_event), [Safari gestures](https://developer.apple.com/documentation/webkitjs/gestureevent).

## 2.9 Text integration and comparison with Windows

Text entry requires more than pointer and key events. Native editable DOM elements expose `beforeinput`, `input`, and `compositionstart`/`compositionupdate`/`compositionend`. `selectionchange`, `Selection`, and `Range` expose DOM selection. These APIs do not automatically describe a selection stored only in GacUI. [beforeinput](https://developer.mozilla.org/en-US/docs/Web/API/Element/beforeinput_event), [composition events](https://developer.mozilla.org/en-US/docs/Web/API/Element/compositionstart_event), [Selection](https://developer.mozilla.org/en-US/docs/Web/API/Selection).

Optional `EditContext` connects custom editors to text services. The VirtualKeyboard API supplies keyboard geometry/overlay control where supported; `VisualViewport` helps observe viewport size/offset/scale changes. Feature-detect them. They do not provide a general gesture recognizer. Section 4 describes selection handles. [EditContext](https://developer.mozilla.org/en-US/docs/Web/API/EditContext), [VirtualKeyboard](https://developer.mozilla.org/en-US/docs/Web/API/VirtualKeyboard_API), [VisualViewport](https://developer.mozilla.org/en-US/docs/Web/API/VisualViewport).

| Concern | Native Windows | Browser |
|---|---|---|
| Basic raw lifecycle | Down/Update/Up messages plus flags/ownership notifications. | Down/Move/Up/Cancel events. |
| Multiple fingers | Independent pointer IDs and optional frames. | Independent pointer IDs; no equivalent hardware-frame contract. |
| Cancellation | Canceled flag and possible capture loss; Up not guaranteed. | Explicit `pointercancel`; stop expecting Up for that stream. |
| Built-in application gesture recognition | `WM_GESTURE`, Interaction Context, motion engines. | No portable complete gesture-event family; Safari has an extension. |
| Browser/OS ownership | Consistent native message handling and recognizer configuration. | Predeclared `touch-action` plus synchronous DOM handling. |
| Capture | Implicit HWND ownership; specialized APIs have their own rules. | Explicit per-ID DOM capture methods as well as implicit capture. |
| Inertia | Available in Windows recognizers/engines. | Browser owns native scrolling inertia; custom GacUI scrolling needs its own. |
| Custom text handles | Custom-drawn controls must integrate or draw them. | Custom GacUI selection is not automatically native DOM selection. |

# 3. Proposed GacUI native interfaces and input design

## 3.1 Scope and existing constraints

This section proposes API contracts and future work only. No implementation is included in this change.

The recommendation is **struct-based callbacks on existing listeners, raw contacts at the native boundary, and shared gesture recognition and mouse fallback above that boundary**. No new public interface is needed. Windows and browsers should deliver equivalent contact lifetimes even when their native event names differ. Linux/macOS/TUI providers may continue providing mouse input only.

The relevant existing code is:

| Area | Current shape and consequence |
|---|---|
| `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h` and `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.cpp` | `INativeWindowListener` callbacks already have no-op definitions. Add struct-based notifications there and, for explicit opt-in, one defaulted configuration method on the existing `INativeWindow`. |
| `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.h` and `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp` | One mouse capture composition and one shared set of mouse-button states. Touch needs a separate per-contact table. Mouse `handled` controls routing; it is not a native default-action policy. |
| `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedController.h` | One mouse capturing/hovering window. Hosted touch needs contact-to-window routing as well as contact-to-composition routing. |
| `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp` | Current mouse message path; add a separate raw-touch path and source deduplication. |
| `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_IO.txt` | Mouse/keyboard protocol has no contact lifetime, touch source, or cancellation data. |
| `<GacUI repo>/Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_IO.cpp` | A single pending mouse move is coalesced. Touch cannot reuse a single global last-move slot. |
| `<GacJS repo>/Gaclib/gaclib/renderer/src/GacUIRendererImpl.ts` | Mouse event forwarding and wheel handling; pointer acquisition and local default-action policy need to be added. |
| `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/index.ts` and `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/worker.ts` | Input crosses a worker boundary. Browser cancellation cannot wait for a core reply. |

The hosted-window coordinate mapping and remote protocol design in the knowledge base should guide the implementation. Preserve native-to-logical conversion at the established boundaries instead of mixing Windows physical pixels, browser CSS pixels, and composition coordinates.

## 3.2 Existing listeners, structs, and one configuration method

All incoming touch information can use methods added to `INativeWindowListener`. The following declaration sketch omits existing members and inheritance. The argument structs would be declared with native input types in `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h`.

```cpp
class INativeWindowListener
{
public:
    virtual void TouchInputInfoChanged(const NativeTouchInputInfo& info);
    virtual void TouchInput(const NativeTouchFrame& frame);

    // Only needed if provider-side recognition is added later.
    virtual void GestureInput(const NativeGestureEvent& gesture);
};

class INativeWindow
{
public:
    virtual bool ConfigureTouchInput(const NativeTouchInputConfig& config);
};
```

The listener methods have default no-op definitions. `ConfigureTouchInput` has a default definition returning false, so unsupported providers retain their current behavior without implementing anything. These are additions to existing interfaces, with **no new touch service, recognizer interface, or listener interface**. All methods are non-pure; adding virtual members preserves provider source compatibility but requires rebuilding consumers for C++ ABI compatibility. If a pure configuration method is required by project conventions, the entire no-feature migration is an override returning false.

The initial implementation needs the two touch callbacks and the configuration method. Keep `GestureInput` optional future work: the shared recognizer raises composition events directly and must not send its results back through native listeners.

| Struct | Information passed by value fields |
|---|---|
| `NativeTouchInputConfig` | Request revision, requested mode, and optional sampling preferences such as actual history. A future provider recognizer can add options here without returning another interface. |
| `NativeTouchInputInfo` | Request revision/result, effective mode, capability values and their validity, and input-connection generation. The same callback reports configuration completion and later capability/availability changes. |
| `NativeTouchFrame` | Ordered actual contact samples, delivery sequence, and optional platform-frame metadata. |
| `NativeGestureEvent` | Recognized action, interaction ID, contact/session association, phase, transforms, and inertia state. |

`NativeTouchInputMode` has two values:

- `MouseOnly`: preserve existing input behavior. This is the initial mode.
- `FrameworkTouch`: deliver contacts acquired from pointer events and suppress/filter duplicate native touch-to-mouse input. Physical mouse input still uses the existing path.

`ConfigureTouchInput` returning false means the request was not accepted and the effective mode is unchanged. Returning true means **accepted for processing**, not that the renderer is already ready. `TouchInputInfoChanged` subsequently reports the matching revision as Applied or Rejected, the effective configuration, and a reason such as Unsupported or Busy. Queue completion through the existing event loop; do not introduce a new synchronous callback during listener installation.

Install listeners and finish binding the host before requesting enhanced input. One acquisition owner configures each real window/surface: the graphics host in direct mode, or the hosting/renderer component that owns the underlying input chain. Other listeners observe and do not negotiate conflicting modes. Hosted child windows share that acquisition policy while retaining separate control routing. Uninstalling an observer does not change native mode.

Configure enhanced input during initialization before accepting user interaction. Reject changes while delivered contacts are active; never split one contact lifetime between modes. An empty application contact set after cancellation does not prove physical fingers have lifted, so a later policy change applies to future native gestures only. The first implementation can limit configuration to initialization and teardown rather than promising live takeover.

For a remote/browser provider, Applied means that the local handlers, CSS, capture bookkeeping, and duplicate filtering are installed. Order that notification before the first frame for the new configuration, and tag frames with its revision/connection generation. Existing-mode input remains ordered before that boundary. The core must not synthesize touch fallback or assume raw input is active merely because the configuration request was accepted. The owner drains/cancels its input before teardown; destroy the surface or complete a supported transition before detaching the owner.

Capabilities include raw-touch support, known maximum contacts, actual history, and optional contact bounds/pressure/orientation. Unknown capacity is different from zero. Browser fields may have fallback values without a hardware-measurement flag; preserve unknown validity rather than inferring measured pressure from 0.5. Start the host's cache in MouseOnly/unknown state; a rejected request or an unsupported provider must not be mistaken for successful setup.

**Could the additions be strictly listener-only?** Yes, a provider could ask a new listener callback for configuration, or enhanced providers could use a fixed startup policy. However, configuration is a request from the host to the provider, while input notifications flow in the other direction. A listener-query design would need rules for multiple listeners, when to query them again, and what happens when the last interested listener leaves. A fixed enabled policy would suppress old mouse behavior even for consumers that only inherit the new no-op callbacks.

The existing Windows, hosted, and remote `InstallListener` implementations only register listeners. `GuiGraphicsHost::SetNativeWindow` installs its listener before finishing its host binding in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp`. Adding an immediate setup callback there would introduce a lifecycle change. One explicit configuration method called after binding is therefore the recommended small exception to listener-only additions. It avoids both new public interfaces and hidden setup rules. No new method is needed on `INativeController` or its service plumbing.

## 3.3 Is input state queried from a window useful?

State associated with a window is useful, but it does not need to be exposed as another native interface. The graphics host already needs contact records, gesture ownership, mouse-fallback decisions, and the latest reported configuration. Store that state in ordinary internal structs updated by callbacks.

| Possible need outside a callback | How to support it without a native service getter |
|---|---|
| Decide whether enhanced input is ready | Read the host's cached `NativeTouchInputInfo`, including applied revision. |
| Show capability information or adapt a UI | Use the last capability notification; retain unknown/unavailable values. |
| Advance hold, inertia, or selection autoscroll | Existing timers read the host's contact/gesture state. No new native query is necessary. |
| Route a gesture or cancel an existing drag | Use the common host's interaction records and existing control/composition objects. |
| Attach diagnostics after initialization | Read a copied host snapshot; do not manufacture Down events or reinterpret already active contacts as new interactions. |

A future read-only method on an existing host/window class could return a snapshot struct if a concrete caller needs it. That would be convenience access to last-known state, not fresher physical input and not an object exposing another interface. Do not add it in the initial native contract: the current requirements are covered by listeners plus cached state.

Callbacks carry immutable data valid for the callback. Consumers that retain data must copy the relevant fields/collections; remote adapters must serialize/copy before returning. Update the owner's cache in input order before notifying control observers, and use existing UI-thread/event-loop ownership. This avoids retaining references to Windows message data or DOM event objects. It also keeps a snapshot query from being mistaken for a way to reverse browser defaults after the event has finished.

## 3.4 Normalized contact contract

| Proposed type/field | Contract |
|---|---|
| `NativeTouchPhase` | `Down`, `Update`, `Up`, `Cancel`. |
| `NativeTouchId` | Identifies one contact lifetime. Platform IDs can be reused even within a connection; add an internal lifetime token for retained gesture/activation records, and a connection generation where needed. |
| `NativeTouchSample.id`, `.phase` | The changed contact and what happened to it. |
| `.position` | Position relative to the receiving native window's client area, with fractional precision. Convert into logical composition coordinates during routing. |
| `.timestamp` | Monotonic time in a documented unit, proposed microseconds, within a documented clock domain. Do not subtract browser timestamps from a Windows clock without conversion. |
| `.primary` | Platform hint only; never filter raw contacts by this value. |
| `.contactBounds`, `.pressure`, `.orientation`, `.validFields` | Optional measurements with explicit validity. Normalize pressure to 0..1, but retain whether it is measured/known. |
| `.modifiers`, `.source` | Relevant modifier state and source classification. Keep touch, real mouse, and optional pen distinct. |
| `.cancelReason` | Acquisition-level termination such as platform interruption, unrecoverable delivery ownership loss, connection loss, or native-surface teardown; allow unknown. Logical target/action cancellation belongs to routed interaction state instead. |
| Optional `.platformFrameId` on each sample | Identifies a hardware input frame only when the backend supplies one. Samples in a transport/history batch may have different frame IDs; this is not a touch-session ID. |
| `NativeTouchFrame.samples` | An ordered batch of actual changed samples. A one-sample batch is valid. |
| `.sequence`, `.configurationRevision`, `.inputGeneration` | Order delivery and identify the applied input configuration/connection. |

The adapter reads platform data while it is valid and copies it before asynchronous forwarding. A batch may contain several contacts and/or historical samples; document chronological ordering and do not process the newest sample twice after reading history. Prediction is an optional separate visual channel, not committed contact input.

For every delivered Down, the normalized stream provides exactly one terminal outcome: Up **or** Cancel. A Windows canceled packet or ownership loss becomes Cancel. A browser `pointercancel` becomes Cancel. Suppress later native packets that would otherwise generate a duplicate terminal event. Up means contact completion, not necessarily a tap; the recognizer decides that.

Maintain separate active records for A and B. `Up(A)` removes A only. No consumer should infer finger count from mouse button state, `primary`, or the number of events received in the last frame.

For browser `lostpointercapture`, distinguish normal cleanup after Up/Cancel from unexpected loss while active. Unexpected loss can cause a **framework** cancellation if reliable routing cannot be maintained; it is not proof that the browser canceled the physical contact. Keep terminal bookkeeping idempotent.

## 3.5 Contact IDs, frames, sessions, and gesture groups

**These identities describe different things.** Neither Windows raw pointers nor browser Pointer Events supplies a portable ID for the whole interval from the first delivered Down until the last contact ends. Windows `pointerId` identifies an individual pointer, while `frameId` identifies samples from one device update. Browser `pointerId` also identifies an individual pointer. [Windows pointer/frame definitions](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-pointer_info), [browser pointer identity](https://developer.mozilla.org/en-US/docs/Web/API/PointerEvent/pointerId).

| Identity | Scope and lifetime |
|---|---|
| Contact lifetime ID | One Down through Up/Cancel. Normalize reused native IDs with a lifetime token; retained records must not confuse a later contact with an earlier one. |
| Platform frame ID | One hardware sampling update, potentially containing several contacts. Windows exposes it; browser events do not promise equivalent grouping. |
| Delivery sequence | Orders normalized/transported batches. An application-assigned browser batch number is not evidence of a simultaneous hardware sample. |
| Touch session ID | A framework ID covering overlapping delivered contacts in one acquisition scope, from its first Down until its active set becomes empty. |
| Gesture interaction ID | One recognized action owned by a control. It can use a subset of contacts, span multiple sessions for double-tap, or outlive contacts during inertia. |

Use an active-ID set in the common acquisition-root coordinator. Its scope is the input connection/generation and real root window or browser surface. In direct native mode this can live with `GuiGraphicsHost`; in hosted mode coordinate it before dispatching contacts to individual hosted windows. Add a session ID when Down arrives to an empty set, retain it while any delivered contact remains, and close it when the final Up/Cancel removes the final ID.

| Delivered event | Active set afterward | Framework session |
|---|---|---|
| A Down | A | Create S1. |
| B Down | A, B | Keep S1. |
| A Up | B | Keep S1; A ending does not end B. |
| C Down | B, C | Keep S1 because B was still active. |
| B Up | C | Keep S1. |
| C Up | Empty | Close S1. |
| Next Down | New contact | Create S2. |

Define this from ordered delivered input, not guesses about gaps between hardware samples. Preserve boundaries and history ordering when batching; a transport batch can contain the end of one session and the beginning of another, so a single session field on the whole batch is insufficient. Associate the computed session with each routed sample/event. Only use genuine native-frame metadata for algorithms that require samples taken together.

Proposed `NativeTouchSessionInfo` is a data struct carried by common routed touch arguments: session ID, acquisition scope/generation, active count, session phase, and completion/cancellation state. The coordinator owns the full set; controls do not each reconstruct it from their partial event streams. Hosted routing preserves the root session identity and adds its own target/interaction identity. If protocol layers already carry normalized session IDs, preserve them instead of allocating new IDs at every layer; otherwise assign them once after ordered acquisition.

Two simultaneous fingers can share a raw session while targeting unrelated controls and separate gestures. Joining a pinch is a control/ancestor ownership decision, not a consequence of matching session IDs. Neither the primary flag nor capture ownership is a session identifier. Windows gesture-instance/sequence fields are documented as internally used and are not a raw-session contract. [GESTUREINFO](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-gestureinfo).

On native Cancel or unrecoverable ownership loss, remove the affected contact without waiting for Up. Record that the session was interrupted; remaining delivered contacts still need their terminal bookkeeping even if the control has already canceled its gesture. If GacUI cancels only a logical action, keep the native contact in the acquisition set while draining its subsequent input. Removing a composition or hosted child cancels its routed interaction, not the root acquisition contact; tearing down the acquisition surface ends that native stream. Do not let a logically canceled contact start another action halfway through its lifetime.

An empty delivered-contact set does not prove that every physical finger is off the screen. The browser can take over scrolling and send `pointercancel` while fingers remain down; their later Up events need not reach GacUI. Session completion therefore distinguishes normal release from cancellation. Fingers outside the acquisition surface are outside this grouping contract. [Browser cancellation](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointercancel_event).

Keep contact-session and gesture lifetimes separate:

```text
Contact session S1: Down -> contact updates -> last Up -> closed
Scroll gesture G1:  Begin -> drag updates    -> inertia updates -> End
```

Inertia retains G1 with zero active contacts and a reference to its originating session; it does not keep a fake finger in S1. A later Down starts a new session and must stop or explicitly arbitrate with G1. A pinch may instead end when only one finger remains, before its broader session ends. These distinctions should be explicit in event structs and tests.

## 3.6 Gesture structs, shared recognition, and inertia

Define gesture output independently of Windows `GID_*` and Safari `GestureEvent`. Implement shared recognition as an ordinary internal helper using structs and the existing event/timer machinery. Its configure, process, advance-time, and cancel operations do not require new public interfaces or a factory returning one.

`NativeGestureInputFrame` contains contact samples converted into the gesture owner's logical coordinate space, preserving identities and times; `NativeTouchFrame` at the native boundary keeps its native client-coordinate contract. The helper produces `NativeGestureEvent` data for the common composition event pipeline. A future provider recognizer can report the same data through the optional `INativeWindowListener::GestureInput` callback, with an explicit coordinate conversion at the boundary. Configure native recognition through the existing-window configuration struct only if that integration is added. Choose shared or provider recognition for an interaction; do not apply both outputs to the same contacts.

`NativeGestureEvent` should contain:

- Kind: `Tap`, `DoubleTap`, `Hold`, `SecondaryTap`, or `Manipulation`. A swipe can be a control policy based on manipulation displacement/velocity; Windows cross-slide and press-and-tap can remain optional recognizer features.
- Phase: `Completed` for discrete actions; `Begin`, `Update`, `End`, or `Cancel` for continuous actions. A Hold begins when its timer threshold is reached and ends/cancels later.
- Stable gesture interaction ID, associated touch session ID(s), participating contact IDs, active contact count, timestamp, and center. Do not substitute a frame ID for any of these.
- Incremental and total translation, scale, and rotation for manipulation. Document translation in logical units, scale as a ratio with identity 1, rotation in radians with one consistent direction, and velocity per second.
- `inertia` flag and optional velocities. A manipulation may continue with zero active contacts, then End after inertia finishes.

An options struct selects tap/hold/manipulation, allowed pan axes, scale/rotation, thresholds, and inertia. Use a consistent logical distance and time basis. Existing timers supply ticks for hold and inertia; the helper must work when the finger is stationary and no Update arrives. Its cancel operation stops recognition and inertia, emits cancellation once where an action has begun, and does not claim to cancel the physical OS contact.

When a second finger joins, rebase the manipulation without a visible jump. When A leaves and B remains, a control can continue one-finger pan, or end the two-finger gesture and drain B; select that policy explicitly. Never restart mouse fallback from B halfway through this interaction. A contact cancellation cancels the affected gesture group unless a recognizer explicitly supports losing that contact safely.

Continued scrolling after release is inertia, not further raw contact movement. Windows can calculate it through configured pan gestures, Interaction Context, or Direct Manipulation; Interaction Context requires the caller to supply timer ticks. A browser supplies momentum when it owns an actual DOM/page scroller. Custom GacUI scrolling from raw pointers needs a motion implementation; browser `touch-action: none` does not attach native inertia to a GacUI content offset. [Windows pan inertia](https://learn.microsoft.com/en-us/windows/win32/wintouch/windows-touch-gestures-overview), [Interaction Context timer](https://learn.microsoft.com/en-us/windows/win32/api/interactioncontext/nf-interactioncontext-processinertiainteractioncontext), [browser scrolling](https://trac.webkit.org/wiki/Scrolling).

For initial parity, prefer shared tap/hold/pan/pinch recognition and one shared inertia helper. Estimate release velocity, advance deceleration using elapsed time, apply viewport bounds, and stop on cancellation or a new owning contact. Lists and text scroll containers reuse this behavior. Native Interaction Context, Direct Manipulation, and Safari gestures are optional later integrations; reconcile their thresholds, ownership, coordinates, and inertia first. Do not invent touch Updates after Up to carry animation.

## 3.7 Routing and per-contact ownership

Use this flow:

```text
Windows pointer messages / browser Pointer Events
    -> native adapter: contact data, source filtering, terminal normalization
    -> optional remote transport
    -> root-surface contact/session coordinator, then hosted-window mapping
    -> GuiGraphicsHost: hit testing, logical touch capture
    -> target/ancestor touch policy and shared recognizer
    -> gesture behavior OR existing mouse event path
```

Add composition events such as `touchDown`, `touchUpdate`, `touchUp`, `touchCancel`, and `gesture` alongside existing mouse events. Their arguments identify the contact/interaction and provide coordinates relative to the current receiving composition. Reflection and script exposure should follow the existing event conventions.

Keep a per-contact logical capture map separate from `mouseCaptureComposition`. Down chooses the initial composition; capture keeps later samples with its owner even when the finger leaves. Hosted windows need an equivalent per-contact window map. Two contacts can target different controls, so merge them into one gesture only when an eligible common owner accepts them. For example, a document viewport may own its pinch group; unrelated buttons should not accidentally form a pinch.

Native delivery capture and logical composition capture are different. Windows provides implicit HWND ownership. The browser driver can retain DOM capture on its stable root while the common core changes logical ownership. Core routing therefore does not need a synchronous browser round trip for each capture change. Do not invent general Win32 pointer capture functions to mirror DOM methods.

Raw event `handled` stops further routing only. A separate gesture ownership/fallback decision selects who owns the interaction and whether compatibility mouse input is allowed. Evaluate target and ancestor policies before delivering a compatibility MouseDown. Proposed `GuiTouchEventArgs.preventMouseFallback` latches suppression for the interaction; setting it after MouseDown cannot undo prior callbacks. A host-level `CancelTouchInteraction(interactionId, reason)` operation should provide explicit logical cancellation without pretending to cancel OS contact.

## 3.8 Mouse fallback that most controls can ignore

Use one shared fallback engine above the adapters. In `FrameworkTouch` mode, native/browser touch promotion is suppressed or filtered at acquisition; the shared engine alone produces GacUI compatibility mouse events. This permits different controls to choose different behavior without violating native stream rules.

| Proposed policy | Behavior | Typical users |
|---|---|---|
| `ImmediateMouse` | The first eligible touch drives the existing mouse Down/Move/Up path immediately. | Ordinary controls outside gesture regions; explicit slider/splitter drag handles. |
| `DeferredMouse` | Keep a tap candidate without mouse Down. If it finishes as a tap, deliver the existing mouse activation sequence. If a gesture wins or cancellation occurs, discard the candidate. | Items or links inside touch-scrollable content. |
| `TouchOnly` | Raw/gesture behavior owns the interaction; no compatibility mouse events. | Selection handles, custom canvases, active pinch/pan behavior. |

The ordinary default is `ImmediateMouse`. A touch-aware scrolling ancestor can make its content `DeferredMouse` before the first Down is promoted, so each ordinary child does not need touch code. Child controls with intentional mouse dragging can explicitly claim `ImmediateMouse`; once committed, an ancestor must not steal that sequence for scrolling unless the target supports real cancellation.

Rules for the shared fallback engine:

1. At most one touch owns the compatibility mouse stream. Use the initial eligible primary contact; do not synthesize a new mouse Down from a non-primary finger already held down.
2. Deliver all other contacts to the touch system. They are not extra mouse buttons.
3. Once an interaction's contact group chooses a gesture or disables fallback, keep that decision for the rest of that group. Do not re-enable mouse behavior merely because one finger lifts. Sharing a touch session ID does not force unrelated controls into the same gesture or fallback policy.
4. Deferred tap success uses the intended logical target and the existing mouse pipeline, with appropriate enter/move, Down, and Up. Validate both item identity and current hit testing; discard the tap if the replay point now reaches another target after layout/virtualization. Revalidate after callbacks, because Down can remove or change the target before Up. Activation happens on release; retain the original Down position for gesture thresholds, and use a documented replay coordinate that still hits the intended target.
5. Movement beyond the threshold, a claimed multi-finger gesture, Hold activation, Cancel, or target removal discards deferred activation. It must produce neither a click nor a normal release for a Down that was never delivered.
6. If a second finger arrives during an already committed immediate mouse drag, keep the existing owner's drag and withhold conflicting gestures by default. A cancel-aware target may explicitly yield. Extra contacts never become mouse owners mid-group.
7. Keep physical-mouse button state and synthetic-touch state separate internally, with one owner for the whole pressed mouse stream. While a real mouse button is held, new touches cannot acquire compatibility mouse ownership. While touch owns a synthetic press, real mouse movement must not enter that drag; retain the physical position separately. A real mouse press first cancels the synthetic press and suppresses its remaining contact group, then takes ownership. Competing contacts may still reach independent raw-touch behaviors, subject to target ownership. Do not leave combined stuck-button state or let a physical move extend a touch-owned text selection.
8. Add optional origin/contact metadata to common mouse event arguments so specialized consumers can distinguish real mouse and touch fallback. Existing consumers may ignore it; preserve defaults and update reflection/serialization where relevant.
9. Implement double-tap policy once. Preserve existing mouse double-click conventions when falling back. A control requiring exclusive double-tap behavior can delay single-tap activation; do not impose that delay on every ordinary button.

This policy is a framework decision, not a late request to Windows or the browser to promote one particular touch. Browser handlers must already know to suppress default promotion before the remote/core layer arbitrates controls.

## 3.9 Cancellation without an accidental click

Treat these as distinct operations:

| Operation | Meaning |
|---|---|
| Native contact cancellation | The platform stopped delivering an ordinary contact lifetime. Normalize to Cancel. |
| Framework interaction cancellation | GacUI abandons a drag/gesture while the physical contact may continue. Drain remaining input without activation. |
| Prevent mouse fallback | Keep raw/gesture input but do not create compatibility mouse input. |
| Prevent browser default action | Synchronously prevent a DOM action, or predeclare browser pan/zoom policy. |
| Release capture | Change routing ownership; it does not necessarily cancel contact. |

Introduce an internal mouse-cancellation path and a composition `mouseCancel` event for a compatibility press already delivered. It clears the host's synthetic button/capture state and lets common button/drag/text behavior reset transient state. **Do not represent cancellation as an ordinary MouseUp**, because existing controls may activate on Up.

Existing `GuiButton` pressing state lives in `<GacUI repo>/Source/Controls/GuiButtonControls.cpp`; existing text dragging state lives in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`. Clearing host capture alone does not clear those states. Update common behavior in the future implementation so ordinary controls inherit cleanup. A custom control with private pressed/drag state must handle cancellation to support `ImmediateMouse` correctly, since native cancellation is unavoidable. It needs a cancel handler rather than a full gesture implementation. The hook remains source-optional; a control without that support must use deferred activation where appropriate, or its containing native input surface must stay in `MouseOnly` mode.

An action already fired on MouseDown cannot generally be undone. In particular, some button configurations click on Down, and list selection currently occurs on Down. Therefore cancellation is not a substitute for `DeferredMouse` in regions where scrolling/hold/pinch may win. The design can preserve old mouse behavior or defer side effects, but cannot promise to roll back arbitrary user callbacks.

Window/control removal, disabling a target, input ownership loss, explicit cancellation, or a lost remote input connection must end owned interactions and stop inertia. This is input-state cleanup, not a new remote reconnection/recovery system. If a contact is still physically active after logical cancellation, track it only to drain its eventual terminal input. A new interaction requires a new Down.

## 3.10 Browser and remote implementation requirements

The GacJS driver must make default-action decisions locally. For a fully custom GacUI input surface, configure `touch-action: none` on that surface before input, synchronously suppress appropriate pointer compatibility defaults, capture contacts on a stable DOM element, and filter duplicated touch activation. Do not change these choices in response to a worker round trip.

Make duplicate filtering an explicit driver contract: correlate touch-origin activation with the interaction already forwarded to GacUI, retaining the required correlation after Up because activation may arrive afterward. Avoid forwarding the same action through both pointer and mouse/click handlers. A blanket time window, `isTrusted`, or `click.detail` alone cannot reliably distinguish touch duplicates from real mouse, keyboard, or accessibility activation. Keep those independent activation paths functional. Preventing a click's default action does not itself stop its dispatch. [Activation rules](https://www.w3.org/TR/pointerevents3/#the-click-auxclick-and-contextmenu-events).

Framework-generated mouse events do not create trusted browser user activation. Gated browser actions need renderer-side integration with the originating trusted interaction. A worker round trip does not necessarily expire activation immediately, but activation can expire or be consumed; a deferred core callback cannot promise to open a picker, popup, or other gated UI. This is a separate constraint from synchronous `preventDefault`. [Browser user activation](https://developer.mozilla.org/en-US/docs/Web/Security/Defenses/User_activation).

Apply this policy to the GacUI surface, not unrelated surrounding page content. Offer a deliberate alternative mode for browser-owned page scrolling/zooming: in that mode, accept native takeover and cancellation instead of promising uninterrupted GacUI contacts. If custom pinch disables page pinch on the surface, preserve another accessible zoom mechanism.

Extend the remote protocol with configuration requests, applied/rejected acknowledgements, and ordered contact frames, including source, IDs, times, optional measurements, and Cancel. Hosted and remote adapters must forward the configuration and listener callbacks, preserve input generations and configuration revisions, and transform coordinates consistently. Preserve native frame metadata when available; assign the framework session once before hosted-window routing as described in section 3.5. A capability must be end-to-end; a Windows renderer with a mouse-only core is still mouse-only.

The current protocol is generated from `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_IO.txt`. Do not assume an old peer ignores unknown messages or fields. Use a version-compatible negotiation path, or require matched protocol versions for enhanced mode and keep older pairs in their existing mode. The implementation must decide this before sending any new message.

Coalesce only safe Update data while preserving per-contact order, group membership changes, and Down/Up/Cancel boundaries. Flush required updates before terminal events. Never let an update for B overwrite the only pending update for A. Keep history when a consumer requests it; do not promise every hardware sample in the ordinary UI path.

## 3.11 Proposed implementation order and acceptance cases

- [ ] Add defaulted methods to the existing interfaces and define the configuration/event structs, data units/lifetimes, capabilities, and reflection. Verify old provider sources still compile without touch overrides; do not add new public interfaces.
- [ ] Implement deterministic common contact routing, gesture timing, cancellation, and mouse fallback using supplied contact frames and a controllable clock.
- [ ] Add Windows `WM_POINTER` acquisition, native cancellation mapping, coordinate conversion, and duplicate-mouse filtering.
- [ ] Extend the remote protocol and hosted routing, then add GacJS Pointer Events with local CSS/default-action/capture policy.
- [ ] Add shared scroll/list behavior, then text caret/selection handles and text-input integration described in section 4.
- [ ] Add optional history/native recognizers only when a consumer needs them; keep pen and precision-touchpad work independently gated.

Future tests should cover: A Down, B Down, A Up, B Update/Up; cancellation with no later Up; canceled native Up; duplicate capture cleanup; stationary hold; movement beyond bounds; two controls receiving different fingers; second-finger arrival during a mouse drag; no mouse promotion of the remaining finger; list scroll without selection/click; tap fallback exactly once; physical mouse movement/press during synthetic drag; touch during a real mouse drag; target deletion/virtualization/layout changes and reentrant Down callbacks; held selection handle with edge autoscroll; remote batching/ordering; and old mouse-only providers. Verify inertia stops on new contact and Cancel. Browser cases also need delayed touch click after Up, touch followed immediately by real mouse input, keyboard/assistive activation while deduplication records remain, and gated actions through the worker path. Use native/browser integration tests as well as shared-core tests; JavaScript `dispatchEvent` alone does not exercise trusted native defaults.

Identity and grouping cases should include reused native pointer IDs; the same session while any delivered contact remains; a new session after the final terminal event; a transport batch spanning two sessions; separate control gestures in one session; pinch ending before the last contact; double-tap spanning two sessions; and inertia outliving all contacts. Configuration cases should include an unsupported provider, rejected/busy requests, an Applied callback ordered before the first frame using that revision, setup completed before enhanced delivery, and multiple observers that cannot independently change the owner's input mode.

When implementation changes reflected types, update `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp` and `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp` as needed and run the prescribed metadata/code generation and tests. This planning change does not modify those files.

# 4. List and text control behavior

## 4.1 List controls

Touch scrolling should be a reusable scroll-container behavior. Most item templates should keep their existing mouse handlers and receive deferred tap fallback when appropriate.

| User action | Proposed response |
|---|---|
| Tap an item | Select or activate according to the control's normal policy after the finger lifts within the tap threshold. Do not make every list both select and activate. |
| Drag over content | Pan the viewport after movement crosses the threshold. Discard pending item activation; preserve existing selection. |
| Flick and release | Continue scrolling with optional inertia and bounds handling. A new Down stops inertia before interpreting the new action. |
| Press and hold | Show the item's context menu where supported. Suppress the ordinary tap/click for that interaction. |
| Select several items | Offer selection mode or checkboxes usable without Ctrl/Shift. Multiple fingers do not automatically mean multiple selection. |
| Drag a nested slider or explicit drag handle | Let that target own the interaction. Do not simultaneously scroll its ancestor. |
| Reach a nested scrolling boundary | Apply an explicit axis-locking and parent-handoff policy. Do not silently create a new mouse click during handoff. |
| Pinch | Usually leave item scale unchanged unless the list represents zoomable content. Do not reinterpret an ordinary two-finger sequence as selection. |

`GuiSelectableListControl` currently selects on `ItemLeftButtonDown` in `<GacUI repo>/Source/Controls/ListControlPackage/GuiListControls.cpp`. Immediate touch promotion would therefore select an item before the framework knows whether the user is scrolling. Use `DeferredMouse` for ordinary item interaction inside a touch-scroll region. Scroll position can build on `GuiScrollView::SetViewPosition` in `<GacUI repo>/Source/Controls/GuiContainerControls.cpp`.

Keep the logical item identity during a pending tap. Virtualized lists may destroy/reuse visual templates while scrolling; a recycled composition must not activate a different item. Selection, focus, and pressed visuals should be separate state, so canceling a pending tap does not unexpectedly erase an existing selection.

Typical framework sequences:

| Scenario | Resulting behavior |
|---|---|
| Tap an ordinary list item | Touch Down -> pending candidate -> Touch Up -> one mouse Down/Up activation sequence on the still-valid item. |
| Scroll starting on an item | Touch Down -> pending candidate -> threshold crossed -> manipulation Begin/Update -> Touch Up -> optional inertia -> End. No item mouse Down/Up. |
| Long-press an item | Touch Down -> hold timer -> Hold Begin -> context menu; release ends the hold without a tap. |
| Second finger begins a supported pinch | Pending tap -> two-contact gesture wins -> no item mouse fallback; ending A does not erase B's contact record. |
| Cancel during scrolling | Cancel -> stop gesture/inertia and pressed feedback; retain the viewport's last valid position unless the control defines a snap-back effect. |

## 4.2 Text controls

Apply policies according to `ViewOnly`, `Selectable`, and `Editable` mode. Touch keyboard support, text entry, selection, scrolling, and link activation are related but separate behaviors.

| User action | Proposed response |
|---|---|
| Tap editable text | Place the caret on successful tap and request the platform text-input experience. |
| Tap a link | Activate only when the tap wins; scrolling must not activate the link. |
| Drag ordinary document content | Scroll the document. Do not automatically start mouse-style range selection. |
| Double-tap or deliberate hold | Select a word using a configurable platform-appropriate policy. Hold may also expose editing commands. |
| Drag a caret handle | Move the insertion point precisely; the handle owns that contact. |
| Drag either selection handle | Move that endpoint while preserving the other endpoint. |
| Hold a handle near a viewport edge | Autoscroll on a timer, recomputing hit testing as content moves, even without further pointer movement. |
| Open the editing menu | Offer Copy/Cut/Paste/Select All as permitted by mode, selection, clipboard policy, and password behavior. |
| Add another finger while dragging a handle | Keep handle ownership and ignore conflicting gestures, or explicitly cancel it before a supported pinch; never run both on the same contact. |
| Cancel selection dragging | Stop dragging/autoscroll and keep the last valid selection by default. Do not fire link activation or a pending click. |

Define endpoint crossing: either switch the active handle's role or preserve anchor/focus direction consistently. Keep handles visible and hit-testable around the selection without covering the exact character being positioned. Enlarge their hit targets independently of their drawn size. Allow skins to choose visuals; a magnifier is an optional aid, not required for the initial implementation.

Existing document behavior starts caret placement/selection dragging on LeftDown and extends selection during mouse movement in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`. Its `Move` operation and `GuiDocumentViewer::EnsureRectVisible` in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp` provide useful selection/visibility primitives. They need touch policy above the existing mouse path.

Text hit testing should use `GetCaretFromPoint`, `GetCaretBounds`, and `IsValidCaret` from `<GacUI repo>/Source/GraphicsElement/GuiGraphicsDocumentInterfaces.h`, rather than assuming each code unit is a legal caret stop. Verify emoji, combining characters, ligatures, bidirectional text, wrapped lines, and embedded objects. Handles must update after layout, scrolling, DPI/zoom changes, and edits.

Typical framework sequences:

| Scenario | Resulting behavior |
|---|---|
| Tap to place caret | Down -> pending tap -> Up -> caret placement/focus -> text-input activation where available. No range drag. |
| Scroll over selectable text | Down -> pan wins -> viewport movement -> Up/inertia. Existing selection remains. |
| Select word by hold | Down -> hold threshold -> word selection -> show handles/menu according to policy -> Up. No delayed ordinary click. |
| Drag selection endpoint | Handle Down -> logical capture -> updates change one endpoint -> optional edge autoscroll -> Up -> release capture. |
| Selection drag canceled | Handle Down -> updates -> Cancel -> stop timer/capture/pressed state; retain last valid selection. |
| Keyboard obscures caret | Platform geometry/viewport change -> resize/scroll visible area -> ensure caret and editing UI remain reachable. |

## 4.3 Does Windows provide selection handles?

**Windows does have text-selection grippers.** Built-in framework text controls provide their own touch-selection behavior. Native Rich Edit exposes `SES_EX_MULTITOUCH` through `EM_SETEDITSTYLEEX`/`EM_GETEDITSTYLEEX` for touch selection, caret placement, and context menus. Availability and behavior belong to that control/version; they are not a feature added automatically to every custom HWND. [Windows text-selection guidance](https://learn.microsoft.com/en-us/windows/apps/develop/input/guidelines-for-textselection), [Rich Edit touch style](https://learn.microsoft.com/en-us/windows/win32/controls/em-geteditstyleex).

GacUI's document controls render `GuiDocumentElement`, rather than embedding a native TextBox/RichEdit control. The design should therefore assume **GacUI draws and manages its own caret/selection handles**. Microsoft also documents custom selection UI for applications that implement their own experience. [Custom selection guidance](https://learn.microsoft.com/en-us/windows/apps/develop/input/guidelines-for-textselection).

TSF/CoreText connects an editor to text services; UI Automation exposes text and selection to assistive clients. The architectural inference is that implementing those interfaces alone does not turn a custom renderer into a native text control or supply its gripper rendering. Touch keyboard, IME composition, candidate-window placement, text bounds, and viewport occlusion still need platform integration. [Custom text input](https://learn.microsoft.com/en-us/windows/apps/develop/input/custom-text-input), [UI Automation text support](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-implementingtextandtextrange).

## 4.4 Does the browser provide selection handles?

Real `<input>`, `<textarea>`, `contenteditable`, and selectable DOM text can participate in browser-managed selection. Whether handles appear, their appearance, and the gestures used depend on browser, OS, and input device. There is no universal iOS-style handle contract for every desktop browser. DOM `Selection`/`Range` controls selection data, not a portable API for styling or commanding native selection handles. [Selection API](https://www.w3.org/TR/selection-api/), [contenteditable](https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/contenteditable).

GacJS currently renders paragraph text with spans and paints its own caret in `<GacJS repo>/Gaclib/gaclib/renderer/src/domRenderer/elementStyles_DocumentParagraph.ts`. Demo roots disable native selection in `<GacJS repo>/Gaclib/website/entry/assets/global.css`. The inspected renderer/entry sources have no textarea/contenteditable composition bridge; printable characters currently come from `keydown` in `<GacJS repo>/Gaclib/gaclib/renderer/src/GacUIRendererImpl.ts`. Native browser handles therefore do not automatically represent the selection stored in GacUI.

Simply removing `user-select: none` would create a second, browser-owned selection unless DOM and GacUI positions were synchronized. A hidden textarea can help text entry, but does not by itself place visible handles around custom-rendered text. A true native editable overlay could use browser selection, but requires explicit synchronization of content, layout, selection, focus, and editing commands.

Do not assume that an asynchronous synthesized GacUI click grants browser user activation or opens a touch keyboard. Establish a local text-input/focus bridge using real browser events, with the core caret/selection synchronized afterward. Platform focus and keyboard restrictions must be verified in supported browsers.

`EditContext` is an optional alternative for custom editors. It exchanges text, selection, composition, and bounds with the browser; the author still owns rendering and interaction. It needs feature detection and a fallback. `beforeinput`/`input` and composition support must be part of the text-input plan; raw `keydown` cannot represent the complete IME/mobile editing experience. [EditContext specification](https://www.w3.org/TR/edit-context/), [Chromium custom editor guidance](https://developer.chrome.com/blog/introducing-editcontext-api).

The recommended initial design is shared GacUI selection semantics and skinnable handles, with platform-specific text-input bridges. Verify Windows and browser behavior on real touch hardware, including IME and the touch keyboard; mouse-only testing cannot establish those behaviors.
