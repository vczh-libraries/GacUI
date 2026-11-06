# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

address and fix the following bugs:

- When doing touch scrolling on a list control, you can use the `TouchingList` from `CppTest`, I press the list control and drag all the way to outside of the window on a touch screen, it crashes.
- When running GacJS with `RemotingTest_Core /Http /FCT`, and if I only use touching on a browser instead of using mouse, you can repro the issue by opening the document tab to insert an image, and touching suddenly becomes no responding. But mouse is still working. This issue does not repro on `CppTest` which is in hosted mode, but it repros when running `RemotingTest_Core /Http /FCT` with `Remoting_Rendering_Win32 /Http`, so I suspect this is a general remote protocol issue rather than a `GacJS` issue.

# UPDATES

## UPDATE

to clarify the second issue happens whenever a sub window is opened, meanwhile hosted mode without remote protocol is working just fine, hope this helps you narrowing down

# TEST [CONFIRMED]

- Extend the existing point-input tests to reproduce scrolling outside a hosted window, including boundary notifications and sequence completion.
- Reproduce touch-only modal opening through the remote protocol, verify a second touch reaches the dialog, and retain a mouse control case.
- Require both regressions to fail before fixes and pass afterwards. Run related unit tests and the focused hosted/remote automation checks; distinguish synthetic input from physical touch coverage.

Observed before fixes:

- Windows `InjectTouchInput` against Debug x64 CppTest reproduces the exception after adding enough TouchingList items and dragging above the native window. CDB stops in `vl::__vwsn::Ensure<demo::TouchingListItemTemplate>`, called by `demo::TouchingListTabPage::GetSwipeTemplate` from `EndSwipe`; the starting row has been virtualized away. The authored source is `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingListTabPage.xml`.
- With Core `/Http /FCT` and the native renderer `/Http`, renderer-side point input opens Control > Document Editor (Toolstrip) > Edit > Object > Insert Image. A subsequent touch on the file dialog's Cancel leaves it open; a mouse click closes it. Ordinary nonmodal Window Manager windows worked in this reproduction.
- The new nested-batch test in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_ChannelServer.cpp` fails: a nested pump receives newly arrived input before the undispatched remainder of the outer batch.
- The new modal point test in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp` fails with `GuiRemoteWindow::AcceptPointRecord#Point event index gap` when a click pumps its sequence's End reentrantly.
- Debug x64 builds the regressions without warnings or errors. The actual-resource interpreted TouchingList test tolerates the null strong-cast result, unlike generated C++; retain both native and interpreted verification.

# PROPOSALS

- No.1 Preserve optional row lookup and retire dispatched input before modal callbacks [CONFIRMED]

## No.1 Preserve optional row lookup and retire dispatched input before modal callbacks

TouchingList's visible-style lookup is optional by contract. Use a weak cast so scrolling, cancellation, or an additional contact can clear a swipe after virtualization without requiring a still-visible row.

Remote dialogs can reenter the event pump from a mouse fallback or deferred point click. Keep pending channel events and their consumed cursor in the channel, so a nested pump processes the old batch remainder before new events. Advance a validated point action's event index before calling application code. Retire a completed remote sequence and hosted routing before End callbacks, preserving the completed payload separately and leaving a nested sequence intact. Keep strict protocol validation and first-contact mouse fallback behavior.

### CODE CHANGE

- Change only the optional lookup in the TouchingList XML, regenerate its consumers, and extend existing scrolling/native/UIA checks.
- Update `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteProtocol_Channel_Async.h/.cpp`, `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteWindow.cpp`, and `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedController.cpp` for reentrant dispatch ownership.
- Cover accepted and rejected point clicks opening modal windows, pending-batch FIFO, starting-row virtualization, and fresh touch after completion. Verify actual HTTP native and browser touch paths.

### CONFIRMED

The starting list row is legitimately absent after virtualization, so the weak lookup lets all existing swipe cleanup paths finish. The native exception is gone without changing list capture or scrolling behavior. Remote dispatch now consumes queued events and advances each point record before calling application code; completion moves the old sequence out of active state before End callbacks. A modal loop can therefore finish the preceding input and start a new sequence without losing its route when the older callback returns.

Validation:

- Debug x64 remote channel, async renderer, and graphics-host mouse/point tests: 106/106 pass, with no memory leak report.
- Debug x64 point scrolling, file/message dialogs, and automation-service tests: 66/66 pass, with no memory leak report. Unrelated compiler ordering and asynchronous file-list allocation/frame timing changes in snapshots are excluded.
- Final Debug x64 build after generation: zero warnings/errors; all seven selected test files pass together, 172/172 cases, with no memory leak report. Metadata generation for both architectures and x64 Metadata_Test succeed. Debug Win32 also builds successfully; its two pre-existing character-code-page warnings are in the unchanged TestRemote_Caret.cpp.
- Debug Win32 remote channel, async renderer, graphics-host mouse/point, and point scrolling tests: 126/126 pass, with no memory leak report. Win32 metadata generation succeeds.
- Debug Win32 CppTest passes real Windows `InjectTouchInput` acquisition, including dragging above the window until the starting rows are virtualized and using fresh touch afterwards. Its UIA Touching suite passes the three new offscreen completion cases and normal shutdown (10,237 assertions).
- Debug Win32 CppTest_Metaonly and interpreted GacUI_Host pass the same UIA Touching suite and normal shutdown (10,241 assertions each).
- Final Debug x64 CppTest and CppTest_Metaonly pass the Touching UIA suites and normal shutdown (10,241 assertions each). CppTest also passes Windows `InjectTouchInput` for the outside-window drag, starting-row virtualization, and fresh touch, then closes through UIA.
- HTTP GacJS Chromium CDP touch: opens, cancels, and reopens the image dialog before any mouse input; then passes the existing gesture, list, momentum, selection, and palette checks. No JavaScript errors or touch-generated mouse/button protocol records.
- HTTP Direct2D native renderer: touch-only image-dialog opening, cancellation, and reopening, followed by all existing gesture, swipe, momentum, selection, palette, and shutdown checks pass. The test resolves the localized Cancel caption independently of Windows PowerShell's script code page and allows native coordinate rounding at fractional DPI. Full swipe input crosses the reveal threshold instead of landing on a rounding-sensitive boundary.
- HTTP Direct2D Windows input injection passes contact translation, multiple contacts, canceled Up, mouse fallback, outside-window TouchingList scrolling, fresh touch, and graceful shutdown. The optional driver initially failed to navigate from TextBox because Windows PowerShell returned the deep Controls JSON as a string; reproduced this independently, reused the remote driver's deep JSON parser, and reran all native injection checks successfully from that page.
- FullControlTest generation requires DarkSkin and fake-dialog localized resources to be loaded in the same compiler run. The normal inventory produced the complete x86 FullControlTest artifacts. Stopped its unrelated remaining generation and ran a focused x64 FakeDialog/DarkSkin/FullControlTest pass after identifying both dependencies; all temporary compiler source switches are restored. Unrelated generated artifacts are excluded.
- Both generated C++ variants change only the nullable row lookup; matching resources are regenerated with no remaining UI compilation errors. TouchingList snapshots record the additional recycling operations; the final rendered state matches the original after normalizing allocation IDs. Unit-test filters and application launch arguments are restored.

Coverage uses Windows input injection, automation/UIA, and Chromium CDP touch. Physical fingers and additional monitor DPI remain hardware coverage.
