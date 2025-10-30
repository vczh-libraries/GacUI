# GacUI Object Models

Most of the important GacUI objects can be categorized into: - **Elements**: An element is a renderable graphics object, including shapes, and texts. - **Compositions**: A composition controls layout with user input events, deciding bounds of elements. - **Controls**: A control provides a set of functionalities that applications most likely need. - **Control Templates**: A control template is designed for a certain kind of control, controlling how controls look like. - **Item Templates**: An item template works like a control template, but it controls repetitive parts in controls, like list items.

## Organization of Objects

GacUI maintains objects in two trees in a window. The boundary and user inputs are controlled by **GuiWindowComposition** and **GuiWindow**.

### Composition Tree

**GuiWindowComposition** is the root object of a composition tree.

All visible effects and user inputs are handled by a composition tree. A composition could has other compositions as its child compositions, and it could also has up to one element as its child element.

When a composition has an element, the element fills the whole area of this composition, telling this composition about size constraints.

A composition can choose to listen to all size constraints of child compositions and the child element. Different kinds of compositions have its own algorithms to controls where to put child compositions, so that child compositions can be required to organized like a list, a table, or even complex behaviors.

When a composition is hidden, all child composition are also hidden, but the size constraints are still there, they are not ignored while doing layouts.

### Control Tree

**GuiWindow** is the root object of a control tree. A control controls behaviors of its child controls.

For example, when a control is disabled, all child controls are also disabled. When a control changes its font, all child controls are also get their fonts changed, unless for those who are explicitly assigned with other fonts.

When a control is focused, it removes the focus from the last control in the same window. In this case, its **focusable composition** receives all keyboard input.

### Control Templates

Visible effects are controlled by control templates or item templates. A control just be assigned a control template when it is created, but another control template can be reassigned to the same control to replace the old control template.

A control template is a composition, but in order to create visible effects for a control when it is performing some work or handling user inputs, there is more thing in a control template that is specifically designed for a control.

If you assign a control template to a wrong kind of control, it crashes.

### Item Templates

Some list controls could also be assigned with an item template. Multiple instances of the assigned item template are created when multiple list items are required to display in this control. When a list item is scrolled out of the control, its item template is deleted.

Instead of relying on compositions, list controls have its own configurable way to organize items, ensuring that an item only takes a minimum resource when it is out of sight.

### Putting Controls and Compositions Together

A control has a **bounds composition** and a **container composition**, which are explicitly specified by its control template.

The **bounds composition** is the bounding composition of this control. When a control is placed inside another control or composition, the **bounds composition** is placed inside the parent control or composition.

The **container composition** is a place to store controls a compositions that are wanted to be place inside this control. When a control or a composition is placed inside this control, they are placed inside the **container composition** of this control.

Whenever you change the parent-child relationships of controls or compositions, the control tree and the composition tree are automatically maintained.

## Life Cycle

When a composition is deleted, all child compositions are also deleted.

When a control is deleted, all child controls are also deleted. All compositions inside a deleted control will also be deleted.

Most of the cases you only need to delete a window, which destroys everything. But sometimes when you delete a composition that is not the **bounds composition** of a control while some children are. And compositions or controls could be still staying in their parent objects when you are deleting them. it could cause memory leaks if you just call the **delete** operator of C++.

In order to prevent from all these issues, **SafeDeleteControl** and **SafeDeleteComposition** are highly recommended to use. They handle every possible cases.

It is safe to delete a **GuiWindow** by using the **delete** operator of C++.

## Global Objects

There are also some global objects in GacUI applications.

**GuiApplication** controls life cycles of all windows, and also offers features so that the UI thread and other threads are able to communicate.

**INativeController** exposed operating system features. When a GacUI application is running on different platforms, different **INativeController** kicks in. To port GacUI to a new platform, or to run GacUI in a virtual environment (e.g. running all GacUI objects in a window that completely controlled by you, typically when developing games), you need to provide your own implementation of **INativeController**.

## Thread Safety

All GacUI objects and their methods are not thread safe, and almost all of them **must only be accessed by the UI thread**, even for readonly operations.

One exception is that, when a composition or a control has not been added to a window, it can be operated by a non-UI thread. But it is still not safe when multiple threads are accessing the same object at the same time.

