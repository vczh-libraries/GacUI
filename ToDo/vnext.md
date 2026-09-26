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
  - Implement colorizer on `GuiDocumentViewer`.
  - Enlarging window slower than shrinking.
  - https://github.com/vczh-libraries/Vlpp/issues/9
  - `<RawElement/>` element.
    - In remote protodol, a `Raw` element is used to pass some raw data (binary or text) to the renderer.
    - Make an element base class for different platform-specific raw element implementation (check if exists).
    - Invent necessary construction to allow extending for future unknown elements.
    - UnitTest
    - Direct2D
    - GDI
    - GacJS
    - TUI
    - Wayland (`GuiWaylandElement` or a better name to expose rendering API?)
    - Cocoa (`GuiCocoaElement` or a better name to expoer rendering API?)
  - Add a metadata run to document model.
    - It has a text field for storing metadata in any application defined format.
    - It has a ordinary run for storing rendered document.
    - Metadata runs can be nested and is serializable.
    - markdown editor demo.
  - Need a way to handle registration failure of global shortcut key, so that an app knows the shortcut key is already taken, or converted to local shortcut key, or not working, instead of just crashing (at least on Windows).
    - Turn global shortcut key in `CppTest_Tui` back to `Ctrl+Shift+Alt+Win+Q` and render failure when anything happens.
    - `GacJS`/`wGac` should render it is treated as a local shortcut key.
- Remote Protocol
  - Is it able to pass text diff? Current `IGuiGraphidsParagraph` recreates so some implementation must be changed in the `GuiDocumentElement`.
- Windows OSProvider
  - L/M/R double click should generates a mouse down message, instead of letting GuiGraphicsHost and other places to do it.
- Allow different shortcut key per OS.
  - Update shortcut key builder in XML compiler.

### TUI

- [TUI_Resize.md](./TUI_Resize.md) for OP window management.
  - Need further investigation and see if the idea actuall works.
  
### Remote Protocol

- `RemotingTest_Win32_Core /FCT` typing performance issue.
  - Half way improved.
  - Will fast typing cause core with HTTP receiving characters in different order?

#### Renderer Switching

- A new `DebugRemoteProtocolWithMultipleRenderers.md`, declare how to test renderer switching, including native, gacjs, both.
  - Custom automation service port on native renderer is required so that two native renderers could exist at the same time, although only one is working.
- During switching renderers, remote protocol core might send old requests/messages to the new renderer. Figure out how to handle it.
  - Core is running single threaded, so remembering the client id might help.
  - Client id from `VlppOS`'s channel will be used to check if an underlying new `ControllerConnect` event is sent from a new renderer, as this part might be parallel, and `Submit` would say disconnected in this case.
- Make sure `DebugRemoteProtocolWithMultipleRenderers.md` works with `wGac` and `iGac`.

#### Refactor RemoteViewModelChannelServer

To remove dependency on `TaskQueue` in `RemoteViewModelChannelServer`:
- The async service is needed as a replacement.
- But creating `IViewModel` instance already needs a running task queue, but it is created before the main window, therefore before calling `Run(mainWindow)`, which then starts the async service.
- This is a bootstrap deadlock, need to carefully rethink the process.
- Or maybe the task queue is still not needed because creating `IViewModel` results in a blocking `OnJsonRequest` which handles callback internally.

### Optimization

- `VlppParser2` consider building ring buffers for each nodes as object pools (opt-out by default)
  - When opt-in, extra AST types and builders are generated.
  - New AST types will be different, containers become linked list, shared pointers become raw pointers.
  - Remote protocols should use it.
  - The new flat-layout option should be added to the whole parser instead of an ast group.
    - A new Parser class will be generated with a new builder, accepting a pool from the outside.
    - The pool will be un-typed, even list and string will be flat-layouted. No dtor is needed dor any AST object.
- Binary protocol for Workflow RPC
- Binary protocol for GacUI Remote Protocol

### GacGen

- Rewrite `GacBuild.ps1` and `GacClear.ps1` in C++, but still keep them just doing redirection for backward compatibility.
- Get rid of `Deploy.bat` in `GacGen.ps1` and `GacGen.exe`.
