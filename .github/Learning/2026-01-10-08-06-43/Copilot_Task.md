# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.3: Hijack current-time reading in editor unit tests

Move the existing `TooltipTimer` (feature injection for `vl::IDateTimeImpl`) to a shared test header and use it in all single-line textbox unit tests, to ensure time-dependent behaviors (e.g. cursor blink / delayed behaviors) do not read the real system clock and cause flaky logs.

### what to be done

- Follow the established scaffold from TASK No.2 (frame rule): in `UnitTestRemoteProtocol::OnNextIdleFrame`, treat each rendered frame as "the UI change done in the previous callback + the assertions done in the current callback"; do not end a callback (except the last one) unless it performs an input that will change UI in the next frame; choose frame titles that describe what happened in the previous callback; keep the final callback minimal so the last logged frame is easy to inspect.
- Move `TooltipTimer` from `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` to `Test/GacUISrc/UnitTest/TestControls.h` so it can be shared by other test files.
- Remove the unnecessary `extern` declaration from `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` after moving `TooltipTimer`.
- Update `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` to include and use the shared `TooltipTimer` from `TestControls.h` (no duplicated definition).
- Use `TooltipTimer` in every `TEST_CASE` in `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` and `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` (construct a local `TooltipTimer` at the beginning of each test case so time is hijacked for the whole test).

### rationale

- `UnitTestRemoteProtocol` logs and assertions are frame-based; making time deterministic reduces flakiness and makes frame-by-frame logs easier to compare.
- Centralizing `TooltipTimer` in `TestControls.h` avoids copy/paste and keeps time-hijacking consistent across editor test files.

# UPDATES

# INSIGHTS AND REASONING

## Current situation and evidence

- `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` defines `TooltipTimer : protected FeatureImpl<vl::IDateTimeImpl>` and uses `vl::InjectDateTimeImpl(this)` in its constructor and `vl::EjectDateTimeImpl(nullptr)` in its destructor, overriding `LocalTime()` to return a controlled time value.
- The knowledge base explicitly supports DateTime implementation injection for testing: `.github/KnowledgeBase/KB_Vlpp_DateTimeOperations.md` documents `InjectDateTimeImpl(...)` and `EjectDateTimeImpl(nullptr)` for restoring the default OS implementation, and emphasizes cleaning up to avoid impacting other tests.
- The unit test style used here is frame-based: `.github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md` describes `UnitTestRemoteProtocol::OnNextIdleFrame` as a per-render-frame phase boundary, which makes any time-sensitive rendering/state changes likely to influence logs across frames.

## Design goal

Make all editor-related unit tests deterministic with respect to `vl::DateTime::LocalTime()` / `vl::IDateTimeImpl::LocalTime()` by:

- Providing a shared, reusable time-hijacking helper (`TooltipTimer`) for unit tests.
- Ensuring injection lifetime is scoped to each `TEST_CASE` (RAII), so the injection cannot leak into other tests.
- Keeping control over time progression explicit (tests advance time only when they call helper APIs).

## Proposed changes (high level)

### 1) Move `TooltipTimer` to `TestControls.h`

- Put the `TooltipTimer` implementation into `Test/GacUISrc/UnitTest/TestControls.h`, so all unit test files can include it via the existing shared test header.
- Keep it as a small RAII helper:
  - Constructor: initialize `currentTime` to a fixed point (as currently done) and call `vl::InjectDateTimeImpl(this)`.
  - Destructor: call `vl::EjectDateTimeImpl(nullptr)` inside a `try/catch` to avoid throwing during stack unwinding (matching the current behavior).
  - Interface: keep the existing `Tooltip()` and `WaitForClosing()` helpers (or equivalent) to advance `currentTime` deterministically; editor tests can choose to call them (or not) depending on what they need to stabilize.
  - `vl::IDateTimeImpl` overrides: forward all operations to `Previous()` except `LocalTime()` (returns controlled time) and `UtcTime()` (computed deterministically from controlled local time using `Previous()->LocalToUtcTime(...)`, same as current code).

Rationale:
- The helper already exists and has proven usage in tooltip tests; centralizing it reduces duplication and makes the injection pattern consistent.
- Scoping by RAII is aligned with the knowledge base guidance to always eject injected implementations.

### 2) Update tooltip unit tests to consume the shared helper

- In `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp`:
  - Delete the local `TooltipTimer` class definition once the shared one exists in `TestControls.h`.
  - Delete the unused `extern IDateTimeImpl* GetOSDateTimeImpl();` declaration (it is not referenced by the current `TooltipTimer` implementation).
  - Keep the existing per-`TEST_CASE` `TooltipTimer timer;` pattern unchanged, now referring to the shared definition.

Rationale:
- Keeps the tooltip tests as the “reference usage” of the helper while removing duplicated implementation.

### 3) Use `TooltipTimer` in single-line editor unit tests

- For `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`:
  - Add `TooltipTimer timer;` as the first statement inside each `TEST_CASE` body so the injection remains active across all `OnNextIdleFrame(...)` callbacks and through the `GacUIUnitTest_StartFast_WithResourceAsText(...)` run.
- For `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`:
  - Do the same in every `TEST_CASE` in all categories, including the existing scaffold tests.

Design notes:
- Prefer constructing the timer before calling `GacUIUnitTest_SetGuiMainProxy(...)` so any time reads occurring during proxy setup or initial frames are also controlled.
- Do not create a shared static/global `TooltipTimer` for a whole file: injection is global; per-test RAII keeps lifetime tight and avoids cross-test contamination if future test execution becomes parallelized.

## Alternatives considered

### A) One injection per test file (global/static)

Pros:
- Less repeated boilerplate in test cases.

Cons:
- Risks leaking the injected implementation into other test files if the test runner interleaves tests or if teardown ordering changes.
- Harder to reason about if multiple tests (or nested runs) start injecting time differently.

### B) Extend `UnitTestRemoteProtocol` / unit test harness with a time provider

Pros:
- Better encapsulation (time control becomes part of the test infrastructure rather than relying on a global injection).

Cons:
- Much larger surface area change and likely touches production-like infrastructure; not appropriate for a “stabilize editor tests” task.

The proposed approach is the smallest change that reuses existing, already-working test-only infrastructure.

## Risks and mitigations

- **Global injection scope**: `InjectDateTimeImpl` affects the process-wide DateTime implementation.
  - Mitigation: strict per-`TEST_CASE` RAII and mandatory `EjectDateTimeImpl(nullptr)` in the destructor (as already implemented).
- **Time not progressing**: Some visual behaviors might depend on time advancing.
  - Mitigation: expose explicit “advance time” helper methods (existing `Tooltip()` / `WaitForClosing()` or a more general “Forward(ms)”) and call them only in test cases that need the behavior.

# !!!FINISHED!!!
