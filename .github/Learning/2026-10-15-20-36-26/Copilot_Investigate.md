# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Implement the macOS raw-element port, then commit and push all local changes.

The common raw-element contract, Windows and browser painters, and Wayland port are completed prerequisites.
Mirror the Wayland raw-element implementation and verification to `iGac`. Skip WASM work items as they are not ready for macOS yet.
Mirror `wGac` changes to `iGac`, follow `.github/Jobs/job.rpXPlat.prompt.md` to do full verification.
You are going to use `GuiCocoaElement` as macOS's `<Raw/>` implementation.

# TEST

Verify the Cocoa raw factory, common state, callback context and bounds, clipping, graphics state and current path, fixed clock pixels, target replacement, remote collection identity, ID/data transitions, late attachment and retained pictures. Run the official import/sync/build sequence twice, the focused native suite in standard and hosted modes, and the GacUI unit suite. Recreate and complete the macOS native-renderer, WebKit MiniHTTP and TUI matrices from job.rpXPlat. Wasm is excluded by request.

# PROPOSALS

- No.1 Implement GuiCocoaElement with callback-scoped CoreGraphics drawing and macOS-owned clock attachments.

## No.1 Implement GuiCocoaElement with callback-scoped CoreGraphics drawing

Derive the reusable element from GuiRawElementBase with concrete factory helpers. Register the renderer in CoreGraphicsMain and return it from CreateRawElement. Borrow the active CGContext for each Rendering callback; preserve clipping, transform, drawing state, text matrix and current path. Keep clock drawing and identity-based collection attachment under <iGac repo>/MacShared, compiled into the native demo and remote renderer. Author the macOS remote hook in <GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp so synchronization preserves it.

### CODE CHANGE

Implemented `GuiCocoaElement` and registered its CoreGraphics renderer and native factory in <iGac repo>/Mac/GraphicsElement/CoreGraphics. Each callback borrows the current render target context with logical bounds, scale and clipping intact. Graphics state, current path and text matrix are restored afterward. There is no cached context or Raw-specific minimum size.

The local and remote FullControlTest attachments live in <iGac repo>/MacShared. They draw only the exact `FullControlTest.Clock` ID from valid `HH:mm:ss` data. The remote attachment subscribes before enumeration and tracks element identity, so duplicate/empty IDs, late installation, ID changes and retained disconnected pictures are supported. The synchronized Apple hook is authored in <GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp.

The initial import exposed unrelated line-ending churn. <iGac repo>/import.sh now reproduces the committed snapshot conventions. Apple legacy typedef collisions were resolved by qualifying GacUI types. The Quartz test fixture was corrected to use the native flipped coordinate system; production rendering did not require a coordinate workaround.

### VERIFICATION

- CodePack, two complete import/sync sequences and the official full iGac build passed. Synchronization preserved the platform helper files and target references.
- Four focused native Raw tests passed. Hosted factory, callback-state and fixed-clock tests passed; the real native-window remote lifecycle case is intentionally native-only. Coverage includes clipping, backing scale, text matrix/path isolation, strict clock input, target recreation, data-only redraw, identity transitions and retained pictures.
- GacUI unit tests passed: 94 files and 1,837 cases. Generated build inventories and macOS snapshot changes are retained separately from authored code. Snapshot differences include file enumeration order, supplementary-character representation and Workflow member order.
- Local, hosted and remote Cocoa clocks were visually inspected across input-free ticks, palette changes and renderer replacement/takeover. Native, WebKit MiniHTTP and TUI results and limitations are recorded in <iGac repo>/TestMatrix_NativeRenderer.md, <iGac repo>/TestMatrix_GacJS.md and <iGac repo>/TestMatrix_Tui.md. Wasm is excluded by request.
- LLDB launch required Developer Tools Access authentication; its probe was canceled and owned processes stopped. Accessibility subsequently returned `-25211`, preventing native alert dismissal and physical/global shortcut verification. Those checks are not marked passed. After the user's window-targeting warning, all input was confined to process-specific HTTP automation or an owned PTY master fd; desktop input helpers were disabled.

## No.2 Preserve the accepted browser view-model host after renderer takeover

### EVIDENCE AND CODE CHANGE

Live WebKit verification reproduced the failure three times: a browser-hosted RVMT session worked until an ordinary renderer took over, then the next RPC reported `RemotingTest_RvmHost disconnected.`. The entry page's unconditional `session.stop()` disposed the accepted host together with the replaced renderer.

<GacJS repo>/Gaclib/website/entry/assets/index.html now retains the host after renderer disconnect/connection-stopped outcomes while disposing renderer and clock state. Host completion, page teardown and fatal outcomes still stop the entire session. Built-page tests cover disconnect, takeover, fatal cleanup, repeated pagehide and host completion after takeover. GacJS documentation describes the resulting lifetime.

### VERIFICATION

GacJS import, codegen, full build and tests passed, including 28 built-page/session tests. Live WebKit covered all five RVMT host modes with Unicode RPC, renderer replacement, rejected additional hosts where applicable, normal exit and idle/blocked host loss. The browser-host blocked-loss test retained a real request and response before stopping the host; Core terminated with the exact fatal mask and one matching page error. Windows-only live suites retain their expected macOS skip.
