# 0. Proposed native and GacUI interface changes

<!-- this section should include header file changing required by the job, declare any new types and methods, no explanation in this section, section 3 will reference the code here, do not delete this comment -->

## 0.1 `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h`

### New types in vl::presentation

```cpp
enum class PointSequenceEndReason
{
	Succeeded,
	Canceled_Unknown,
	Canceled_PlatformCanceled,
	Canceled_CaptureLost,
	Canceled_WindowDestroyed,
};

template<typename TPoint, typename TRect>
struct WindowPointInfo_
{
	vuint64_t				sequenceId = 0;
	vint					pointId = 0;
	vuint64_t				timestamp = 0;
	vint					activePointCount = 0;
	TPoint					position;
	bool					primary = false;
	bool					ctrl = false;
	bool					shift = false;
	bool					alt = false;
	bool					osSuper = false;
	Nullable<TRect>			contactBounds;
	Nullable<double>		pressure;
	Nullable<double>		orientation;
	Nullable<vuint64_t>		platformFrameId;
};

using NativeWindowPointInfo = WindowPointInfo_<NativePoint, NativeRect>;
using WindowPointInfo = WindowPointInfo_<Point, Rect>;

struct PointSequenceEndInfo
{
	vuint64_t				sequenceId = 0;
	vuint64_t				timestamp = 0;
	bool					accepted = false;
	PointSequenceEndReason	reason = PointSequenceEndReason::Succeeded;
};
```

### INativeWindowListener: public additions

```cpp
virtual void				PointSequenceStarted(const NativeWindowPointInfo& firstPoint, bool& accept);
virtual void				PointDown(const NativeWindowPointInfo& info);
virtual void				PointUpdate(const NativeWindowPointInfo& info);
virtual void				PointUp(const NativeWindowPointInfo& info);
virtual void				PointEntered(const NativeWindowPointInfo& info);
virtual void				PointLeaved(const NativeWindowPointInfo& info);
virtual void				PointSequenceEnded(const PointSequenceEndInfo& info);
```

## 0.2 `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h`

### New types in vl::presentation::compositions

```cpp
enum class GuiPointEventsType
{
	Started,
	Down,
	Update,
	Up,
	Enter,
	Leave,
};

struct GuiPointEventArgs
	: public GuiEventArgs
	, public WindowPointInfo
	, public Description<GuiPointEventArgs>
{
	GuiPointEventsType eventType;
	const collections::List<Nullable<Point>>& points;

	GuiPointEventArgs(GuiPointEventsType eventType, const collections::List<Nullable<Point>>& points);
	GuiPointEventArgs(GuiGraphicsComposition* composition, GuiPointEventsType eventType, const collections::List<Nullable<Point>>& points);

	const collections::List<Nullable<Point>>& GetPoints() const;
};

struct GuiPointSequenceStartedEventArgs
	: public GuiPointEventArgs
	, public Description<GuiPointSequenceStartedEventArgs>
{
	bool							accept = false;

	GuiPointSequenceStartedEventArgs(const collections::List<Nullable<Point>>& points);
	GuiPointSequenceStartedEventArgs(GuiGraphicsComposition* composition, const collections::List<Nullable<Point>>& points);
};

struct GuiPointSequenceEndedEventArgs
	: public GuiEventArgs
	, public PointSequenceEndInfo
	, public Description<GuiPointSequenceEndedEventArgs>
{
	const collections::List<Nullable<Point>>& points;

	GuiPointSequenceEndedEventArgs(const collections::List<Nullable<Point>>& points);
	GuiPointSequenceEndedEventArgs(GuiGraphicsComposition* composition, const collections::List<Nullable<Point>>& points);

	const collections::List<Nullable<Point>>& GetPoints() const;
};

typedef GuiGraphicsEvent<GuiPointEventArgs>					GuiPointEvent;
typedef GuiGraphicsEvent<GuiPointSequenceStartedEventArgs>	GuiPointSequenceStartedEvent;
typedef GuiGraphicsEvent<GuiPointSequenceEndedEventArgs>		GuiPointSequenceEndedEvent;
```

### GuiGraphicsEventReceiver: public additions

```cpp
GuiPointSequenceStartedEvent		pointSequenceStarted;
GuiPointEvent					pointAction;
GuiPointSequenceEndedEvent		pointSequenceEnded;
```

## 0.3 `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.h`

### GuiGraphicsHost: protected additions

```cpp
vuint64_t													pointSequenceId = 0;
bool														pointSequenceAccepted = false;
GuiGraphicsComposition*										pointCaptureComposition = nullptr;
Point													pointsOrigin;
NativeSize												pointsScale;
collections::List<Nullable<Point>>			points;

void							PointCapture(const NativeWindowPointInfo& firstPoint);
void							PointUncapture();
void							ResetPointCapture();
void							RebasePoints(GuiGraphicsComposition* composition);
void							RaisePointEvent(GuiPointEventArgs& arguments);
void							RaisePointSequenceStartedEvent(GuiPointSequenceStartedEventArgs& arguments);
void							RaisePointSequenceEndedEvent(GuiPointSequenceEndedEventArgs& arguments);
void							OnPointInput(
									const NativeWindowPointInfo& info,
									GuiPointEventsType eventType);
```

### GuiGraphicsHost: private additions

```cpp
void							PointSequenceStarted(const NativeWindowPointInfo& firstPoint, bool& accept) override;
void							PointDown(const NativeWindowPointInfo& info) override;
void							PointUpdate(const NativeWindowPointInfo& info) override;
void							PointUp(const NativeWindowPointInfo& info) override;
void							PointEntered(const NativeWindowPointInfo& info) override;
void							PointLeaved(const NativeWindowPointInfo& info) override;
void							PointSequenceEnded(const PointSequenceEndInfo& info) override;
```

## 0.4 `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedController.h`

### GuiHostedController: protected additions

```cpp
vuint64_t						pointSequenceId = 0;
bool							pointSequenceAccepted = false;
GuiHostedWindow*					pointSequenceWindow = nullptr;

NativeWindowPointInfo			ConvertPointToHostedWindow(const NativeWindowPointInfo& info);
void							ForwardPointInput(
									const NativeWindowPointInfo& info,
									void (INativeWindowListener::* callback)(const NativeWindowPointInfo&));
void							ResetPointSequence();

void							PointSequenceStarted(const NativeWindowPointInfo& firstPoint, bool& accept) override;
void							PointDown(const NativeWindowPointInfo& info) override;
void							PointUpdate(const NativeWindowPointInfo& info) override;
void							PointUp(const NativeWindowPointInfo& info) override;
void							PointEntered(const NativeWindowPointInfo& info) override;
void							PointLeaved(const NativeWindowPointInfo& info) override;
void							PointSequenceEnded(const PointSequenceEndInfo& info) override;
```

## 0.5 `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h`

### GUIREFLECTIONBASIC_TYPELIST: additional entries

```cpp
			F(presentation::PointSequenceEndReason)\
			F(presentation::NativeWindowPointInfo)\
			F(presentation::WindowPointInfo)\
			F(presentation::PointSequenceEndInfo)\
```

### GUIREFLECTIONEVENT_TYPELIST: additional entries

```cpp
			F(presentation::compositions::GuiPointEventsType)\
			F(presentation::compositions::GuiPointEventArgs)\
			F(presentation::compositions::GuiPointSequenceStartedEventArgs)\
			F(presentation::compositions::GuiPointSequenceEndedEventArgs)\
```

# 1. Windows touch and gesture capabilities

This is a research report and proposed plan, not an implemented feature. Step 1 targets Windows point input; Step 2 adds the GacJS browser renderer. Gesture facilities are retained here as research for later steps and are not used for recognition in the first version. Other providers retain their existing mouse behavior. Platform references were checked on 2026-10-09; the sequence contract in sections 0 and 3 was revised on 2026-10-14.

The contact-input paths are Windows `WM_POINTER*` on Windows 8 or newer and browser Pointer Events. The catalogs below cover their application-facing touch, gesture, feedback, and testing facilities. Pen and touchpad facilities are identified separately. Device-driver development is outside this plan.

## 1.1 Choosing an input path

| Windows API | What it provides | Suggested use |
|---|---|---|
| Pointer input, `WM_POINTER*` | Separate contacts, contact data, history, and cancellation information. Windows 8 onward. | Primary Windows input path. |
| Interaction Context | Recognizes gestures from pointer data supplied by the application. | Optional Windows recognition implementation. |
| `WM_GESTURE` | Automatic gestures produced by default pointer processing. | Platform behavior reference; raw contacts still use the pointer path. |
| Direct Manipulation | A larger system for moving/scaling content, inertia, and composition. | Optional integration for native viewports and touchpads. |

`WM_POINTER` is the only Windows contact-input path in this plan. Passing unhandled pointer input to `DefWindowProc` can produce `WM_GESTURE` or compatibility mouse input. Handling the pointer stream consistently is required for the per-sequence point/fallback decision in section 3. Feeding Interaction Context or providing a shared gesture recognizer is later work. Recognition is separate from contact acquisition; enabling a recognizer does not promise independent, uninterrupted raw and gesture streams. [Pointer default processing](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown), [Interaction Context](https://learn.microsoft.com/en-us/windows/win32/input_intcontext/interaction-context-portal).

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

Passing unhandled pointer input to `DefWindowProc` allows default gestures or mouse promotion. Microsoft warns that consuming some input and default-processing the rest can produce undefined behavior. The first-version design instead makes one point/fallback decision for an entire overlapping-contact sequence, retaining OS mouse fallback for a rejected sequence. Its native processing and gesture-suppression requirements must be verified as described in section 3.8. `EnableMouseInPointer` controls the opposite direction and is a process-level choice. [Default pointer processing](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown), [EnableMouseInPointer](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemouseinpointer).

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

Browsers internally recognize pan, pinch zoom, and other actions, but there is **no portable general set of pan/pinch/rotate/swipe events for application controls**. Custom recognition can be built from contacts when GacUI owns the interaction; that is later work, beyond the first-version point API. For example, distance between two pointers provides pinch scale. [Pointer-based pinch example](https://developer.mozilla.org/en-US/docs/Web/API/Pointer_events/Pinch_zoom_gestures).

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

Safari/WebKit additionally provides proprietary `gesturestart`, `gesturechange`, and `gestureend`. `GestureEvent.scale` is relative to the initial distance and `rotation` is relative to the initial orientation. This is useful research for optional later integration, not a cross-browser gesture API or a Step 1 dependency. Do not process both these gestures and a shared recognizer for the same interaction. [Apple GestureEvent](https://developer.apple.com/documentation/webkitjs/gestureevent), [GestureEvent reference](https://developer.mozilla.org/en-US/docs/Web/API/GestureEvent).

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

# 3. First-version point input design

## 3.1 Scope and header inventory

This is a design proposal, not an implemented feature. It follows Step 1 in `<GacUI repo>/ToDo/1.5.1.0.md`. Section 0 contains the complete proposed header additions for this point-input contract. Its blocks add declarations to the named namespaces, classes, and existing reflection macros; they do not replace existing classes or remove their members. Definitions and routing behavior belong in the corresponding implementation files.

Use **point** in new API names. The first implementation acquires touchscreen contacts through Windows `WM_POINTER*`, not legacy `WM_TOUCH`. The browser port uses Pointer Events in Step 2. Physical mouse input retains its existing API; pen hover, pen-specific editing, precision-touchpad integration, optional history/prediction, and specialized OS pointer messages remain outside Step 1.

The first version provides point sequence negotiation, point data, sequence-wide capture and routing, cancellation, and platform fallback. It does not add gesture callbacks, gesture event arguments, a gesture recognizer, inertia, a touch input service, or a new public interface. **There is no `INativeWindow::ConfigureTouchInput` or startup `MouseOnly`/`FrameworkTouch` mode.** Acceptance is queried for the current occurrence, when its first point goes down.

Sections 1 and 2 retain platform research, including gesture facilities for later work. Section 4 is retained unchanged as future control-behavior research; its deferred activation, momentum, selection handles, and gesture terminology do not add requirements to Step 1. Step 3 addresses controls, and Step 4 revisits gesture APIs.

| Header declarations | Implementation responsibility |
|---|---|
| Section 0.1: point data and `INativeWindowListener` callbacks | Native acquisition and a synchronous sequence-start query. No composition references, composition hit testing, or composition capture. |
| Section 0.2: event arguments and receiver members | Expose point events to composition handlers and Workflow using existing event conventions. |
| Section 0.3: `GuiGraphicsHost` additions | Hit-test the first point, ask the target/ancestor chain, capture that composition, and route the complete sequence. |
| Section 0.4: `GuiHostedController` additions | Keep the sequence on the first selected hosted window and forward its listener events. |
| Section 0.5: reflection type-list additions | Make the new data and arguments available to the existing reflection pipeline. |

Windows acquisition state belongs to the private `WindowsForm` class in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp`. Its sequence counter, active contacts, and accepted/rejected state stay there; they do not require a new public Windows header API. Existing `INativeWindow`, controller services, and built-in control headers need no additions for this contract.

## 3.2 Sequence identity and lifetime

A point sequence covers a continuous chain of overlapping contacts in one native window. The first Down while no contacts are active allocates a nonzero `sequenceId`. Until cancellation, every subsequent point belongs to that sequence while at least one contact remains. The last normal Up ends it with `PointSequenceEndReason::Succeeded`. Any cancellation ends the entire public sequence immediately with a `Canceled_*` reason, as detailed in section 3.7. A contact can stop moving without leaving the active set.

| Incoming contact transition | Host `points` for an accepted sequence | Sequence behavior |
|---|---|---|
| A Down | [A] | Allocate S1; A has `pointId == 0`. Query Started before reporting A Down. |
| B Down | [A, B] | B has `pointId == 1`. Keep S1, its acceptance, and its routing target. |
| A Up | [empty, B] | Keep S1 even though its first point has ended. |
| C Down | [empty, B, C] | C has `pointId == 2`. Keep S1 because B bridges the two contacts; do not reuse slot 0. |
| B Up | [empty, empty, C] | Keep S1. |
| C Up | [empty, empty, empty] | Report the final Up if accepted, then End S1 with `Succeeded`. |
| D Down | [D] | Allocate S2; D has `pointId == 0`. Query acceptance again. |

The sequence ID is unique for the lifetime of its native window, including rejected sequences. It is not a hardware frame ID or a finger ID. `pointId` is a zero-based `vint` index assigned in Down order within that sequence: 0 for the first contact, 1 for the next, and so on. It remains unchanged for that contact, and its slot is never reused or compacted during the sequence. Map OS pointer IDs to these indices; reused OS IDs must not alias an earlier contact. If the platform reports several new contacts together, assign consecutive indices in the order their Down callbacks are delivered. The pair of sequence ID and point ID identifies a contact. Remote connections will additionally scope those identities by window and connection generation in Step 2.

The native provider owns sequence allocation and active-contact bookkeeping, even when a sequence is rejected and no point payload is delivered. `GuiGraphicsHost` consumes the supplied sequence ID instead of reconstructing a different sequence from the events it accepts. Hosted forwarding preserves the root native ID and cannot restart the sequence for another logical window.

## 3.3 Native sequence query and listener delivery

On the first native Down, read its data and establish the sequence record. Then call `INativeWindowListener::PointSequenceStarted` before dispatching `PointDown` or allowing fallback processing of that first Down. This is an ordinary input-time callback, not a callback triggered by listener installation or window creation.

The provider asks its installed listeners through the existing listener mechanism. Each callback starts with a false `accept` value; combine their answers with logical OR. Default implementations do nothing and therefore decline. A listener can accept according to its current state, including a decision obtained synchronously from its own downstream handling. The native provider only sees that answer; it never inspects GacUI compositions.

If any listener accepts, latch acceptance for the entire sequence and broadcast its point notifications through the ordinary listener list. **The native layer does not select a composition, remember a composition, or choose a listener according to composition hit testing.** `GuiGraphicsHost` or another listener owns its own downstream routing. All installed listeners can observe the accepted native stream. A graphics host whose own composition chain declined still suppresses its composition point-data events even if an independent native listener accepted.

If all listeners decline, suppress all subsequent **point-data callbacks** for that sequence and let the platform perform fallback as detailed in section 3.8. Continue private acquisition bookkeeping until the sequence ends. Do not query again for a later point, a new position, a changed control state, or a newly installed listener.

`PointSequenceStarted` and `PointSequenceEnded` are lifecycle boundaries, distinct from point-data callbacks. They remain paired for rejected sequences too: rejection suppresses Down/Update/Up/Enter/Leave delivery, but End closes the occurrence with `accepted == false`. This also gives the composition sequence query a matching End. End is not a successful point release and must not activate anything.

Listener installation/removal follows existing broadcast and lifetime rules; do not introduce a per-sequence native subscription or listener-selection system. Installation during a sequence does not replay Started or renegotiate acceptance. A stateful listener that did not observe Started ignores that sequence until the next Started. No callback is made to an uninstalled/destroyed listener.

`PointSequenceEndInfo.accepted` reports the decision at the reporting layer. At the native window it is the aggregate listener decision. The hosted adapter and graphics host retain their own local answers and use those when reporting End downstream. `PointSequenceEndInfo.reason` reports how the whole sequence ended: `Succeeded` for normal completion, or a `Canceled_*` member for cancellation. Acceptance and completion reason are independent; rejecting point input is not cancellation. There is no separate point-cancel callback or event.

All new listener methods have non-pure, no-op definitions in `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.cpp`. Existing listener sources need no new overrides. Adding virtual members still requires rebuilding C++ consumers; this is not a binary-compatibility promise.

## 3.4 Composition query and sequence-wide routing

`GuiGraphicsHost::PointSequenceStarted` records the sequence ID and calls `PointCapture` with the first point. The capture helper converts its position to logical client coordinates, hit-tests once, and stores the result in `pointCaptureComposition` before routing the query. A miss declines without creating a composition route.

Raise `pointSequenceStarted` at the captured composition and propagate through its parents using the existing mouse-event conventions. `compositionSource` remains the original target; `eventSource` is the first composition on that path with an event receiver. Each handler receives positions/contact bounds and all nonempty entries in `points` relative to its receiving composition.

A target or ancestor accepts by setting `GuiPointSequenceStartedEventArgs.accept` to true. That answer is sticky during the query: handlers may accept but must not revoke an earlier acceptance. `handled` retains its existing meaning of stopping further parent propagation. Setting `handled` alone does not accept point input. With no acceptance on the visited path, the answer remains false. Return the result synchronously through the native Started callback.

For an accepted sequence, `pointAction` starts at the captured composition and follows the same parent propagation rules. Its `eventType` is `GuiPointEventsType::Down`, `Update`, `Up`, `Enter`, or `Leave`. The separate `pointSequenceStarted` query uses `Started`; it is not also delivered through `pointAction`. Recalculate position/contact bounds and every nonempty `points` entry for each receiving composition. A later finger landing over another control still goes to the original target. Moving outside its bounds or releasing the first finger does not retarget the sequence.

The host owns the mutable `points` list declared in section 0.3. `GuiPointEventArgs` and `GuiPointSequenceEndedEventArgs` borrow that same list through a const reference; Started inherits it. Native listener arguments do not contain a list. The list is the only retained per-contact state in the graphics host. `pointsOrigin` records its current coordinate origin in logical units; `pointsScale` records the native size of a 96-unit reference at that origin's DPI. Rebase occupied slots in place by the old/new scale ratio and coordinate origins, including the current native-window client origin, so parent propagation, window movement, and DPI changes preserve stationary contacts. Empty slots remain empty.

For a locally accepted sequence, maintain the list before invoking handlers:

- Seed slot 0 from the first point before Started. The following first Down uses that slot instead of appending it again.
- Each later Down appends its position at the next `pointId`. Update/Enter/Leave refresh the existing active slot.
- Up empties its slot before raising `pointAction` with `eventType == GuiPointEventsType::Up`. The event's own `position` still reports the final Up position.
- Never erase, compact, or reuse a slot during the sequence. `points.Count()` is the total number of contacts that have gone down; the number of nonempty entries is the number still down.
- Successful End sees all slots empty after the final Up. Canceled End preserves the last known positions of all contacts that have not delivered Up, including the contact whose native cancellation ended the sequence. Do not synthesize Ups or empty those slots before End handlers run.

For a rejected sequence, clear the provisional first-point data after the query and keep only its target/lifecycle bookkeeping for `pointSequenceEnded`; there are no composition point-data events. Its End has `accepted == false` and an empty list, meaning point tracking was not accepted, not that all physical contacts necessarily lifted. The full-list guarantee applies only to locally accepted sequences, even if another native listener accepted.

For either outcome, route End to the captured composition and then release point capture and clear the list. End exposes the same list object as preceding events; a successful accepted sequence retains its all-empty slots through every End handler. The `accepted` field on composition End is the chain's own query result.

The host has **one captured composition per sequence**, not one per finger. Point state is separate from mouse capture, mouse button state, keyboard focus, and mouse hover. Accepting a point sequence does not capture the physical mouse.

The three receiver events must be associated with their owning composition in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.cpp`, following existing initialization. Argument constructors must receive the host-owned list and initialize their reference. `GuiPointEventArgs` also requires its event type; Started passes `GuiPointEventsType::Started` to its base. There are no parameterless or composition-only constructors for these borrowed-list arguments. `GetPoints` returns the same const reference for read-only reflection access. Reflect `GuiPointEventsType`, `eventType`, and `pointAction`, and use one action handler in authored resources instead of five separate subscriptions.

## 3.5 Capture/release functions, reuse, and handler lifetime

Section 0.3 declares the new pair `GuiGraphicsHost::PointCapture` and `GuiGraphicsHost::PointUncapture`, at the same helper level as `MouseCapture` and `MouseUncapture`. Their purpose is the same: remember the initial composition for subsequent routed input and release it when the interaction finishes.

`PointCapture` runs once at Started. It must not replace an existing sequence's target on a secondary Down. `PointUncapture` runs after `pointSequenceEnded` propagation, including for a rejected sequence, and clears the stored sequence/capture state, coordinate origin/scale, and list only after End handlers have observed them. The first point's Up is insufficient while another point remains. `ResetPointCapture` performs the corresponding state reset from existing host/window disconnection paths.

These are GacUI composition capture/release functions. Do not implement them by calling `INativeWindow::RequireCapture` or `ReleaseCapture`, which control mouse capture. Windows touch contacts already have implicit native ownership; other providers need their own native delivery mechanism. No general receiving-side Win32 pointer capture/release function is being proposed.

Before implementing point propagation and capture, inspect `MouseCapture`, `MouseUncapture`, `ResetMouseCapture`, `OnMouseInput`, and `RaiseMouseEvent` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp`. Reuse or factor out the common first-hit selection, source/eventSource assignment, parent traversal, coordinate rebasing, and `handled` stopping logic where it fits. The section 0 entry points do not require duplicating that implementation. Typed helpers local to the implementation file can serve both mouse and point event arguments.

Preserve the differences in state and lifetime: mouse release depends on held mouse buttons, while point release depends on the entire sequence ending. Existing `RaiseMouseEvent` is typed to mouse arguments/member pointers; adapt or extract its common routing instead of pretending point data is a mouse event. Point End can reuse parent traversal without a changed-point payload, while rebasing its shared list for each receiver.

**Deleting the receiving composition inside its mouse handler is a known crash, and the same behavior is expected for point handlers.** This change does not fix that issue, retain compositions to hide it, snapshot the tree to survive it, or add reentrant-deletion recovery. Point handlers follow the existing rule of deferring destruction/reparenting until after input dispatch. Tests must not demand safe synchronous deletion inside a point handler.

Outside active event dispatch, extend the existing disconnection/reset hooks to clear point capture just as they clear mouse capture. If a captured target is detached/destroyed through those normal paths before native End, clear its reference and stop composition delivery for the remaining sequence; do not hit-test a replacement or change to mouse fallback mid-sequence. Native input bookkeeping still ends normally, but no events can be delivered to a removed target. Do not manufacture a second sequence or a synthetic successful Up as cleanup.

## 3.6 Hosted windows and coordinate conversion

The normal native path is: Windows provider -> native listeners -> `GuiGraphicsHost` -> captured composition -> its parents. The provider never depends on the composition layer.

Hosted mode inserts `GuiHostedController` between the real native window and logical windows. At Started, select one eligible logical window at the first point's position, store it in `pointSequenceWindow`, translate that point into its client coordinates, and query its installed listeners. Its graphics host then performs composition hit testing and returns its own answer. Propagate the result to the original native query before returning.

Preserve the sequence ID and selected logical window for all subsequent points and End, including later fingers outside that window. Coordinate conversion uses that window's current rendering/client offset; it must not select another window based on a later point. Existing hosted mouse helpers that repeatedly hit-test cannot be reused unchanged, but their coordinate and listener-forwarding logic should be reviewed for reuse.

Native position/contact bounds are relative to the receiving native window's client area. Hosted adapters translate into the selected logical window's native client space; `GuiGraphicsHost` uses existing `INativeWindow::Convert` conversions to reach GUI coordinates. Composition propagation rebases the changed-point payload and all nonempty list entries to each receiver, including the list on End. The list already contains logical coordinates: rebase it using its tracked origin and scale instead of converting the previous receiver's coordinates as though they were native samples. Rebase to the current window client origin and scale before applying each new sample, preserving stationary contacts when that window moves or changes DPI. Do not mix Windows screen pixels, native client units, and GUI units.

Keep point routing independent of the existing mouse `capturingWindow`, `hoveringWindow`, and window-manager drag state. On rejection, the existing mouse fallback path performs its own normal routing; the remembered point-query window does not override mouse targeting. `ResetPointSequence` clears a removed hosted target without retargeting an in-progress sequence.

## 3.7 Point data, event coverage, and cancellation

Section 0.1 defines all new native data types; section 0.2 exposes their logical equivalents in composition arguments.

| Field or event | Contract |
|---|---|
| `sequenceId`, `pointId` | Nonzero sequence identity and zero-based Down-order index, as defined in section 3.2. Point ID 0 is valid. |
| `timestamp` | Monotonic microseconds in the provider's clock domain; use source sample time where available. It is not wall-clock time. |
| `position`, `contactBounds` | Native client coordinates in `NativeWindowPointInfo`; logical receiver-relative coordinates in `WindowPointInfo`. Use existing geometry types and DPI conversion. |
| `activePointCount` | Active count after the reported contact transition. Started includes its first point; final normal Up has count zero. Boundary events do not change the count. For accepted composition events it equals the number of nonempty list entries, not the list length. |
| `points` | Composition-only borrowed reference to the host's shared list. `pointId` indexes its slot; empty means that contact has delivered Up. Started and End expose the same list, with rejection handled as in section 3.4. |
| `primary` | Platform hint. Never filter out other fingers or promote an already-down finger when the primary ends. |
| `ctrl`, `shift`, `alt`, `osSuper` | Modifier state at the sample. These do not encode contact count or acceptance. |
| `pressure` | Optional normalized 0..1 measurement. Empty means unavailable/unknown, not zero. |
| `orientation` | Optional contact orientation in radians, clockwise from the upward axis; convert the platform representation. |
| `platformFrameId` | Optional genuine platform frame identity, not sequence identity or transport-batch numbering. |
| `PointDown` / `pointAction` (`Down`) | A contact joins the accepted sequence. |
| `PointUpdate` / `pointAction` (`Update`) | Contact data changes or is reported again, including stationary pressure/size updates. |
| `PointUp` / `pointAction` (`Up`) | A contact ends normally. This alone does not recognize a tap or activate a control. |
| `PointSequenceEnded` / `pointSequenceEnded` | The whole sequence ends once. `reason` is `Succeeded` after the final normal Up, or a `Canceled_*` reason without synthetic Ups for contacts still down. |
| `PointEntered`, `PointLeaved` / `pointAction` (`Enter`, `Leave`) | Native client-surface boundary notifications for an active contact. They retain the sequence target and bubble; they are not hit-test transitions between arbitrary compositions. |

The complete Step 1 contact event set is represented above. `WM_POINTERDOWN`, `WM_POINTERUPDATE`, and `WM_POINTERUP` supply transitions. `WM_POINTERENTER`/`WM_POINTERLEAVE` supply boundaries only while the contact belongs to a live sequence. `WM_POINTERCAPTURECHANGED` for an active contact ends the sequence with `Canceled_CaptureLost`; `POINTER_FLAG_CANCELED` ends it with `Canceled_PlatformCanceled`. A canceled native Up produces End rather than a successful Up, so its list entry remains populated during End. Use `Canceled_WindowDestroyed` for native-window teardown and `Canceled_Unknown` when no more specific reason is available.

Other messages cataloged in section 1.2 are not additional contact events: activation, non-client processing, device changes, touch hit testing, and specialized routing keep their platform responsibilities. Wheel input retains the existing wheel path. Step 1 does not add hover-only sequences, pen handling, browser raw-update events, or a public native pointer-capture service. Later browser capture notifications also need normalization rather than a separate public callback for every DOM event.

Every normally completed contact in an accepted sequence has one Up. If cancellation occurs, terminate the whole public sequence with exactly one End immediately, even if several contacts remain down. Emit no further point-data callbacks or synthetic Ups for that sequence. The host keeps the last known positions in occupied list slots through End propagation, then releases its capture and clears its state.

The provider separately retains enough private native bookkeeping to consume or default-process trailing packets according to the original acceptance decision. Surviving contacts and new Downs overlapping that drain period do not start another public sequence, requery acceptance, or switch fallback modes. Retire a capture-lost/canceled native contact without waiting for an Up that may never arrive; track other still-owned contacts until their native termination. A fresh Down after that native activity has finished can start a new sequence. Ignore delayed boundary/capture cleanup for retired contacts. A removed listener/target cannot receive later callbacks.

Copy native information while it is valid. Callback arguments and the shared list view are borrowed for the handler duration; retained records copy values. The const reference prevents handlers from changing the host's list, but it is not an immutable snapshot: the same object is updated for later receivers and later events, and cleared after End. Copy the positions if they must outlive a callback. The initial API delivers one changed point per callback, not a promise of simultaneous hardware sampling. If history is added later, preserve chronological order and do not duplicate the newest sample.

Native window destruction emits canceled End before listener teardown, preserving remaining occupied slots for End handlers. It does not fabricate Ups. A later native Up/capture-loss notification must not repeat End. Unsupported providers simply never emit the new callbacks and continue existing mouse behavior.

## 3.8 Dynamic platform fallback and Windows implementation

The decision belongs to each sequence's first occurrence. A point-aware region can accept one sequence and decline the next at the same position if its state changes. No window-creation setting substitutes for this query.

| Latched decision | Native processing | GacUI composition delivery |
|---|---|---|
| Accepted by at least one native listener | Handle the entire sequence as point input; suppress/filter duplicate touch-promoted mouse input. | A locally accepting host delivers Started, point data on its captured target/parents, then End. A locally declining host delivers Started/End only. |
| Rejected | Keep private contact bookkeeping and use platform fallback for the entire sequence, starting with its first Down. | Started and End only; fallback mouse events use the existing mouse pipeline. |

On Windows, the intended fallback is OS-generated compatibility mouse input through the existing mouse-message path. GacUI does not synthesize an `ImmediateMouse`/`DeferredMouse` stream in Step 1. Do not consume the first Down and then ask Windows to reconstruct it later. Do not switch point handling/fallback in the middle of a sequence.

Windows documentation warns that selectively consuming pointer input while passing other input to `DefWindowProc` can produce undefined behavior. Keep handling consistent for the complete sequence and verify accepted -> rejected -> accepted transitions on native input. The documentation is not proof that every sequence-switching strategy works; native fallback is an explicit integration acceptance requirement. [WM_POINTERDOWN](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown).

Rejecting point input also rejects delivery to any GacUI gesture path. There is no gesture path in this version, for accepted or rejected sequences. Windows default processing can produce gestures instead of mouse input, so forwarding a rejected Down to `DefWindowProc` alone is not a sufficient mouse-only fallback design. The provider must suppress those gesture behaviors while preserving mouse promotion. `SetGestureConfig` documents blocking all gestures; it may be used privately for suppression, not recognition. Its HWND-wide policy does not decide point acceptance and must not become a public input-mode/configuration API. Verify the combination rather than adding a framework mouse emulator if it fails. [Gesture configuration](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setgestureconfig).

Native delivery capture and composition capture are separate. Windows implicitly owns a contact at its native target window; `GuiGraphicsHost` captures one composition for the sequence using the pair in section 3.5. [Native pointer ownership](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown).

Duplicate filtering must retain real mouse input and OS fallback for rejected sequences. Do not suppress all touch-origin mouse events globally. An already-down secondary finger does not become a new primary mouse contact when the first finger lifts. Keep enough sequence/source bookkeeping to distinguish accepted-point duplicates from the rejected OS mouse path.

## 3.9 Demonstration and verification for Step 1

The FullControlTest page requested by Step 1 consumes raw point events. It accepts in `pointSequenceStarted` and subscribes once to `pointAction`, using `eventType` when an action needs distinct handling. It reads active positions from the shared list using point IDs as indices, uses one point to drag, and computes two-point translation, rotation, and scale directly in its demo behavior. Rebase geometry when active contacts change so adding/lifting a finger does not jump the content. This demonstrates point input without adding a gesture API or changing built-in list/text controls.

Add point IO to the existing automation service so ordered Down/Update/Up samples and sequence-cancellation stimuli exercise the same acquisition, query, capture, routing, and fallback decisions. Cancellation stimuli produce End with a reason through the native path; they do not invoke a separate point-cancel callback. Direct composition-event injection alone cannot verify native fallback. Keep IDs and simultaneous contacts explicit; do not model two fingers as two mouse buttons. The concrete command grammar/internal parser changes belong to that implementation work, not a new native service in section 0.

Required cases include:

- Acceptance by the target, acceptance only by a parent, and no acceptance. `handled` without `accept` does not opt in.
- One `pointAction` subscription receives Down/Update/Up/Enter/Leave with the corresponding `eventType`. Started has `eventType == GuiPointEventsType::Started` and remains a separate query; End remains a separate lifecycle event.
- One Started/Ended pair with accepted=false and no point-data events on rejection; OS mouse fallback still reaches an ordinary control.
- Accepting/rejecting successive sequences at the same position after changing handler state, without startup configuration or listener reinstall.
- A Down, B Down elsewhere, A Up, B Update/Up: one sequence ID and one original composition/logical-window capture.
- A/B/C overlapping contacts as in section 3.2; IDs 0/1/2 follow Down order despite slot 0 becoming empty. The next sequence starts again at point ID 0, with a new sequence ID.
- Started and first Down share slot 0. Later Down appends, updates change the indexed slot, and Up empties it before the handler without reducing list length.
- The same list object reaches all point and End handlers; its positions are correct for each receiving composition. Successful End sees all slots empty.
- Cancellation with several points still down: one canceled End retains occupied slots and previous Up holes, with no synthetic Ups or later point events. Trailing native packets cannot restart the sequence or switch fallback. Rejected End has an empty, unavailable tracking list.
- Primary release without promoting a remaining finger to mouse.
- Movement outside the original composition/native window, boundary notifications, capture loss without Up, canceled Up, and duplicate terminal cleanup.
- Physical mouse input while points are active; duplicate filtering does not lose rejected fallback.
- Multiple native listeners with different answers: accepted native data is broadcast, while a locally rejecting host raises no composition point stream.
- `PointCapture`/`PointUncapture` pairing, including rejected sequences and release after End; mouse capture remains independent.
- Deferred target removal through existing lifetime hooks; no stale capture reuse or retargeting. Safe in-handler deletion is explicitly not required.
- Hosted window movement/DPI conversion during a sequence; coordinates remain correct for the same captured target.
- Old providers/listeners with no overrides; existing mouse behavior remains available.

Use deterministic routing/sequence tests plus native integration tests. Windows injected multi-contact input can verify routing/fallback, and real touch hardware validates the interaction. Do not claim these passed merely because mock listeners or direct event injection passed. When factoring shared mouse routing, also run existing mouse-input tests to ensure its propagation/capture behavior is preserved.

Implement reflection in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp` for native enum/value types, `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp` for `GuiPointEventsType`, arguments/fields (including `eventType`) and read-only `Points` properties backed by `GetPoints`, and `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionCompositions.cpp` for `pointSequenceStarted`, `pointAction`, and `pointSequenceEnded`. Follow the existing type-list/event macros. Do not register the reference member with `CLASS_MEMBER_FIELD` or apply `EVENTARGS_CONSTRUCTOR` to these types: they have no default/composition-only constructors. The host constructs these borrowed-list arguments; do not expose reflected constructors that could retain a temporary script-supplied list. Native listener callbacks remain C++ notifications.

During implementation, run prescribed unit tests, metadata/resource generation, and focused FullControlTest/UIA checks for affected changes. Step 1 does not update the remote-protocol SOP or knowledge-base pages. The GacUI_Windows tutorial addition and those documentation updates belong to Step 2. This document edit changes no source, reflection implementation, demo, or generated output.

## 3.10 Browser and remote follow-up in Step 2

Step 2 adds the point demonstration to the GacUI_Windows tutorial, ports the contract to the remote protocol, and updates `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` to exercise the page with native renderers before verifying GacJS/WASM. It also covers the wGac fallback and knowledge-base updates. Those knowledge-base pages must be self-contained and must not refer to `<GacUI repo>/TODO_Touch.md`.

The desired browser/remote contract remains: query the first occurrence, preserve its decision/ID, keep one target through hosted/host routing, and pair sequence boundaries. Extend `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_IO.txt` with sequence queries/results and point data when implementing Step 2; do not restore removed startup configuration requests or configuration revisions.

GacJS currently forwards input from `<GacJS repo>/Gaclib/gaclib/renderer/src/GacUIRendererImpl.ts` across the bridge in `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/index.ts` and `<GacJS repo>/Gaclib/website/remote-protocol-wasm/src/worker.ts`. An asynchronous core answer cannot retroactively suppress the original browser event's default action. CSS `touch-action` must permit the intended ownership before contact begins; changing it during Down cannot transfer the current gesture. [Browser ownership rules](https://www.w3.org/TR/pointerevents3/#declaring-direct-manipulation-behavior).

Step 2 must resolve how the browser makes a timely local decision consistent with the current sequence query, or define a constrained browser fallback policy that can actually be implemented. Do not claim an arbitrary worker-round-trip query preserves both native browser fallback and uninterrupted custom point delivery. Creation-time opt-in must not silently replace the public per-occurrence contract.

Accepted custom input also needs stable DOM capture and correlation of touch-origin compatibility mouse/click events. Canceling a pointer event alone does not suppress click dispatch. Preserve real mouse, keyboard, and accessibility activation. Browser takeover/cancellation ends the entire public sequence with a `Canceled_*` reason without waiting for Up, preserving the occupied composition-list slots until End completes. [Compatibility activation](https://www.w3.org/TR/pointerevents3/#compatibility-mapping-with-mouse-events).

Transport must preserve Started before point data, the decision for that occurrence, Down-order point indices, per-point ordering, and one End after the final normal Up or immediately on cancellation. Remaining native cleanup after canceled End stays internal and cannot generate more point messages. Coalesce only safe updates; never let B overwrite A or lose sequence boundaries. Include connection generation, preserve IDs through adapters, and require compatible peers before new messages are used. Existing mouse-only pairs keep their current protocol path.

## 3.11 Later controls and gestures

Momentum scrolling and long pressing for lists/documents belong to Step 3. Gesture API design belongs to Step 4. Neither is a prerequisite for the raw-point demonstration or a reason to introduce a recognizer in Step 1.

The gesture research in sections 1 and 2 remains useful: Windows can supply recognized manipulation/inertia data; portable browser code generally needs its own recognition/motion logic, with Safari-specific extensions as optional research. This difference does not change initial sequence acceptance or routing.

Section 4 remains future control-behavior research exactly as written. Its policy names such as `DeferredMouse` and manipulation/hold sequences are not Step 1 API declarations. Later control implementation must reconcile those policies with sequence negotiation rather than restore startup configuration or per-finger routing.

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
