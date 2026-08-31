# Eazy Resource

## XML Enhancement

- `<eval Eval="expression"/>` tags.
- Consider `-ani` binding, create an animation controller object that change the binded property, with predefined interpolation and other stuff.
  - All types that can do interpolation are value types, consider following formats:
    - "NAME:initial value"
    - "NAME(initial value from expression)"
    - Need to be consistent with animation object
  - Consider multiple `-ani` batch control, state configuration and transition, story board, connection to animation coroutine, etc.

## Non-XML Format

- A new non-XML instance format
  - Integrate ez:Layout
  - New syntax to easily do control localization and UI animation
    - Easy to define and apply template (e.g. menu cascading & toolstrip sharing, dialog button layout)
  - Compile to XML
- EazyForm
  - A UI builder for connecting data structure to UI
  - Either schemed JSON or reflectablr C++ data structure consumable
  - Limited freedom of controlling UI
  - Can be created in a repo like GacUILibs
  