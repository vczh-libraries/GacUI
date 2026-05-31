# TODO Task Log 6

## Task

Implement wheel operations for `RunIOCommandOnNativeWindow`.

The requested manual verification only needed up/down, but left/right also had to be implemented correctly.

## Setup

- Application: `CppTest`
- Automation base URL: `http://localhost:8888/Automation/CppTest`
- Dump URL: `GET /Controls`
- IO URL: `POST /IO`
- Debugger: `cdb`

## Source Locations I Investigated

- `Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`
  - Wheel command parsing and dispatch.
- `Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.h`
  - Existing model for wheel events.
- `Source/PlatformProviders/Windows/WinNativeWindow.cpp`
  - Native callback names:
    - `VerticalWheel`
    - `HorizontalWheel`
- `Source/Controls/GuiScrollControls.cpp`
  - Used to understand how wheel input affects list scrolling.

## Commands Implemented

Wheel IO supports:

```text
!MouseWheelUp:ticks
!MouseWheelDown:ticks
!MouseWheelLeft:ticks
!MouseWheelRight:ticks
```

Each can include modifiers:

```text
!MouseWheelDown:1,ctrl
!MouseWheelUp:2,shift
!MouseWheelRight:1,alt
```

Wheel values are calculated as:

```text
ticks * 120 * direction
```

Direction mapping:

```text
Up    = +1 vertical
Down  = -1 vertical
Right = +1 horizontal
Left  = -1 horizontal
```

The command dispatches:

- `VerticalWheel(info)` for up/down
- `HorizontalWheel(info)` for left/right

## Automation Observations

I started `CppTest` under `cdb`.

I clicked `Add 10 items` multiple times to make the left list scrollable.

The add-button center from `/Controls` was:

```text
Add 10 items center = (372,177)
```

I sent repeated clicks:

```text
!MouseMove:372,177
!LeftClick:372,177
!LeftClick:372,177
!LeftClick:372,177
!LeftClick:372,177
!LeftClick:372,177
```

Then I moved the mouse over the left list box:

```text
!MouseMove:120,330
```

Before wheel input, the first visible numeric labels looked like:

```text
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,...
```

After:

```text
!MouseWheelDown:3
```

the first visible numeric labels shifted down to:

```text
12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,...
```

After:

```text
!MouseWheelUp:3
```

the visible list moved back upward and again showed earlier items.

## How I Found Where To Interact

I used `/Controls` to:

1. Locate `Add 10 items`.
2. Click it enough times to create scrollable content.
3. Locate the left list box area by inspecting the visible numeric labels and their bounds.
4. Move the mouse inside the list box before wheel input.
5. Compare visible numeric labels before and after wheel commands.

## What Changed For This Task

- Implemented vertical wheel parsing.
- Implemented horizontal wheel parsing.
- Implemented wheel value normalization to Windows-style `120` deltas.
- Reused current mouse position and modifier state when building `NativeWindowMouseInfo`.

## Verification Result

Task 6 is verified. Vertical wheel input scrolls the left list box up and down. Horizontal wheel dispatch is implemented through `HorizontalWheel` with the correct sign convention even though the visible manual app path only exercised vertical scrolling.
