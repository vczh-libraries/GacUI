This is a task to make browser and potential future renderers could do user typing correctly.

## Task 1

Currently the DOM tree lacks of a mechanism to report which composition is editable.

Add to `GuiGraphicsComposition::(Get|Set)EditableAttributes`, the header file is already edited:
- It does nothing but a simple getter/setter.
- It does not trigger any event/
- It is not responsible to editing or any other feature.
- You should complete the reflection registration for it.

Currently only `GuiDocumentViewer` and `GuiDocumentLabel` will update its value, but it should be able to only change `GuiDocumentCommonInterface`:
- `documentControl->GetFocusableComposition` will be the target.
- Remove `focusableComposition` from `GuiDocumentCommonInterface::InstallDocumentViewer` and use `_sender->GetFocusableComopsition` instead.
- When a control got or lost focus, update `EditableAttributeValues::focused`. Currently disabling a control should automatically make it lost control so no need to handle enabling/disabling.
- `GuiDocumentEditMode::Editable` sets `EditableAttributeValue::editable` to true, other values set it to false.
- `EditableAttributeValue::featured` will always be true, as a hint that it belongs to a text box.

This new property is expected to completely have no side effect on core, only affecting remote protocol DOM tree content, the protocol definision is already edited.
To make a editable focusable composition into a dom tree, check if `EditableAttributeValues::featured` is true:
- If `featured` is true, `(ElementBoundary|RenderingDomContent)::editableAttribute` should copy `EditableAttributeValues`.
- If `featured` is false, `(ElementBoundary|RenderingDomContent)::editableAttribute` should be null.
- In order to make a visible featured composition always exist in the dom tree, `GuiRemoteGraphicsRenderTarget::AfterPushedClipper` is the key place:
  - Add a `Nullable<EditableAttributeValue> GetEditableAttributeFromGenerator` after hit test and cursor.
  - `if (arguments.hitTestResult || arguments.cursor)` will need to test `arguments.editableAttribute`.
- `GuiRemoteEventDomDiffConverter`, `GuiRemoteProtocolDomDiffConverter`, and `GuiRemoteProtocolSchema_FrameOperations.h` might need to be taken care of the make sure rendering commands -> dom -> dom diff -> recovered dom are all doing good.

To verify, add a remote protocol based unit test case, which is a window with multiple text boxes, enable/disable/focus something for a few times, and see if the DOM tree is properly updated:
- It does not easy to know which dom node binds to which control, but since a text box's focus composition is its bounds composition, so it is stable throughout the control's life time.
- You can first focus each one and see if each time it focus to a different dom node, and now you know their connections.
- Begin to perform other actions and see if all dom nodes are properly updated.
- `UnitTestRemoteProtocol::GetLastRenderingFrame()->renderingDom` has a dom, but each frame all dom nodes are different objects, you can tell the connection between frames by `RenderingDom::id`. Be awared that it only has visible nodes, so if a text box is hidden by itself or its any parent node, you can't find it in a dom tree.
- Since there is no actual meaningful change in `GuiRemoteRendererSingle`, so you can skip testing against test app features:
  - Make sure unit test cases cover.
  - Just launch `CppTest`, `CppTest_Metaonly` and `RemotingTest_Rendering_Win32`, find some text box and type something in them, to see if the code does not break, as a smoking test.
  - No need to run any other verification on test apps.

## Task 2
