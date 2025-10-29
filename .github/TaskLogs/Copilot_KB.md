# !!!KNOWLEDGE BASE!!!

# DOCUMENT REQUEST

## TOPIC

List all things that one should do if he want to add a new control to GacUI. It may includes:
- Adding a new control template definition and control template name
- Define and register a new control template implementation
- A sample code for a new control
- Register reflection for the control
- Make the control visible to GacUI XML Compiler
- Any other thing you find that is necessary

Existing controls are developed following the same pattern, you should carefully read them before making any guidance.

### Insight

Based on my analysis of `GuiButton` and `GuiSelectableButton`, here is the comprehensive process to add a new control to GacUI:

## ASK

I would like you to refine some mistakes in the `Insight` topic including:

- `10. Additional Files to Update` is not necessary, please remove. By saying unnecessary I don't mean vcxproj files don't need to be touch, the real reason is there are multiple vcxproj and vcxitems files and user should make their choice and there is no standard answer. But if you find any general suggestion, you can replace the content with your new finding.
- You have identified many important points about create a new control from stratch, but what about a new control inheriting from another?
- Add a new sub section for a simple demo about how to define a control. I think existing sections are very useful, this item is for adding a new section, not for updating existing sections.

### Insight

**Refinement completed:**

1. **Section 10 replaced** with comprehensive guidance on creating controls that inherit from other controls, covering:
   - Class definition differences (inheriting from parent control instead of `GuiControl`)
   - Constructor patterns (calling parent constructor)
   - Template inheritance (parent template as base)
   - Reflection registration (using `CLASS_MEMBER_BASE` with parent)
   - Minimal changes approach (avoiding duplicate work)
   - Real example from `GuiSelectableButton` inheriting from `GuiButton`

2. **Section 13 added** with a complete minimal control demo showing:
   - Header file structure with essential components
   - Implementation file with lifecycle methods and event handling
   - Template declaration with property macro
   - Theme registration
   - Reflection setup
   - XML loader registration
   - Explanation of the core pattern for all controls

3. **Key insights for inherited controls:**
   - Don't re-attach parent's event handlers
   - Attach to parent's events to extend behavior
   - Only override lifecycle methods if adding new behavior
   - Template inherits from parent template
   - Reflection uses `CLASS_MEMBER_BASE(ParentControl)` instead of `CLASS_MEMBER_BASE(GuiControl)`

The original content remains intact with these additions providing deeper guidance for both inheritance scenarios and practical implementation examples.

#### 1. Control Class Definition (in `Source/Controls/*.h`)

The control class must:
- Inherit from `GuiControl` (or another control base class) and `Description<YourControl>`
- Use `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(TemplateName, BaseControlType)` macro to specify the template type
- Declare member variables for control state
- Override lifecycle methods from `GuiControl`:
  - `BeforeControlTemplateUninstalled_()` - cleanup before template removal
  - `AfterControlTemplateInstalled_(bool initialize)` - setup after template installation
  - `OnParentLineChanged()` - handle parent hierarchy changes
  - `OnActiveAlt()` - handle ALT key activation
  - `IsTabAvailable()` - control TAB navigation availability
- Attach event handlers in constructor to `boundsComposition->GetEventReceiver()` for mouse/keyboard events
- Define public events using `compositions::GuiNotifyEvent`
- Define properties with getters/setters
- Take a `theme::ThemeName` parameter in constructor and pass it to base class

Example pattern from `GuiButton`:
- Constructor: `GuiButton(theme::ThemeName themeName) : GuiControl(themeName)`
- Template macro: `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ButtonTemplate, GuiControl)`
- Events: `BeforeClicked`, `Clicked`, `AfterClicked`
- State management: `UpdateControlState()` method to sync with template

#### 2. Control Implementation (in `Source/Controls/*.cpp`)

Implement:
- Constructor: Initialize state, set up event handlers on `boundsComposition`
- Destructor: Cleanup (usually minimal, automatic cleanup happens)
- `BeforeControlTemplateUninstalled_()`: Clear template-specific state
- `AfterControlTemplateInstalled_(bool initialize)`: Call template methods like `TypedControlTemplateObject(true)->SetState(controlState)`
- Event handlers: Mouse events (leftButtonDown, leftButtonUp, mouseEnter, mouseLeave), keyboard events (keyDown, keyUp)
- Property getters/setters
- Call `TypedControlTemplateObject(true)` to access the typed template object

#### 3. Control Template Definition (in `Source/Controls/Templates/GuiControlTemplates.h`)

Add three entries:

a) **Template declaration in `GUI_CONTROL_TEMPLATE_DECL` macro**:
```cpp
F(GuiYourTemplate, GuiBaseTemplate)
```

b) **Template properties macro definition**:
```cpp
#define GuiYourTemplate_PROPERTIES(F)\
    F(GuiYourTemplate, PropertyType, PropertyName, DefaultValue)
```

c) **Forward declaration and class declaration**:
The macros `GUI_TEMPLATE_CLASS_FORWARD_DECL` and `GUI_TEMPLATE_CLASS_DECL` are applied to generate these automatically.

Properties are defined using the `F` macro with: template class name, property type, property name, default value.

#### 4. Control Template Implementation (in `Source/Controls/Templates/GuiControlTemplates.cpp`)

The template implementation is auto-generated by:
```cpp
GUI_CONTROL_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_IMPL)
```

This macro expands to create:
- Property getter/setter implementations with change events
- Constructor that initializes event handlers
- Destructor that calls `FinalizeAggregation()`

#### 5. Theme Name Registration (in `Source/Application/Controls/GuiThemeManager.h`)

Add the control to `GUI_CONTROL_TEMPLATE_TYPES` macro:
```cpp
#define GUI_CONTROL_TEMPLATE_TYPES(F) \
    ... existing entries ...\
    F(YourTemplate, YourControl)
```

This generates the corresponding `theme::ThemeName::YourControl` enum value.

#### 6. Reflection Registration (in `Source/Reflection/TypeDescriptors/`)

Three steps are required:

a) **Add to type list** (in `GuiReflectionPlugin.h`):
Add to `GUIREFLECTIONCONTROLS_CLASS_TYPELIST` macro:
```cpp
F(presentation::controls::GuiYourControl)
```

b) **Register control class** (in `GuiReflectionControls.cpp`):
```cpp
BEGIN_CLASS_MEMBER(GuiYourControl)
    CLASS_MEMBER_BASE(GuiBaseControl)
    CONTROL_CONSTRUCTOR_CONTROLT_TEMPLATE(GuiYourControl)
    
    CLASS_MEMBER_GUIEVENT(EventName)
    CLASS_MEMBER_PROPERTY_FAST(PropertyName)
    CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(PropertyWithEvent)
    CLASS_MEMBER_METHOD(MethodName, {L"param1" _ L"param2"})
END_CLASS_MEMBER(GuiYourControl)
```

c) **Register template class** (in `GuiReflectionTemplates.cpp`):
Templates are auto-registered via the `GUI_CONTROL_TEMPLATE` macro expansion which applies to all templates declared in `GUI_CONTROL_TEMPLATE_DECL`.

#### 7. Instance Loader Registration (in `Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp`)

Add to the loader registration in `IGuiPlugin::Load()`:

For a normal control:
```cpp
ADD_TEMPLATE_CONTROL(GuiYourControl, YourControl);
```

For a virtual control (one that uses another control's implementation but different theme):
```cpp
ADD_VIRTUAL_CONTROL(VirtualName, GuiActualControl, ThemeName);
```

This uses the `GuiTemplateControlInstanceLoader<T>` template class to register the control with the instance loader manager.

#### 8. Control Header File Inclusion

Add includes to:
- `Source/Controls/IncludeForward.h` - forward declaration
- `Source/Controls/IncludeAll.h` - full header inclusion

#### 9. Sample Pattern from GuiButton

Key patterns observed:
- State machine pattern: `controlState` variable tracks button visual state (Normal, Active, Pressed)
- Event chaining: `BeforeClicked` → `Clicked` → `AfterClicked`
- Mouse tracking: Separate flags for `mousePressingDirect`, `mousePressingIndirect`, `mouseHoving`
- Keyboard support: SPACE and ENTER keys trigger click
- Focus integration: `SetFocusableComposition(boundsComposition)` and `autoFocus` property
- Template communication: Call template setters like `SetState()`, `SetSelected()` in `AfterControlTemplateInstalled_`

#### 10. Creating a Control that Inherits from Another Control

When creating a control that inherits from another control (e.g., `GuiSelectableButton` inherits from `GuiButton`):

**Class Definition Differences:**
- Inherit from the parent control instead of `GuiControl`: `class GuiDerivedControl : public GuiParentControl, public Description<GuiDerivedControl>`
- Still use `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE` but specify parent control as base: `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(DerivedTemplate, GuiParentControl)`
- Do NOT re-attach event handlers that parent already handles (e.g., don't re-attach `leftButtonDown` if parent handles it)
- Attach to parent's events instead if needed (e.g., `GuiSelectableButton` attaches to `GuiButton::AfterClicked`)

**Constructor Pattern:**
- Call parent constructor: `GuiDerivedControl(theme::ThemeName themeName) : GuiParentControl(themeName)`
- Initialize additional events on `boundsComposition` (inherited from parent)
- Attach handlers to parent's events if extending behavior

**Template Inheritance:**
- Template class should inherit from parent template: `DerivedTemplate` properties macro can be separate
- In `GuiControlTemplates.h`, add `F(GuiDerivedTemplate, GuiParentTemplate)` to `GUI_CONTROL_TEMPLATE_DECL`
- Define `GuiDerivedTemplate_PROPERTIES(F)` macro with additional properties

**Reflection Registration:**
- Use `CLASS_MEMBER_BASE(GuiParentControl)` instead of `CLASS_MEMBER_BASE(GuiControl)`
- Still use `CONTROL_CONSTRUCTOR_CONTROLT_TEMPLATE(GuiDerivedControl)`
- Only register new properties/events/methods introduced by derived class

**Minimal Changes:**
- Parent already handles basic lifecycle (`BeforeControlTemplateUninstalled_`, etc.) - override only if needed
- Parent's event handlers are inherited - no need to re-implement
- Focus on adding new functionality, not repeating parent's work

**Example Pattern from `GuiSelectableButton`:**
- Extends `GuiButton` with selection state
- Attaches to parent's `AfterClicked` event to toggle selection
- Adds new properties: `GroupController`, `AutoSelection`, `Selected`
- Adds new template property: `Selected` in `SelectableButtonTemplate`
- Template calls both parent methods and new methods: `SetState()` (from parent) and `SetSelected()` (new)

#### 11. Template Property Access Pattern

Controls access template properties via:
- `TypedControlTemplateObject(true)` - gets the typed template with existence check
- `TypedControlTemplateObject(false)` - gets the typed template without check
- Template properties have auto-generated `Get/Set` methods and `Changed` events

#### 12. Control Template Macro System

The macro system provides:
- `GUI_TEMPLATE_CLASS_DECL`: Generates class declaration with properties
- `GUI_TEMPLATE_CLASS_IMPL`: Generates implementation (constructor, destructor, property accessors)
- `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE`: Links control to its template type with automatic casting
- Property macros: Generate private field, getter, setter, and change event

#### 13. Simple Demo: Minimal Control Definition

Here's a simplified example showing the minimal code structure for creating a custom control:

**Step 1: Header File (`Source/Controls/GuiMyControls.h`)**
```cpp
class GuiMyControl : public GuiControl, public Description<GuiMyControl>
{
    GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(MyControlTemplate, GuiControl)
protected:
    // State variables
    bool myState = false;
    
    // Lifecycle overrides
    void BeforeControlTemplateUninstalled_() override;
    void AfterControlTemplateInstalled_(bool initialize) override;
    
    // Event handlers
    void OnMouseClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
    
public:
    GuiMyControl(theme::ThemeName themeName);
    ~GuiMyControl();
    
    // Events
    compositions::GuiNotifyEvent StateChanged;
    
    // Properties
    bool GetMyState();
    void SetMyState(bool value);
};
```

**Step 2: Implementation File (`Source/Controls/GuiMyControls.cpp`)**
```cpp
void GuiMyControl::BeforeControlTemplateUninstalled_()
{
    // Cleanup before template removal
}

void GuiMyControl::AfterControlTemplateInstalled_(bool initialize)
{
    // Sync state to template
    TypedControlTemplateObject(true)->SetMyState(myState);
}

void GuiMyControl::OnMouseClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
{
    SetMyState(!myState);
}

GuiMyControl::GuiMyControl(theme::ThemeName themeName)
    : GuiControl(themeName)
{
    StateChanged.SetAssociatedComposition(boundsComposition);
    boundsComposition->GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiMyControl::OnMouseClick);
}

GuiMyControl::~GuiMyControl()
{
}

bool GuiMyControl::GetMyState()
{
    return myState;
}

void GuiMyControl::SetMyState(bool value)
{
    if (myState != value)
    {
        myState = value;
        TypedControlTemplateObject(false)->SetMyState(myState);
        StateChanged.Execute(compositions::GuiEventArgs(boundsComposition));
    }
}
```

**Step 3: Template Declaration (`Source/Controls/Templates/GuiControlTemplates.h`)**
```cpp
// Add to GUI_CONTROL_TEMPLATE_DECL macro:
F(GuiMyControlTemplate, GuiControlTemplate)

// Define template properties:
#define GuiMyControlTemplate_PROPERTIES(F)\
    F(GuiMyControlTemplate, bool, MyState, false)
```

**Step 4: Theme Registration (`Source/Application/Controls/GuiThemeManager.h`)**
```cpp
// Add to GUI_CONTROL_TEMPLATE_TYPES macro:
F(MyControlTemplate, MyControl)
```

**Step 5: Reflection (`Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp`)**
```cpp
BEGIN_CLASS_MEMBER(GuiMyControl)
    CLASS_MEMBER_BASE(GuiControl)
    CONTROL_CONSTRUCTOR_CONTROLT_TEMPLATE(GuiMyControl)
    
    CLASS_MEMBER_GUIEVENT(StateChanged)
    CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(MyState)
END_CLASS_MEMBER(GuiMyControl)
```

**Step 6: XML Loader (`Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp`)**
```cpp
// Add in IGuiPlugin::Load():
ADD_TEMPLATE_CONTROL(GuiMyControl, MyControl);
```

This demonstrates the core pattern - each control has:
- A C++ class managing state and events
- A template defining visual properties
- Reflection for runtime access
- XML loader for declarative usage
- Theme integration for consistent styling

#### Summary of Files to Modify

1. `Source/Controls/Gui*Controls.h` - Control class declaration
2. `Source/Controls/Gui*Controls.cpp` - Control implementation  
3. `Source/Controls/Templates/GuiControlTemplates.h` - Template declaration and properties
4. `Source/Controls/Templates/GuiControlTemplates.cpp` - Template implementation (usually just macro)
5. `Source/Application/Controls/GuiThemeManager.h` - Theme name registration
6. `Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h` - Add to type list
7. `Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp` - Control reflection
8. `Source/Reflection/TypeDescriptors/GuiReflectionTemplates.cpp` - Template reflection (automatic)
9. `Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp` - XML loader registration
10. `Source/Controls/IncludeForward.h` and `IncludeAll.h` - Header organization

# DRAFT-LOCATION

This document should be added to the **GacUI** project in `Index.md` under the **Design Explanation** section.

It will be a new topic entry that covers the complete process of adding a new control to the GacUI framework.

# DRAFT-TITLE

Adding a New Control

# DRAFT-CONTENT

## Overview

Adding a new control to GacUI requires coordinated changes across multiple files to integrate the control into the class hierarchy, template system, theme management, reflection system, and XML compiler. This document provides a comprehensive guide covering control classes, templates, inheritance patterns, registration, and a minimal working example.

## Control Class Definition

### Header File Structure (`Source/Controls/*.h`)

The control class must:

- **Inherit from base class**: `GuiControl` (or another control) and `Description<YourControl>` for reflection
- **Specify template type**: Use `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(TemplateName, BaseControlType)` macro
- **Declare state variables**: Member variables to track control state
- **Override lifecycle methods** from `GuiControl`:
  - `BeforeControlTemplateUninstalled_()` - cleanup before template removal
  - `AfterControlTemplateInstalled_(bool initialize)` - setup after template installation
  - `OnParentLineChanged()` - handle parent hierarchy changes
  - `OnActiveAlt()` - handle ALT key activation
  - `IsTabAvailable()` - control TAB navigation availability
- **Attach event handlers**: In constructor to `boundsComposition->GetEventReceiver()` for mouse/keyboard events
- **Define public events**: Using `compositions::GuiNotifyEvent`
- **Define properties**: With getters/setters
- **Accept theme parameter**: Constructor takes `theme::ThemeName` parameter and passes to base class

**Example pattern from `GuiButton`:**

```cpp
class GuiButton : public GuiControl, public Description<GuiButton>
{
    GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ButtonTemplate, GuiControl)
protected:
    ButtonState controlState;
    void BeforeControlTemplateUninstalled_() override;
    void AfterControlTemplateInstalled_(bool initialize) override;
    void OnMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
    void UpdateControlState();
public:
    GuiButton(theme::ThemeName themeName);
    ~GuiButton();
    
    compositions::GuiNotifyEvent BeforeClicked;
    compositions::GuiNotifyEvent Clicked;
    compositions::GuiNotifyEvent AfterClicked;
    
    bool GetAutoFocus();
    void SetAutoFocus(bool value);
};
```

## Control Implementation

### Implementation File (`Source/Controls/*.cpp`)

Implement:

- **Constructor**: Initialize state, set up event handlers on `boundsComposition`
  - Pattern: `GuiYourControl(theme::ThemeName themeName) : GuiControl(themeName)`
  - Set up events: `eventName.SetAssociatedComposition(boundsComposition)`
  - Attach handlers: `boundsComposition->GetEventReceiver()->eventName.AttachMethod(this, &GuiYourControl::Handler)`
- **Destructor**: Cleanup (usually minimal, automatic cleanup happens)
- **`BeforeControlTemplateUninstalled_()`**: Clear template-specific state
- **`AfterControlTemplateInstalled_(bool initialize)`**: Sync state to template
  - Call template methods: `TypedControlTemplateObject(true)->SetState(controlState)`
- **Event handlers**: Mouse events (`leftButtonDown`, `leftButtonUp`, `mouseEnter`, `mouseLeave`), keyboard events (`keyDown`, `keyUp`)
- **Property getters/setters**: Update state and notify template
- **Template access**: Use `TypedControlTemplateObject(true)` to get typed template with existence check, or `TypedControlTemplateObject(false)` without check

**Key patterns from `GuiButton`:**

- State machine pattern: `controlState` variable tracks visual state (Normal, Active, Pressed)
- Event chaining: `BeforeClicked` → `Clicked` → `AfterClicked`
- Mouse tracking: Separate flags for `mousePressingDirect`, `mousePressingIndirect`, `mouseHoving`
- Keyboard support: SPACE and ENTER keys trigger click
- Focus integration: `SetFocusableComposition(boundsComposition)` and `autoFocus` property
- Template communication: Call template setters in `AfterControlTemplateInstalled_`

## Control Template System

### Template Declaration (`Source/Controls/Templates/GuiControlTemplates.h`)

Three required entries:

**1. Add to `GUI_CONTROL_TEMPLATE_DECL` macro:**

```cpp
F(GuiYourTemplate, GuiBaseTemplate)
```

**2. Define template properties macro:**

```cpp
#define GuiYourTemplate_PROPERTIES(F)\
    F(GuiYourTemplate, PropertyType, PropertyName, DefaultValue)
```

Properties are defined using the `F` macro with: template class name, property type, property name, default value.

**3. Forward declaration and class declaration:**

The macros `GUI_TEMPLATE_CLASS_FORWARD_DECL` and `GUI_TEMPLATE_CLASS_DECL` are applied to `GUI_CONTROL_TEMPLATE_DECL` to generate these automatically.

### Template Implementation (`Source/Controls/Templates/GuiControlTemplates.cpp`)

The template implementation is auto-generated by:

```cpp
GUI_CONTROL_TEMPLATE_DECL(GUI_TEMPLATE_CLASS_IMPL)
```

This macro expands to create:
- Property getter/setter implementations with change events
- Constructor that initializes event handlers
- Destructor that calls `FinalizeAggregation()`

## Theme Integration

### Theme Name Registration (`Source/Application/Controls/GuiThemeManager.h`)

Add the control to `GUI_CONTROL_TEMPLATE_TYPES` macro:

```cpp
#define GUI_CONTROL_TEMPLATE_TYPES(F) \
    ... existing entries ...\
    F(YourTemplate, YourControl)
```

This generates the corresponding `theme::ThemeName::YourControl` enum value used in control constructors.

## Reflection Registration

### Three-Step Registration Process

**Step 1: Add to type list** (`Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h`):

Add to `GUIREFLECTIONCONTROLS_CLASS_TYPELIST` macro:

```cpp
F(presentation::controls::GuiYourControl)
```

**Step 2: Register control class** (`Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp`):

```cpp
BEGIN_CLASS_MEMBER(GuiYourControl)
    CLASS_MEMBER_BASE(GuiBaseControl)
    CONTROL_CONSTRUCTOR_CONTROLT_TEMPLATE(GuiYourControl)
    
    CLASS_MEMBER_GUIEVENT(EventName)
    CLASS_MEMBER_PROPERTY_FAST(PropertyName)
    CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(PropertyWithEvent)
    CLASS_MEMBER_METHOD(MethodName, {L"param1" _ L"param2"})
END_CLASS_MEMBER(GuiYourControl)
```

**Step 3: Template registration** (`Source/Reflection/TypeDescriptors/GuiReflectionTemplates.cpp`):

Templates are auto-registered via the `GUI_CONTROL_TEMPLATE` macro expansion applied to all templates declared in `GUI_CONTROL_TEMPLATE_DECL`.

## XML Compiler Integration

### Instance Loader Registration (`Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp`)

Add to loader registration in `IGuiPlugin::Load()`:

**For a normal control:**

```cpp
ADD_TEMPLATE_CONTROL(GuiYourControl, YourControl);
```

**For a virtual control** (uses another control's implementation with different theme):

```cpp
ADD_VIRTUAL_CONTROL(VirtualName, GuiActualControl, ThemeName);
```

This uses `GuiTemplateControlInstanceLoader<T>` to register the control with the instance loader manager, making it available in GacUI XML files.

## Header File Organization

### Include Files

Add includes to:
- `Source/Controls/IncludeForward.h` - forward declaration
- `Source/Controls/IncludeAll.h` - full header inclusion

This ensures proper compilation order and accessibility throughout the framework.

## Creating a Control that Inherits from Another Control

When creating a control that inherits from another control (e.g., `GuiSelectableButton` inherits from `GuiButton`):

### Class Definition Differences

- **Inherit from parent control**: `class GuiDerivedControl : public GuiParentControl, public Description<GuiDerivedControl>`
- **Specify parent in template macro**: `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(DerivedTemplate, GuiParentControl)`
- **Do NOT re-attach parent event handlers**: Don't re-attach `leftButtonDown` if parent already handles it
- **Attach to parent's events instead**: Example - `GuiSelectableButton` attaches to `GuiButton::AfterClicked`

### Constructor Pattern

- **Call parent constructor**: `GuiDerivedControl(theme::ThemeName themeName) : GuiParentControl(themeName)`
- **Initialize additional events**: On `boundsComposition` (inherited from parent)
- **Attach handlers to parent's events**: If extending behavior

### Template Inheritance

- **Template inherits from parent template**: In `GuiControlTemplates.h`, add:
  ```cpp
  F(GuiDerivedTemplate, GuiParentTemplate)
  ```
- **Define additional properties**: Separate `GuiDerivedTemplate_PROPERTIES(F)` macro with new properties

### Reflection Registration

- **Use parent as base**: `CLASS_MEMBER_BASE(GuiParentControl)` instead of `CLASS_MEMBER_BASE(GuiControl)`
- **Still use standard constructor macro**: `CONTROL_CONSTRUCTOR_CONTROLT_TEMPLATE(GuiDerivedControl)`
- **Only register new members**: Properties/events/methods introduced by derived class

### Minimal Changes Approach

- **Parent handles lifecycle**: Override `BeforeControlTemplateUninstalled_`, etc. only if needed
- **Parent's event handlers inherited**: No need to re-implement
- **Focus on new functionality**: Don't repeat parent's work

### Example Pattern from `GuiSelectableButton`

- **Extends `GuiButton`** with selection state
- **Attaches to parent's `AfterClicked`** event to toggle selection
- **Adds new properties**: `GroupController`, `AutoSelection`, `Selected`
- **Adds new template property**: `Selected` in `SelectableButtonTemplate`
- **Template calls both**: `SetState()` (from parent) and `SetSelected()` (new)

## Template Property Access Pattern

Controls access template properties via:

- `TypedControlTemplateObject(true)` - gets the typed template with existence check
- `TypedControlTemplateObject(false)` - gets the typed template without check
- Template properties have auto-generated `Get/Set` methods and `Changed` events

## Control Template Macro System

The macro system provides:

- `GUI_TEMPLATE_CLASS_DECL`: Generates class declaration with properties
- `GUI_TEMPLATE_CLASS_IMPL`: Generates implementation (constructor, destructor, property accessors)
- `GUI_SPECIFY_CONTROL_TEMPLATE_TYPE`: Links control to its template type with automatic casting
- Property macros: Generate private field, getter, setter, and change event

## Minimal Working Example

This example demonstrates the essential code structure for creating a custom control:

### Step 1: Header File (`Source/Controls/GuiMyControls.h`)

```cpp
class GuiMyControl : public GuiControl, public Description<GuiMyControl>
{
    GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(MyControlTemplate, GuiControl)
protected:
    // State variables
    bool myState = false;
    
    // Lifecycle overrides
    void BeforeControlTemplateUninstalled_() override;
    void AfterControlTemplateInstalled_(bool initialize) override;
    
    // Event handlers
    void OnMouseClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
    
public:
    GuiMyControl(theme::ThemeName themeName);
    ~GuiMyControl();
    
    // Events
    compositions::GuiNotifyEvent StateChanged;
    
    // Properties
    bool GetMyState();
    void SetMyState(bool value);
};
```

### Step 2: Implementation File (`Source/Controls/GuiMyControls.cpp`)

```cpp
void GuiMyControl::BeforeControlTemplateUninstalled_()
{
    // Cleanup before template removal
}

void GuiMyControl::AfterControlTemplateInstalled_(bool initialize)
{
    // Sync state to template
    TypedControlTemplateObject(true)->SetMyState(myState);
}

void GuiMyControl::OnMouseClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
{
    SetMyState(!myState);
}

GuiMyControl::GuiMyControl(theme::ThemeName themeName)
    : GuiControl(themeName)
{
    StateChanged.SetAssociatedComposition(boundsComposition);
    boundsComposition->GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiMyControl::OnMouseClick);
}

GuiMyControl::~GuiMyControl()
{
}

bool GuiMyControl::GetMyState()
{
    return myState;
}

void GuiMyControl::SetMyState(bool value)
{
    if (myState != value)
    {
        myState = value;
        TypedControlTemplateObject(false)->SetMyState(myState);
        StateChanged.Execute(compositions::GuiEventArgs(boundsComposition));
    }
}
```

### Step 3: Template Declaration (`Source/Controls/Templates/GuiControlTemplates.h`)

```cpp
// Add to GUI_CONTROL_TEMPLATE_DECL macro:
F(GuiMyControlTemplate, GuiControlTemplate)

// Define template properties:
#define GuiMyControlTemplate_PROPERTIES(F)\
    F(GuiMyControlTemplate, bool, MyState, false)
```

### Step 4: Theme Registration (`Source/Application/Controls/GuiThemeManager.h`)

```cpp
// Add to GUI_CONTROL_TEMPLATE_TYPES macro:
F(MyControlTemplate, MyControl)
```

### Step 5: Reflection (`Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp`)

```cpp
BEGIN_CLASS_MEMBER(GuiMyControl)
    CLASS_MEMBER_BASE(GuiControl)
    CONTROL_CONSTRUCTOR_CONTROLT_TEMPLATE(GuiMyControl)
    
    CLASS_MEMBER_GUIEVENT(StateChanged)
    CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(MyState)
END_CLASS_MEMBER(GuiMyControl)
```

### Step 6: XML Loader (`Source/Compiler/InstanceLoaders/GuiInstanceLoader_Plugin.cpp`)

```cpp
// Add in IGuiPlugin::Load():
ADD_TEMPLATE_CONTROL(GuiMyControl, MyControl);
```

### Core Pattern Summary

Each control has:

- **A C++ class** managing state and events
- **A template** defining visual properties
- **Reflection** for runtime access
- **XML loader** for declarative usage
- **Theme integration** for consistent styling