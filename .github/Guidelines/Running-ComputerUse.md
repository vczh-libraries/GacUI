# Basic Computer Use

This instruction is important for both Unit Test, CLI and GacUI applications.
MSVC compiled applications are inevitably (but not always) prompting a native dialog when crashed.
GacUI applications could invoke native dialogs proactivately, even when `FakeDialogService` is used.
This document describes how to handle native UI to unblock the debugging or testing process.

- Go to `Windows Specific` section if you are on Windows.
- Go to `Linux Specific` section if you are on Linux.
- Go to `macOS Specific` section if you are on macOS.

## Windows Specific

UI Automation must not be used as the fallback for native windows. It can fail outright when the screen is locked, and calling UIA while a modal native dialog is blocking the application can leave the agent waiting forever.

When an application stops responding, assume a native modal window could be blocking its UI thread. Do not keep polling the application-level automation endpoint forever. Inspect native windows from a separate PowerShell process by using Win32 APIs, or take screenshots and then use Win32 APIs to interact with the dialog.

The safest pattern is:

- Identify the target process.
- Enumerate top-level windows and child windows with Win32.
- Read class names, window text, control ids and rectangles.
- Prefer direct control messages such as `BM_CLICK`, `WM_SETTEXT` and combo-box messages over mouse input.
- If direct control messages are not enough, use screenshots and absolute window/control rectangles to decide where to click.
- Always close or cancel the native dialog before returning to application-level automation.

Useful PowerShell helper:

```powershell
$source = @'
using System;
using System.Text;
using System.Runtime.InteropServices;

public static class NativeUi
{
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
    public delegate bool EnumChildProc(IntPtr hWnd, IntPtr lParam);

    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc callback, IntPtr lParam);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr hWnd, EnumChildProc callback, IntPtr lParam);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int maxCount);
    [DllImport("user32.dll")] public static extern int GetClassName(IntPtr hWnd, StringBuilder text, int maxCount);
    [DllImport("user32.dll")] public static extern int GetDlgCtrlID(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr SendMessage(IntPtr hWnd, uint msg, IntPtr wParam, string lParam);

    public const uint BM_CLICK = 0x00F5;
    public const uint WM_CLOSE = 0x0010;
    public const uint WM_SETTEXT = 0x000C;
    public const uint CB_SELECTSTRING = 0x014D;
    public const uint CB_SETCURSEL = 0x014E;

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }
}
'@

Add-Type -TypeDefinition $source

function Get-WindowTextValue([IntPtr]$Handle) {
    $text = New-Object Text.StringBuilder 512
    [void][NativeUi]::GetWindowText($Handle, $text, $text.Capacity)
    $text.ToString()
}

function Get-ClassNameValue([IntPtr]$Handle) {
    $text = New-Object Text.StringBuilder 256
    [void][NativeUi]::GetClassName($Handle, $text, $text.Capacity)
    $text.ToString()
}

function Format-NativeWindow([IntPtr]$Handle, [string]$Prefix = "") {
    [uint32]$processId = 0
    [void][NativeUi]::GetWindowThreadProcessId($Handle, [ref]$processId)
    $rect = New-Object NativeUi+RECT
    [void][NativeUi]::GetWindowRect($Handle, [ref]$rect)
    "{0}{1:X8} pid={2} id={3} visible={4} class={5} rect=({6},{7},{8},{9}) text={10}" -f `
        $Prefix,
        $Handle.ToInt64(),
        $processId,
        [NativeUi]::GetDlgCtrlID($Handle),
        [NativeUi]::IsWindowVisible($Handle),
        (Get-ClassNameValue $Handle),
        $rect.Left,
        $rect.Top,
        $rect.Right,
        $rect.Bottom,
        (Get-WindowTextValue $Handle)
}

function Show-NativeWindowsForProcess([int]$ProcessId) {
    [NativeUi]::EnumWindows({
        param($window, $lParam)
        [uint32]$ownerProcessId = 0
        [void][NativeUi]::GetWindowThreadProcessId($window, [ref]$ownerProcessId)
        if ($ownerProcessId -eq $ProcessId) {
            [Console]::WriteLine((Format-NativeWindow $window))
            [NativeUi]::EnumChildWindows($window, {
                param($child, $childParam)
                [Console]::WriteLine((Format-NativeWindow $child "  "))
                return $true
            }, [IntPtr]::Zero) | Out-Null
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null
}
```

If a visual check is needed, take a screenshot without UIA:

```powershell
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
$bitmap.Save("$env:TEMP\native-window.png", [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()
```

When outputting from inside a delegate callback, use `[Console]::WriteLine(...)`. Plain PowerShell output inside the callback can be swallowed as the callback return value.

### Native Message Dialog

Message boxes are usually top-level dialogs with class `#32770`. Their text is usually in `Static` children, and buttons are `Button` children.

Steps:

- Call `Show-NativeWindowsForProcess <pid>`.
- Find the visible `#32770` window.
- Read the `Static` child text to understand the prompt.
- Enumerate `Button` children and choose the intended response.
- Click the button with `BM_CLICK`.

Standard button ids:

- `1`: `OK`
- `2`: `Cancel`
- `3`: `Abort`
- `4`: `Retry`
- `5`: `Ignore`
- `6`: `Yes`
- `7`: `No`

Example:

```powershell
function Click-DialogButtonByText([int]$ProcessId, [string]$ButtonText) {
    $script:button = [IntPtr]::Zero

    [NativeUi]::EnumWindows({
        param($window, $lParam)
        [uint32]$ownerProcessId = 0
        [void][NativeUi]::GetWindowThreadProcessId($window, [ref]$ownerProcessId)

        if ($ownerProcessId -eq $ProcessId -and (Get-ClassNameValue $window) -eq "#32770") {
            [NativeUi]::EnumChildWindows($window, {
                param($child, $childParam)
                if ((Get-ClassNameValue $child) -eq "Button" -and (Get-WindowTextValue $child) -eq $ButtonText) {
                    $script:button = $child
                    return $false
                }
                return $true
            }, [IntPtr]::Zero) | Out-Null
        }

        return ($script:button -eq [IntPtr]::Zero)
    }, [IntPtr]::Zero) | Out-Null

    if ($script:button -eq [IntPtr]::Zero) {
        throw "Button not found: $ButtonText"
    }

    [void][NativeUi]::SendMessage($script:button, [NativeUi]::BM_CLICK, [IntPtr]::Zero, [IntPtr]::Zero)
}
```

If a message box has multiple buttons, do not blindly click the first button. Read the prompt and button text. If the goal is only to unblock after collecting diagnostic information, prefer the least destructive option, usually `Cancel`, `No`, or the close button. If the dialog reports a crash or runtime error and offers diagnostic buttons, capture the text/screenshot first, then choose the action that closes the dialog without changing project files.

If no button can be found but the dialog must be dismissed, send `WM_CLOSE` to the dialog:

```powershell
[void][NativeUi]::SendMessage($dialogHandle, [NativeUi]::WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero)
```

### Native Color Picking Dialog

The standard Windows color dialog is also a `#32770` dialog. Its common control ids are:

- `1`: `OK`
- `2`: `Cancel`
- `706`: red edit box
- `707`: green edit box
- `708`: blue edit box
- `719`: `Define Custom Colors`
- `712`: `Add to Custom Colors`

The color grid itself is usually owner-drawn or static, so UIA-style discovery is not helpful. The most reliable programmatic path is to set the RGB edit fields and click `OK`.

Example:

```powershell
function Set-DialogEditTextById([IntPtr]$Dialog, [int]$Id, [string]$Text) {
    $script:edit = [IntPtr]::Zero

    [NativeUi]::EnumChildWindows($Dialog, {
        param($child, $lParam)
        if ([NativeUi]::GetDlgCtrlID($child) -eq $Id -and (Get-ClassNameValue $child) -eq "Edit") {
            $script:edit = $child
            return $false
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null

    if ($script:edit -eq [IntPtr]::Zero) {
        throw "Edit control not found: $Id"
    }

    [void][NativeUi]::SendMessage($script:edit, [NativeUi]::WM_SETTEXT, [IntPtr]::Zero, $Text)
}

function Click-DialogButtonById([IntPtr]$Dialog, [int]$Id) {
    $script:button = [IntPtr]::Zero

    [NativeUi]::EnumChildWindows($Dialog, {
        param($child, $lParam)
        if ([NativeUi]::GetDlgCtrlID($child) -eq $Id -and (Get-ClassNameValue $child) -eq "Button") {
            $script:button = $child
            return $false
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null

    if ($script:button -eq [IntPtr]::Zero) {
        throw "Button control not found: $Id"
    }

    [void][NativeUi]::SendMessage($script:button, [NativeUi]::BM_CLICK, [IntPtr]::Zero, [IntPtr]::Zero)
}

# Example: set RGB to red, then accept.
Set-DialogEditTextById $dialogHandle 706 "255"
Set-DialogEditTextById $dialogHandle 707 "0"
Set-DialogEditTextById $dialogHandle 708 "0"
Click-DialogButtonById $dialogHandle 1
```

To cancel, click button id `2`. After accepting a color, return to the application and remove selection/highlight if necessary before visually verifying the applied text color; selected text can hide the actual color.

If the RGB edits are not visible until custom colors are expanded, click id `719` first. If a specific palette color must be selected and no edit fields are usable, take a screenshot, use the `Static` color-grid rectangle from `GetWindowRect`, and click the palette cell by coordinate.

### Native Font Picking Dialog

The standard Windows font dialog is a `#32770` dialog. Its common control ids are:

- `1`: `OK`
- `2`: `Cancel`
- `1136`: font combo box
- `1137`: font style combo box
- `1138`: size combo box
- `1139`: color combo box, when effects/color are enabled
- `1140`: script combo box
- `1040`: strikeout checkbox, when effects are enabled
- `1041`: underline checkbox, when effects are enabled

Use combo-box messages to choose values. This works when the screen is locked because it talks to the HWND directly.

Example:

```powershell
function Select-DialogComboTextById([IntPtr]$Dialog, [int]$Id, [string]$Text) {
    $script:combo = [IntPtr]::Zero

    [NativeUi]::EnumChildWindows($Dialog, {
        param($child, $lParam)
        if ([NativeUi]::GetDlgCtrlID($child) -eq $Id -and (Get-ClassNameValue $child) -like "ComboBox*") {
            $script:combo = $child
            return $false
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null

    if ($script:combo -eq [IntPtr]::Zero) {
        throw "Combo box not found: $Id"
    }

    $result = [NativeUi]::SendMessage($script:combo, [NativeUi]::CB_SELECTSTRING, [IntPtr](-1), $Text)
    if ($result.ToInt64() -lt 0) {
        throw "Combo item not found: $Text"
    }
}

# Example.
Select-DialogComboTextById $dialogHandle 1136 "Arial"
Select-DialogComboTextById $dialogHandle 1137 "Bold Italic"
Select-DialogComboTextById $dialogHandle 1138 "20"
Click-DialogButtonById $dialogHandle 1
```

For checkboxes such as underline or strikeout, enumerate child `Button` controls, confirm their text and id, and use `BM_CLICK` to toggle. If the dialog exposes a color combo, use id `1139` with the same combo helper.

To cancel, click button id `2`. After accepting a font, verify the resulting application state through the application itself, screenshots, or exposed rendering state. Some applications intentionally read only a subset of the font dialog result, so verify the specific properties that the application is expected to apply.

### Native File Picking Dialog

Native file dialogs come in two major shapes:

- Older common dialogs expose direct child controls such as `Edit`, `ComboBox`, `SysListView32`, and `Button`.
- Newer Explorer-style dialogs may contain nested `DirectUIHWND`/`SHELLDLL_DefView` controls where the file list is not easy to operate by messages.

For both shapes, the most reliable path is to type the absolute file path into the filename edit and click `Open`/`Save`.

Common old-style open dialog ids:

- `1`: `Open` or `Save`
- `2`: `Cancel`
- `1148`: filename edit or combo/edit child
- `1136`: file type combo box
- `1137`: current folder combo box
- `1121`: shell view

Example:

```powershell
function Set-FileDialogName([IntPtr]$Dialog, [string]$Path) {
    $script:edit = [IntPtr]::Zero

    [NativeUi]::EnumChildWindows($Dialog, {
        param($child, $lParam)
        if ([NativeUi]::GetDlgCtrlID($child) -eq 1148 -and (Get-ClassNameValue $child) -eq "Edit") {
            $script:edit = $child
            return $false
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null

    if ($script:edit -eq [IntPtr]::Zero) {
        throw "File name edit control not found."
    }

    [void][NativeUi]::SendMessage($script:edit, [NativeUi]::WM_SETTEXT, [IntPtr]::Zero, $Path)
}

Set-FileDialogName $dialogHandle "C:\path\to\file.png"
Click-DialogButtonById $dialogHandle 1
```

To cancel, click button id `2`.

For folder navigation:

- Prefer typing a full absolute path in the filename field and clicking `Open`.
- To move to a folder without selecting a file, type the folder path in the filename field and click `Open`.
- To select a visible file by clicking, enumerate the shell view/list control and use screenshots/rectangles to calculate the file row/cell, then click it. Use this only when full-path entry is not accepted.
- If the dialog uses an address bar and the filename edit is unavailable, use keyboard input only after confirming focus with screenshots. Prefer `Alt+D`, type the folder path, press `Enter`, then type/select the file.

After accepting a file, verify the application state through the application-level automation, log output, or screenshot. If the app remains blocked, enumerate windows again; an error prompt may have appeared because the file does not exist, the extension is filtered out, or the application rejected the file.

### Other/General Native Windows

Not every native modal window is a standard dialog. Runtime-library errors, crash dialogs, debugger prompts and custom native windows may use different classes, child structures, or owner-drawn controls. The same rules still apply:

- Enumerate top-level windows for the target process.
- Inspect visible windows first, then hidden windows if the app is still blocked.
- Record the top-level window class, title, process id, rectangle and all child controls.
- Read all `Static`, `Edit`, `RichEdit`, `Button`, `SysLink`, `ComboBox`, `ListBox`, `SysListView32` and `ToolbarWindow32` children.
- Take a screenshot if text or buttons are owner-drawn.
- Prefer explicit button clicks by id or text.
- Use `WM_CLOSE` only after capturing useful text/screenshots and only when choosing a visible button is impossible or unsafe.
- If the target process shows no windows but remains blocked, check for child/helper processes, debugger processes, crash-reporting processes, or windows owned by a different process id.

Keep the native-dialog helper separate from the application under test. A modal dialog blocks the application's UI thread, so any app-level automation command that requires the UI thread can hang or time out until the native dialog is handled.

Do not panic or wait indefinitely. Once a native window is suspected, switch to Win32 enumeration and close the blocking dialog deterministically.

## Linux Specific

(to be editing...)

## macOS Specific

(to be editing...)
