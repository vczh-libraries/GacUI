In `GuiPointSequenceStartedEventArgs` there must be two additional fields default to true:
- `generateLeftButtonClick`
- `generateRightButtonClick`
If event subscriber doesn't explicitly set them to false, the left/right button click gesture will be activated like what they are now.
`IGuiPointGesture::OnPointSequenceStarted` should accept `GuiPointSequenceStartedEventArgs` instead of just `NativeWindowPointInfo`.
The other two functions also receive dedicated event args to keep them aligned, also making `IGuiPointGesture::OnPointAction` no need to have two arguments.
Add two remote protocol based unit test to verify each.

Also address a bug. When a tooltip is displaying and its parent window is hidden or closed, the tooltip should be closed first. It applies to all popup showing when the parent window/control is specified, it trackes when the specified window or the window of the specified control.
