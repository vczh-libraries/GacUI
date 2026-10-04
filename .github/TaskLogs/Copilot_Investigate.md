# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

The goal of this task is to create a CLI version of `UiaList`:
- `UiaListCli.exe` will be an interactive CLI application, and it is Windows only.
- New project file will be `Tools/UiaList/UiaListCli/UiaListCli.vcxproj`
- It should be released to `../Release/Tools/Executables/UiaListCli/UiaListCli.vcxproj`:
  - Common code will be moved back from `../Release/Tools/Executables/UiaListApp/UiaList` to be under a new lib project file `../Release/Tools/Executables/UiaList/UiaList.vcxproj`
  - `UiaListApp` source files moved from `../Release/Tools/Executables/UiaListApp/Source` to its parent folder, and the `Source` folder will be deleted.
  - In this way relative position of source files are the same in `Tools/UiaList` so they could be just copied directly.
  - Make sure `../Tools/Tools/Build.ps1 -Project UpdateRelease` copies source files in the same coding format, and `../Release/Tools/CopyExecutables.ps1` copies executable files.
- In the same knowledge base page `KB_GacUI_Design_UiaList.md` in `GacUI` repo, add the input command and output format about `UiaListCli`:
  - Run `../Tools/Jobs/job.Windows.copilotInitAll.prompt.md` but skip learning, now the new KB page is spreaded to all repos.
  - Update `../Release/README.md` to add a new hyperlink of the new tool, link to the same page as `UiaListApp`.

You are going to design the input command format and output JSON format to make the interactive CLI application renders all features as `UiaListApp`, make sure all data could be exposed, all actions could be run from `UiaList` implemented view model. Command names are designed like `Verb-Target`, all result should be in JSON format, here are some examples:

1) Listing Processes

User could type `List-Process`, and the tree will be exposed via JSON, window ID could be used in other commands.

2) Exploring a Window

User could type `Print-Window ID` to list all controls in a tree that exposed via JSON, each control exposes limited information for identify and offer ID for other commands, e.g., node/control type, text, id

3) Others

`Query-Properties ID` could be used to read all values of a node,
`Query-Providers ID` for read all providers, including properties, `Run-ProviderInterface::Method ID` to call a function,
`Help-Provider ProviderInterface` to explain the correct format of all methods, some might need extra parameters,
etc, complete all of them in a similar way.

This tool is offered for coding agents, like you in codex, for better computer use, not for human (but you don't need to say this in the knowledge base page).

To verify, there are already some test apps in `Tools/UiaList/Verification`, make sure everything in `UiaListApp` are accessible in `UiaListCli`.
You are also going to test `UiaListCli` against `CppTest` to make sure `UiaListCli` is actually working on every kinds of control.

## DETAILS

### Shared implementation and scope

- Implement the CLI as another frontend of the existing `Tools/UiaList/UiaList` library. Reuse its discovery, UIA session, catalogs, validation, actions, capture and view models; do not create a second UIA implementation or drive `UiaListApp` through its HTTP endpoint.
- Use `Tools/UiaList/README.md` and `Tools/UiaList/Verification/README.md` as the current behavior contract. Feature parity includes discovery, complete Raw View trees, properties and setters, provider readouts/actions, returned elements, text-range workspaces, and preview capture/hit testing. GUI-only presentation such as tabs, inline editors and modal dialogs becomes commands with equivalent data and commit behavior; the CLI does not need to display these widgets.
- The inspector's supported SDK catalog is broader than GacUI's implemented providers. Preserve the existing boundary: known standard descriptors and acquired client interfaces are exposed; custom provider internals and arbitrary methods of opaque COM results are not generically discoverable. Provider names are aliases for corresponding client pattern operations, not interfaces queried directly on another process.
- `uialist::UiaListViewModel::Post` in `Tools/UiaList/UiaList/ViewModel/UiaListViewModel.cpp` currently publishes through the GacUI main-thread dispatcher, and preview publication uses its image service. Keep the required owner dispatcher/native services running without opening visible inspector windows. Blocking console input or waiting for a command must not block completion publication. Keep UIA COM objects and their release on the existing MTA worker, and view-model access on its owner thread.
- Expose shared typed results and completion/failure notifications where the current view model only offers presentation strings. In particular, `uialist::ActionCommandViewModel` in `Tools/UiaList/UiaList/ViewModel/ActionViewModel.cpp` currently formats getter results into status text. Do not parse those strings back into values. If generated interfaces change, edit `UiaList/UI` and regenerate through GacBuild; never hand-edit `UiaList/Source`.

### Input, output and completion contract

- Use one command per line: `Verb-Target [ID] [JSON arguments object]`. Keep the names in the original examples. Command/interface/parameter identifiers have stable case-sensitive spelling independent of Windows display language. Target IDs are opaque whitespace-free strings. The final arguments object uses JSON quoting/escaping, including `\n` for multiline input; document each command's exact required arguments through help.
- Support both an interactive console and redirected stdin/stdout. Redirected streams use UTF-8. Ignore blank lines; process nonempty command lines sequentially. No prompts, banners, progress text, ANSI formatting or unsolicited records belong on stdout. Flush one complete compact JSON response followed by a newline for each command, so a caller can read its result before sending the next command.
- Use a common response envelope with `command`, `ok`, `generation`, `result` and `error`. `generation` is the current inspection-generation string, or null before selection. Success has a command-specific `result` and null `error`; failure has null `result` and an error object containing a stable `code`, `message`, and operation/target/HRESULT when applicable. Help and exit acknowledgements use this same envelope.
- A response means that the command has completed, not merely that it was queued. For discovery, tree and inspection commands, wait for their own publication. For setters and mutations, include the actual readback and resulting generation after the existing refresh completes. A successful operation that removes its target, such as Window.Close, reports that outcome, clears affected selection/references and returns the resulting generation or null without requiring properties from the destroyed target. Getter commands need their own completion signal: root `GetIsBusy()` alone does not cover their asynchronous execution. Never retry a mutation automatically.
- Local syntax/type/ID/capability rejection produces a structured command error before dispatch and makes no target mutation. Preserve the existing expected-error classification of `uialist::native::UiaFailure` in `UiaCatalog.Windows.cpp`; report runtime unavailability with the failing operation and whether it occurred during invocation or readback, without promising that an already dispatched mutation was rolled back. Unexpected provider, infrastructure and invariant failures remain fatal: report JSON when possible, flush, and exit nonzero without retry/recovery loops or a CLI-created message box. The existing GUI fatal reporting must remain functional after sharing this boundary.
- `Exit-Application` completes normal shutdown and emits its final acknowledgement before returning from the process; EOF also shuts down normally after accepted work completes. Drain workers and release session/capture resources before exit. Do not require a second process, an HTTP listener or a shutdown acknowledgement from the inspected application.

### Command coverage

The following command families complete the original examples. Fill in exact argument/result schemas in the knowledge-base page during implementation, deriving operation coverage from the existing catalogs rather than maintaining a second independent list.

| Command | Required behavior |
| --- | --- |
| `Help-Command` | List all commands or describe one, including argument schema, result schema, side effects and ID lifetime. Return JSON examples. |
| `List-Process` | Explicitly refresh discovery and return the retained process hierarchy and each process's own qualifying windows, with usable window IDs. Preserve surviving selection using the existing identity rules. |
| `Print-Window ID` | Select/inspect the specified discovered window and return its complete published Raw View tree. Repeating it for the current window reads the cached generation; use explicit refresh to reread the target. |
| `Refresh-Window ID` | Explicitly refresh the selected window's tree/capture and return the new generation and tree. Reject an ID that is not the selected window. |
| `Select-Node ID`, `Query-Node ID` | Select/reveal a cached node, or inspect an element and expose its identity/summary. Inspection also accepts returned element references outside the selected tree, without inventing ancestry or changing the selected root. |
| `Query-Properties ID` | Return supported properties with numeric/catalog names, typed values, source pattern and setter metadata. Retain supported false, zero, empty, null, mixed and read-only values. Omit only Unsupported properties as the GUI does. |
| `Set-Property ID {"propertyId":...,"value":...}` | Use only an existing mapped setter, validate current capability and argument type, commit exactly once, and return actual readback. UIA has no generic property setter. |
| `Query-Providers ID` | Return every supported section, provider/client names, pattern IDs, typed readouts, commands, parameters and enabled state/reason. Include the Element section and versioned interfaces. |
| `Help-Provider ProviderInterface` | Explain the mapped client operations, stable command names, parameter/result types, bounds, choices, nullability and getter/mutation semantics. Distinguish catalog support from support on the currently inspected target. |
| `Run-ProviderInterface::Method ID {...}` | Invoke the mapped operation on an element or range with typed named arguments. Include parameterized getters, mutations, explicit StartListening/Cancel and workspace operations. Returned references must be reusable by later commands. |
| `Query-Range ID` | Expose the range workspace, owning document, readouts and available operations, including acquisition results, clone/comparison, endpoint operations, search, attributes, text, geometry, relationships and mutations. |
| `Query-Preview ID` | Return the selected window's cached capture status, bounds, pixel size, DPI, timestamp and generation. When available, expose its existing BMP bytes as base64 with the media type; keep large image payloads out of ordinary tree/property responses. |
| `HitTest-Preview ID {"x":...,"y":...}` | Use image-local coordinates against the matching cached capture/tree generation, returning the matched node and bounds or no match. Select/reveal the node as a preview click does; do not send target input or focus. |
| `Exit-Application` | Complete normal CLI shutdown. |

- Derive help and dispatch from `uialist::native::ActionSpec`, `ArgumentSpec`, `SetterSpec` and the catalogs in `Tools/UiaList/UiaList/ViewModel/UiaSession.Windows.h` and `UiaCatalog*.Windows.*`. Parameterless getters may exist as GUI readouts rather than visible command buttons; they still need complete JSON exposure.
- Add stable machine identifiers where descriptor names are presentation text. For example, the text-range catalog's `GetAttributeValue [44]` is a read-all-attributes operation, not a literal COM method name. Qualify version-specific operations correctly, including the client-only `IUIAutomationTextRange3`; do not invent a corresponding provider interface. Help must expose every available range operation and any inspector helper command explicitly.

### Identity and typed JSON

- Use distinct opaque string IDs for windows, elements and ranges, scoped to the CLI session and the relevant discovery/inspection generation. Never treat a collection index, HWND, RuntimeId display string or text label as sufficient command identity. Return PID, process creation time, HWND and runtime IDs as separate metadata where available.
- Refresh, target switch, property inspection replacement and mutation must follow the existing session/reference ownership rules. Report which generation and references remain valid; reject expired, wrong-kind and wrong-document operands before dispatch. A stale ID must never resolve to a different object. Invalidate text ranges after window refresh or target mutation even when the native session object survives, as well as when the owning session is rebuilt; local range clone/find/endpoint operations retain their valid workspace operands.
- Returned elements are usable references, including elements not in the selected Raw View tree. Return a tree node ID only when an actual matching node exists. Range comparison and endpoint-transfer operands must belong to the same document, including ranges obtained through different interfaces of that document. Opaque ObjectModel/IAccessible results remain explicitly opaque.
- Serialize `uialist::native::ValueData` from `UiaSession.Windows.h` with its `kind` and `vartype` plus the appropriate payload. Preserve Null, Unsupported, Mixed, Boolean, Signed, Unsigned, Real, String, Element, Range, Array and Opaque separately. Keep full strings and array dimensions/lower bounds; display text may be supplementary but must not replace typed data.
- Encode signed/unsigned 64-bit values, HWNDs and other precision-sensitive identities as strings. Encode nonfinite real values with an explicit tagged representation, never invalid JSON numeric tokens. Include numeric enum/flag values and canonical names without losing unknown values. Use invariant numeric formatting and correct JSON escaping for Unicode, control characters and embedded NUL results; reject embedded NUL inputs for methods that accept null-terminated strings.
- Return trees as flat preorder arrays with parent IDs and root IDs, retaining sibling order and complete ancestry. Include node identity, semantic control type, client interface, name/text, AutomationId when available, provider names, bounds and offscreen state. This preserves the full tree without requiring 1,000 levels of JSON nesting or silently limiting traversal.
- Unavailable capture must not block node/property inspection, masquerade as a successful black image, or restore/activate/move the target. Preview hit testing uses the existing physical-coordinate mapping once and never mixes generations.

### Projects, release and documentation

- Add the console project and filters to `Tools/UiaList/UiaList.sln` for Debug/Release and Win32/x64, referencing the shared library. Follow `Tools/BestPractices.md` for toolset, SDK, C++ standard, runtime, reflection and Debug leak settings.
- In `../Release/Tools/Executables`, make `UiaList`, `UiaListApp` and `UiaListCli` sibling project directories and register the new library/frontend in `Executables.sln`. Both frontends reference the common library; generated/view-model/dependency objects must not be duplicated within one executable. Adjust includes, manifests, filters and project references for the new relative layout. Keep embedded resources and standalone executable deployment.
- The source copier is implemented in `../Tools/Tools/BuildRelease.ps1`, called by `Build.ps1 -Project UpdateRelease`. Update its current nested `UiaListApp/UiaList` and `UiaListApp/Source` destinations and add the CLI source inventory. Copy source bytes directly from GacUI, preserving formatting; do not maintain modified release copies. Remove the obsolete copied directories after migration without deleting the release project files.
- Add `UiaListCli` to the explicit executable list in `../Release/Tools/CopyExecutables.ps1`, retaining its check-all-inputs-before-copying behavior. UpdateRelease currently builds Release x86, and CopyExecutables deploys from `Executables/Release`; this shipped configuration requires its own verification.
- Extend the existing UiaList KB page with complete command and JSON schemas, ID/refresh rules, failure behavior and runnable multi-command examples covering discovery, property edits, provider calls, returned references, ranges and preview. Update the KB index description, `Tools/UiaList/README.md` (currently says no CLI frontend), `../Release/Tools/README.md` (old layout/tool count), and the requested link in `../Release/README.md`.
- Before the requested skip-learning synchronization, reconcile/upload the edited GacUI knowledge base into `../Tools/Copilot/KnowledgeBase` using the existing `copilotInit.ps1 -UpdateKB` workflow. The skip-learning job starts at Sync Back, which copies the central Tools KB into each repository; running it first would overwrite the newly edited page. Then run the requested job with learning skipped and verify the updated page/index survive in every synchronization target. These are implementation steps, not actions to execute during this review.

## VERIFICATION

1. Build both frontends in the source solution and packaged `Executables.sln` using the repository build wrappers for Debug/Release × Win32/x64. The packaged solution calls its 32-bit platform x86; the wrapper maps Win32 accordingly. If XML interfaces change, run GacBuild for the existing architecture outputs and verify generated inventories. A tools-only change may skip GacUI's unit suite under `Project.md`; any actual GacUI/skin changes must trigger their required generation and tests.

2. Launch the CLI through `copilotExecute.ps1 -Mode CLI -Executable UiaListCli` in an interactive session, using `-Interactive` when console handles are required. Also exercise redirected input/output. Send several commands without restarting; parse every response independently and verify flushing, ordering, completion/readback, exact JSON framing, help, blank lines, malformed input, unknown commands, invalid types/IDs, normal exit and EOF. Validate typed round trips for Unicode/multiline/NUL results, false/zero/empty/null/mixed/unsupported values, arrays, enum values, nonfinite numbers and precision-sensitive integers. Rejected input must produce no mutation in the independent fixture log.

3. Use the independent native/synthetic applications in `Tools/UiaList/Verification` and run `CheckCatalog.ps1`. Map CLI help, readouts and handlers to the current SDK inventory and the applicable A00/A01-A35 and R01-R10 acceptance cases, not just to commands visible as GUI buttons. The documented baseline is 35 patterns, 175 properties, 44 text attributes and 41 roles plus metadata; compare with the selected SDK rather than treating those counts as permanent. Test each supported operation's actual target, typed arguments, exact call count and resulting state using the fixture logs. Use fresh fixture states when comparing GUI/CLI behavior; normalize transient IDs rather than comparing them literally.

4. Cover process pruning/identity and navigation, duplicate titles/AutomationIds, surviving versus removed windows on refresh, Raw View-only and cross-process descendants, external returned elements, range document identity, dynamic capability changes, read-only/mapped setters and explicit listening/cancel. Traverse the existing 10,000-sibling and 1,000-level fixture completely and parse its JSON without truncation or stack failure. Exercise stale references after refresh/mutation/switch, operation-specific completion, expected unavailability and separate fatal-failure runs. Accepted mutations execute exactly once on their retained original target; stale completions never publish into a newer selection.

5. Verify preview parity with the fixture: decode the base64 BMP, compare dimensions/bounds/DPI and matching generation, and exercise cached hit tests including deep/overlapping/outside-parent/offscreen nodes. Selection/reveal must not increment target input/focus/action counters. Test unavailable capture while tree/property inspection still works. Run cross-architecture inspection with Win32/x64 clients against both target architectures, then check normal Debug shutdown and leaks. If instability is found, fix it and obtain the required 25 consecutive successful reruns.

6. Drive `CppTest` through the CLI itself, using the feature inventory in `Test/UIA_CppTest_Shared.cs` and the FullControlTest showcase procedure: nested tabs, buttons/selection, lists/list views/trees, grids, text/document editing, calendars, Easy Layout, menus/toolbars/dialogs, window/transform operations and palette refresh. Assert meaningful role/name/pattern/value/state/bounds and actual post-action behavior, reacquiring providers after rebuilds. Also cover `CppTest_Metaonly` for ordinary native windows alongside hosted logical windows sharing one HWND. Existing C# UIA suites are useful provider baselines but do not substitute for testing CLI parsing/dispatch/JSON. CppTest's provider subset cannot prove the inspector's entire SDK coverage.

7. Protect the shared GUI frontend: rerun `Smoke.ps1 -GetterRegression -NavigationRegression` and the relevant deep-tree regression after common-model/library changes. Verify property edits/readouts, returned references, ranges, preview selection and GUI error reporting retain their existing behavior. Use separate fixture processes for destructive operations such as Window.Close.

8. Run UpdateRelease and confirm copied source-byte equality, complete project/filter references and absence of obsolete nested paths; repeat the copy to ensure it does not recreate the old layout. Build/deploy with CopyExecutables, verify the deployed `../Release/Tools/UiaListCli.exe` matches `Executables/Release/UiaListCli.exe`, and smoke-test that Release Win32 binary plus the packaged GUI without development-tree/resource-file dependencies. Check KB page/index contents and all updated links after the central upload and skip-learning sync.

9. Record reproducible commands, configurations, passed coverage and remaining environment limits. A locked desktop must not stop available interactive-CLI, fixture or UIA checks. Record genuinely unavailable physical-input, foreground-focus, capture or multi-monitor/DPI checks separately; unsupported capability results do not count as success-path coverage. Do not reuse historical GUI results as evidence for the new frontend.

## REVIEW COMMENTS

No unresolved review comments. The missing contracts and verification requirements above have reasonable resolutions and belong in DETAILS and VERIFICATION.

# UPDATES

## UPDATE

It looks like you have implemented many python scripts in Tools/UiaList/Verification, you should use powershell, please rewrite all of them, and make sure they work. Since you are not touching any C++ code so no other verification is needed In Tools/UiaList/Verification/README.md you don't need to mention the date.

# TEST [CONFIRMED]

Baseline: verify that the source and packaged solutions lack UiaListCli. Implement executable-boundary verification against the existing independent UIA fixture, catalog, deep tree and CppTest showcase. Success requires the complete TODO_TASK verification matrix, normal shutdown and clean Debug leak reports.

For the PowerShell conversion, run every replacement entry point against the existing binaries: smoke plus EOF, native acceptance, failure framing/capability/fatal paths, preview/lifecycle, both hosted/native showcase modes, and direct ValueData verification. Preserve existing assertions, independent fixture logs and JSON transcripts. No C++ changes, rebuilds or unrelated verification are required for this update.

# PROPOSALS

Baseline confirmed on 2026-10-03: UiaListCli.vcxproj is absent and the source solution contains only Gaclib, UiaList and UiaListApp. CheckCatalog.ps1 passes with 175 properties, 35 patterns, 44 attributes, 41 roles, one metadata descriptor and 42 matching localization keys. These existing catalogs are the source of operation coverage.

- No.1 Add a JSON command view model over the existing inspector library [CONFIRMED]
- No.2 Replace Python verification drivers with PowerShell [CONFIRMED]

## No.1 Add a JSON command view model over the existing inspector library [CONFIRMED]

Keep the existing owner dispatcher, discovery, UIA MTA worker, capture and session lifetime. Add a command view model that publishes typed native results and completion after each operation, with stable opaque references and JSON schemas derived from catalog descriptors. The console frontend owns UTF-8 line transport and shutdown, with no visible inspector or HTTP listener. Share error reporting through an explicit frontend callback while preserving the GUI fatal dialog. Register source and packaged console/library projects, update source-byte copying and deployment, and publish the complete command contract through the shared knowledge base.

### CODE CHANGE

Implemented the console transport in `Tools/UiaList/UiaListCli`, the command/typed-result layer in `UiaList/ViewModel/CliViewModel.*` and `CliCommands.cpp`, and shared descriptor/completion/failure changes in the existing UiaList library. Source and packaged project/filter inventories include the shared library and both frontends in all configurations. The Tools copier owns byte-preserving migration; Release deploys seven executables. The upstream Parser2 owner changes repair complete JSON string escaping/unescaping and strict token recognition, with regenerated output imported through the repository workflow. Verification drivers, independent fixture extensions and the complete shared KB protocol are included.

### Implementation progress — 2026-10-04

The proposal is being implemented with the shared native action descriptors, dispatcher, UIA worker, capture worker and view models. The CLI now provides fifteen command families and typed JSON envelopes, generation-scoped references, synchronous publication completion and explicit readback. Global action schemas are extracted from the same descriptors used by the GUI; invocation capability checks no longer re-run readout getters. Release now has sibling common-library and frontend projects and a byte-copy source helper.

JSON regression testing found upstream VlppParser2 defects in C0 escaping, escaped NUL parsing, hexadecimal decoding and malformed-token acceptance. Their owner sources and a regression were updated, generated JSON lexer rebuilt, the required Parser2 test/generator sequence run, and regenerated release output imported into GacUI/Release. The TypeScript package build passed. Further direct CLI ValueData tests will cover values normalized by Windows UIA before delivery.

Current evidence: source Debug x64 and Release x64, packaged Debug x64 builds passed. Redirected smoke passed 24 commands and EOF. Independent CLI acceptance passed 143 commands including mutations with exact call counts, all 35 pattern IDs, external elements, wrong-document ranges, range operations, navigation and complete 11,053-node flat tree (10,000 siblings and 1,000 levels). Final full configuration, GUI, showcase, deployment and synchronization checks remain pending.

CDB at ExecuteRange/StringValue observed a BSTR byte length of 2 when the independent provider returned seven UTF-16 units beginning A,NUL. Windows UIA shortened the result before the shared converter. It also normalized the text Tabs SAFEARRAY bounds from 2..3 to 0..1 and returned Unsupported for nonstandard I8/UI8 text attributes. NaN and both infinities survived through UIA and the CLI. These delivery limits are recorded separately from direct serializer preservation checks; no claim is made that an inspector can recover bytes the platform removes.

Session-start review found that creating the worker session before RefreshWindow advanced the generation could suppress an expected constructor failure as stale, leaving completion pending. Session creation now runs inside the matching Read Raw View generation; only release is queued before refresh. Discovery window records also retain process creation time for surviving opaque-ID checks. These changes require final rebuild and disappearance regression.

All eight source/package configurations have passed; cross-architecture smoke passed all four client/target combinations. The final 143-command acceptance and separate invocation/readback/capability/fatal tests passed. The direct ValueData serializer verified all twelve kinds, NUL/control/Unicode strings, precise integers and nonfinite values. GUI getter/navigation regression passed on retry after an initial property-publication timeout (the fixture recorded exactly one setter and subsequent inspection showed the expected value); stability repetition remains required.

### Verification results — 2026-10-04

- All eight final source/package Debug/Release × Win32/x64 builds passed. Logs: `%TEMP%/UiaListCli-source-last-CONFIG-PLATFORM.txt` and `UiaListCli-package-last-CONFIG-PLATFORM.txt`. Both frontends are built in each solution. No generated XML interface was edited; GacUI core/skins were not changed, so the tools-only Project.md exemption applies to the GacUI unit suite.
- Parser2's required generator/test sequence passed: AstGen 3, AstParserGen 46, LexerAndParser 4, its Generated tests 24, ParserGen 142, its Compiler 172 and Generated tests 463, BuiltIn Compiler 56, JSON 131, XML 25, Workflow 717 and C++ 888. Regeneration stages were followed by wrapper rebuilds. Logs are `%TEMP%/UiaListCli-Parser2-*.txt`; TypeScript build passed. Unrelated generated C++ parser/state-number trace churn was restored after review.
- Fresh expanded native/synthetic acceptance: 276 commands, fixture 32616 (Debug Win32), x64 Debug CLI, transcript `UiaListCli-24040.jsonl`, log `UiaListCli-acceptance-fresh.txt`. Includes all 35 patterns, typed arguments/exact call counts/state readback, enum and flag variants, current capability rejection, external elements, document identity, all text attributes, complete flat 11,053-node Raw View tree, 10,000 siblings and 1,000 levels. Separate process ancestry/pruning and Element3/TextRange2 context-menu checks passed (`UiaListCli-extra-operations.txt`, transcript 16652); the range menu reached its document exactly once.
- Expected invocation failure, committed range edit followed by readback failure, changed capability from another client, fatal provider failure/nonzero exit, invalid UTF-8, raw NUL and pipelined framing passed (`UiaListCli-failures-latest.txt`). Independent logs prove no retry after the readback failure and no setter for rejected input.
- Cross-architecture 24-command smoke plus EOF passed for all four Win32/x64 client/target pairs (transcripts 11028, 4156, 20984 and 14920). Twenty-five consecutive Debug x64 smoke/EOF runs then passed (`UiaListCli-stability-25.txt`). Real PTY console help/error/exit also completed; redirected responses were independently JSON-parsed.
- Direct ValueData tests passed all twelve kinds, false, NUL/control/Unicode strings, precise signed/unsigned integers, array lower bounds and nonfinite values. The fixture also delivered NaN and both infinities through real UIA. Windows UIA's BSTR/SAFEARRAY/I8/UI8 normalization limits above remain distinct from serializer behavior.
- Preview/lifetime passed 22 commands (`UiaListCli-lifecycle.txt`, transcript 18468): BMP decode and dimensions/DPI/timestamp/generation, deep overlapping descendants outside ancestor bounds, offscreen exclusion, no target input/focus/action log changes, minimized capture unavailable with usable properties, surviving window IDs, removed window rejection and cleared stale help.
- CppTest Release x64: 364 commands and 1,512 assertions; CppTest_Metaonly Release x64: 369 commands and 1,518 assertions. Logs `UiaListCli-showcase-hosted-full.txt` / `UiaListCli-showcase-native-full.txt`, transcripts 24824/6096. The CLI itself drove nested tabs, lists, six views, trees, grids, text/ranges/embedded controls, calendars, layouts/rebuilds, menus, actual formatting changes, modal dismissal, window transforms/lifecycle and palette replacement. Both applications closed normally. Targets were existing 2026-09-26 binaries; no HTTP provider baseline was substituted for these CLI runs.
- Shared GUI getter/navigation regression passed after the final common-model changes: 118.2 seconds, exactly one native Invoke, exit zero (`UiaListCli-gui-latest.txt`). An earlier attempt during build contention timed out waiting for the displayed numeric value, despite exactly one setter and subsequent correct readback. No product fix was attributed to that timing observation. Earlier showcase-driver assumptions were corrected (fresh list state, automatic combo collapse, the `Rebuild tables` caption, and seeding a nonempty selection before Bold).
- Debug CLI loaded and replaced the full deep tree, acquired/queried a range, acknowledged Exit and returned zero under attached CDB (`UiaListCli-debug-driver.txt`, transcript 29312). `UiaListCli-leaks-latest.log` contains no CRT leak report. Expected first-chance Windows UIA/graphics notifications were filtered; no product exception was hidden as a successful command.
- UpdateRelease completed (`UiaListCli-UpdateRelease.txt`), followed by another source copy and CopyExecutables. All 39 copied source/header/manifest files are byte-identical; all project/filter paths resolve and old nested UiaListApp paths are absent. The deployed CLI SHA256 is `bdddc974a1590d2ad640b9901001210570112a1c62de18179e11e0afcc03a06a`, identical to packaged Release Win32 output. Isolated deployed CLI smoke/EOF and packaged Release GUI properties/Refresh/normal Close/no-endpoint checks passed (`UiaListCli-deployed-smoke.txt`, `UiaListCli-packaged-gui.txt`). UpdateRelease also propagated existing owner snapshots for Vlpp, GacUICompiler and the makefile; unrelated tutorial generated-source deletions from copying were restored.
- The edited KB page/index were uploaded with `copilotInit.ps1 -UpdateKB`, reconciled with central content, then `copilotInitAll.ps1` and `CheckRepo.ps1 CheckAll` ran with learning skipped. Pre-existing differences outside the two edited files were line endings only; central versions were preserved. The new page/index match in all eight sync targets. Learning updates: 0; new learnings: 0; nine repositories are affected including Tools.

Final GUI deep-tree regression passed all 25 consecutive replacements (11,053 to 51 nodes), then closed with the full deep tree loaded. The wrapper and CDB completed with exit zero; `UiaListCli-gui-deep-leaks.log` contains no CRT leak report. Reproduce using `DeepTree.ps1 -Cycles 25 -CloseWithDeepTree -AsPort 8891 -FixtureProcessId PID` against a Debug inspector launched through the wrapper. Per-cycle evidence is in `%TEMP%/UiaListCli-gui-deep-25.txt`.

Final `GuiFailure.ps1 -AsPort 8891 -FixtureProcessId 32616` passed expected and fatal provider failures (`UiaListCli-gui-failures.txt`). Expected HRESULT 0x80040201 appears beside the getter while the dialog remains Ready; the native fixture records one call. Fatal HRESULT 0x8000FFFF produces the existing native diagnostic, another single call and unsuccessful termination. Initial driver assumptions about the status location, PowerShell variable scope and native OK button ID were corrected. The Debug process exits with 0x80000003: CDB traced this to `D2D1Debug3!GlobalCleanup` during `ExitProcess` called by `UiaListViewModel::ReportFailure`, with live graphics objects on this immediate fatal path (`UiaListCli-gui-fatal-debug.log`). This preserves the existing GUI fatal policy; it is separate from the successful leak-free normal shutdown tests. The CLI fatal path reports JSON and exits nonzero without this GUI diagnostic.

### Verification limits

No claim is made for physical desktop input/foreground focus, protected-content capture or multiple-monitor/DPI arrangements. UIA can normalize provider values before delivery; the CLI preserves delivered typed data and the direct unit verifies the otherwise normalized cases. Unsupported capability outcomes are not success-path evidence. The broader historical GUI acceptance tables remain requirements for platform arrangements not exercised in this task.

### CONFIRMED

The shared command view model provides the requested JSON console frontend while retaining the existing UIA worker, native catalogs, session ownership and GUI behavior. Independent fixture records confirm typed dispatch and exactly-once mutations; independently parsed responses confirm framing, completion, readback and reference lifetime. Native coverage, both real showcase runs, the eight-configuration build matrix, deployment checks, Parser2 regressions and repeated CLI/GUI lifecycle checks passed. The documented platform and fatal-shutdown limits above bound these results; no unsupported response is substituted for a successful operation.

Source and release project layouts, byte-preserving copying, executable deployment and protocol documentation are complete. The skip-learning synchronization updated the page/index in all eight targets from central Tools content: 0 learning updates, 0 new learnings. Changes are committed and pushed in the nine affected repositories; the final GacUI commit includes this confirmation and the verification drivers.

## No.2 Replace Python verification drivers with PowerShell [CONFIRMED]

Replace all six Python entry points with PowerShell scripts and share process launch, UTF-8 JSON framing, response assertions and transcript handling in one helper. Retain each driver's existing behavior and independent expectations, including raw invalid UTF-8, exact provider call counts, returned references and range lifetime, deep trees, preview geometry, real showcase operations, and precise serializer values. Use existing executables through the repository execution wrapper. Update the verification README to describe PowerShell and remove date references. This supersedes the original choice of Python without changing the C++ implementation.

### CODE CHANGE

Replaced `CliSmoke.py`, `CliAcceptance.py`, `CliFailure.py`, `CliLifecycle.py`, `CliShowcase.py` and `CliValueTests/Verify.py` with their `.ps1` counterparts. `CliCommon.ps1` owns wrapper launch, asynchronous stdout/stderr consumption, per-response deadlines, exact JSON envelopes, typed structural assertions, UTF-8 transcripts and shared access to live fixture logs. The failure driver now records invalid-encoding and pipelined responses in its transcript as well. The preview driver uses a small PowerShell Add-Type Win32 declaration in place of ctypes. The README now documents PowerShell 7 parameters and commands and omits dates in its headings and prose. No C++ or release files changed.

### CONFIRMED

All six replacement entry points passed against the existing binaries. The shared helper is exercised by every entry point. The converted acceptance and showcase drivers retain the original command and assertion counts; no assertion was removed to obtain a passing run.

| PowerShell driver | Passing evidence |
| --- | --- |
| `CliSmoke.ps1` | 24 commands plus a separate one-command EOF session; transcripts 25376 and 14884. |
| `CliAcceptance.ps1` | 276 commands, independent exact provider-call/argument/state checks, all patterns and the complete 11,053-node tree; transcript 668. |
| `CliFailure.ps1` | Invalid UTF-8/raw NUL, pipelined framing, expected invocation error, committed range change followed by readback failure, externally changed capability and separate fatal/nonzero exit. Primary/secondary/fatal sessions recorded 15/6/3 responses in transcripts 29828/17040/30176. |
| `CliLifecycle.ps1` | 22 commands covering actual BMP dimensions/DPI/generation and deep/overlapping/outside-parent/offscreen hits, unchanged input logs, minimized capture and surviving/removed identities; transcript 22072. |
| `CliShowcase.ps1 -Hosted` | CppTest: 364 commands, 1,512 assertions, target and wrapper normal exit; transcript 34380. |
| `CliShowcase.ps1` | CppTest_Metaonly: 369 commands, 1,518 assertions, target and wrapper normal exit; transcript 29184. |
| `CliValueTests/Verify.ps1` | All twelve kinds, NUL/control/Unicode strings, precise integers, nonfinite values and array bounds. |

Transcripts are `%TEMP%/UiaListCli-PID.jsonl`. Driver logs are `%TEMP%/UiaListCli-powershell-{smoke,acceptance,failure,lifecycle,showcase-hosted,showcase-native}.txt`; the direct value verifier printed its passing result. Native cases used the existing Debug Win32 fixture (PID 9588) with the Debug x64 CLI; showcase targets were the existing Release x64 binaries. The fixture and both showcase processes closed normally after their runs.

Conversion testing corrected three PowerShell integration details: select one `pwsh` executable when several are installed, retain null at stdout EOF instead of coercing it to an empty string, and open fixture logs with shared read/write access. Final runs passed with those changes. Script syntax and whitespace checks passed. No C++ rebuild, catalog run, GUI regression suite or unrelated verification was performed, as requested. Both confirmed proposals are retained: the console implementation remains, with its verification drivers now in PowerShell.
