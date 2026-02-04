# Accessing Controls

When a window or a custom control is created from GacUI XML Resource, and you need the pointer to a specific sub object, you don't have to**public**them. Here are functions to access a specific control, composition or component by its**ref.Name**attribute, or to access a control by its text, defined in the**vl::presentation::controls**namespace.

## FindObjectByName(rootObject, name)

The first parameter must be an object that created by an \<Instance/\>, e.g. a window or a custom control. The second parameter must be an object that directly inside that particular \<Instance/\> with**ref.Name**attribute assigned. If nothing is found, the function crashes.
```c++
auto button = FindObjectByName(window, L"buttonSomething");
```


## TryFindObjectByName(rootObject, name)

Same as**FindObjectByName**, but it returns null if nothing is found.

## FindObjectByName(rootObject, name1, name2, ...)

This is a shortcut of a chain call: to**... FindObjectByName(FindObjectByName(rootObject, name1), name2) ...**. It is useful when your target object is inside nested custom controls in a window, you need to provide all**ref.Name**attributes in the hierarchy.**IMPORTANT:**such custom controls must be created by \<Instance/\>.

## FindControlByText\<Type\>(control, text)

The first parameter must be a control. The is the text of the target sub control. If nothing is found matching the type and the text, the function crashes. Otherwise it would return a random one within all matching controls. Unlike**FindObjectByName**, you can use this function to search into nested custom controls.
```c++
auto button = FindControlByText<GuiButton>(window, L"Click Me!");
```


## TryFindControlByText\<Type\>(control, text)

Same as**FindControlByText**, but it returns null if nothing is found.

