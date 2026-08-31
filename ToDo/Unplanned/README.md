# Unplanned Features

Unplanned features with detailed are stored in the same folder, here lists all ideas that cannot be categorized.
- [Control Features](./ControlFeatures.md)
- [Drag and Drop](./DragAndDrop.md)
- [Eazy Layout](./EazyLayout.md)
- [Eazy Resource and XML Upgrades](./EazyResource.md)
- [GacUI Shared Libs](./GacUISharedLibs.md)
- [Graphics](./Graphics.md)
- [Universal Release](./UniversalRelease.md)

## Ideas

- GacUI Binary Resource (when new features are added)
  - Upgrade GacUI XML Resource to 1.3, force on all resources instead of only depended or depending resource.
  - Require binary pattern "[GXR-1.3]" at the beginning of the binary resource.
  - Resource compiler and loader will check the version and only accept 1.3.
- Remote protocol player for Windows (and port to others)
  - GUI runs compiled XML resource in an separated process
  - Single GUI mode
  - Multiple GUI mode (with a simple window manager, and a test app for displaying instructions)
  - Implemented in both C++ and TypeScript
- Strict check in different for-each loops.
- `__vwsn::EventAttachOnce` (GuiApplication.h) is it possible to make it a workflow syntax `attachonce`?

## OS Provider Features

- Windows
  - `INativeImage::SaveToStream` handle correctly for git format. It is possible that LoadFromStream need to process diff between git raw frames.
- Test Automation.
  - Standard test for OS providers, may need a test purpose automation service to do https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput

## Porting to New Platforms

- Port GacUI to other platforms:
  - Unit Test (**Remote**)
  - Windows
    - GDI (**Normal**, **Hosted**, **Remote**)
    - Direct2d (**Normal**, **Hosted**, **Remote**)
    - UWP (Remote)
  - Linux (wGac repo)
  - macOS (iGac repo)
  - HTML5 Dom/CSS (**Remote**)
    - WebAssembly (Remote)
  - CLI (Hosted)
    - Dedicated skin
- Port GacUI to other languages:
  - Applications written in other language can:
    - Implement view model (**Workflow RPC**).
    - Render the UI (**GacJS**).
  - Languages:
    - TypeScript
      - **CLI**
      - **HTTP**
      - WebAssembly
    - .NET
    - Python

## GacUI

- DarkSkin Color Theme.
  - Create a `DarkSkinPalette` class with a static getter method to retrive default colors.
    - Update all `Style.xml` colors to use `DarkSkinPalette`.
  - Add a static setter to `DarkSkinPalette`.
    - A window can be called to update all its controls' and components' template.
    - The above function will be called inside the setter.
- New default control templates with animation, written in XML generated C++ code.
- `INativeWindow` add callback for state changing.
  - Including `MaximizedBox`, `MinimizedBox`, `Border`, `SizeBox`, `IconVisible`, `TitleBar`, `Icon`, `Title`, `SizeState`.
  - In `GuiControlHost` or `GuiWindow`, setting border or state doesn't update the control template, it is updated in that callback.
  - Delete `GuiControlHost` and `GuiWindow`'s `OnVisualStatusChanged`.
- Rewrite calculator state machine demo, when "+" is pressed, jump into "WaitingAnotherOperandForPlus" state machine, instead of storing the operation in a loop. So there will be no loop except for waiting for numbers.
- Add `MoveToScreenCenterAfterLayout` as what is done in `FakeDialogServiceBase::ShowModalDialogAndDelete`.
- Use the embedded data codegen / compress / decompress functions from `VlppParser2` to replace one in `GacUI`.
- Use collection interfaces on function signatures.
  - Only if `Vlpp` decides to add collection interfaces.

## GacUI Resource Compiler

- In the final pass, only workflow scripts are printed.
  - Use WorkflowCompiler.exe to do codegen externally.
- Remove all loader implementation, enabling custom control types from developers.
  - Try not to include `GacUI.cpp` if `VCZH_DEBUG_METAONLY_REFLECTION` is on.
  - `mynamespaces::VirtualClasses_X` for adding virtual classes deriving from `mynamespaces::X`.
    - Containing methods like: `GuiSelectableButton* CheckBox()`.
  - `mynamespaces::InstanceLoader_X` for implementing different kinds of properties / constructors that do not exist in `mynamespaces::X`.
    - following a naming convention, e.g. `GuiTableComposition`'s rows/columns properties.
    - searching for correct default control templates.
- Incremental build inside single resource.
  - Calculate dependencies by only parsing.
  - Cache workflow assembly per resource in file.
  - Codegen c++ from multiple workflow assembly.

## New C++/Doc Compiler based on VlppParser2
