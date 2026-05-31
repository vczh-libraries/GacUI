# TODO Task Log 4

## Task

Implement document-element IO, especially `!Type:<TEXT>`, with correct CRLF behavior:

- ignore every `\r`
- emit `\r\n` for every `\n`

## Setup

- Application: `CppTest`
- Automation base URL: `http://localhost:8888/Automation/CppTest`
- Dump URL: `GET /Controls`
- IO URL: `POST /IO`
- Debugger: `cdb`

## Source Locations I Investigated

- `Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`
  - Command parsing and char dispatch.
- `Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.cpp`
  - Document dump reporting.
- `Source/Controls/TextEditorPackage`
  - Used to understand how document controls respond to char input and CRLF.
- `Source/GraphicsElement/GuiGraphicsDocumentElement.*`
  - Used to extract selection and document XML.

## Implementation Notes

The `!Type:<TEXT>` implementation walks the text payload one character at a time.

The behavior is:

```text
if character == '\r':
    ignore it
if character == '\n':
    send '\r'
    send '\n'
otherwise:
    send the character directly
```

For every emitted character, `RunIOCommandOnNativeWindow` builds a `NativeWindowCharInfo` containing the current modifier state and sends `Char` to each listener.

The document dump reports:

- selection begin and end positions
- `WrapLine` and password-char flags when applicable
- document XML produced from the document model

## Automation Observations

I started `CppTest` under the debugger and switched to the `Control` tab.

The main document element was found by searching `/Controls` for `elementDocument` and choosing the largest document-area node.

The center used for the main document was:

```text
Document center = (362,263)
```

I clicked the document:

```text
!LeftClick:362,263
```

Then I typed:

```text
!Type:Alpha\nBeta\r\nGamma
```

The endpoint returned:

```text
Queued
```

After waiting for execution, `/Controls` reported the document state:

```text
Document:Selection(0,16)-(0,16),WrapLine
```

The XML contained:

```xml
<nop>Alpha<br/>Beta<br/>Gamma</nop>
```

This shows:

- the standalone `\n` became a new paragraph/line break in the document
- the `\r\n` sequence did not create an extra blank paragraph because `\r` was ignored and `\n` was converted to CRLF
- the caret moved to the end of the inserted content

## How I Found Where To Interact

I did not guess the document coordinates. I used `/Controls`:

1. Search all nodes for the `elementDocument` property.
2. Compute each document node area from bounds.
3. Pick the largest one because the page contains multiple document-related elements.
4. Click the center of that node.
5. Type with `!Type`.
6. Re-read `/Controls` and inspect the document XML.

## What Changed For This Task

- `!Type:<TEXT>` emits native char events.
- CR characters are ignored.
- LF characters are translated to CRLF.
- Document element dumping emits the document XML and selection state.

## Verification Result

Task 4 is verified. Typing multiple lines into the document through `/IO` edits the document, and `/Controls` reports the correct XML and selection state.
