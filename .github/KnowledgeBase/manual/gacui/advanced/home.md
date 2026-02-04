# Advanced Topics

Here are advanced topics about using GacUI. Creating a tool using GacUI doesn't always involve these topic, but they are needed for applications with professional user interfaces.


- `empty list item`
  **Interop with C++ View Model**
  Only UI objects are available to**Workflow**script. If you want your application to do**real thing**, which usually requires interop with resources and the operating system, You would like to use a native language to complete these jobs.
  In this section, it introduces a way to let**Workflow**in GacUI XML Resource be able to talk to your own C++ classes and functions.
- `empty list item`
  **Data Bindings**
  Data binding is an important part of the Model-View-ViewModel pattern. It models the application logic (ViewModel) in a way that is familiar to software engineers. You can use whatever data structure you like, not only a class or an interface could be connected to the UI, but also list and tree data structures using data bindings.
  ViewModel becomes testable alone, and also very easy to interop with the UI.
- `empty list item`
  **Animations**
  In GacUI 1.0, you still need to write code to control the detail of animations. But with the animation mechanism with data bindings, GacUI does all the rest. You don't need to worry about how to update UI objects.
- `empty list item`
  **Localization**
  If your application will be used by people using different languages, The localization mechanism is just for you. With the help of data bindings, your application is able to switch languages on the fly, and it requires nothing but specifying UI text separately.
- `empty list item`
  **Cross XML Resource References**
  When your application grows big, it becomes difficult to control dependencies between UI objects and view models. GacUI allows different resource file to depend on each other.
  A GacUI XML Resource could use not only data resources, but also generated classes and interfaces from anothre resource file. The dependency also help GacUI to introduce incremental build, which significantly improve the build performance.
- `empty list item`
  **ALT Sequence and Control Focus**
  Switching focus between controls is an important part of UI accessibility. By pressing**ALT**on a GacUI application, the UI allow you to locate a control using a customizable key sequence.
- `empty list item`
  **TAB and Control Focus**
  Switching focus between controls is an important part of UI accessibility. By pressing**TAB**on a GacUI application, it jumps from one control to another, if the focused control doesn't take**TAB**as key input.
- `empty list item`
  **Creating New List Controls**
  GacUI already offers a set of list controls with powerful data bindings. But if it still don't meet your requirement, you could deeply customize a list control using C++.
  In this section you will learn how to customize: - Keyboard and mouse controlling - Virtual list item algorithm - Layouting list items - Data accessing - Creating your own way to performa data binding - and more ...
- `empty list item`
  **Porting GacUI to Other Platforms**
  GacUI decouples with any OS, sub systems and even renderer, from the first day it is created. If you like GacUI but it doesn't run on your platform, you could port GacUI to it, not by changing GacUI, but only creating a new adaptor to talk to the OS!

