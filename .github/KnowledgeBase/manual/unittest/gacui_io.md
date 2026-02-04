# IO Interaction

To calculate the location of a control or a composition for a following mouse interaction, use**protocol-\>LocationOf(controlOrComposition, ratioX?, ratioY?, offsetX?, offsetY?)**. It calculates the global location within an object using**left + width * ratioX + offsetX**and**top + height * ratioY + offsetY**. You can omit the last 4 parameters to get the center of the object. Such location can be passed to mouse interaction functions directly.

## Mouse Interactions

Here are a list of available mouse interaction functions in the**protocol**object:
- **MouseMove(location)**: move the mouse to the specified location
- **_LDown(location?)**: trigger a left button down event.
- **_LUp(location?)**: trigger a left button up event.
- **_LDBClick(location?)**: trigger a left button double click event.
- **LClick(location?) and LClick(location, ctrl, shift, alt)**: trigger multiple events to simulate a click with optional modifier keys.
- **LDBClick(location?) and LDBClick(location, ctrl, shift, alt)**: trigger multiple events to simulate a double click with optional modifier keys.
- M (middle button) version and R (right button) version of above functions.

## Mouse Wheel Interactions

Here are a list of available mouse wheel interaction functions in the**protocol**object:
- **_Wheel(up, position?) and _Wheel(up, position, ctrl, shift, alt)**: scroll the wheel forward. A negative number means backard.**WheelDown**or**WheelUp**for shortcuts to scroll with jumps, a jump scales to 120.
- **_HWheel(right, position?) and _HWheel(right, position, ctrl, shift, alt)**: scroll the horizontal wheel towards right. A negative number means towards left.**HWheelLeft**or**HWheelRight**for shortcuts to scroll with jumps, a jump scales to 120.

## Key Interactions

Here are a list of available key interaction functions in the**protocol**object:
- **_KeyDown(key)**: trigger a key down event.
- **_KeyDownRepeat(key)**: trigger a key down event triggers by keep pressing a key.
- **_KeyUp(key)**: trigger a key up event.
- **KeyPress(key) and KeyPress(key, ctrl, shift, alt)**: trigger multiple events to simulate a key press with optional modifier keys.
- **TypeString(string)**: trigger a sequence of char events to type a string. Key events will not be triggered.

