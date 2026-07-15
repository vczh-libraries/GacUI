# !!!LEARNING!!!

# Orders

- Process staged tasks one by one with verification [19]
- Verify generated artifacts with downstream consumer checks [14]
- Port fixes from imports to source repositories [12]
- Crash early instead of adding error-tolerance fallbacks [8]
- Keep design documentation aligned with code after refactoring [7]
- Proactively remove code made redundant by refactoring [6]
- Make `Stop()` drain asynchronous work before returning [6]
- Use `WString::IndexOf` with `wchar_t` (not `const wchar_t*`) [4]
- Use `collections::BinarySearchLambda` on contiguous buffers (guard empty) [4]
- Validate expectations against implementation and existing tests [4]
- Use `vl::Exception` for expected semantic failures and `CHECK_ERROR` for invariants [3]
- Extract abstractions only for real shared behavior [3]
- Fix behavior at the owning state instead of patching symptoms [3]
- Capture dependent lambdas explicitly [2]
- Don't assume observable changes are batched [2]
- Do not assume async callback owners are heap allocated [2]
- Use `ERROR_MESSAGE_PREFIX` for meaningful `CHECK_ERROR` / `CHECK_FAIL` messages [2]
- Prefer simple calls before interface casts [2]
- Treat Debug memory leak dumps as required failures [2]
- Prefer well-defined tests over ambiguous edge cases [2]
- Prefer `operator<=> = default` for lexicographic key structs [1]
- Prefer two-pointer merge for sorted range maps [1]
- Use named sentinel constants instead of raw values [1]
- Use `Variant<T...>::Index()` to check active alternative [1]
- Avoid references into containers when mutating them [1]
- Prefer designated initializers for aggregate-like structs [1]
- Prefer raw pointers unless shared ownership is required [1]
- Construct `Nullable<WString>` explicitly in function calls [1]
- Sort serialization metadata by deterministic keys, not pointer addresses [1]
- Start async callbacks after most-derived construction [1]
- Do not rely on `Event<T>` handler invocation order [1]
- Use RAII scope cleanup instead of manual catch cleanup [1]
- `collections::Dictionary` copy assignment is deleted (use move/swap) [1]
- Dereference `Ptr<T>` via `.Obj()` (not `*ptr`) [1]
- `vl::regex` separator regex: `L"[\\/\\\\]+"` [1]
- Use 2-space indentation in embedded XML/JSON literals [1]
- `collections::List` has deleted copy constructor; use `std::move()` for structs with `List` members [1]
- Compare type descriptors by pointer when descriptor identity is available [1]
- Parse and validate before queuing asynchronous work [1]
- Use Win32 messages for native dialogs when UIA is unavailable [1]
- Keep generated makefiles platform-invariant [1]

# Refinements

## Capture dependent lambdas explicitly

When a C++ lambda uses another local lambda (or any local variable), capture it explicitly (e.g. `[&]` or `[CreateBindableTree]`). Lambdas do not implicitly capture other lambdas, and missing captures can show up as confusing compile errors.

## Don't assume observable changes are batched

When verifying callbacks from an observable collection, do not assume multiple operations collapse into a single notification. For example, for `vl::collections::ObservableList<T>`, `Clear()` followed by multiple `Add()` calls triggers one callback pair per operation; test expectations should match the actual per-operation granularity.

## Crash early instead of adding error-tolerance fallbacks

For public API arguments that callers can validate with query methods, fail with `CHECK_ERROR` when the argument is invalid instead of accepting impossible protocol states.

When an invariant says a value must exist or a conversion must succeed, prefer using it directly or using a strong cast so a violation crashes or throws immediately. Do not add speculative null checks, weak casts, or silent fallbacks that hide protocol or ownership bugs. Fix the real cause instead of making the code tolerant of states that should be impossible.

Build-generation scripts must also preserve failures from compiler or dependency probes across command substitutions and formatting pipelines. Stop before emitting tracked output when a required header or package is missing, and declare the dependency in the canonical environment bootstrap so the original diagnostic is reported instead of a malformed generated rule.

## Process staged tasks one by one with verification

When a request is split into explicit tasks, complete and verify each task before starting the next one. This keeps commits easy to understand and review, limits side effects to the current task, and avoids having to diagnose many unrelated issues at the same time. If a task has its own finishing instructions, finish that task properly before moving on.

When a task boundary says to commit and push, do that before starting the next task so each task remains independently reviewable.

## Use `ERROR_MESSAGE_PREFIX` for meaningful `CHECK_ERROR` / `CHECK_FAIL` messages

For source-code `CHECK_ERROR` / `CHECK_FAIL` calls with real diagnostic messages, follow the repo pattern: define `ERROR_MESSAGE_PREFIX` at the beginning of the function with the full class/function context, use it in the message, and undefine it at the end. Simple unsupported stubs such as `CHECK_FAIL(L"Not Supported!")` can stay as-is.

## Prefer simple calls before interface casts

Do not add explicit interface casting (e.g. `dynamic_cast<IFoo*>(...)`) just because a similar file does it. Start with the simplest direct call, and only introduce interface casts when the compiler or access rules actually require it; unnecessary casts make tests noisier and can obscure which API surface is being exercised.

## Prefer well-defined tests over ambiguous edge cases

When a scenario’s expected behavior is unclear or undocumented (e.g. calling an operation while an object is in an “invisible”/non-effective state), avoid turning it into a test requirement. Prefer fewer tests that validate the public contract and real-world usage; if an edge case is important, first clarify the intended semantics from implementation and/or documentation.

Use timing to diagnose a performance problem, not as a lasting pass/fail assertion. When a dedicated deterministic test already owns startup-retry behavior, shared protocol tests should synchronize incidental listener startup and retain their meaningful repetition, FIFO, framing, routing, and shutdown checks instead of depending on a scheduler race.

## Do not assume async callback owners are heap allocated

Asynchronous callbacks and handle-close paths must not rely on the owning object being allocated with `new` or outliving callbacks by convention. Track active operations explicitly and make shutdown drain or detach every callback path that can reference the object.

When registered waits or async request callbacks can race with `Stop()`, give each wait/request its own context and use atomics to transfer ownership between registration, callback execution, and stop. Do not close handles or buffers until the owner path has either unregistered a not-yet-started callback or waited for a started callback to finish.

## Extract abstractions only for real shared behavior

When refactoring client/server or similar paired implementations, extract common state and helper behavior only when it genuinely simplifies both sides. Preserve intentional differences in small derived redirects or callbacks instead of forcing an abstraction just to increase reuse.

For role-specific implementations, extract a base that contains only truly shared state and operations. Keep transport-only or role-only members in the concrete subtype so no implementation inherits fields that do not apply to it.

When several platform implementations must satisfy the same behavioral contract, define and register each scenario once in a platform-neutral runner and parameterize only the concrete types and genuine platform seams. Duplicating test registrations per platform invites feature drift.

## Make `Stop()` drain asynchronous work before returning

If an API exposes `Stop()`, callers should be able to rely on it as the shutdown boundary: after it returns, no pending action, wait callback, overlapped I/O, or async completion should still touch the object. Do not paper over a broken `Stop()` with sleeps in tests; fix the stop path.

Use the platform's final callback boundary when available. For WinHTTP async requests, `WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING` is the point to release pending request tracking for successfully submitted requests; release immediately only for failures before submission. For registered wait handles, `Stop()` should unregister not-started waits or wait for active callbacks, then close dependent handles and state.

Renderer clients should explicitly stop their network transport before stack-owned channel wrappers are destroyed, so callback shutdown completes before local wrapper storage goes out of scope.

Named-pipe shutdown should cancel pending overlapped pipe I/O before waiting for read callbacks to drain, especially when the remote side closes the pipe first. Channel-client destruction should also avoid stopping an already-disconnected transport connection.

## Port fixes from imports to source repositories

Do not treat files copied into `Import` or generated release files as the source of truth. When a fix affects imported `Vlpp` files, make the upstream change in `Vlpp`, regenerate its release output, and then copy the generated files downstream. When a `.github` instruction or script fix is needed, port it through `Tools/Copilot`.

When a downstream repo such as `GacUI` exposes a bug in imported `VlppOS` inter-process code, fix and verify it in `VlppOS`, regenerate `VlppOS\Release`, and then import the generated release files downstream.

When validating GacUI remoting reveals a transport issue, keep the same source-of-truth rule: fix `VlppOS`, regenerate its release, copy the generated output into `GacUI\Import`, then validate the downstream scenario again.

For dependency release syncs, copy generated files from the upstream `Release` folder into the downstream `Import` folder and exclude `IncludeOnly` unless the task explicitly requires it. Do not hand-edit the downstream import copy.

When importing multiple dependency releases into GacUI, keep the chain explicit: regenerate and import `VlppOS` and `Workflow` release artifacts, then validate the GacUI remoting scenarios that consume both imported APIs.

Shared Ubuntu build infrastructure must be changed in the canonical `Tools/Ubuntu` source first, committed and pushed there, and then propagated into a repository with `vgo uci REPO`. Do not make a propagated repository-local copy the source of truth.

If a Workflow task exposes a `VlppReflection` collection-wrapper issue, fix the wrapper behavior in `VlppReflection`, regenerate and verify its release output, then update the Workflow import from that release instead of patching Workflow's imported copy.

When a VlppOS public namespace refactor changes released APIs, regenerate the VlppOS release and update Workflow and GacUI from that release before repairing downstream build breaks; do not patch imported copies.

## Validate expectations against implementation and existing tests

Before encoding expectations (especially for return value conventions and error semantics), read the relevant implementation and check existing tests for established patterns. This reduces churn from mismatched assumptions (e.g. public API returning a normalized error value even if internals use different sentinel codes).

This also applies to enums and API surface: verify that enum values and method names/signatures actually exist before using them.

For regression tests, verify that the proposed case actually distinguishes broken and fixed behavior. A positive control can pass under both implementations, so retain a root-cause-sensitive reproduction and assert the relevant observable boundary, such as callback count and range, rather than only a coarse aggregate result.

For performance regressions, establish a same-machine baseline and instrument construction, connection, exchange, draining, and shutdown separately. Rule out each plausible hypothesis with direct state and timing evidence before changing production code; a plausible platform theory or a nearby commit is not a substitute for measuring the actual slow boundary.

## Prefer `operator<=> = default` for lexicographic key structs

When a struct is a pure lexicographic key (e.g. `{begin, end}`), prefer `auto operator<=>(const T&) const = default;` to generate a correct, consistent ordering and equality set automatically, instead of hand-writing `<`, `==`, etc.

## Use `collections::BinarySearchLambda` on contiguous buffers (guard empty)

`collections::BinarySearchLambda` expects a contiguous buffer pointer and count (e.g. `&keys[0]`, `keys.Count()`), plus a search item, an out `index`, and an orderer that returns `std::strong_ordering`. When using it on `Dictionary::Keys()`, guard the empty-map case before taking `&Keys()[0]`.

If you are searching for “overlap” rather than exact ordering, provide a custom orderer that defines “before / after / overlap” semantics for your ranges.

If multiple entries can satisfy “overlap”, binary search can return any matching entry. Scan backward (and/or forward) from the returned index to locate the first overlapping entry you intend to process.

For nearest-neighbor lookup on a sorted key buffer, implement helpers for largest key `<= item` and smallest key `> item`, and use those helpers for line/range iteration instead of mixing text positions with raw array indices.

## Prefer two-pointer merge for sorted range maps

When combining or diffing two maps that are already sorted by range keys, iterate both in one pass using a two-pointer “merge sort merge phase” approach. This avoids nested scans and keeps merge/diff logic linear in the number of runs.

## Treat Debug memory leak dumps as required failures

On Windows Debug unit-test runs with memory leak checking enabled, a test can pass all assertions and still fail engineering acceptance if the final execution log contains a CRT leak dump. Read the end of the log after tests pass and fix leaks instead of ignoring the appended report.

GacUI helper threads and stack-allocated callbacks are common sources of these post-pass leak dumps. Join helper threads and detach callbacks before scope exit instead of treating the final leak report as unrelated noise.

## Use named sentinel constants instead of raw values

When a protocol needs a sentinel value such as an administrator or system client id, use a named constant and validate it explicitly. Do not scatter raw values like `-1` through call sites; it makes sender/receiver semantics hard to audit and easy to misuse.

## Use `Variant<T...>::Index()` to check active alternative

For `Variant<T...>`, use `Index()` (active alternative index) to branch on the stored type; do not assume helper methods like `GetType()` exist.

## Avoid references into containers when mutating them

When iterating a container and performing mutations like `Remove()`/`Add()`, avoid holding references (`auto&&`) to elements across mutations, because internal storage can be reallocated or reordered. Copy keys/values you still need to local variables before modifying the container.

## Prefer designated initializers for aggregate-like structs

For small structs used as value objects (especially those with default member initializers), prefer designated initializers like `{ .field = value }` for clarity and to avoid compile-time issues from positional aggregate initialization.

## Prefer raw pointers unless shared ownership is required

When passing an object without transferring or extending ownership, prefer a raw pointer parameter over constructing `Ptr<T>` only to adapt the call. Reserve `Ptr<T>` for APIs that need shared ownership, lifetime extension, or storage in shared-pointer-managed state.

## Construct `Nullable<WString>` explicitly in function calls

When passing string literals to a function parameter typed as `Nullable<WString>`, wrap them in `WString(...)` (or otherwise construct a `WString`) to make the conversion explicit. Direct assignment to a `Nullable<WString>` field may compile via an assignment operator, but function-call argument conversion can require explicit construction.

## Sort serialization metadata by deterministic keys, not pointer addresses

When serializing metadata into stable binary output, do not let pointer-address ordering decide indices or item order. Collect items for membership checks if needed, then sort the serialized lists by deterministic keys such as type names or owner-qualified member signatures before assigning indices and writing the stream. This applies to type descriptors, methods, properties, events, and generated custom-type lists whose order would otherwise depend on allocation order or ASLR.

## Start async callbacks after most-derived construction

Objects that dispatch asynchronous callbacks should not begin listening or queue callbacks from base constructors. Initialize state in constructors, then expose an explicit `Start()` boundary that callers invoke after the most-derived object is fully constructed, so callbacks can safely dispatch to final overrides.

## Do not rely on `Event<T>` handler invocation order

When multiple handlers attached to the same `Event<T>` can both observe, mutate, or throw, do not assume attach order controls which handler runs first. Existing `Event<void(...)>` storage can make invocation order depend on handler pointer ordering, so fixes should remove order dependence instead of moving logic into another handler or making tests depend on allocator behavior.

## Use RAII scope cleanup instead of manual catch cleanup

When a helper temporarily suppresses callbacks, changes ownership flags, or otherwise establishes scoped state, use a small scope object whose destructor restores state during normal return and exception unwinding. Avoid manual `try`/`catch` blocks that only restore state and rethrow; C++ stack unwinding should own that cleanup.

## `collections::Dictionary` copy assignment is deleted (use move/swap)

`collections::Dictionary` does not support copy assignment. When you need to replace one dictionary with another, use move semantics (when appropriate), or rebuild/swap explicitly instead of `a = b`.

## Dereference `Ptr<T>` via `.Obj()` (not `*ptr`)

`Ptr<T>` is a smart pointer wrapper; to dereference it, use `ptr.Obj()` to get a raw pointer first (e.g. `*ptr.Obj()`). The `*ptr` syntax is not supported.

## Use `WString::IndexOf` with `wchar_t` (not `const wchar_t*`)

`vl::WString::IndexOf` searches for a single character when given a character parameter; pass character literals like `L'\r'` / `L'\n'`, not string literals like `L"\r"` / `L"\n"` (which are `const wchar_t*`).

## Use 2-space indentation in embedded XML/JSON literals

When writing XML or JSON inside a C++ string literal (e.g. `LR"GacUISrc(... )GacUISrc"` resources), indent the XML/JSON with 2 spaces (not tabs) to match the repo’s formatting rules for embedded structured text.

## Verify generated artifacts with downstream consumer checks

When a generator produces artifacts for another language or toolchain, validate the generated output with that downstream consumer instead of only checking that the generator runs. For example, generated TypeScript declarations for JSON output should be type-checked against real generated JSON examples with `tsc --noEmit`.

When generated RPC JSON values or request/response transcripts are part of the contract, run the preparation step that copies/regenerates them, include the new folders in the downstream project configuration, type-check them, and sample or audit the generated files for protocol invariants such as request/response id pairing.

For generated build files, compare the selected source list with the owning project metadata and platform add/remove rules, then perform a clean build. This catches stale tracked output and generators that appear to succeed after silently losing a compiler or dependency-scanner failure.

When generated build metadata must be platform-invariant, generate it with native and simulated host selection and compare tracked `makefile` and `vmake.txt` output byte for byte. Then perform clean and no-op incremental builds and inspect host-local dependency files, ensuring stable tracked output does not break platform-specific includes or incremental rebuilds.

When a generator produces runnable sample applications, verify the generated output through the actual app workflow too. For example, generated ChatBot RPC code should be checked by running the server and multiple clients through joins, chat messages, client exit, and server shutdown, not only by confirming generation succeeds.

When a shared dispatcher schema such as `Rpc.d.ts` changes, type-check the shared schema itself as well as generated fixtures so envelope changes are caught even before concrete generated values instantiate every request shape.

For a released VlppOS namespace change, validate Workflow through the ChatBot SOP and validate GacUI through `RemotingTest_Core /RPT /Http` with `RemotingTest_Rendering_Win32 /Http`, plus GacJS against the HTTP remoting core. An upstream build alone does not prove the imported public surface works.

## `vl::regex` separator regex: `L"[\\/\\\\]+"`

In `vl::regex::Regex`, both `/` and `\\` are escaping characters, and incorrect escaping inside `[]` can throw errors like `Illegal character set definition.`

To split paths by either `/` or `\\`, a verified pattern is `L"[\\/\\\\]+"`, and using `Regex::Split(..., keepEmptyMatch=false, ...)` conveniently drops empty components (so `//` behaves like `/`).

## `collections::List` has deleted copy constructor; use `std::move()` for structs with `List` members

When a C++ struct contains `vl::collections::List` fields, the struct's implicit copy constructor is deleted. Appending such structs to a `vl::collections::List` by copy will fail at compile time with `error C2280`. Use `std::move()` when adding these structs to destination lists (e.g. `list.Add(std::move(model))`). Note: there is no repo-provided `Move()` utility; always use `std::move()` from `<utility>`.

## Use `vl::Exception` for expected semantic failures and `CHECK_ERROR` for invariants

When a failure is part of the public or script-visible semantics and tests are expected to catch it as a recoverable error, throw `vl::Exception`. Reserve `CHECK_ERROR` / `CHECK_FAIL` / `vl::Error` for internal invariant violations and states that indicate implementation corruption. For example, duplicate RPC registration can remain a catchable semantic exception when samples intentionally verify it, while impossible local type ids should fail as invariants.

At script-visible reflection boundaries, translate recoverable collection operation failures into `vl::Exception` so Workflow `catch` blocks and RPC exception transport can observe them normally; do not expose `vl::Error` for expected user-data access failures.

## Compare type descriptors by pointer when descriptor identity is available

`GetTypeDescriptor<T>()` and `GetTypeDescriptor(typeName)` guarantee one descriptor instance per type in a loaded type manager, so prefer direct `ITypeDescriptor*` pointer comparison over comparing type-name strings. Use `TypeInfo<T>::content.typeName` only where the type manager cannot be loaded yet; if a name lookup is unavoidable, resolve the descriptor once and compare pointers inside hot or repeated paths.

## Parse and validate before queuing asynchronous work

When an API needs to report syntax or validation errors synchronously but execute accepted work asynchronously, split those phases explicitly. Parse and validate the request on the caller/transport path, return errors immediately, then queue only a parsed command object for main-thread or background execution. This keeps modal or blocking work off the response path without hiding malformed input behind an async boundary.

## Use Win32 messages for native dialogs when UIA is unavailable

When native Windows dialogs must be handled under conditions where UI Automation is unreliable or unavailable, use screenshots and Win32 enumeration/messages from a separate process. Enumerate top-level and child windows, read text/class/control ids, then use standard messages such as `BM_CLICK`, `WM_SETTEXT`, and combo-box selection messages to confirm, cancel, or fill controls. Do not wait indefinitely on the application under test while a native modal dialog is open; that app may be blocked until the external Win32 operation closes the dialog.

## Proactively remove code made redundant by refactoring

When a change makes a construction unnecessary or no longer meaningful, delete it as part of the same change instead of leaving it behind. This includes redirection/adapter layers that only forward, transport methods that exist solely for a now-collapsed path, specialized helpers superseded by a generic one, duplicated post-processing a callee already performs, and null checks the callee already handles (e.g. when `BoxValue`/`UnboxValue` already accept null, or when an `Invoke*` helper already reads/checks the result). Do not leave redirections that exist only because of history. It is acceptable to take the risk of breaking tests while removing redundant code, and then fix the tests afterward, rather than preserving dead structure to keep tests green.

Remove unused dependency parameters and fields as part of the refactor too. If every meaningful call path already receives a preconverted representation, do not keep serializer or adapter state only for a fallback that current callers neither need nor should use.

Preserve helper layers that still own observable behavior. For example, flat RPC dispatcher wrappers that only forward can be removed, but JSON wrappers that record generated TypeScript artifacts should stay until their recording responsibility is moved elsewhere.

When a destructor only resets `Ptr`, shared-pointer, or similar owning members to null, remove that destructor/reset code and let member destruction release resources naturally.

When splitting a monolithic implementation into focused files, delete empty source stubs and duplicate state fields in the same cleanup. Update project metadata and includes immediately so the old file names do not remain as stale references.

For application refactors, remove helper wrappers that only duplicate an already-clear direct call. For example, direct `GetChannels()[WString::Unmanaged(RpcChannel)]` access is preferable to a `GetRpcChannel` helper when the intended behavior is still to fail if the channel is missing.

## Keep design documentation aligned with code after refactoring

When a refactoring changes architecture or behavior, update the corresponding design documents in the same task rather than deferring it. After a structural change, re-read the related documents and reconcile anything that became misaligned (for example, descriptions of a transport path that no longer exists). Treat documentation drift left by a previous refactoring as part of the current cleanup.

Public namespace refactors require corresponding knowledge-base and documentation-site updates; publish the site when its generated or documented surface actually changes.

Once a shared behavioral suite already exists, follow-up platform implementation task documents should point to binding and running that suite rather than requesting duplicate test cases. Reuse authoritative platform details from existing design documents and preserve explicit out-of-scope sections.

## Fix behavior at the owning state instead of patching symptoms

When a bug is caused by state attached to a temporary or overly broad owner, move the state to the object or region that semantically owns it. Avoid compensating fixes such as forcing override values on every affected descendant, duplicating a template/resource just to mask inherited state, or adding side-channel code that only makes the symptom disappear. If a proposed fix looks like a patch, revisit the ownership boundary and preserve naturally correct inherited/default behavior for unaffected parts.

When an incremental API receives encoded code units but downstream logic consumes complete logical values, keep partial assembly or decoding state in the stream-processing object. Do not change the public input unit or force every caller to assemble complete values merely to fit an internal helper.

When a lifecycle guarantee applies to every subclass, publish completion and final state in the common native entry/exit path. Do not patch one derived class with a private completion event or duplicate the guarantee only in factory-created wrappers.

## Keep generated makefiles platform-invariant

Tracked `makefile` and `vmake.txt` output should remain identical across Linux and macOS. Keep every platform-guarded translation unit in the stable source list; the inactive translation unit should compile harmlessly through its preprocessor guards. Put monorepo-wide platform policy in the canonical shared `makefile-cpp` and select it at make execution time—for example, Darwin compile/framework options and Linux `-luring`—with libraries placed after object prerequisites.

Do not embed host-preprocessed `clang++ -MM` dependency output in tracked makefiles. Continue running the dependency probe as fail-fast validation, discard its host-specific output, and emit stable source-only object rules that depend on the shared make fragment. Generate ignored `Obj/*.d` files during actual compilation with `-MMD -MP` and include them for correct host-local incremental dependency tracking.
