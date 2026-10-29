# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

[TODO_Injection4.md](TODO_Injection4.md) , in GacJS, implementation of renderer side filesystem/datetime/locale should be put in GacJS/Gaclib/gaclib/renderer/src/injections/inject(DateTime|Locale|FileSystem).ts, from GacUIRendererImpl.ts functions redirect to what are externed from these files, these requests must be grouped like others.

## Referenced task

This is a continuation of previous works:
- TODO_Injection.md
- TODO_Injection2.md

1. Fix `RemotingTest_Core`. When starting it with `/Http`, it crashes without admin permission because it listens to +:8888.
I would like you to do a fallback, if listening to +:8888 does not work, fallback to localhost:8888 instead. And the `HttpServer` class will expose a "GetLocalhostOnly()" property telling you the choice, and `RemotingTest_Core` needs to say it listens to `localhost` only when returning true.

### DETAILS

- The owning implementation is `<VlppOS repo>/Source/InterProcess/Windows/HttpServer.Windows.cpp`. Registration occurs in the `HttpServerApi` base constructor in `<VlppOS repo>/Source/InterProcess/Windows/HttpServerApi.Windows.cpp`, before `HttpServer::Start()`. Catching only a failure from Core's `Start()` cannot implement this fallback.
- Attempt the wildcard registration first, then exactly the equivalent localhost registration if it fails. Preserve the constructor's port, base path and trailing slash; do not hard-code Core's endpoint in the library. Restrict this recovery to URL registration, preserving other initialization failures and existing explicit-prefix callers such as the Windows automation service.
- Expose `bool GetLocalhostOnly()` from `HttpServer`: false after successful wildcard registration, true after successful localhost fallback. If both registrations fail, terminate through the existing error path with the attempted URLs and Windows error codes; do not silently switch transport or port.
- Localhost registration still undergoes HTTP.sys access checks, so fallback is not a guarantee of permission on every nonadministrator account. The property identifies the selected URL prefix, not a socket-level loopback restriction. See Microsoft's [registration API](https://learn.microsoft.com/en-us/windows/win32/api/http/nf-http-httpaddurltourlgroup) and [URL-prefix rules](https://learn.microsoft.com/en-us/windows/win32/http/urlprefix-strings). Do not add automatic URL reservations or elevation.
- In `<GacUI repo>/Test/GacUISrc/RemotingTest_Core/GuiMain.cpp`, report localhost-only operation after successful listener startup and before waiting for a view-model host. Cover all channel-server construction branches, including `/RVMT` with `/Cli`. Protocol port 8888 remains separate from the `/AsPort` automation setting.
- Publish the VlppOS changes through the prescribed release/import workflow; do not hand-edit amalgamations. Update the affected descriptions in `<GacUI repo>/Project.md` and `<GacUI repo>/Test/RemotingHelpers/README.md`. The current no-fallback rule in `<Tools repo>/Copilot/KnowledgeBase/KB_VlppOS_InterProcessNetworkProtocolsAndChannels.md` is superseded by this task; update that canonical guidance and synchronize its copies.

### VERIFICATION

1. During implementation, use the repository wrappers to build and run relevant VlppOS HTTP tests and the required downstream GacUI checks after import. Cover wildcard success, wildcard failure followed by localhost success, and failure of both registrations. Assert the property and diagnostics, including a non-default library port/base path.
2. Run Core unelevated with `/FCT /Http`, connect a localhost renderer and complete a real UI interaction. Require the notice only when fallback actually occurred, and verify Core's independent automation endpoint. Record actual URL-registration results; an elevated wildcard-success run does not verify unelevated fallback.
3. Exercise `/RPT /Http`, `/RVMT /Http` with a network host, and `/RVMT /Http /Cli:<host-path>`. Smoke-test `CppTest_Rvm /Http`, which shares the modified transport. Preserve ordinary shutdown and listener restart behavior, and inspect Debug leak output.

2. In `wGac` and `iGac`, `test.sh` accepts `-remoteFS` and other arguments, it should use double -. Fix them and update their markdown files accordingly. Do not update the document if the argument is not mentioned. If C++ apps accept single - you should also change it to use double -. No verification is needed.
Fix `TODO_Injection3.md` accordingly as this task is about to submit to `iGac` execution, in this case `iGac` knows that double - is the correct one.

### DETAILS

- Normalize the three injection options to `--remoteDT`, `--remoteLocale` and `--remoteFS` in both `<wGac repo>/test.sh` and `<wGac repo>/test_core.sh`, and both equivalent iGac scripts. The other long wrapper options already use double dashes.
- Update usage strings, parser cases and forwarding together. The existing `"/${argument#-}"` strips one dash; double-dash input requires `"/${argument#--}"` to avoid emitting the invalid `/-remoteFS`. Preserve foreground and `--unblock` forwarding.
- Keep executable slash switches unchanged. `<GacUI repo>/Test/GacUISrc/SharedArguments.h` accepts `/remoteDT`, `/remoteLocale` and `/remoteFS`; the platform entry points delegate to it and currently accept no single-dash injection switches. No C++ spelling change is needed. Shared parser and renderer composition remain owned by GacUI and copied by the platform synchronization scripts.
- Update existing descriptions in `<wGac repo>/AGENTS.md`, `<wGac repo>/README.md`, `<wGac repo>/README_CN.md`, `<iGac repo>/AGENTS.md`, `<iGac repo>/readme.md` and `<GacUI repo>/Project.md`, including wording that calls these options single-dash. Do not add unrelated argument documentation. Preserve historical executed commands in `<wGac repo>/TestMatrix_Injection3.md`, with a note giving the new spelling for future runs.
- In `<GacUI repo>/TODO_Injection3.md`, update the wrapper-option table and both launcher examples to double dashes, retaining the executable slash column. State that the launcher normalization must be present before iGac executes that task.

### VERIFICATION

- No verification is required for this subtask, as requested.

3. Update `GacJS` to implement all 3 injections. File system implementation will follow WASM's core side implementation.
- In `Gaclib/website/entry`, currently we have `?rvmhost`. We need to extend it to:
  - ?rvmhost&remoteDT&remoteLocale&remoteFS, case insensitive, all optional.
  - `inject*` decides how `ControllerGlobalConfig::inject*` is set.
  - Add 4 check boxes and a "REFRESH" button to the first row, when "REFRESH" is clicked, each check box decides if options after "?" is used, and jumps to the current URL with new options appended. All default to unchecked.
  - Additionally, support `corehost=localhost`, with a check box and a text box, text box should be disabled when the check box is not checked.
- Before running `GacJS` to verify, local changes are recommended to push first, in order to verify:
  - You can change `GacJS` to offer fake implementation, for example, a different locale and format list, a different time zone, a fake file system, and see if they correctly affects dedicated pages in `FullControlTest`. Since local changes are already pushed, you can easily identify code that offers fake implementations and revert them once they are no longer needed. DO NOT check in fake implementations, they are meant to be discarded after finishing this task.
  - You need to turn options in core/renderer on and off and make sure core picks the right implementation.
  - When a renderer disconnects, you can still use core side automation service to read the UI and see if fallback works.
- Update `GacJS`'s document to simply introduce these new features.

### DETAILS

- Add the controls and query parsing to the HTTP entry page in `<GacJS repo>/Gaclib/website/entry/assets/index.html`. The four checkboxes represent `rvmhost`, `remoteDT`, `remoteLocale` and `remoteFS`; the additional `corehost` checkbox is a fifth checkbox with a text box initially containing `localhost`. Absent options are unchecked; when an option is present in the current URL, initialize its control accordingly. Disable the host text box while its checkbox is unchecked.
- Query parameter names are case insensitive. Map `remoteDT`, `remoteLocale` and `remoteFS` to `ControllerGlobalConfig.injectDateTime`, `.injectLocale` and `.injectFileSystem`; the `inject*` wording above does not introduce additional query names. Parse flags by presence and start all three advertised capabilities false when omitted. Pass these settings to the connection handshake in `<GacJS repo>/Gaclib/website/remote-protocol-http/src/rendererClient.ts`. `rvmhost` independently controls the browser view-model host.
- REFRESH rebuilds the current URL from the selected controls and navigates to it. Replace or remove all existing case variants of these options rather than appending duplicates; preserve the pathname, fragment and unrelated query parameters. A checkbox change alone does not alter an established connection.
- An omitted `corehost` uses `localhost`. Validate an enabled value as a hostname, IPv4 address or IPv6 address, without a scheme, path or separate port; preserve the value's case and bracket IPv6 correctly when constructing URLs. Pass the same destination to both the renderer and the optional browser view-model host. The remote-protocol port remains 8888.
- The new entry controls configure the HTTP client, not Core's injection preferences. Start native Core with the matching `/remoteDT`, `/remoteLocale` and `/remoteFS` switches. Keep the shared renderer handlers independent of transport. The existing Wasm demo pages have a separate startup path and Core currently calls `SetupRemoteNativeController` with all options false; adding Wasm startup switches is not necessary for this HTTP-entry change, and browser flags alone must not be presented as enabling Wasm Core injection.
- Synchronize the current protocol using `<GacJS repo>`'s import/code-generation workflow before implementing handlers. Its current generated protocol lacks the injection messages and capability fields. Do not hand-edit generated packages. Account for other new required handlers brought in by synchronization, including `ImageSave` when verifying document persistence with embedded images; image encoding is independent of filesystem injection.
- Preserve the existing Core contracts in `<GacUI repo>/Source/PlatformProviders/Remote/GuiRemoteControllerInjection.h/.cpp` and `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_Injection.txt`. A feature is remote only when Core enables it and the connected renderer advertises it. Otherwise DateTime/Locale use the predecessor; an enabled but unavailable filesystem exposes an empty read-only `/`, never Core storage. Renderer replacement reevaluates capabilities.
- DateTime implements the two timezone conversions using the browser's rules for the requested date, including daylight-saving transitions and milliseconds. Exchange calendar fields, not native timestamp internals. Core retains its UTC clock, construction and arithmetic; do not replace its clock with the browser's current time.
- Locale must cover every `InjectionLocaleOperation`: defaults, enumeration, format lists, day/month names and case mapping. Formatting, comparison and search remain on Core. Return format patterns understood by Core's formatter, not already formatted sample strings or JavaScript option objects. Define the browser's available-locale list and invariant/default mapping explicitly rather than claiming that preferred languages enumerate every supported locale. Keep the behavior for an explicitly requested locale distinct from the enumeration list.
- As clarified, the filesystem is browser storage: follow `OpfsFileSystemImpl` in `<VlppOS repo>/Source/FileSystem.Wasm.cpp`, documented in `<GacUI repo>/.github/KnowledgeBase/KB_VlppOS_FileSystemOperations.md`. Use the page origin's Origin Private File System (OPFS), not the native Core filesystem, a file-picker grant or an in-memory substitute. Data persists across page refreshes in that origin; do not clear it as part of startup or REFRESH. Document the secure-context/storage requirements and ordinary unavailable-storage behavior.
- Match the Wasm path and operation contracts: `/` is the root and working directory, separators are `/`, normalize `.` and `..`, reject traversal above root, and preserve Unicode names. Cover all `InjectionFileOperation` values. Report supported metadata honestly; unavailable timestamps remain absent. Metadata-preserving copy returns false without modifying the destination because OPFS cannot preserve source timestamps. Rename follows the existing copy/remove behavior, with its destination/root restrictions.
- Match Wasm stream behavior too: ReadOnly and ReadWrite snapshot the whole file; ReadWrite preserves existing bytes and creates missing files, while WriteOnly starts empty. Successful open returns a non-null binary payload even for an empty file. Commit replaces the entire content, including zero bytes, and responds only after persistence completes. Preserve Base64 byte transport, read-only behavior and the distinction between missing files and empty files. Core already buffers the returned content and commits on close; its native renderer's truncate-on-ReadWrite behavior must not be copied into the OPFS implementation.
- OPFS is asynchronous. Integrate completion with the renderer's request/response dispatch so each request receives its matching response after the operation finishes, dependent operations stay ordered, and rejected promises do not leave Core waiting indefinitely. Preserve normal filesystem failure results and connection ownership; delayed work must not respond through a replacement connection. Existing streams remain bound to the renderer that opened them, and closing one after disconnect/replacement intentionally fails.
- REFRESH with `rvmhost` unloads the browser-owned view-model host. Losing an accepted host is fatal to the current RVMT Core, so restart that Core for such a refresh instead of adding host recovery. Use FCT, or a separately hosted RVMT session, for renderer-only disconnection/replacement checks. Update existing entry usage and the relevant descriptions in `<GacJS repo>/doc/Protocol.md`, `<GacJS repo>/doc/NetworkProtocol.md` and `<GacJS repo>/doc/Testing_Protocol.md` with these options, two-sided opt-in, OPFS behavior and lifecycle limits.

### VERIFICATION

1. These are implementation checks, not work to execute during this review. Import/regenerate the protocol, then run GacJS's prescribed build and test commands. Cover query defaults, mixed-case names, combined flags, URL reconstruction and repeated REFRESH without duplicates, plus the disabled/enabled host text box. Verify a non-default host reaches both clients, including IPv6 URL construction.
2. For each feature, exercise all four Core/renderer flag combinations, then all three features enabled together. Core off always retains its predecessor. Core on with renderer off uses the DateTime/Locale predecessor or the empty filesystem; both on selects the browser implementation. Use temporary distinguishable timezone/locale/storage fixtures and request evidence where native and browser results could coincide.
3. Exercise actual FullControlTest browser UI and inspect Core automation. Check timezone conversion in both directions, milliseconds and date rollover, plus dates with different timezone offsets. Check locale defaults, format lists, names and case conversion. Use fresh API queries for transitions: the localization page caches its date/time and its locale selector has fixed choices, so unchanged labels or that dropdown alone cannot prove fallback or enumeration.
4. Verify OPFS path operations, Unicode names, file/folder enumeration and mutation, metadata and unsupported metadata-preserving copy. Cover missing files, empty files, binary round trips, ReadWrite preservation, WriteOnly replacement, zero-byte commit, failed persistence and visibility after close. Seed fixtures into the same origin's OPFS, including `<GacUI repo>/Test/TestDocument.bin`; load/edit/save/reload through both document editors, check embedded images and export, and verify saved files survive REFRESH. Ensure remote filesystem operations do not create or modify files in Core's native filesystem.
5. With FCT, disconnect the renderer, wait for the existing transport to report loss, and use Core automation with fresh queries to check DateTime/Locale fallback and the empty filesystem. Reconnect with capabilities off and on, confirming routing follows the new renderer. Test closing an old open stream after loss/replacement in a separate expected-failure run. Do not interpret a closed browser tab alone as proof that Core has already observed disconnection, and do not add a new heartbeat or shutdown handshake.
6. Verify `rvmhost` still serves a real view-model call at the selected destination, and document/reproduce the required Core restart when REFRESH ends that host. Retain existing Wasm demo startup/rendering behavior after the shared protocol/renderer changes; report unavailable platform coverage separately.
7. Committing and pushing the real implementation before temporary fake-provider checks is recommended above to establish a clean comparison point. Remove only temporary providers/probes and their generated derivatives afterward, rebuild, and repeat a real-browser smoke check before the final commit/push. Never commit fake implementations or treat successful build output as browser verification.

## REVIEW COMMENTS

No unresolved review comments. The details and implementation verification above resolve the identified gaps using the existing contracts, including the clarified OPFS requirement.

# TEST [CONFIRMED]

Baseline VlppOS Debug x64 build passed with zero warnings/errors. The new real registration case fails with Windows error 5 for http://+:39653/VlppOSRegistration/Nested/ before Start. This reproduces the constructor failure without elevation.

1. Reproduce HTTP.sys wildcard registration failure; cover wildcard success, localhost fallback and dual failure with preserved port/path and explicit-prefix behavior, then validate downstream Core/RVM startup and automation.
2. Normalize wGac/iGac launcher switches and existing documentation by inspection; runtime verification is explicitly excluded.
3. Regenerate GacJS protocol, cover injection behavior, asynchronous ordering/connection ownership, entry options and OPFS persistence in unit and browser tests; exercise live Core opt-in combinations and renderer lifecycle. Keep each feature implementation in its requested injection module with grouped forwarding handlers.

# PROPOSALS

- No.1 Retry only failed protocol URL registration with the equivalent localhost prefix
- No.2 Normalize launcher injection switches to double dashes [CONFIRMED]
- No.3 Implement grouped browser injections and HTTP entry configuration

## No.1 Retry only failed protocol URL registration with the equivalent localhost prefix

Keep explicit-prefix HttpServerApi construction unchanged. Add a protected fallback-prefix constructor used by HttpServer, and retain whether the fallback was selected. Share the two-attempt registration decision with deterministic tests using a registration callback; real calls still use HttpAddUrlToUrlGroup. Preserve both error codes and URLs on dual failure. Core reports localhost-only immediately after Start through its common templated startup path, covering split CLI and combined RVMT servers.

### CODE CHANGE

Add registration fallback and GetLocalhostOnly in VlppOS, deterministic registration and real transport coverage, then regenerate the library and import it into GacUI. Update Core and canonical transport documentation before downstream verification.
### Verification progress

VlppOS Debug x64 build passed with zero warnings/errors. All 322 cases in 16 files passed with no appended leak report. Actual unelevated wildcard error 5 followed by localhost success was observed, including a nested non-default endpoint and restart. A second simultaneous registration failed with wildcard error 5 and localhost error 183. Deterministic registration tests also cover wildcard success and explicit-prefix no-fallback behavior. No elevated wildcard runtime success is claimed.

The downstream GacUI Debug x64 solution build passed with zero warnings/errors. The selected seven regression files passed all 235 cases, including the complete injection capability matrix, replacement/disconnection, stream ownership and document/image persistence tests. No appended Debug leak report was present.

## No.2 Normalize launcher injection switches to double dashes

### CODE CHANGE

Update both launcher scripts in wGac/iGac, strip both leading dashes when forwarding slash switches, update only existing option descriptions and the iGac execution task. Preserve historical test commands with a future-spelling note. Runtime verification is excluded by the request.

### CONFIRMED

Both launchers and existing option descriptions now use double dashes, while native slash switches stay unchanged. Changes are committed and pushed in wGac and iGac. No launcher runtime verification was performed, as requested.

## No.3 Implement grouped browser injections and HTTP entry configuration

The old generated protocol lacks all injection operations and capabilities. Import and regenerate it through the existing root workflow. Put DateTime, Locale and OPFS algorithms in the three requested injections modules, with grouped forwarding methods in GacUIRendererImpl. Keep OPFS work in a per-renderer ordered queue; capture the response owner and suppress completion after stop. Ordinary OPFS failures produce protocol failure responses. DateTime exchanges calendar fields; Locale returns portable Core patterns and defines a finite browser candidate list explicitly. Add case-insensitive entry configuration, destination validation, and both-client forwarding, and implement image persistence independently.

### CODE CHANGE

Implement the three modules, connection capabilities, entry controls and tests for actual storage persistence, timezone conversions, locale queries, query reconstruction and connection ownership. Browser smoke and Core flag/lifecycle checks follow the prescribed build/test commands.

### Verification progress

The generated protocol now includes all injections and ImageSave, and represents nullable Binary without confusing null and empty content. The root GacJS build passed. Initial renderer tests passed 99 cases; other completed non-browser suites passed. The first root test run had browser timeouts under concurrent native compilation and a clock-pixel mismatch; isolated browser reruns are pending. Add explicit delayed-completion, capability-handshake and image-encoding checks before final verification.

Temporary live probes may attach fresh API queries to a Core automation-triggered key to observe routing after disconnection, avoiding the localization page's cached date. Keep these probes out of commits, restore the real Core source afterwards, rebuild, and repeat a real-browser smoke check.
