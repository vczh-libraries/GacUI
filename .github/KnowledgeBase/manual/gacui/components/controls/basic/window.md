# ControlHost and \<Window\>


- **\<Window/\>**
  - **C++/Workflow**: (vl::)presentation::controls::GuiWindow*
  - **Template Tag**: \<WindowTemplate/\>
  - **Template Name**: Window

**\<Window/\>**is where we start our journey.
```
<Instance ref.CodeBehind="false" ref.Class="demo::MainWindow>
  <Window ref.Name="self" Text="This is the main window" ClientSize="x:640 y:480">
    <att.BoundsComposition-set PreferredMinSize="x:480 y:320">
    <!-- Here to put contents inside a window -->
  </Window>
</Instance>
```

- **\<Window/\>**is a**ControlHost**, which has to be a top-level window, is usually a base class of an[<Instance>](../../../.././gacui/xmlres/tag_instance.md). Here we create a class**demo::MainWindow**inheriting from**\<Window/\>**.
- **ref.CodeBehind**set to**false**so that GacUI doesn't generate a separated pair of C++ source files for this class.
- **Text**is the title of this window.
- **ClientSize**is the initial size when the window is loaded. This size doesn't include the border and the title.
- **BoundsComposition.PreferredMinSize**is the minimum client size of this window. When the window size is being changed by dragging the border, it cannot go smaller than this size.

## Adding something to a window

**\<Window/\>**is just a control. Compositions and controls in the window will be added to its**ContainerComposition**. You don't have to explicitly use**att.ContainerComposition**.

As a**GuiInstanceRootObject**, components can also be added to a window.

Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Xml/Instance_Window)for details.

## GuiControlHost Properties

### Focused (WindowGotFocus, WindowLostFocus)

A focused window receives keyboard inputs. It will deliver all inputs to the focused control, except for**ALT**sequence.

### Activated (WindowActivated, WindowDeactivated)

A focused window must be activated.

Calling**SetParent**of a control host's**NativeWindow**makes a becomes another window's sub window.

### ShowInTaskBar

The default value is different in different sub classes.

When this property is set to**true**, a icon (and probably with text) appears in the task bar.

### EnableActivate

The default value for this property is**true**.

When this property is set to**false**, this window is not allowed to be activated by user interaction. But this property will be automatically set to**false**when the window is required to be activated by code. For example, calling**Show**instead of**ShowDeactivated**, or calling**SetActivated**.

### TopMost

The default value for this property is**false**.

When this property is set to**true**, it stays in front of all windows whose**TopMost**is**false**.

### ClientSize

This property is the client size of a window.

The value doesn't change according to DPI or text scaling settings. If text scaling is set to 200%, a 320x240 client size is taking 640x480 pixels in the screen.

### Location

This property is the location of a window.

This value changes acfording to DPI or text scaling settings. The coordinate is in screen space. In a multiple monitors computer, a visible window could have negative numbers in its location.

**SetBounds**could be called to set**Location**and**ClientSize**at the same time.

### TimerManager

This property returns a manager object that runs tasks repeatedly until the task itself tells the manager object to stop.

[Animations](../../../.././gacui/xmlres/tag_animation.md)are built on top of it.

### Opening

When this property is**true**, the window is visible.

## GuiControlHost Events

### WindowOpened

This event is raised right after a window is visible.

### WindowClosing

This event is raised before a window is closed.

The second argument for this event is a**GuiRequestEventArgs**. By setting the**cancel**field to**true**, it stops then window from being closed.

### WindowReadyToClose

This event is raised right before a window is closed when**WindowClosing**does not cancel the operation.

### WindowClosed

This event is raised right after a window is closed.

### WindowDestroying

This event is raised before a window is being deleted. At this moment, everything in the window is still alive.

## Displaying a GuiControlHost

You could**Show**or**ShowDeactivated**a window without changing its size.

You could also**ShowRestored**,**ShowMaximized**or**ShowMinimized**a window.

To make a window invisible, you could call**Hide**or**Close**. Calling**Close**on the main window cause the application to begin an existing process. These two functions behaves the same for other windows.

## Controlling \<Window\> Border

The following properties control what components should appear in the window's border:
- MaximizedBox
- MinimizedBox
- Border
- SizeBox
- IconVisible
- TitleBarA window's control template could accept or reject such changing by setting the following properties in**\<WindowTemplate/\>**:
- MaximizedBoxOption
- MinimizedBoxOption
- BorderOption
- SizeBoxOption
- IconVisibleOption
- TitleBarOptionThey could be**AlwaysTrue**,**AlwaysFalse**or**Customizable**.

### Sample


- Source code:[control_basic_window](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_window/Resource.xml)
- ![](https://gaclib.net/doc/gacui/control_basic_window.gif)

## Displaying a \<Window\>

The following functions displays a**\<Window/\<**as a sub window:
- **ShowModal**: This function makes a window visible as a model window. A model window will disable its owner when it is visible. The second argument will be called as a callback after this window is closed, and then its owner window becomes enabled.
- **ShowModalAndDelete**: Just like**ShowModal**, but after it is closed, the window will be deleted. Please check out[this tutorial project](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Xml/Instance_MultipleWindows)for details. It calls this function in C++ by specifying a function name in the button's**Clicked**event. With**ref.CodeBehind="true"**, a place holder is generated in[this separated file](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Xml/Instance_MultipleWindows/UI/Source/MainWindow.cpp).
- **ShowModalAsync**: Just like**ShowModal**, but it returns a**system::Async^**(or**Ptr\<reflection::description::IAsync\>**in C++). This function can be used in[$Await](../../../.././workflow/lang/coroutine_async.md), it simplies**Workflow**scripts in GacUI XML Resource a lot. Please check out[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_Controls/AddressBook/UI/Resource.xml)and[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/DocumentEditorBase.xml)for details. They use**$Await window.ShowModalAsync(mainWindow);**followed by**delete window;**as an alternative way to**ShowModalAndDelete**without involving lambda expressions as callbacks.

## \<Window\>::ClipboardUpdated Event

This event is raised when the content of the clipboard is changed.

It happens before**clipboardNotify**in any composition inside this window.

