# TODO Task Log 2

## Task

Implement `RunIOCommandOnNativeWindow` for mouse move and mouse button commands.

## Setup

- Application: `CppTest`
- Automation base URL: `http://localhost:8888/Automation/CppTest`
- Dump URL: `GET /Controls`
- IO URL: `POST /IO`
- Debugger: `cdb`
- HTTP `/IO` final behavior: returns `Queued`.

Because `/IO` is now queued, each request returning `Queued` only means the command was accepted. I waited and then observed state through `/Controls`.

## Source Locations I Investigated

- `Source/Utilities/SharedServices/GuiSharedAutomationService.h`
  - Defines `IoCommandState` and the command grammar.
- `Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`
  - Where mouse command parsing and listener dispatch were implemented.
- `Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.h`
  - Used as the best local model for IO state:
    - current mouse position
    - pressed buttons
    - pressed keys
    - modifier flags
    - wheel values.
- `Source/PlatformProviders/Windows/WinNativeWindow.cpp`
  - Used to compare native listener callback names and mouse info shape.

## Implementation Notes

The mouse side of `RunIOCommandOnNativeWindow` now supports:

- `!MouseMove:X,Y`
- `!MouseMove:X,Y,ctrl`
- `!MouseMove:X,Y,shift`
- `!MouseMove:X,Y,alt`
- `!LeftDown:X,Y`
- `!LeftUp:X,Y`
- `!LeftClick:X,Y`
- `!LeftDbClick:X,Y`
- Equivalent middle and right button commands.

Mouse command parsing records:

- the last mouse position
- left/middle/right button state
- current ctrl/shift/alt modifier state

When a button command specifies temporary modifiers, the implementation presses those modifiers before dispatching the mouse callback and releases them afterwards.

Coordinates are interpreted as GacUI coordinates and converted through `INativeWindow::Convert` before native listener callbacks receive them.

## Automation Observations

I started `CppTest` under `cdb` and dumped `/Controls`.

From the JSON tree I found the `Add 10 items` label and its composition bounds. The center used for IO was:

```text
Add 10 items center = (372,177)
```

I then sent:

```text
POST /IO
!MouseMove:372,177
```

The endpoint returned:

```text
Queued
```

After a short wait, the button visual state changed, which showed that the mouse move reached the button.

I then sent:

```text
POST /IO
!LeftClick:372,177
```

The endpoint returned:

```text
Queued
```

After the click, `/Controls` reported added list labels. The visible numeric labels contained the expected `0..9` range. Because the default page has multiple list-like controls, the dump showed two runs of numeric labels, which was expected.

## Combo Box Drop Down Verification

Next I found the `Text` combo box area from the dump:

```text
Text combo center = (79,126)
```

I clicked it with mouse IO and `/Controls` reported a subwindow in `subWindowsInZOrder`. That subwindow represented the combo drop down.

Then I interacted with the mode tabs:

- Switching to `Check` produced item marker compositions containing rectangle element descriptions.
- Switching to `Radio` produced item marker compositions containing ellipse element descriptions.

Observed marker samples included:

```text
Background:#3F3F46FF,Rectangle
Border:#54545CFF,Rectangle
Background:#3F3F46FF,Ellipse
Border:#54545CFF,Ellipse
```

## How I Found Where To Interact

I did not hardcode UI coordinates. I used `/Controls` to find:

- label text
- surrounding composition bounds
- the center of the target composition
- the active subwindow after opening the combo box

The JSON tree made it possible to identify the drop down without relying on operating system UI automation.

## What Changed For This Task

- Implemented mouse movement command parsing.
- Implemented mouse button command parsing.
- Implemented click and double-click callback sequences.
- Added temporary modifier handling for mouse commands.
- Preserved button and mouse position state in `IoCommandState`.

## Verification Result

Task 2 is verified. Mouse move highlights the target button, clicking `Add 10 items` adds visible items, and the combo drop down is reported and can be interacted with through `/IO`.
