# vnext

## Milestone

### Vlpp

- `Variant` and `Union` with full support.
  - Document.
  - Document `vl::Overloading`.
- Extensible CLI argument parser acception different OS convention, serialization and module dependencies.
  - Structured error report.
  - Extensible error message localization relying on GacUI XML Resource localization feature.
  - Pre-made main function per OS, defining arguments for different renderers for the current OS, including remoting server with predefine protocols.
  - Allow accepting custom parser, regex parser will be offered in `VlppRegex`.

### Compiler Tools

- Workflow compiler tool consuming metadata binaries files.
- Compiler tools parse a config, and unit test should construct a config and use the same driver function for compiling.
  - Config and its processing should be included in the libarary, each tool should just a simple call to it, so that the same logic could be reused in unit test.
  - VlppParser2
  - Workflow Compiler + CppMerge
  - GacUI Xml Resource Compiler

### GacUI

- More optimistic remote protocol strategy to reduce messages.
- Windows
  - Ensure `INativeWindow::(Before|After)Closing()` is not called on non-main-window between the main window is closed and the application exits.
- GacUI
  - Enlarging window slower than shrinking.
  - https://github.com/vczh-libraries/Vlpp/issues/9
  - Need a way to handle registration failure of global shortcut key, so that an app knows the shortcut key is already taken, or converted to local shortcut key, or not working, instead of just crashing (at least on Windows).
    - Turn global shortcut key in `CppTest_Tui` back to `Ctrl+Shift+Alt+Win+Q` and render failure when anything happens.
    - `GacJS`/`wGac` should render it is treated as a local shortcut key.
- Windows OSProvider
  - L/M/R double click should generates a mouse down message, instead of letting GuiGraphicsHost and other places to do it.
- Allow different shortcut key per OS.
  - Update shortcut key builder in XML compiler.

### Optimization

- `VlppParser2` consider building ring buffers for each nodes as object pools (opt-out by default)
  - When opt-in, extra AST types and builders are generated.
  - New AST types will be different, containers become linked list, shared pointers become raw pointers.
  - Remote protocols should use it.
  - The new flat-layout option should be added to the whole parser instead of an ast group.
    - A new Parser class will be generated with a new builder, accepting a pool from the outside.
    - The pool will be un-typed, even list and string will be flat-layouted. No dtor is needed dor any AST object.
