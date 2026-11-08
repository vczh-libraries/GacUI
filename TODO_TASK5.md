Fix some issue from the previous execution of `TODO_TASK3.md`:
- Repro in `Document Editor (Ribbon|Toolstrip)` tabs:
  - Need to change the design, where short press on a selected area of a text box, it should act like trivial left click, which moves the caret, instead of doing nothing.
  - Unlike trivial left click, short press after the end of a paragraph or below the last paragraph does not move the caret. It should.
  - When the context menu is opened, short press in other area of the text box cannot close the context menu, while clicking outside of the text box can. It shyould close the context menu
  - I think in order to deliver the same experience, handling both the short press generated left/right/double click and trivial click should go the same path, `GuiDocumentCommonInterface` probably need to fix like that.
    - One of the reason might be two click is handled in different compositions, if this is the case:
      - The background composition should pay attention about composition fields in event arguments, tell if the click is directly generated from gesture or not. When clicking on a scroll bar, `eventSource` and `compositionSource` will be different. Generated clicks or any point/gesture events except `pointSequenceStarted` should have `eventSource` and `compositionSource` pointing to the same composition, which is the `sender`.
      - Different compositions have different offset, deal with x/y carefully.
      - Eventually clicks should deliver to text box's original event handler, but I don't require two compositions to use the same event handler because gesture generated clicks might need extra care. But in this case it should call the original event handler directly instead of triggering the event.
      - "original event handler" means the ones who initially reacts to trivial/real clicks.
  - Around `DocumentEditorBase.xml`'s `ShortcutBuilder-bind` code:
    - It originall changes because the shortcut is not working in `Document Editor (Toolstrip)`.
    - It is expected, both ribbon and toolstrip use the same set of shortcut key, so only ribbon is expected to respond to them.
    - Please revert the original change around `ShortcutBuilder-bind` code, you can git blame to know the original implementation.
    - If anything in `DebugRemoteProtocolSop.md` mentioning checks shortcut keys, only do it on ribbon tab page, and don't do it on toolstrip tab page.

Fix some issue from the previous execution of `TODO_TASK1.md`:
- Repro in `TouchingList` tabs:
- `MomentumScrolling` seems to have a lower damping, I would like to:
  - Cut the deceleration to half, so that it stops faster.
  - Originally it calculates the momentum scrolling starting speed in 0.5 seconds of moving:
    - Verify again to make sure: the calculation of the average speed consider the timeing gap between the last scrolling gesture and `pointSequenceEnded`:
      - If the figure rests for 1 seconds before releasing, the starting speed of momentum scrolling should be 0.
      - If the figure releases right after scrolling, you will have enough scrolling gesture to calculate the starting speed.
    - Change it to 0.25 second.
  - If you think adding a timestamp to `pointSequenceEnded` is needed, you need to fix at least:
    - `GuiNativeWindow.h`
    - Remote protocol
    - Automation Service
    - Any platform provider message redirection, e.g. hosted mode
    - To add timestamp to all point events.

- To verify, following job:rpWindows but:
  - No need to generate test matrix card.
  - Only verify `CppTest`, `RemotingTest_Rendering_Win32`.
  - Only verify `GacJS` if remote protocol is touched.
  - Only perform tests on 3 tabs mentioned above.

- Update KB pages and website to update stale facts caused by this task, if anything is originally not mentioned, no need to add.
- Commit and push all local changes, and then publish the website, skip learning in `job:copilotInitAll`.
