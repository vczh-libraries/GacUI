# TODO Task Log 5

## Task

Implement keyboard operations and verify pure keyboard navigation through the document editing workflow.

## Setup

- Application: `CppTest`
- Automation base URL: `http://localhost:8888/Automation/CppTest`
- Dump URL: `GET /Controls`
- IO URL: `POST /IO`
- Debugger: `cdb`

## Source Locations I Investigated

- `Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`
  - Key parsing, key state, char state, temporary modifiers.
- `Source/UnitTestUtilities/GuiUnitTestProtocol_IOCommands.h`
  - Used as the model for key state and modifier handling.
- `Source/Application/GraphicsHost/GuiGraphicsHost_Alt.cpp`
  - Used to understand ALT navigation.
- `Source/Controls/TextEditorPackage`
  - Used to understand selection movement with arrow keys and modifiers.
- `Test/GacUISrc/Generated_Dialogs`
  - Used to identify color dialog ALT keys:
    - red field: `R`
    - green field: `G`
    - blue field: `B`
    - OK: `O`
- `Test/GacUISrc/Generated_DarkSkin` and generated UI sources
  - Used to infer the document toolbar ALT route.

## Key Parsing Requirement

The user explicitly said:

```text
key name can be read from INativeInputService::GetKey|GetKeyName, do not list them by yourself.
```

I followed this by resolving key names with:

- `INativeInputService::GetKey`
- fallback scanning through `GetKeyName`

I did not maintain a hand-written key-name table in the automation service.

I also skipped fallback names equal to `?`, because unknown key names should not accidentally match normalized input.

## Commands Implemented

Keyboard IO supports:

```text
!KeyDown:Key1+Key2+...+KeyN
!KeyUp:Key1+Key2+...+KeyN
!KeyPress:Key1+Key2+...+KeyN
```

State tracked:

- currently pressed keys
- ctrl
- shift
- alt
- caps lock toggle

`!KeyPress:Ctrl+A` presses Ctrl, presses A, releases A, releases Ctrl.

## Automation Path Found Through The UI

To switch to the `Control` tab by keyboard:

```text
!KeyPress:Alt
!KeyPress:L
!KeyPress:3
```

The `Alt` dump showed labels including:

```text
[L]1
[L]2
[L]3
[M]
[W]
[X]0
```

To focus the main document:

```text
!KeyPress:Alt
!KeyPress:D
!KeyPress:2
```

To type:

```text
!Type:ABCDEFG
```

To select `CDE`:

```text
!KeyPress:Left
!KeyPress:Left
!KeyDown:Shift
!KeyPress:Left
!KeyPress:Left
!KeyPress:Left
!KeyUp:Shift
```

Before color was applied, `/Controls` reported:

```text
Document:Selection(0,5)-(0,2),WrapLine
```

The XML still contained:

```xml
<nop>ABCDEFG</nop>
```

To open the text color dialog through keyboard:

```text
!KeyPress:Alt
!KeyPress:T
!KeyPress:1
!KeyPress:C
```

The color dialog opened with fields for red, green, and blue plus OK and Cancel.

To choose green:

```text
!KeyPress:Alt
!KeyPress:R
!KeyPress:Ctrl+A
!Type:0

!KeyPress:Alt
!KeyPress:G
!KeyPress:Ctrl+A
!Type:255

!KeyPress:Alt
!KeyPress:B
!KeyPress:Ctrl+A
!Type:0

!KeyPress:Alt
!KeyPress:O
```

To focus the document again and collapse the selection:

```text
!KeyPress:Alt
!KeyPress:D
!KeyPress:2
!KeyPress:End
```

## Automation Observations

The final document dump reported:

```text
Document:Selection(0,7)-(0,7),WrapLine
```

The document XML split the text into three runs:

```xml
<nop>AB</nop>
<font color="#00FF00"><nop>CDE</nop></font>
<nop>FG</nop>
```

This is exactly the requested edit:

- original text is semantically `ABCDEFG`
- `CDE` is selected and recolored
- green is `#00FF00`
- pressing `End` collapses the selection at the end

## What Changed For This Task

- Implemented keyboard command parsing.
- Implemented key down/up/press dispatch.
- Implemented key state tracking.
- Implemented modifier state calculation for key and char events.
- Used `INativeInputService` for all key-name resolution.

## Verification Result

Task 5 is verified for `CppTest`. Pure keyboard automation reaches the document, edits text, selects `CDE`, applies green through the color dialog, and reports the final document XML correctly.

## Rerun After IO Queue Boundary Fix

After changing `/IO` so parsing stays synchronous and accepted commands are queued inside `RunIOCommandOnNativeWindow`, I reran the Task 5 validation against debugger-attached `CppTest`.

First I verified response semantics:

```text
POST /Automation/CppTest/IO
!MouseMove:nope
```

The service returned `Syntax Error! ...` immediately, confirming malformed command text no longer disappears behind `InvokeInMainThread`.

Then I verified a valid keyboard command:

```text
POST /Automation/CppTest/IO
!KeyPress:Alt
```

The service returned:

```text
Queued
```

The first live keyboard attempt exposed a queued-closure `Variant` copy assertion. I fixed that by storing the parsed command variant in a reference-counted holder before queuing the lambda, so the function object copies only the holder pointer. After this change, `!KeyPress:Alt` returned `Queued` and the Alt labels appeared in `/Controls`.

I reran the Task 5 workflow:

```text
!KeyPress:Alt
!KeyPress:L
!KeyPress:3
!KeyPress:Alt
!KeyPress:D
!KeyPress:2
!Type:ABCDEFG
!KeyPress:Left
!KeyPress:Left
!KeyDown:Shift
!KeyPress:Left
!KeyPress:Left
!KeyPress:Left
!KeyUp:Shift
!KeyPress:Alt
!KeyPress:T
!KeyPress:1
!KeyPress:C
!KeyPress:Alt
!KeyPress:R
!KeyPress:Ctrl+A
!Type:0
!KeyPress:Alt
!KeyPress:G
!KeyPress:Ctrl+A
!Type:255
!KeyPress:Alt
!KeyPress:B
!KeyPress:Ctrl+A
!Type:0
!KeyPress:Alt
!KeyPress:O
!KeyPress:Alt
!KeyPress:D
!KeyPress:2
!KeyPress:End
```

The color dialog was reported as a subwindow titled `选择颜色`. Its RGB text boxes reported `0`, `255`, and `0`, and the color preview reported `#00FF00FF`.

After OK, `/Controls` reported:

```xml
<nop>AB</nop><font color="#00FF00"><nop>CDE</nop></font><nop>FG</nop>
```

After refocusing the document and pressing `End`, `/Controls` reported:

```text
Document:Selection(0,7)-(0,7),WrapLine
```

This confirms the parse/queue fix did not regress Task 5 keyboard automation.
