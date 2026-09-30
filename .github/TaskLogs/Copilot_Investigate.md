# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

The task is about starting GacUI in GacJS with wasm.

The first step is to implement a remote protocol communication layer for wasm.
Files should be put in `Test/RemotingHelpers/RemotingServer/Wasm`.
All files there should be in `#if defined VCZH_WASM`.
Currently only WASM platform is using it, so no need to add them to any vcxproj.
In this folder you are going to maintain one or more pair of C++ source files, exposing some text exchanging functions to JavaScript, their signatures look like this in TypeScript
```TypeScript
function SendDataToWasmCore(data: string):void;
function StartApplication(receiver: function(data:string):void):void;
```
Renderer starts core in a worker thread using `StartApplication` and install a receiver function.
Renderer talks to core using `SendDataToWasmCore`.
Core talks to renderer using the receiver.
All functions should be in `EMSCRIPTEN_BINDINGS(GacUIWasmApplication)`

Although WASM communication layer is an `INetworkProtocolServer` but it works like having a remote client already installed because the receiver and the `SendDataToWasmCore` is working like a singleton. Global variables will be needed but keep them minimum, e.g., to store the receiver and the singleton `INetworkProtocolServer`. `StartApplication` can just check if the `INetworkProtocolServer` pointer is already there, does nothing and return if yes.

You can change the signature of exposed functions if anything more is needed to implement `INetworkProtocolServer`.

The second step is to implement `Test/Linux/WasmFCT` and `Test/Linux/WasmRPT`:
- The first one launches `FullControlTest`.
- The second one launches `RemoteProtocolTest`.
- The second one launches `RemoteViewModelTest`.
They should be built with web assembly. Although many files will be generated to their `Bin` folder, but everything will be ignored except `app.wasm` and `app.mjs`.

The third step is to use them in `GacJS` repo. In `GacJS/Gaclib/website` we already have an `index.html` talking to a core application live. Now we do:
- `/wasm-fct/index.html`
- `/wasm-rpt/index.html`
- `/wasm-rvmt/index.html`
  - In this web page you need to addicionally implement a view model in TypeScript, you can check out how the original `index.html` is doing.
They will be put to assets, and after building `website` you could find them in `.../entry/lib/dist`.
You need to manually copy those wasm files to this folder say `.../dist/wasm-(fct|rpt|rvmt)/app.(wasm|mjs)`, sitting in the same folder having `index.html`.
**IMPORTANT**: DO NOT copy those files during `npm run build`, instead you should just add a `GacJS/copy-wasm.sh`, assuming `website` and `GacUI` already built, and it just copy those app files to `dist`. If `dist` or `app.*` is missing it warns and stops.
You are going to create a `remote-protocol-wasm` package just like `remote-protocol-http` but they calls exposed functions.
Now after building the project, you should start `entry` and test if those UI is working, you can follow `GacUI/Jobs/DebugRemoteProtocolSop.md` to try every feature.

Currently it only works on Linux, add `GacUI/.github/Jobs/job.rpWasm.prompt.md` to describe how to build and run the test, follow the language in `job.rp(Windows|XPlat).prompt.md`, short, informative, precise.

commit and push all local changes.

# UPDATES

## UPDATE

FYI to correct my mistake, there should be 3 more wasm test apps created in GacUI, and they should use VCZH_DEBUG_NO_REFLECTION, built with `-o` before copying to GacJS.

## UPDATE

I would like you to perform a small refactoring as a follow up

1) Currently it seems `Wasm(FCT|RPT|RMVT)` test apps are using the x86 version of generated files (e.g. darkskin). This is incorrect, there are actually merged versions which support both x64 and x86 in `Source/Skins/DarkSkin/Source`. Twist `vmake` a little bit to do this.

2) I don't think we need `Replace Renderer` in wasm pages. As we have planned, move `ConnectToWasmCore` functionalities to `StartApplication`, but instead add a `Reload` button, default to disabled. When `Exit` or `Force Exit` is clicked and when the application actually closed, `Reload` becomes enabled, clicking it reload the application and it becomes `disabled` again. Instead of destroy and reconnect a renderer, we are going to actually restart the application, anything state will be lost, view models should be recreated, and that's fine. In this case we no longer need `ConnectToWasmCore` and `DisconnectFromWasmCore`.

Rerun verification to test if all 3 pages are working.

## UPDATE

plus make sure `Wasm(FCT|RPT|RVMT)` could be built with `-f -r` but in this case only create a CLI application that does nothing, aka compile all the code into the binary, with an empty main function return 0.

## UPDATE

I think this is doable because in wasm adding a main function seems no side effect

## UPDATE

or you can just #ifdef VCZH_GCC the main function anyway

## UPDATE

My mistake, I mean -f -o not -f -r

# TEST [CONFIRMED]

The requested Wasm transport, three Wasm application configurations, browser package/pages, and copy script are absent from both checkouts. This is a new capability, not a failing existing regression.

Verify three `build.sh -bw -o` builds using merged DarkSkin, the x86 demo sources and `VCZH_DEBUG_NO_REFLECTION`; only `Bin/app.mjs` and `Bin/app.wasm` are tracked/deployed. Also fully build each native project with `build.sh -f -o`, selecting x64 demo sources, and require its CLI to exit silently with code 0. Rebuild Wasm after the native checks. Run GacJS import, codegen, build, and unit tests. Serve entry with its launcher and run browser operations from DebugRemoteProtocolSop.md against every Wasm page, including Unicode input, rendering, orderly shutdown, RPT fatal errors and live TypeScript RPC. Verify Reload is disabled while starting/running and after canceled Exit, becomes enabled only after successful Core completion, and recreates clean application/view-model state after both Exit and Force Exit. Check copy-script missing-input behavior. Record browser-only limitations and test results below.

# PROPOSALS

- No.1 Reuse the channel and renderer stacks over a worker-owned Wasm transport [DENIED]
- No.2 Create transport connections at startup and reload the whole Wasm application [CONFIRMED]

## No.1 Reuse the channel and renderer stacks over a worker-owned Wasm transport

Implement a test-only `INetworkProtocolServer` in Test/RemotingHelpers/RemotingServer/Wasm. A dedicated browser worker owns the module and callback; a C++ application thread runs blocking GacUI code. Marshal ordered owned text back to the module worker. Keep one application/server singleton and use explicit logical connection IDs so RVMT can retain independent host and renderer channel admission. Preserve existing JSON channel and generated RPC contracts. Export exception-translating Embind wrappers in GacUIWasmApplication and convert UTF-16 at the boundary.

Use three separate no-reflection build configurations, sharing application composition and selecting the proper generated x86 demo at compile time. Package pthread bootstrap into app.mjs so deployment needs only the requested two app files. Extend canonical Tools/Ubuntu scripts for the explicit optimized Wasm invocation and optional embedded worker packaging; propagate with vgo uci.

Add a remote-protocol-wasm package with worker transport and renderer composition, three static asset pages, a TypeScript view-model host, explicit copy-wasm.sh, static-server Wasm MIME/isolation headers, documentation and a Linux job. The website build must not copy Wasm outputs.

### CODE CHANGE

- Added the guarded test transport and shared application composition. Each demo has an independent optimized, no-reflection build configuration; RVMT uses the existing broker, requester and generated x86 RPC implementation.
- Added the GacJS worker/channel package, shared renderer adapter, three static pages, TypeScript RVM host, renderer replacement control, explicit copy script and MIME/isolation headers. Added portable channel tests and a separate Linux browser suite, plus the requested job and operating documentation.
- Extended canonical Tools/Ubuntu packaging to accept explicit Wasm `-o` and embed the pthread bootstrap. Initial browser startup exposed a relative `app.mjs` import from a Blob worker; supplying `mainScriptUrlOrBlob: import.meta.url` fixes its base URL. Changes were committed/pushed in Tools before propagation with `vgo uci GacUI`.
- GacJS import/codegen exposed an outdated keyboard-header path: VlppOS moved the declarations into `WindowTypes.h`. Updated the importer, generator, parity test and documentation, then regenerated the outputs normally.
- Optimized browser startup exposed UTF-16 corruption in the Vlpp amalgamation: `ready` became `yyyyy`, and an exported exception became repeated periods. Temporary boundary diagnostics confirmed the original UTF-32 string was intact and `wtou16` was already corrupted before Embind. The optimized inlined converter deferred writes through `vuint16_t*` while reading `char16_t*`. Changed the owning Vlpp UTF-16 converter to read/write its actual character types, regenerated `Vlpp/Release/Vlpp.cpp` with CodePack, and copied it into GacUI/Import. Removed diagnostic instrumentation. The standalone Vlpp Wasm suite passed both before and after this fix (32 files, 473 cases), showing why the amalgamated browser build is required to cover this optimization-dependent failure.
- Fatal-path browser tests exposed window destruction throwing during exception unwinding. Broadcast the original Core error inside `GuiMain`, before unwinding, as the native test app does. The browser immediately terminates a fatally failed session and rethrows that original channel error; only normal shutdown waits for Core finalization and RPC service release. This keeps a secondary Wasm abort from replacing the original RPT or host-loss message.
- Fatal cancellation uses `Worker.terminate()` directly instead of queuing a command into the failed runtime. Initial worker-lifetime checks used Vitest's one-second polling deadline and falsely reported remaining workers: [Chromium allows two seconds before forcibly terminating busy worker execution](https://chromium.googlesource.com/chromium/src/+/HEAD/third_party/blink/renderer/core/workers/worker_thread.cc). The tests now wait for actual Chromium worker targets to disappear with a 15-second deadline, and also close a live RVMT page. They verify targets exist during operation; no fixed delay substitutes for termination. Normal shutdown still finalizes Core first. No special packet interception or custom pthread shutdown is needed.
- Normal completion also terminates the worker from its owning page after receiving Core's return code. Calling Emscripten pthread cleanup from the module worker intermittently left that worker alive after RPT exited; relying on the browser's worker lifetime removes that teardown race.
- Browser tests wait for the expected DOM changes after input and use renderer idle signals for layout. They exercise active renderer takeover and inspect its normal `ControllerConnectionStopped` result, all five mouse buttons and modifier combinations, both wheel axes, double clicks, palette colors, UTF-16 RPC text and worker termination.

### DENIED BY USER

The follow-up supersedes the architecture-specific DarkSkin input and dynamic renderer replacement in this proposal. Its earlier verification remains recorded below as historical evidence. No.2 retains the shared protocol stack but uses the merged DarkSkin sources and a fresh application lifecycle on Reload.

All three apps were built in GacUI with `build.sh -bw -o`. Compiler/linker logs contain `VCZH_DEBUG_NO_REFLECTION` and `-O3`, with the x86 generated resources. The six deployed files match their build outputs byte for byte; only each app's `Bin/app.mjs` and `Bin/app.wasm` are tracked. No native Core or external RPC host was running for the browser tests.

The final Linux Chromium run passed all seven tests in `GacJS/Gaclib/website/entry/test/Testing_Wasm.js` (67.34 seconds):

| App | Verified operations | Result |
| --- | --- | --- |
| WasmFCT | Add/clear both 0–9 lists; exact search and rich-editor markers; bracket key codes; retain both editors across tabs, Aurora palette refresh and renderer replacement; shortcut labels/activation; five mouse buttons, double clicks, modifiers and both wheel axes; Force Exit | Passed; Core returned 0 and all workers disappeared |
| WasmRPT | Home button; three complete DataGrid rows and Clear; document hyperlink/dialog; repeated renderer takeover with retained state; shortcuts and full mouse matrix; confirmed File-menu close; injected fatal error | Passed; normal Core returned 0; fatal packet and browser error preserved exactly `This is a fatel error!` |
| WasmRVMT | Initial/repeated TypeScript Translate; rejected second host leaves original usable; renderer replacement retains state; `WasmUTF16你好` round trip; Force Exit; accepted host loss before the next RPC and while a reply is withheld; closing a live page | Passed; normal Core returned 0; both host-loss errors preserved exactly `RemotingTest_RvmHost disconnected.`; no worker targets remained |

Additional verification:

- Native GacUI: 93/93 files, 1813/1813 cases passed after refreshing the Vlpp import.
- Native optimized Vlpp: 32/32 files, 467/467 cases passed. Browser Wasm Vlpp: 32/32 files, 473/473 cases passed with `wasm_main returns 0`.
- GacJS import, codegen and build passed. All 11 packages' final test commands passed, including the five new Wasm channel cases and native Workflow stdio RPC integration. Generated imports and snapshots were refreshed through the normal generators.
- `copy-wasm.sh` passed missing-destination, missing-final-input, no-partial-deployment and successful-copy fixture checks. A fresh website build produced all three HTML pages without deploying any app.mjs/app.wasm; the explicit copy step deployed them afterward.
- Source/configuration diffs and shell syntax checks passed. Emscripten's generated app.mjs files retain the compiler's whitespace; they were not manually reformatted.

Coverage is Linux Chromium browser input. Native OS global hot keys, browser-reserved key combinations, and Windows/macOS browser runs were not tested. Each page owns an independent Core; same-page renderer replacement is the supported takeover operation.

One intermediate native Workflow stdio integration run aborted in `Collection_Interface_Nested_InByval_OutByref` with `vl::Error`; the isolated rerun and subsequent complete package runs passed. A full rerun under LLDB with a breakpoint on `vl::Error::Error` exited 0 without hitting the breakpoint; the intermittent abort was not reproduced and no Workflow source was changed.

## No.2 Create transport connections at startup and reload the whole Wasm application

Compile DarkSkin from `Source/Skins/DarkSkin/Source`, including its merged resource implementation, and regenerate the three makefiles through `build.sh -bw -o`. The application demos have only architecture-specific generated source directories in this checkout, so their Wasm32 sources and RPC metadata remain x86; shared dialogs already use their merged sources.

The native follow-up uses the corrected command `build.sh -f -o`. Select x64 demo sources for native compilation through a make variable, keeping x86 for Emscripten. A `VCZH_GCC`-guarded `main` returns 0; an empty `GuiMain` satisfies the linked GacUI application entry reference. Native builds compile/link the full common library, merged skin and demo source inventory, without starting GacUI. No shared build-tool changes are needed.

Keep only `StartApplication` and `SendDataToWasmCore` as application Embind exports. Startup declares the fixed number of transport connections (renderer plus an optional separate RVMT host) and installs their callbacks when the server starts, before `ready`. Channel handshakes still assign channel client IDs. Remove dynamic connect/disconnect commands and renderer takeover from the Wasm package and pages.

Replace the button with Reload, disabled during startup and operation. Enable it only after normal Core completion, including RPC finalization, then use `location.reload()` to create a fresh worker/module, HTML renderer and TypeScript host on click. The new page discards old DOM/masks and Reload is disabled immediately before navigation and by default in HTML. Retain fatal error reporting and page-close worker termination.

### CODE CHANGE

- Updated the shared vmake input to the merged DarkSkin header, implementation, reflection and resource files; makefiles and binaries are regenerated through the normal build wrapper.
- Added the native empty entry points and architecture selection to the shared app/vmake. Verification also requires all three full optimized native builds and successful, silent CLI exits before the final Wasm rebuild and browser deployment.
- `StartApplication(receiver, connectionCount)` records the fixed transport count. Server startup installs those connections before `ready`; removed both connection-management exports and their C++ methods. The ordinary channel handshake still runs afterward, preserving Core client ID 1 and RVMT's service-before-renderer ordering.
- TypeScript declares all channel sets when constructing `WasmApplication`; the worker accepts only startup and data commands. Page Reload uses browser navigation after successful Core completion. Removed renderer-generation/takeover handling and the obsolete independent-host disconnection browser cases.
- Updated portable channel tests, browser feature/reload tests and operating documentation/SOP. Verification will rebuild all three optimized no-reflection apps, run GacJS import/codegen/build/test and browser tests for features, Exit cancellation, both shutdown buttons, repeated Reload with lost UI state and a recreated RVMT host, fatal errors and worker cleanup. Native GacUI unit tests can be skipped under Project.md's test-app-only exception; no shared GacUI implementation or generated skin content is being changed.

### CONFIRMED

All three projects passed a full native `build.sh -f -o` build and their CLI binaries exited with code 0 and no output. Compiler logs show `-O2`, `VCZH_DEBUG_NO_REFLECTION`, merged DarkSkin sources and x64 demo sources. FCT/RPT/RVMT compiled 183/184/183 translation units respectively, including the full common library and demo inventory. The empty native entry points satisfy linking without starting an application.

All three projects were then rebuilt successfully with `build.sh -bw -o`. The final logs show the same source inventory with merged DarkSkin, x86 demo sources, `VCZH_DEBUG_NO_REFLECTION` and `-O3`. The application binding contains only `StartApplication` and `SendDataToWasmCore`; both removed export names are absent from all three final Wasm binaries. The explicit copy step deployed the six app files, and their SHA-256 hashes match the GacUI outputs.

The final Linux Chromium run passed all five browser tests in `Testing_Wasm.js` (56.66 seconds):

| App | Verified operations | Result |
| --- | --- | --- |
| WasmFCT | Both lists; search/rich editors and bracket key codes; editor retention across tabs; Aurora palette; shortcuts and mouse matrix; Exit and Force Exit followed by Reload | Passed; edited text and palette reset, input works after restart, every normal Core returned 0 and its workers disappeared |
| WasmRPT | Home button; DataGrid and Clear; document dialog; shortcuts and mouse matrix; canceled/confirmed Exit; Force Exit; repeated Reload; File-menu close; fatal error | Passed; canceled Exit leaves Reload disabled, restart resets the button state, normal Core returned 0, fatal error remains exactly `This is a fatel error!` with Reload disabled |
| WasmRVMT | Initial/repeated Translate; `WasmUTF16你好` round trip; Exit and Force Exit followed by Reload; new input after each restart; live-page closure | Passed; each new host starts with `Hello, !` and translates new input, normal Core returned 0 and no worker targets remained |

Every page starts with Reload disabled. It becomes enabled only after successful Core completion; clicking it disables it and navigates to a fresh page, creating new module, renderer and host state. Startup-only transport connections preserve the existing channel handshake and RVMT service acquisition order, so separate connect/disconnect exports are unnecessary.

Additional verification:

- GacJS import, codegen and build passed. All 11 packages' test commands passed, including five Wasm channel cases and native Workflow stdio RPC integration.
- A normal website build left all three HTML pages present without deployed Wasm files. Calling `copy-wasm.sh` while a native full build had removed a Wasm input failed before deploying any files; the final explicit copy succeeded.
- Source/configuration diff checks passed. Generated makefiles and `vmake.txt` came from the build wrapper; generated `app.mjs` formatting was retained. Only `app.mjs` and `app.wasm` are tracked from each `Bin` directory.
- Native GacUI unit tests were skipped under Project.md's test-app-only exception. No production GacUI or generated skin source changed in this follow-up.

Coverage is Linux Chromium. Native OS global hot keys, browser-reserved shortcuts and Windows/macOS browser runs remain outside this verification. The owned entry server was stopped after testing.
