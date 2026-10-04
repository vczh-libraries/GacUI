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

# TEST [CONFIRMED]

Baseline: verify that the source and packaged solutions lack UiaListCli. Implement executable-boundary verification against the existing independent UIA fixture, catalog, deep tree and CppTest showcase. Success requires the complete TODO_TASK verification matrix, normal shutdown and clean Debug leak reports.

# PROPOSALS

Baseline confirmed on 2026-10-03: UiaListCli.vcxproj is absent and the source solution contains only Gaclib, UiaList and UiaListApp. CheckCatalog.ps1 passes with 175 properties, 35 patterns, 44 attributes, 41 roles, one metadata descriptor and 42 matching localization keys. These existing catalogs are the source of operation coverage.

- No.1 Add a JSON command view model over the existing inspector library

## No.1 Add a JSON command view model over the existing inspector library

Keep the existing owner dispatcher, discovery, UIA MTA worker, capture and session lifetime. Add a command view model that publishes typed native results and completion after each operation, with stable opaque references and JSON schemas derived from catalog descriptors. The console frontend owns UTF-8 line transport and shutdown, with no visible inspector or HTTP listener. Share error reporting through an explicit frontend callback while preserving the GUI fatal dialog. Register source and packaged console/library projects, update source-byte copying and deployment, and publish the complete command contract through the shared knowledge base.

### CODE CHANGE

Implementation pending. Verify command completion, exactly-once mutation/readback, typed values, reference expiration and range document checks at the executable boundary before marking this proposal confirmed.