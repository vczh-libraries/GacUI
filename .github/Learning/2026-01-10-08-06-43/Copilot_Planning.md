# !!!PLANNING!!!

# UPDATES

## UPDATE
Do not change existing frame name. Although I would like to describe what happened in the previous frame, but you must keep it like a title

# EXECUTION PLAN

## STEP 1: Move `TooltipTimer` to `TestControls.h`

Change:
- Move the `TooltipTimer` helper from `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` into the shared test header `Test/GacUISrc/UnitTest/TestControls.h` so every unit test file can reuse it.
- Keep the behavior identical (RAII injection + deterministic time), so existing tooltip tests keep working unchanged.

Proposed code (add to `Test/GacUISrc/UnitTest/TestControls.h`, near the end of the file):

	class TooltipTimer : protected vl::feature_injection::FeatureImpl<vl::IDateTimeImpl>
	{
	private:
		vl::DateTime			currentTime;

	public:
		TooltipTimer()
		{
			currentTime = vl::DateTime::FromDateTime(2000, 1, 1, 0, 0, 0, 0);
			vl::InjectDateTimeImpl(this);
		}

		~TooltipTimer()
		{
			try
			{
				vl::EjectDateTimeImpl(nullptr);
			}
			catch (...)
			{
			}
		}

		void Tooltip()
		{
			currentTime = currentTime.Forward(600);
		}

		void WaitForClosing()
		{
			currentTime = currentTime.Forward(5100);
		}

	protected:
		vl::DateTime FromDateTime(vint _year, vint _month, vint _day, vint _hour, vint _minute, vint _second, vint _milliseconds) override
		{
			return Previous()->FromDateTime(_year, _month, _day, _hour, _minute, _second, _milliseconds);
		}

		vl::DateTime FromOSInternal(vuint64_t osInternal) override
		{
			return Previous()->FromOSInternal(osInternal);
		}

		vuint64_t LocalTime() override
		{
			return currentTime.osMilliseconds;
		}

		vuint64_t UtcTime() override
		{
			return Previous()->LocalToUtcTime(currentTime.osInternal);
		}

		vuint64_t LocalToUtcTime(vuint64_t osInternal) override
		{
			return Previous()->LocalToUtcTime(osInternal);
		}

		vuint64_t UtcToLocalTime(vuint64_t osInternal) override
		{
			return Previous()->UtcToLocalTime(osInternal);
		}

		vuint64_t Forward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Forward(osInternal, milliseconds);
		}

		vuint64_t Backward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Backward(osInternal, milliseconds);
		}
	};

Why:
- `vl::InjectDateTimeImpl(...)` is process-wide; keeping a single shared RAII helper makes the injection pattern consistent and reduces the chance of forgetting to eject.
- Editor unit tests should not read the real clock; deterministic `LocalTime()` stabilizes frame-based assertions and logs.

## STEP 2: Update tooltip tests to use the shared helper

Change:
- In `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp`:
  - Remove the local `TooltipTimer` class definition (it now lives in `TestControls.h`).
  - Remove `extern IDateTimeImpl* GetOSDateTimeImpl();` (it is unused after removing the local class).
  - Keep `TooltipTimer timer;` in each `TEST_CASE` to ensure injection is scoped to the test.

Why:
- Prevents duplicated implementations of the same injection helper.
- Keeps the injection lifetime explicit per `TEST_CASE` (no leak into other tests).

## STEP 3: Hijack time in all single-line textbox editor unit tests

Change:
- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`:
  - Add a local `TooltipTimer timer;` as the first statement inside every `TEST_CASE` body.
- In `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp`:
  - Add a local `TooltipTimer timer;` as the first statement inside every `TEST_CASE` body.

Proposed code pattern (per `TEST_CASE`):

		TEST_CASE(L"...")
		{
			TooltipTimer timer;
			...
		});

Why:
- Ensures any `vl::DateTime::LocalTime()` reads during test setup, frame rendering, and frame callbacks never touch the OS clock.
- Per-test RAII keeps injection lifetime tight and avoids cross-test contamination.

## STEP 4: Align `OnNextIdleFrame` scaffold in the key scaffold test

Change:
- Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` to follow the established frame rule:
  - Treat frame N as: UI changes issued in callback (N-1) + assertions in callback (N).
  - Do not place assertions after inputs inside the same callback (those changes belong to the next frame).
  - Use frame titles that describe what happened in the previous callback.
  - Keep the final callback minimal so the last logged frame is easy to inspect.
  - Keep existing frame titles unchanged.

Proposed structure for `Scaffold/SmokingTest`:

			protocol->OnNextIdleFrame(L"Init", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"");
				textBox->SetFocused();
				protocol->TypeString(L"0123456789");
			});

			protocol->OnNextIdleFrame(L"Typing", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"0123456789");
				protocol->KeyPress(VKEY::KEY_LEFT);
				protocol->TypeString(L"X");
			});

			protocol->OnNextIdleFrame(L"Moved Cursor and Edited", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
				TEST_ASSERT(textBox->GetText() == L"012345678X9");
				window->Hide();
			});

Why:
- Matches the repo’s frame-based unit test convention, making logs predictable and assertions easier to reason about.
- Avoids testing "intermediate" states that might not be committed until the next rendered frame.

# TEST PLAN

- **Tooltip tests**: Run `Test/GacUISrc/UnitTest/TestApplication_Tooltip.cpp` cases and confirm behavior and frame logs remain stable.
- **Single-line editor tests**: Run `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp` and `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` and confirm all assertions pass.
- **Repeatability**: Repeat the above tests multiple times to confirm logs are no longer sensitive to the OS clock.
- **Injection cleanup regression**: Run at least one unrelated unit test after these to confirm `TooltipTimer` always ejects and does not affect subsequent tests.

# !!!FINISHED!!!
