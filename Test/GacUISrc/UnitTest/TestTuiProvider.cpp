#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/TUI/TuiController.h"
#include "../../../Source/PlatformProviders/TUI/TuiGraphics.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::console;

namespace tui_provider_tests
{
	using namespace vl::console::unittest;

	class TuiTestBackend : public Object, public ITuiBackend
	{
	public:
		vint width = 16;
		vint height = 8;
		vint starts = 0;
		vint stops = 0;
		vint presents = 0;
		vint nextEvent = 0;
		vuint64_t time = 0;
		List<TuiBackendEvent> events;
		Array<TuiPixel> frame;

		TuiColorMode Start(const TuiStartOptions&) override { starts++; return TuiColorMode::TrueColor; }
		void Stop() override { stops++; }
		bool TryGetConsoleSize(vint& x, vint& y) override { x = width; y = height; return true; }
		vuint64_t GetMonotonicTime() override { return time; }
		bool ReadEvent(vint milliseconds, TuiBackendEvent& event) override
		{
			time += std::max((vint)1, milliseconds);
			if (nextEvent == events.Count()) return false;
			event = events[nextEvent++];
			if (event.type == TuiBackendEventType::Resize) { width = event.width; height = event.height; }
			return true;
		}
		void Render(const TuiPixel* buffer, vint x, vint y, TuiColorMode) override
		{
			presents++;
			frame.Resize(x * y);
			for (vint i = 0; i < frame.Count(); i++) frame[i] = buffer[i];
		}
	};

	class TuiTestNativeServices : public Object, public INativeController
	{
	public:
		SharedCallbackService callbacks;
		INativeCallbackService* CallbackService() override { return &callbacks; }
		INativeResourceService* ResourceService() override { return nullptr; }
		INativeAsyncService* AsyncService() override { return nullptr; }
		INativeClipboardService* ClipboardService() override { return nullptr; }
		INativeImageService* ImageService() override { return nullptr; }
		INativeScreenService* ScreenService() override { return nullptr; }
		INativeWindowService* WindowService() override { return nullptr; }
		INativeInputService* InputService() override { return nullptr; }
		INativeDialogService* DialogService() override { return nullptr; }
		INativeAutomationService* AutomationService() override { return nullptr; }
		WString GetExecutablePath() override { return L"TuiTest"; }
	};

	class TuiTestController : public TuiControllerBase
	{
	public:
		Func<void()> test;
		WString title;
		vint pumps = 0;
		TuiTestController(INativeController* services) : TuiControllerBase(services) {}
		void Starting() override { test(); Stop(); }
		void PumpPlatformEvents() override { pumps++; }
		void ApplyTitle(const WString& value) override { title = value; }
	};

	class TuiTestWindowListener : public INativeWindowListener
	{
	public:
		vint moved = 0;
		vint destroying = 0;
		vint destroyed = 0;
		vint keys = 0;
		vint releases = 0;
		vint chars = 0;
		vint mouse = 0;
		vint wheel = 0;
		NativeWindowKeyInfo key;
		NativeWindowCharInfo character;
		NativeWindowMouseInfo position;
		void Moved() override { moved++; }
		void Destroying() override { destroying++; }
		void Destroyed() override { destroyed++; }
		void KeyDown(const NativeWindowKeyInfo& info) override { keys++; key = info; }
		void KeyUp(const NativeWindowKeyInfo&) override { releases++; }
		void Char(const NativeWindowCharInfo& info) override { chars++; character = info; }
		void MouseMoving(const NativeWindowMouseInfo& info) override { mouse++; position = info; }
		void VerticalWheel(const NativeWindowMouseInfo& info) override { wheel = info.wheel; }
	};

	class TuiTestTimerListener : public INativeControllerListener
	{
	public:
		vint ticks = 0;
		void GlobalTimer() override { ticks++; }
	};

	void TuiRunTest(const Func<void(TuiTestBackend*, TuiTestController*)>& test)
	{
		auto backend = Ptr(new TuiTestBackend);
		ScopedTuiBackend scoped(backend);
		TuiTestNativeServices services;
		TuiTestController controller(&services);
		controller.test = [&]() { test(backend.Obj(), &controller); };
		TEST_ASSERT(TUI::InstallListener(&controller));
		TUI::Start({});
		TEST_ASSERT(TUI::UninstallListener(&controller));
		TEST_ASSERT(backend->starts == 1 && backend->stops == 1);
		TEST_ASSERT(!TUI::IsInUse());
		TEST_ASSERT(GetTuiApplication() == nullptr);
	}
}

using namespace tui_provider_tests;

TEST_FILE
{
	TEST_CASE(L"TUI scalar offsets, tab stops, CRLF, wrapping and caret sides")
	{
		TuiGraphicsLayoutProvider provider;
		auto paragraph = provider.CreateParagraph(L"A\t\x4E2D\U0001F600\r\nZ", nullptr, nullptr);
#ifdef VCZH_WCHAR_UTF16
		const vint supplementaryEnd = 5;
		TEST_ASSERT(!paragraph->IsValidCaret(4));
#else
		const vint supplementaryEnd = 4;
#endif
		TEST_ASSERT(paragraph->GetSize() == Size(8, 2));
		TEST_ASSERT(paragraph->GetCaretBounds(2, true).LeftTop() == Point(4, 0));
		TEST_ASSERT(paragraph->GetCaretBounds(3, true).LeftTop() == Point(6, 0));
		TEST_ASSERT(!paragraph->IsValidCaret(supplementaryEnd + 1));
		TEST_ASSERT(paragraph->GetCaretBounds(supplementaryEnd + 2, true).LeftTop() == Point(0, 1));
		TEST_ASSERT(paragraph->GetNearestCaretFromTextPos(3, false) == 3);
		bool front = true;
		TEST_ASSERT(paragraph->GetCaret(3, IGuiGraphicsParagraph::CaretMoveRight, front) == supplementaryEnd);
		TEST_ASSERT(paragraph->GetCaret(supplementaryEnd, IGuiGraphicsParagraph::CaretMoveLeft, front) == 3);
		TEST_ASSERT(paragraph->GetCaretFromPoint(Point(5, 0)) == 3);
		auto natural = paragraph->GetSize();
		paragraph->SetFont(0, 1, L"OtherFont");
		paragraph->SetSize(0, 1, 1000);
		TEST_ASSERT(paragraph->GetSize() == natural);

		auto wrapped = provider.CreateParagraph(L"ABCD", nullptr, nullptr);
		wrapped->SetWrapLine(true);
		wrapped->SetMaxWidth(2);
		TEST_ASSERT(wrapped->GetSize() == Size(2, 2));
		TEST_ASSERT(wrapped->GetCaretBounds(2, true).LeftTop() == Point(2, 0));
		TEST_ASSERT(wrapped->GetCaretBounds(2, false).LeftTop() == Point(0, 1));
		wrapped->SetMaxWidth(5);
		wrapped->SetParagraphAlignment(Alignment::Right);
		TEST_ASSERT(wrapped->GetCaretBounds(0, true).LeftTop() == Point(1, 0));
		auto empty = provider.CreateParagraph(L"", nullptr, nullptr);
		TEST_ASSERT(empty->GetSize() == Size(0, 1));
		TEST_ASSERT(empty->IsValidCaret(0));
		TEST_ASSERT(TuiEllipsizeText(L"AB\r\nABCDE", 3) == L"AB\r\nAB\u2026");
		TEST_ASSERT(TuiEllipsizeText(L"\u4E2D\U0001F600", 2) == L"\u2026");
		TEST_ASSERT(TuiEllipsizeText(L"ABC", 0) == L"");
	});

	TEST_CASE(L"TUI inline objects are atomic and reset to native text")
	{
		TuiGraphicsLayoutProvider provider;
		auto paragraph = provider.CreateParagraph(L"AxyzB", nullptr, nullptr);
		IGuiGraphicsParagraph::InlineObjectProperties object;
		object.size = Size(6, 3);
		object.breakCondition = IGuiGraphicsParagraph::Alone;
		object.callbackId = 12;
		TEST_ASSERT(paragraph->SetInlineObject(1, 3, object));
		TEST_ASSERT(!paragraph->SetInlineObject(2, 2, object));
		TEST_ASSERT(!paragraph->IsValidCaret(2));
		TEST_ASSERT(paragraph->GetSize() == Size(8, 3));
		object.backgroundColor = Color(20, 30, 40);
		TEST_ASSERT(paragraph->SetInlineObject(1, 3, object));
		object.callbackId = 13;
		TEST_ASSERT(!paragraph->SetInlineObject(1, 3, object));
		object.callbackId = 12;
		TEST_ASSERT(paragraph->GetNearestCaretFromTextPos(2, true) == 1);
		TEST_ASSERT(paragraph->GetNearestCaretFromTextPos(2, false) == 4);
		vint start = -1;
		vint length = -1;
		auto hit = paragraph->GetInlineObjectFromPoint(Point(2, 1), start, length);
		TEST_ASSERT(hit && hit.Value().callbackId == 12 && start == 1 && length == 3);
		TEST_ASSERT(paragraph->ResetInlineObject(1, 3));
		TEST_ASSERT(paragraph->IsValidCaret(2));
		TEST_ASSERT(paragraph->GetSize() == Size(5, 1));

		auto baseline = provider.CreateParagraph(L"A#B", nullptr, nullptr);
		object.size = Size(2, 3);
		object.baseline = 1;
		TEST_ASSERT(baseline->SetInlineObject(1, 1, object));
		TEST_ASSERT(baseline->GetCaretBounds(0, true).y1 == 0);
		TEST_ASSERT(baseline->GetSize() == Size(4, 3));
		for (auto condition : {IGuiGraphicsParagraph::StickToPreviousRun, IGuiGraphicsParagraph::StickToNextRun})
		{
			auto joined = provider.CreateParagraph(condition == IGuiGraphicsParagraph::StickToPreviousRun ? L"AB#" : L"A#B", nullptr, nullptr);
			object.size = Size(2, 1);
			object.baseline = -1;
			object.breakCondition = condition;
			TEST_ASSERT(joined->SetInlineObject(condition == IGuiGraphicsParagraph::StickToPreviousRun ? 2 : 1, 1, object));
			joined->SetWrapLine(true);
			joined->SetMaxWidth(3);
			TEST_ASSERT(joined->GetSize() == Size(3, 2));
			TEST_ASSERT(joined->GetCaretBounds(1, false).LeftTop() == Point(0, 1));
		}
	});

	TEST_CASE(L"TUI inline callbacks use paragraph coordinates and resize the next layout")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			class Callback : public Object, public IGuiGraphicsParagraphCallback
			{
			public:
				Rect location;
				Size OnRenderInlineObject(vint callbackId, Rect bounds) override
				{
					TEST_ASSERT(callbackId == 12);
					location = bounds;
					return Size(4, 3);
				}
			} callback;
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			TuiGraphicsLayoutProvider provider;
			auto paragraph = provider.CreateParagraph(L"A#B", &target, &callback);
			IGuiGraphicsParagraph::InlineObjectProperties object;
			object.callbackId = 12;
			object.size = Size(2, 1);
			TEST_ASSERT(paragraph->SetInlineObject(1, 1, object));
			target.StartHostedRendering();
			target.StartRendering();
			paragraph->Render(Rect(3, 4, 12, 8));
			TEST_ASSERT(callback.location == Rect(1, 0, 3, 1));
			TEST_ASSERT(paragraph->GetSize() == Size(6, 3));
			target.StopRendering();
			target.StopHostedRendering();
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI window geometry, events, timer work and explicit stopping")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			TuiTestWindowListener listener;
			window->InstallListener(&listener);
			TEST_ASSERT(window->GetClientSize() == NativeSize(16, 8));
			TEST_ASSERT(!window->IsVisible());
			window->SetBounds(NativeRect(NativePoint(100, 200), NativeSize(40, 20)));
			TEST_ASSERT(window->GetBounds().LeftTop() == NativePoint());
			TEST_ASSERT(window->GetClientSize() == NativeSize(40, 20));
			TEST_ASSERT(backend->width == 16 && backend->height == 8);
			window->SetTitle(L"Terminal title");
			TEST_ASSERT(controller->title == L"Terminal title");
			window->ShowMaximized();
			window->Hide(true);
			TEST_ASSERT(window->IsVisible() && window->IsActivated());
			TEST_ASSERT(!window->IsCustomFrameModeEnabled());
			TEST_ASSERT(window->Convert(NativePoint(3, 4)) == Point(3, 4));
			TEST_ASSERT(controller->ResourceService()->GetDefaultFont().size == 1);
			TEST_ASSERT(controller->ResourceService()->GetDefaultFont().fontFamily == L"TuiFont");
			List<WString> fonts;
			controller->ResourceService()->EnumerateFonts(fonts);
			TEST_ASSERT(fonts.Count() == 1 && fonts[0] == L"TuiFont");
			TEST_ASSERT(!controller->AutomationService());

			TuiBackendEvent key;
			key.type = TuiBackendEventType::KeyDown;
			key.keyInfo.code = VKEY::KEY_A;
			key.keyInfo.alt = true;
			backend->events.Add(key);
			auto release = key;
			release.type = TuiBackendEventType::KeyUp;
			backend->events.Add(release);
			TuiBackendEvent character;
			character.type = TuiBackendEventType::Char;
			character.charInfo.code = L'a';
			backend->events.Add(character);
			TuiBackendEvent mouse;
			mouse.type = TuiBackendEventType::MouseMove;
			mouse.mouseInfo.x = 3;
			mouse.mouseInfo.y = 4;
			mouse.mouseInfo.alt = true;
			backend->events.Add(mouse);
			mouse.type = TuiBackendEventType::MouseVerticalWheel;
			mouse.mouseInfo.wheel = -120;
			backend->events.Add(mouse);
			TuiBackendEvent resize;
			resize.type = TuiBackendEventType::Resize;
			resize.width = 10;
			resize.height = 5;
			backend->events.Add(resize);
			for (vint i = 0; i < 6; i++) controller->RunOneCycle();
			TEST_ASSERT(listener.keys == 1 && listener.releases == 1 && listener.chars == 1);
			TEST_ASSERT(listener.key.alt && !listener.key.osSuper && listener.character.code == L'a');
			TEST_ASSERT(listener.mouse == 1 && listener.position.x == 3 && listener.position.y == 4);
			TEST_ASSERT(listener.wheel == -120);
			TEST_ASSERT(window->GetClientSize() == NativeSize(10, 5) && listener.moved >= 2);

			TuiTestTimerListener timer;
			controller->CallbackService()->InstallListener(&timer);
			vint invoked = 0;
			const auto owner = Thread::GetCurrentThreadId();
			controller->AsyncService()->InvokeInMainThread(window, [&]()
			{
				TEST_ASSERT(Thread::GetCurrentThreadId() == owner);
				invoked++;
			});
			controller->AsyncService()->DelayExecuteInMainThread([&]() { invoked++; }, 0);
			controller->StartTimer();
			for (vint i = 0; i < 20 && invoked != 2; i++) controller->RunOneCycle();
			TEST_ASSERT(invoked == 2 && timer.ticks > 0);
			controller->Stop();
			const auto ticks = timer.ticks;
			controller->Timer();
			TEST_ASSERT(timer.ticks == ticks && !controller->RunOneCycle());
			controller->CallbackService()->UninstallListener(&timer);
			controller->DestroyNativeWindow(window);
			TEST_ASSERT(listener.destroying == 1 && listener.destroyed == 1);
		});
	});

	TEST_CASE(L"TUI Run opens the physical window before dispatching application work")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			TEST_ASSERT(!window->IsVisible());
			bool invoked = false;
			controller->AsyncService()->InvokeInMainThread(window, [&]()
			{
				TEST_ASSERT(window->IsVisible());
				invoked = true;
				controller->Stop();
			});
			controller->StartTimer();
			controller->Run(window);
			TEST_ASSERT(invoked);
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI composed frame, clipping, border merging, alpha and wide-cell repair")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			TuiGraphicsRenderTarget target(window);
			target.StartRendering();
			target.Fill(Rect(0, 0, 16, 8), Color(255, 0, 0));
			target.StopRendering();
			TEST_ASSERT(backend->presents == 0);
			window->Show();
			target.StartHostedRendering();
			target.StartRendering();
			target.Fill(Rect(0, 0, 16, 8), Color(10, 20, 30));
			target.Border(Rect(0, 1, 8, 2), Color(255, 255, 255), TuiLineStyle::Thin, {});
			target.Border(Rect(3, 0, 4, 4), Color(255, 255, 255), TuiLineStyle::Thin, {});
			TEST_ASSERT(TUI::GetBuffer()[19].GetChar32() == U'\u253C');
			target.StopRendering();
			target.StartRendering();
			target.PushClipper(Rect(2, 2, 8, 6), nullptr);
			target.Border(Rect(0, 2, 10, 7), Color(255, 255, 255), TuiLineStyle::Double, {});
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].GetChar32() == U'\u2550');
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 2].GetChar32() == 0);
			target.Print(Point(7, 3), U'\u4E2D', Color(255, 255, 255), Color(0, 0, 0, 0), {});
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 7].GetChar32() == 0);
			target.PopClipper(nullptr);
			target.Print(Point(4, 4), U'\u4E2D', Color(255, 255, 255), Color(0, 0, 0, 0), {true, true, true, true});
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 5].glyph == TuiPixelGlyph::WideCharContinuation);
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 4].character.style.bold);
			target.Fill(Rect(5, 4, 6, 5), Color(200, 100, 50));
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 4].GetChar32() == 0);
			target.Fill(Rect(10, 0, 11, 1), Color(110, 120, 130, 128));
			auto color = TUI::GetBuffer()[10].backgroundColor;
			TEST_ASSERT(color.r == 60 && color.g == 70 && color.b == 80);
			target.StopRendering();
			TEST_ASSERT(backend->presents == 0);
			target.StopHostedRendering();
			TEST_ASSERT(backend->presents == 1);
			target.StartHostedRendering();
			target.StopHostedRendering();
			TEST_ASSERT(backend->presents == 2 && backend->frame[19].GetChar32() == 0);
			controller->DestroyNativeWindow(window);
		});
	});
}
