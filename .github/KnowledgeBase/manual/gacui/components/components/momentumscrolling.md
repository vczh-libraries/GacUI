# MomentumScrolling

`vl::presentation::compositions::MomentumScrolling` is an optional `GuiComponent` that continues a single-contact scrolling gesture after successful release. Ordinary scroll views do not enable momentum automatically. The component runs on the UI thread and emits the existing scrolling payload; it needs no renderer-specific messages.

## Ownership and attachment

Create the component with the parameterless constructor returning `MomentumScrolling*`, then transfer ownership to one root with `GuiInstanceRootObject::AddComponent`. Call `AttachControl` for each target. Attachments do not own controls, and duplicate attachment is a no-op. Each control has independent sampling and playback state.

`DetachControl` stops that control and removes its listeners; detaching an unattached control is a no-op. Component detachment or destruction stops all targets. Disposed controls are retired without accessing destroyed receivers. A fresh Started, cancellation, an extra contact or a graphics-host change prevents stale continuation.

Set `AcceptPointEvents` explicitly on non-scroll views when default acceptance is wanted. Attachment does not change that property. `GuiScrollView` and derived controls already default it to true. Subscribe at the persistent `BoundsComposition`: `GetPointEventComposition()` queries active capture and returns null while idle or after End. An accepting child remains the sole owner of its sequence.

## Selection and movement

Momentum preserves selected list items and document ranges while scrolling the viewport. Selectable and Editable document content still reject point acceptance so mouse compatibility can select text immediately. Attaching the component does not convert that selection drag into scrolling. ViewOnly content and scrollable chrome can initiate momentum.

The component selects scrolling offers without marking them handled. Normal controls and application handlers consume movement. Generated events start unhandled, target both source fields at the bounds and do not contribute to velocity sampling. Existing axis locking, enabled/visible scrollbar checks and range clamping still apply.

## Deceleration

`Deceleration` is a finite positive double, initially `1.0`. Velocity is the signed average over the final half second, or the shorter sequence duration, including stationary time through End. Playback captures the current setting and decelerates at `320 * Deceleration` logical pixels per second squared.

Scrolling 160 logical pixels in the last 0.5 seconds yields 320 pixels per second. At the default setting, continuation stops after one second and adds 160 logical pixels before clamping. A value of 2.0 stops in 0.5 seconds; 0.5 stops in two seconds. Increments are integrated at 0.1-second elapsed boundaries plus the final partial interval, carrying fractional distance. Delayed timer callbacks catch up using elapsed time.

## XML example

This root owns one component. The initially unchecked checkbox attaches or detaches its scroll container.

```XML
<Instance ref.Class="demo::MomentumWindow">
  <ref.Members><![CDATA[
    var momentum: MomentumScrolling* = new MomentumScrolling*();
  ]]></ref.Members>
  <ref.Ctor><![CDATA[{ self.AddComponent(momentum); }]]></ref.Ctor>
  <Window ref.Name="self" Text="Momentum" ClientSize="x:480 y:320">
    <ez:Layout>
      <ez:Top>
        <CheckBox ref.Name="enabled" Text="Momentum Scrolling" Selected="false">
          <ev.SelectedChanged-eval><![CDATA[{
            if (enabled.Selected) { self.momentum.AttachControl(view); }
            else { self.momentum.DetachControl(view); }
          }]]></ev.SelectedChanged-eval>
        </CheckBox>
      </ez:Top>
      <ez:Fill>
        <ScrollContainer ref.Name="view">
          <Label Text="Scrollable content">
            <att.BoundsComposition-set PreferredMinSize="x:600 y:1200"/>
          </Label>
        </ScrollContainer>
      </ez:Fill>
    </ez:Layout>
  </Window>
</Instance>
```

See the [TouchingList tutorial](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Windows/TouchingList) for list selection and horizontal swipe deletion alongside optional vertical momentum.

