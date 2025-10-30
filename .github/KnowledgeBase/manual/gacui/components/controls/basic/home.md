# Basic Controls

Controls visualize data and react to user input. **\<Window/\>** is also a control.

Use **SafeDeleteControl** and **SafeDeleteComposition** to delete a control or a composition containing controls. Otherwise memory leaks may happen.

A control has three main composition properties: - **BoundsComposition**: This is a composition of the control's border. To make a control appear inside another composition, just simply adds **BoundsComposition** as a child composition to that composition. - **ContainerComposition**: This is a composition of the control's inner border. To make another composition inside a control, just simply adds that composition as a child composition to **ContainerComposition**. - **FocusComposition**: If a control is focusable, this property returns a non-null value. **FocusComposition** receives [most of composition events](../../../.././gacui/components/compositions/events.md). Ideally, events provided by the control itself is sufficient enough, **FocusComposition** is designed for the implementation of the control and the control template. **BoundsComposition** and **ContainerComposition** will not change during a control's life time. **FocusComposition** changes when the control template changes.

Many properties are shared across control classes.

## States

The following properties and events control states for a control.

### VisuallyEnabled (VisuallyEnabledChanged)

When a control's **Enabled** is set to **false**, **VisuallyEnalbed** of the control itself and all direct or indirect child controls is also set to **false**.

A visually disabled control doesn't react to user input.

**VisuallyEnabled** of the control template automatically updates when this property is changed.

### Enabled (EnabledChanged)

The default value for this property is **true**.

When a control's **Enabled** is set to **false**, **VisuallyEnalbed** of the control itself and all direct or indirect child controls is also set to **false**.

A visually disabled control doesn't react to user input.

#### Sample

- Source code: [control_basic_enabled](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_enabled/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_basic_enabled.gif)

### Visible (VisibleChanged)

The default value for this property is **true**.

When a control's **Visible** is set to **false**, the control disappears, but it still affect the minimum size of all parent compositions.

### Text (TextChanged)

The default value for this property is **""**.

This property controls the text displayed on the control.

**Text** of the control template automatically updates when this property is changed.

### DisplayFont (DisplayFontChanged)

**DisplayFont** reflects the expected font for this control.

The expected font for this control is, either the **Font** of this control if it is not empty, or the nearest parent control's non-empty **Font**. If no font is found all the way to the top level control, it becomes **GetCurrentController()-\>ResourceService()-\>GetDefaultFont()**

**DisplayFont** of the control template automatically updates when this property is changed.

### Font (FontChanged)

The default value for this property is **null**.

When a control's **Font** is changed, **DisplayFont** of the control itself and all direct or indirect child controls with an empty **Font** is also changed,

#### Sample

- Source code: [control_basic_font](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_font/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_basic_font.gif)

### Context (ContextChanged)

The default value for this property is **null**.

This property offers a way to communicate between a control and its control template or item templates. When the value is changed, the **Context** property of its control template or item templates will be changed to that value.

Please check out [ this tutorial project ](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_Controls/TriplePhaseImageButton) as an example. In this project, it creates a new control **TriplePhaseButton** which has different images for different states. This new control passes itself to the **Context** of a button inside this control, and than the control template of this button communicates directly to this new control, to make the button behaves like a triple-phases button (unlike double-phases buttons like check boxes).

### Focused (FocusedChanged)

The default value for this property is **false**.

Calling **SetFocus** to try to move focus to this control. A focused control accepts keyboard and character input to the window.

### Tag

The default value for this property is **null**.

This property accepts any value. It does nothing but associates a value to a control.

## Control Template

The following properties and events control the control template for a control.

### ControlThemeName (ControlThemeNameChanged)

A **ThemeTemplates** contains control template implementations for controls. Each control template factory fields are optional. Multiple **ThemeTemplates** could be registered and unregistered using **RegisterTheme** and **UnregisteredTheme**. The later registered **ThemeTemplates** has a higher pririty.

When calling **GetCurrentTheme-\>CreateStyle(themeName)**, it goes through all registered theme and find if any **ThemeTemplates** has an implementation for the specified **themeName**.

**ControlThemeName** property specifies which control template should be used for this template.

Different control classes have different default values for this property. In most of the cases this property is not expected to be changed during the runtime.

When **ControlThemeName** is changed and **ControlTemplate** is empty, the control template for this control will be changed immediately. **SetControlThemeNameAndTemplate** is for changing both **ControlThemeName** and **ControlTemplate** at the same time to improve the performance.

### ControlTemplate (ControlTemplateChanged)

This property is used when you doesn't want the registered control template for this control.

The default value for this property is empty. The value for this property must be a **vl::Func** for the function type **templates::GuiControlTemplate*(const reflection::description::Value&)**.

Please check out [ this page ](../../../.././gacui/xmlres/instance/properties.md) for details about using this property in GacUI XML Resource.

When **ControlTemplate** is assigned with an empty value, **ControlThemeName** kicks in and update the control template for this control. When **ControlTemplate** is assigned with a non-empty value, this property is evaluated to create a control template for this control. **SetControlThemeNameAndTemplate** is for changing both **ControlThemeName** and **ControlTemplate** at the same time to improve the performance.

### ControlTemplateObject

This property returns the currently used control template instance.

A control template is also a composition, and it has **BoundsComposition** and **ContainerComposition** too. But they are not same values as the same property of the control.

**FocusComposition** is the same to control and control templates.

## Context

The following properties and events provide access of the control's context.

### Parent (ControlSignalTrigerred with ParentLineChanged)

A control appears inside its **Parent**, otherwise this property returns **null** (usually for windows and menus).

To add a control directly inside another control, just simply call **AddChild**, which adds the child control's **BoundsComposition** to the parent control's **ContainerComposition**. It is also OK when the child control's **BoundsComposition** is not the direct child composition of the parent control's **ContainerComposition**.

**GetChildrenCount**, **GetChild** and **HasChild** provide access for child controls.

### RelatedControlHost (ControlSignalTrigerred with ParentLineChanged)

**RelatedControlHost** is the root parent control for a control, which is usually a window or a menu. otherwise this property returns **null** (usually for windows and menus).

## Behaviors

The following properties and events control the behavior of a control.

### AcceptTabInput

The default value for this property **true**.

When this property is set to **true**, pressing **TAB** key when this control is focused type a **TAB** character into this control.

### TabPriority

The default value for this property is **-1**.

Pressing **TAB** in a window cause the focus to move between all controls that have a non-negative value **TabProperty** from low to high.

### Alt (AltChanged)

The default value for this property is **""**.

Pressing **ALT** causes a window prints all acceptable keys to move focus to all controls that have a non-empty **Alt**.

**Alt** could contain multiple upper-cased letters and digits, which is a sequence that moves the focus to this control after pressing **ALT**.

For controls that are not focusable but manage focus of direct child controls (like Tab control), or for controls that manage focus of child controls in another window (like menus), **SetActivatingAltHost** is an important function to activate this feature for this control. This function is only for control authors to use. Pressing **ESC** or **BACKSPACE** goes back to the upper level window or control during a **ALT** sequence.

### TooltipControl

The default value for this property is **null**.

A legal value for this property must be a control that is not contained in any composition. When the mouse stops on a control for a while, its **TooltipControl** will appear at the mouse cursor.

When a control is deleted, it also deletes its **TooltipControl**.

**DisplayTooltip** and **CloseTooltip** could be call to manually control a tooltip proactively.

### TooltipWidth

The default value for this property is **0**.

This property controls the width of **TooltipControl**.

#### Sample

- Source code: [control_basic_tooltip](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_tooltip/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_basic_tooltip.gif)

## Service Objects

**AddService** (ControlSignalTrigerred with ServiceAdded) associates a nullable service object to this control with an identifier that could be any string.

**QueryService** or **QueryTypedService\<T\>** searches for a service object using an identifier all the way up to the top-level control. If multiple service objects are found, the one in the nearest parent control wins.

Some service identifiers are hard-coded in controls, even if you try to associate a service objects using thses identifiers, it takes no effect on some controls: - vl::presentation::compositions::IGuiAltAction - vl::presentation::compositions::IGuiAltActionContainer - vl::presentation::compositions::IGuiAltActionHost - vl::presentation::controls::IGuiMenuService - vl::presentation::controls::IGuiMenuDropdownProvider - vl::presentation::controls::IToolstripUpdateLayoutInvoker - vl::presentation::compositions::IGuiTabAction Predefined services are full name of GacUI classes, it is easy to avoid conflict with your own service objects.

