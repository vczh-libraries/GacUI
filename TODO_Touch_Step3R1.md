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

### DETAILS

- Keep recognition in the shared graphics host, with the four fixed implementations and their definitions in the requested order. There is no end-user gesture registration API, OS recognizer, new wire message, or renderer-specific recognizer. Preserve the completed native, hosted, remote and automation sequence contracts.
- Moving initialization after acceptance needs a separate Started-query sample: the current host initializes the manager before asking handlers because the query borrows its slot-0 list. Build a temporary one-slot list and rebase the sample/list for each receiver, then initialize the accepted sequence and establish capture before invoking gesture Started callbacks. Rejected sequences still receive native End; they must not consult stale manager state. See `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp`.
- Retain manager-owned contact storage, coordinate conversion and capture; move recognition state into the individual gestures. Supply the recognizers with the current sample, sparse contact positions and terminal reason/timestamp. Started is not an additional Down. Remove all external manager dependencies, including `GuiControl::GetPointEventComposition` in `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp` and manager-inspecting tests; preserve that property through a narrow host capture query.
- Deliver `pointGesture` directly to the accepted composition, with the same source conventions as point Action/End and no parent handoff. Raw point bookkeeping and delivery must continue after a gesture wins. The first `selected = true` is sticky within the dispatch and for the sequence; subsequent handlers cannot undo it. `handled` remains a separate per-event flag. Reset selection and candidate state for each accepted sequence, and never inspect a candidate's stale result from an earlier sequence.
- Click candidates never select themselves or emit gestures. Eligibility requires a successful End, exactly one Down during the entire sequence, and no excursion strictly beyond `StaticPointRange`; evaluate final Up as well as intermediate positions. Use wide arithmetic for the squared radial test and `StaticPointRange * StaticPointRange`. Exactly eight logical units remains eligible. Use monotonic microsecond timestamps: Right wins at `500000`, Left below it, without a timer. Selected scrolling/manipulation returns no click. The existing `fallbackClick` opt-in is addressed in the lifecycle review comment below.
- Scrolling uses point 0, locks horizontal on an equal-axis diagonal, and cannot switch axes. Any second Down permanently stops its gesture events for that sequence, even after the extra contact lifts; no remaining contact replaces point 0. An unselected scrolling candidate may keep offering its locked gesture until selected or disqualified. A manipulation candidate declined at Started never offers itself again in that sequence.
- Interpret gesture payloads as increments applied to the current object/view. The first scrolling distance includes the full projected displacement from initial Down through threshold crossing; later distances are projected increments. Manipulation starts with identity `(rotation = 0, scale = 1, distance = (0,0))`. In accepted-composition coordinates, `p' = scale * R(rotation) * p + distance`; a two-contact step uses `distance = newCentroid - scale * R(rotation) * oldCentroid`, so an off-origin pinch rotates/scales about its contact centroid.
- Move the existing Touching membership rules into the manipulation recognizer: one contact translates; two translate/rotate/scale; more than two pause. Compare occupied point IDs, not only count, and rebase without movement on membership changes, coincident pairs, and recovery from a paused state. Keep the selected gesture through these pauses. Preserve canonical native screen samples and reproject both baselines and current contacts using current conversion. Keep transform calculations in double precision; because the requested `Point` translation is integral, define deterministic rounding and carry transformed fractional residuals so repeated fractional rotations/translations do not accumulate truncation drift.
- Add the event and argument types consistently to the receiver declarations/initialization and reflection type inventories, while keeping `IGuiPointGesture` internal. The native `Variant` cannot be registered as an ordinary reflected field with the current library; its Workflow-facing representation remains deliberately unresolved in the reflection review comment below. Read `GetCallbackClick` in the description as `GetFallbackClick`.

### VERIFICATION

- Extend the existing protocol-driven coverage in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp`; assert public events/results instead of the removed manager getters. Cover accepted/rejected/accepted sequences, native End after rejection, sticky selection, handled events, declined initial manipulation, independent capture, cancellation and fresh-sequence reset.
- Cover radial movement below/at/above eight, diagonal ties, reversal, final-Up-only movement, crossing then returning, time below/at `500000`, and a second contact both before and after scroll selection. Assert no scroll resumption or click after the second contact, and no fallback after any cancellation reason.
- Check identity, one-contact translation, off-origin two-contact rotation/scale, unchanged repeated samples, all 1/2/3-contact transitions, sparse/replaced IDs, coincident points, and fractional round trips. Change DPI/window position during a sequence and verify both stationary contacts and baselines use current conversion.
- Preserve the rejected-input mouse stream separately from accepted-sequence deferred clicks. Verify click button/modifiers/order, current hit testing at the original start position, point cleanup before synthesized mouse input, and unchanged physical mouse state. Keep rendering-independent assertions in ordinary tests.

2) `GuiScrollView`

A new empty `OnPointGesture` will be added to `GuiControl` for overriding.
Now `GuiScrollView` only needs to handle `OnPointGesture`, to select this gesture, and keep scrolling when `OnPointGesture` is called.

### DETAILS

- Attach the new default-empty virtual hook once to the persistent bounds composition, following the existing point adapters in `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp`. Scrolling must consume only the scrolling payload, only for its owned sequence, and only when unhandled and visually enabled.
- Select and mark handled when the locked axis has an existing, visible, visually enabled scrollbar; use the ordinary range-clamped scroll APIs in `<GacUI repo>/Source/Controls/GuiContainerControls.cpp`. Subtract the signed incremental distance from the current view coordinate, leaving the other axis unchanged. After clamping, reversal acts on the current view position. An unavailable axis does not select manipulation, switch axes, or hand off to a parent.
- Remove the old axis/displacement coupling and redundant scroll recognition fields. Do not silently remove deferred tap/hold behavior when deleting the old End override: whether that small lifecycle hook remains, or its fallback policy moves elsewhere, is part of the lifecycle review comment.

### VERIFICATION

- Retain real list scrolling coverage in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp`, including useful rendered before/after frames, view-position assertions, both directions, clamping/reversal, stable selection, and no item activation after a drag.
- Check absent/hidden/disabled scrollbars, a disabled control, nested scroll views and an accepting child. Verify exactly one owner moves, no fallback when movement crossed the threshold, and unchanged tap/right-hold behavior after the lifecycle decision.

3) `Touching` tab in `FullControlTest`

The logic is moved so now this test page only need to handle `pointGesture` events.

### DETAILS

- Migrate `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingTabPage.xml` so recognizer state and contact-to-transform mathematics live in the predefined gesture. Its gesture handler selects manipulation and applies the delivered transform to the displayed center/axis; it must not select scrolling.
- Preserve the current raw drawing contract `centerX,centerY,axisX,axisY,count` followed by contact-ID/position triples, the rectangle and contact markers, and state across tab changes. Native and browser painters remain renderers of supplied state. The required acceptance/marker/completion information is the subject of the lifecycle review comment; do not remove markers to work around missing event data.

### VERIFICATION

- Reuse the existing Touching assertions in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` for translation, pinch/rotation, membership rebasing, three-contact pause, coincident points, source-token reuse and cancellation. Verify both transformed geometry and marker count/positions, with an explicit rounding tolerance for the integral translation payload.
- Confirm successful and canceled completion clear markers, the next sequence starts cleanly, ordinary mouse dragging does not manipulate the rectangle, and an accepted manipulation never generates a fallback click. Exercise the actual XML handlers in generated-C++ and interpreted Workflow consumers once the reflection decision is implemented.

4) New `TouchingList` tab in `FullControlTest` besides `Touching`.

This is a text list will big item (item template needed). On top there is "Clear" button to delete everyting, and "Add 5 Items" button to add 5 text items with some lorem ipsum make up content.

Dragging up and down scrolls the list items, this should be automatically happens.

Dragging right nothing happens.

Dragging left will move the item towards left, with the right space filled by red. The red zone will have right aligned label called "DELETE IT". The label will have a reasonable margin, and it defines the maximum space of the red zone. The item should not be dragged more than the maximum red zone. When the sequence ends and the red zone is extended to its maximum space, the item will be deleted in `InvokeInMainThread`, immediate deletion will crash GacUI because following messages still need the alive composition.

The dragging will need to be implement in the list itself, as gesture does not have a "to parent fallback" like `pointSequenceStarted` event.

### DETAILS

- Add the adjacent Misc tab through the existing FullControlTest resource inventory. The page/list owns gesture handling and data mutation; the custom text-item template owns presentation. Fit/wrap each row to the viewport width so there is no enabled horizontal scrollbar competing with swipe handling.
- Let the list bounds accept the sequence. Use horizontal `ScrollingGesture` for swiping, leaving vertical gestures to the built-in scroll view; selecting manipulation at Started would prevent vertical scrolling. Do not let individual item templates accept the sequence. Remember the initially hit row's data identity using the initial-hit metadata supplied by the lifecycle design resolved below; check that the item belongs to this list. Starting on empty space acquires no deletion target.
- The custom list handler selects/handles unhandled horizontal gestures. Accumulate reveal width as the negative horizontal movement, clamped to `[0, maximum]`; right-only dragging stays at zero and reversal retracts an existing reveal. Measure maximum from the right-aligned `DELETE IT` label plus padding. Keep the opaque sliding foreground and red background clipped within a stable row slot.
- Delete exactly once only after successful single-contact completion with the final reveal at maximum. A partial release resets the row. Cancellation or any additional contact resets without deleting, including after maximum reveal was reached; consume the contact/end metadata supplied by that lifecycle design even when the scrolling recognizer has stopped emitting events. Final Up position must contribute before the completion decision.
- Queue removal with `InvokeInMainThread` using the underlying text-item/view-model identity, never a cached row composition or index. Virtualized templates can disappear and indices can change. Resolve current membership at execution; a prior Clear or removal makes the queued deletion inapplicable, and newly added items must not be deleted. Clear resets swipe presentation/state. Keep the known prohibition on synchronous composition deletion.

### VERIFICATION

- Verify Clear on populated/empty lists and exactly five new items per Add action; add enough items to overflow. Check tab/button names and roles, usable row sizes, wrapped text, label padding, clipping and the measured maximum reveal.
- Exercise vertical scrolling, right-only movement, partial/full left swipes, reversal, final-Up-only motion, cancellation, a second contact, and starts outside rows. Assert item identity/count as well as the visible red region; full reveal alone must not delete before successful End.
- Cover Clear/Add before queued removal, deletion of a middle row, and scrolling rows out of view and back. No other row may inherit swipe state or be deleted through a stale index. Capture meaningful rendered frames for partial reveal, reset and deferred removal.

5) Verification and Documentation

Native renderer, remote protocol with native renderer, GacJS, should all verify, but to keep the verification fast, only verify `Touching`, `TouchingList` and list control scrolling, additionally make sure touching on text is fallbacked to mouse so a simple dragging should select some text.

`KB_GacUI_Design_PointInput.md` needs to update to introduce this new architure, emphasis that the extensibility only opens to GacUI maintainers. A complete rewrite is welcome to organize the document in this way and remember this is a page to introduce API and its design:
- `INativeWindowListener` events, how to implement and how to handle. Mentioning only Windows and remote protocol supports touch experience.
- Remote protocol, how to send point events in a correct way. Mentioning only Windows and GacJS supports touch experience.
- How to handle point events in `GuiGraphicsEventReceiver`.
- List predefined gestures and click fallbacks.
- Controls that already use gestures.

### DETAILS

- The requested text-selection check changes existing behavior: editable multiline text currently inherits scroll-view point acceptance, and `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp` explicitly expects touch dragging to scroll without changing selection. Use the existing rejected-sequence mouse Down/Move/Up path for selectable/editable text content; a deferred click at End cannot implement range selection.
- Apply this policy at the text-content input area, with lifecycle ownership alongside the handlers maintained by `GuiDocumentCommonInterface::ReplaceMouseArea` in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`. When appropriate for enabled Selectable/Editable content, mark the Started query handled without accepting so enclosing default scroll views cannot claim it. Ensure interception occurs below accepting bounds and is reattached on template changes. Preserve explicit custom acceptance, ViewOnly document scrolling and scrollbar/chrome behavior.
- Update `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_PointInput.md` in the requested API/design order. Explain routing, selected versus handled, capture/lifetime, coordinate units, cancellation, fixed gesture priority, resolved payload semantics and both mouse fallback paths. Keep the maintainer-only extension boundary explicit, and distinguish implemented native acquisition from remote forwarding support.
- This review edits only this task document. The builds, resource generation and runtime checks below belong to later implementation. The broader touch plan does not expand this step into inertia, OS gesture integration, text-selection handles, mobile keyboard support or an unrelated transport/application matrix.

### VERIFICATION

- Follow `<GacUI repo>/Project.md` and the repository build/execution wrappers. After reflection changes, perform the required Debug Win32 and x64 metadata generation plus x64 `Metadata_Test`; regenerate FullControlTest with `GacUI_Compiler`, check error artifacts, repeat the required metadata generation after compiler execution, then rebuild consumers. Run required UnitTest coverage and inspect completed summaries and Debug leak output. Generated code and snapshots must come from their owning tools.
- Once the reflection review is resolved, test both payload alternatives, event base conversion and mutation of `selected` on the original arguments in `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp`. Verify actual XML event handlers through generated C++ without reflection and interpreted Workflow with full reflection; metadata generation alone does not prove either consumer works.
- Extend the existing focused `-Scenario Touching` in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` to cover TouchingList and text fallback, and run both `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1`. Keep the requested live checks focused on these pages, list scrolling and text selection.
- Perform three distinct live runs: standalone native FullControlTest, Core with a native remote renderer, and Core with GacJS. Use the existing setup guides and distinct Core/renderer automation ports. Drive the remote-native run through renderer IO, and the GacJS run through browser touch input/emulation; Core-only injection does not verify browser acquisition/forwarding. Use native automation when desktop injection is unavailable, and label its coverage accurately.
- In each run check Touching transforms/markers, TouchingList swipe outcomes and list scrolling without accidental selection/activation. Drag selectable/editable text and assert a nonempty changing selection range; repeat inside a surrounding scroll view and after template replacement. Update the old multiline scroll assertion rather than retaining contradictory expectations. Keep a ViewOnly scrolling and scrollbar-start check separate.
- Record application/renderer, transport, input method and observed state. In the browser verify the outer page stays stationary and release creates no duplicate compatibility click. Distinguish synthetic input from real hardware; unavailable checks remain unverified. During implementation keep authored changes, generated artifacts and snapshots in their prescribed separate commits.

## REVIEW COMMENTS

### Variant reflection for Workflow handlers

**review comment**: The proposed native `Variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation>` has no existing reflection mapping in `<GacUI repo>/Import/VlppReflection.h`. Registering `gesture` with the ordinary field macro requires type metadata that this specialization does not have, while both FullControlTest pages need to inspect gesture data from Workflow. The native C++ shape alone therefore does not define a usable XML-facing API. Per the follow-up instruction, leave this unresolved for now.

**suggested solution**: One possible later design is to keep the native variant, register both payload structs, and expose typed nullable read-only properties for the active alternative on `GuiPointGestureEventArgs`. Existing nullable boxing and the event getter bridges in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp` provide relevant patterns. This is a proposal for later review, not a selected representation or a request to implement a reflection-library extension now.

actually I have an idea, it looks like GuiPointGestureEventArgs should be a class instead of a struct, so we can actually change the property to object type in reflection only, but in fact such object type will becomes null, *_Scrolling and *_DraggingAndRotation in reality. Assigning values with other type will throw directly. This could be done by adding GetGesture method to GuiPointGestureEventArgs. And we still keep the strong typed `gesture` member exposed in C++, so that C++ code has compile time type check. This will be the most simplist change, just need to document it in `KB_GacUI_Design_PointInput.md`.

### Gesture-only handlers lack lifecycle and contact information

**review comment**: The proposed gesture arguments contain only a transform and `selected`; the described gestures do not emit a terminal notification. Literal gesture-only consumers cannot retain all current behavior: Touching needs contact IDs/positions for its markers and End/cancellation to clear them; TouchingList needs the initial row and successful-versus-canceled completion; `GuiScrollView::OnPointSequenceEnded` in `<GacUI repo>/Source/Controls/GuiContainerControls.cpp` is currently the only default requester of `fallbackClick`. Removing those hooks would either lose required behavior or change the existing opt-in click policy.

**suggested solution**: Keep minimal raw Started/Action/Ended handlers for acceptance, contact-marker/row bookkeeping and completion, including the existing fallback opt-in, while moving all recognition and manipulation mathematics into the gesture system. If "only pointGesture" is literal, instead specify gesture phase, borrowed contact information and terminal reason, and explicitly decide how click opt-in survives. Resolve that API boundary before implementing the affected consumers.

I believe if any gesture consumer needs to know the end or cancellation, `pointSequenceEnded` still useful for this. I believe in the design above, `pointSequenceEnded` will still shot for a selected gesture. And after this new design, although we have for `GuiControl::OnPoint*` methods, but only `GUiControl::OnPointSequenceStarted` needs to accept, and only `GuiScrollView::OnPointGesture` needs to select and respond to a gesture. What do you think?