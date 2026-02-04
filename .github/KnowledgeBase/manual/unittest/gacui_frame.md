# Snapshots and Frames

If you let the unit test framework create a window from a GacUI XML resource, You can set the 4th parametero of**GacUIUnitTest_StartFast_WithResourceAsText**to offer some callbacks. The type is**GacUIUnitTest_Installer**and it has these members:
- **Func\<void()\> initialize;**it is called before the window is created.
- **Func\<void(GuiWindow*)\> installWindow;**it is called before the window is opened.
- **Func\<void()\> finalize;**it is called after the window is closed.
- **Nullable\<UnitTestScreenConfig\> config;**more configurations, including font list, screen size, etc.

A test case typically begins with a call to**GacUIUnitTest_SetGuiMainProxy**with multiple frames created. You are recommended to follow the rule:
- The name of the first frame should be**Ready**.
- Names of other frames describe what has been done in the previous frame.
- In each frame visible UI changes must be triggered.
- As each IO operation is completed after all events are handled in the GacUI application, if an operation calls any blocking function, wrap it in**InvokeInMainThread**.
- When an action is taken, if the side effect happens immediately, assertion should be placed right after the action.
- When an action is taken, if the side effect happens async but fast, assertion should be placed at the beginning of the next frame.
- Whenever possible keep the last frame only calling the**window-\>Close();**function. This rule could sometimes conflict with the previous rule, the previous rule has higher priority.

## About Visible UI Change

A visible UI change means something that cause GacUI to refresh any window in your application. This includes changing texts, sizes, layouts; showing or hiding windows or controls; enabling or disabling controls; select anything or type anything in a control; etc. The unit test framework will detect whether any visible UI change has been made in a frame, and the test fail if there is not.**IMPORTANT:**DO NOT make a frame just for assertion as it doesn't cause any visible UI change.

## About Blocking Function

Blocking functions are anything that does not return but the application is still running. For example,**ShowDialog**,**ShowModal**,**ShowModalAndDelete**,**InvokeInMainThreadAndWait**, etc. Any IO interaction inside a frame only returns after GacUI handles all events. For example,**protocol-\>LClick(location);**on a button only returns after**Clicked**as well as all other mouse events are handled. If in any of this event a blocking function is called,**LClick**blocks the current frame, and the unit test framework detects such action and fail the test. In this case, call**LClick**in**InvokeInMainThread**instead.
```c++
GetApplication()->InvokeInMainThread(window, [=]()
{
    protocol->LClick(location);
});
```


