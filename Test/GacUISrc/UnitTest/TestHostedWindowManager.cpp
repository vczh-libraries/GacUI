#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Hosted/GuiHostedWindowManager.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace vl::presentation;

namespace hosted_window_manager_tests
{
	extern WString GetTestBaselinePath();
}
using namespace hosted_window_manager_tests;

//#define UPDATE_SNAPSHOT

using Window = hosted_window_manager::Window<wchar_t>;

struct WindowManager : hosted_window_manager::WindowManager<wchar_t>
{
	WString						unitTestTitle;
	List<Pair<vint, WString>>	snapshots;

	Regex						regexEventDelimiter{ L",/s*" };
	List<WString>				events;

	void OnOpened(Window* window) override		{ events.Add(WString::FromChar(window->id) + L"O"); }
	void OnClosed(Window* window) override		{ events.Add(WString::FromChar(window->id) + L"o"); }
	void OnEnabled(Window* window) override		{ events.Add(WString::FromChar(window->id) + L"E"); }
	void OnDisabled(Window* window) override	{ events.Add(WString::FromChar(window->id) + L"e"); }
	void OnGotFocus(Window* window) override	{ events.Add(WString::FromChar(window->id) + L"F"); }
	void OnLostFocus(Window* window) override	{ events.Add(WString::FromChar(window->id) + L"f"); }
	void OnActivated(Window* window) override	{ events.Add(WString::FromChar(window->id) + L"A"); }
	void OnDeactivated(Window* window) override	{ events.Add(WString::FromChar(window->id) + L"a"); }

	WindowManager(const wchar_t* _unitTestTitle)
	{
		unitTestTitle = GenerateToStream([&](StreamWriter& writer)
		{
			while (auto c = *_unitTestTitle++)
			{
				writer.WriteChar(c == L' ' ? L'_' : c);
			}
		});
	}

	void EnsureCleanedUp()
	{
		TEST_ASSERT(!mainWindow);
		TEST_ASSERT(registeredWindows.Count() == 0);
		TEST_ASSERT(events.Count() == 0);

		auto snapshotPath = FilePath(GetTestBaselinePath()) / (unitTestTitle + L".txt");
#ifdef UPDATE_SNAPSHOT
		FileStream fileStream(snapshotPath.GetFullPath(), FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf8);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		for (auto [lines, snapshot] : snapshots)
		{
			writer.WriteLine(snapshot);
		}
		writer.WriteLine(L"END");
#else
		FileStream fileStream(snapshotPath.GetFullPath(), FileStream::ReadOnly);
		BomDecoder decoder;
		DecoderStream decoderStream(fileStream, decoder);
		StreamReader reader(decoderStream);
		for (auto [lines, snapshot] : snapshots)
		{
			// "lines = lines" to workaround clang++ 13.0.0-2
			WString baseline = GenerateToStream([&, lines = lines](StreamWriter& writer)
				{
					for (vint i = 0; i < lines; i++)
					{
						writer.WriteLine(reader.ReadLine());
					}
				});
			TEST_ASSERT(snapshot == baseline);
			TEST_ASSERT(reader.ReadLine() == L"");
		}
		TEST_ASSERT(reader.ReadLine() == L"END");
#endif
	}

	void CheckWindowStatus()
	{
		SortedList<Window*> activeWindows;
		{
			auto current = activeWindow;
			while (current)
			{
				if (current->visible && current->enabled)
				{
					activeWindows.Add(current);
				}
				current = current->parent;
			}
		}

		for (auto window : registeredWindows.Values())
		{
			TEST_ASSERT(window->active == (window == activeWindow));
			TEST_ASSERT(window->renderedAsActive == activeWindows.Contains(window));
			TEST_ASSERT(!window->active || window->enabled);
			TEST_ASSERT(!window->active || window->renderedAsActive);
			TEST_ASSERT((window->parent != nullptr) == (window != mainWindow));

			for (auto child : window->children)
			{
				TEST_ASSERT(child->parent == window);
			}

			if (!window->visible)
			{
				TEST_ASSERT(!topMostedWindowsInOrder.Contains(window));
				TEST_ASSERT(!ordinaryWindowsInOrder.Contains(window));
			}
			else if (window->IsEventuallyTopMost())
			{
				TEST_ASSERT(topMostedWindowsInOrder.Contains(window));
				TEST_ASSERT(!ordinaryWindowsInOrder.Contains(window));
				auto current = window;
				while (current && !current->visible)
				{
					current = current->parent;
				}
				if (current && current != window && current->IsEventuallyTopMost())
				{
					TEST_ASSERT(topMostedWindowsInOrder.IndexOf(window) < topMostedWindowsInOrder.IndexOf(current));
				}
			}
			else
			{
				TEST_ASSERT(!topMostedWindowsInOrder.Contains(window));
				TEST_ASSERT(ordinaryWindowsInOrder.Contains(window));
				auto current = window;
				while (current && !current->visible)
				{
					current = current->parent;
				}
				if (current && current != window)
				{
					TEST_ASSERT(!current->IsEventuallyTopMost());
					TEST_ASSERT(ordinaryWindowsInOrder.IndexOf(window) < ordinaryWindowsInOrder.IndexOf(current));
				}
			}
		}

		TEST_ASSERT(
			topMostedWindowsInOrder.Count() + ordinaryWindowsInOrder.Count() ==
			From(registeredWindows.Values()).Where([](auto window) { return window->visible; }).Count()
			);
	}

	void AssertEvents(const wchar_t* eventNames)
	{
		List<Ptr<RegexMatch>> matches;
		if (wcscmp(eventNames, L"*") != 0)
		{
			regexEventDelimiter.Split(eventNames, true, matches);
		}

		SortedList<WString> expected, actual;
		CopyFrom(expected, From(matches).Select([](auto && match) { return match->Result().Value(); }));
		CopyFrom(actual, events);
		events.Clear();
		TEST_ASSERT(CompareEnumerable(expected, actual) == 0);
	}

	void TakeSnapshot(const wchar_t* title)
	{
		vint w = 0;
		vint h = 0;
		for (auto window : registeredWindows.Values())
		{
			if (w < window->bounds.Width().value) w = window->bounds.Width().value;
			if (h < window->bounds.Height().value) h = window->bounds.Height().value;
		}

		WString snapshot = GenerateToStream([=](StreamWriter& writer)
		{
			writer.WriteLine(title);
			for (vint y = 0; y < h; y++)
			{
				writer.WriteChar(L'[');
				for (vint x = 0; x < w; x++)
				{
					auto window = HitTest({ {x},{y} });
					if (!window)
					{
						writer.WriteChar(L' ');
					}
					else if (!window->enabled)
					{
						writer.WriteChar((x + y) % 2 == 0 ? '~' : window->id + (L'a' - L'A'));
					}
					else if (!window->renderedAsActive)
					{
						writer.WriteChar(window->id + (L'a' - L'A'));
					}
					else
					{
						writer.WriteChar(window->id);
					}
				}
				writer.WriteLine(WString::Unmanaged(L"]"));
			}
		});
		snapshots.Add({ h + 1,snapshot });
	}

	void DontTakeSnapshot(const wchar_t* title)
	{
		snapshots.Add({ 1,WString::Unmanaged(title) + WString::Unmanaged(L"\r\n") });
	}
};

#define WM_TEST_CASE(NAME)							\
	TEST_CASE(NAME)									\
	{												\
		WindowManager wm(NAME);						\

#define TAKE_SNAPSHOT_INTERNAL(COMMAND, TITLE)		\
	do{												\
		COMMAND;									\
		TEST_ASSERT(wm.needRefresh);				\
		wm.needRefresh = false;						\
		wm.CheckWindowStatus();						\
		wm.TakeSnapshot(TITLE);						\
	}while(false)									\

#define EVENTS(...) do{ wm.AssertEvents(L ## #__VA_ARGS__); }while(false)
#define TAKE_SNAPSHOT(COMMAND) TAKE_SNAPSHOT_INTERNAL(COMMAND, L ## #COMMAND)
#define TAKE_SNAPSHOT_INITIAL() TAKE_SNAPSHOT_INTERNAL((void)nullptr, L"<Initial>")

#define DONT_TAKE_SNAPSHOT(...)						\
	do {											\
		__VA_ARGS__;								\
		TEST_ASSERT(!wm.needRefresh);				\
		wm.CheckWindowStatus();						\
		wm.DontTakeSnapshot(L ## #__VA_ARGS__);		\
	}while(false)									\

NativeRect Bounds(vint x, vint y, vint w, vint h)
{
	return { { {x},{y} },{ {w},{h} } };
}

TEST_FILE
{
	WM_TEST_CASE(L"Start and stop")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		mainWindow.SetBounds(Bounds(0, 0, 6, 4));

		wm.Start(&mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		TAKE_SNAPSHOT_INITIAL();

		wm.Stop();										EVENTS(Xo, Xf, Xa);
		wm.UnregisterWindow(&mainWindow);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Detect Illegal Operations")
	{
		Window mainWindow(L'X');
		TEST_ERROR(mainWindow.SetBounds(Bounds(0, 0, 8, 7)));
		mainWindow.visible = true;
		TEST_ERROR(wm.RegisterWindow(&mainWindow));
		mainWindow.visible = false;
		wm.RegisterWindow(&mainWindow);
		TEST_ERROR(wm.RegisterWindow(&mainWindow));
		{
			Window anotherMainWindow(L'X');
			TEST_ERROR(wm.RegisterWindow(&anotherMainWindow));
		}

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		windowB.SetParent(&windowA);
		TEST_ERROR(windowA.SetParent(&windowB));
		TEST_ASSERT(windowA.parent == nullptr);
		TEST_ASSERT(windowB.parent == &windowA);

		TEST_ERROR(wm.Start(&windowB));
		wm.Start(&mainWindow);
		TEST_ERROR(mainWindow.SetParent(&windowA));
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);

		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		TAKE_SNAPSHOT_INITIAL();

		TEST_ERROR(wm.UnregisterWindow(&mainWindow));
		wm.Stop();										EVENTS(Xo, Xa, Ao, Aa, Bo, Bf, Ba);
		TEST_ERROR(wm.Stop());
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		{
			Window unregisteredWindow(L'Y');
			TEST_ERROR(wm.UnregisterWindow(&unregisteredWindow));
		}
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Activing windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Bf, Ba);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Af, Aa);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Xf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Bf, Ba);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Deactivating windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		TAKE_SNAPSHOT_INITIAL();

		DONT_TAKE_SNAPSHOT(windowA.Deactivate());		EVENTS(*);
		DONT_TAKE_SNAPSHOT(mainWindow.Deactivate());	EVENTS(*);
		TAKE_SNAPSHOT(windowB.Deactivate());			EVENTS(XF, Bf, Ba);
		TAKE_SNAPSHOT(mainWindow.Deactivate());			EVENTS(Xf, Xa);

		wm.Stop();										EVENTS(Xo, Ao, Bo);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Activing many windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		Window windowI(L'I');
		wm.RegisterWindow(&windowI);

		Window windowJ(L'J');
		wm.RegisterWindow(&windowJ);

		Window windowK(L'K');
		wm.RegisterWindow(&windowK);

		mainWindow.SetBounds(Bounds(0, 0, 12, 10));
		windowA.SetBounds(Bounds(1, 3, 6, 4));
		windowB.SetBounds(Bounds(2, 4, 6, 4));
		windowC.SetBounds(Bounds(3, 5, 6, 4));
		windowI.SetBounds(Bounds(3, 1, 6, 4));
		windowJ.SetBounds(Bounds(4, 2, 6, 4));
		windowK.SetBounds(Bounds(5, 3, 6, 4));

		windowB.SetParent(&windowA);
		windowC.SetParent(&windowB);
		windowI.SetParent(&mainWindow);
		windowJ.SetParent(&windowI);
		windowK.SetParent(&windowI);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);
		TEST_ASSERT(windowC.parent == &windowB);
		TEST_ASSERT(windowI.parent == &mainWindow);
		TEST_ASSERT(windowJ.parent == &windowI);
		TEST_ASSERT(windowK.parent == &windowI);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		windowI.Show();									EVENTS(IO, IF, IA, Aa, Ba, Cf, Ca);
		windowJ.Show();									EVENTS(JO, JF, JA, If);
		windowK.Show();									EVENTS(KO, KF, KA, Jf, Ja);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ia, Kf, Ka);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Aa, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowI.Activate());				EVENTS(IF, IA, Xf);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, If);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, Jf, Ja);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Ia, Kf, Ka);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo, Co, Io, Jo, Ko);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.UnregisterWindow(&windowI);
		wm.UnregisterWindow(&windowJ);
		wm.UnregisterWindow(&windowK);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Deactivating many windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		Window windowI(L'I');
		wm.RegisterWindow(&windowI);

		Window windowJ(L'J');
		wm.RegisterWindow(&windowJ);

		Window windowK(L'K');
		wm.RegisterWindow(&windowK);

		mainWindow.SetBounds(Bounds(0, 0, 12, 10));
		windowA.SetBounds(Bounds(1, 3, 6, 4));
		windowB.SetBounds(Bounds(2, 4, 6, 4));
		windowC.SetBounds(Bounds(3, 5, 6, 4));
		windowI.SetBounds(Bounds(3, 1, 6, 4));
		windowJ.SetBounds(Bounds(4, 2, 6, 4));
		windowK.SetBounds(Bounds(5, 3, 6, 4));

		windowB.SetParent(&windowA);
		windowC.SetParent(&windowB);
		windowI.SetParent(&mainWindow);
		windowJ.SetParent(&windowI);
		windowK.SetParent(&windowI);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);
		TEST_ASSERT(windowC.parent == &windowB);
		TEST_ASSERT(windowI.parent == &mainWindow);
		TEST_ASSERT(windowJ.parent == &windowI);
		TEST_ASSERT(windowK.parent == &windowI);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		windowI.Show();									EVENTS(IO, IF, IA, Aa, Ba, Cf, Ca);
		windowJ.Show();									EVENTS(JO, JF, JA, If);
		windowK.Show();									EVENTS(KO, KF, KA, Jf, Ja);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowK.Deactivate());			EVENTS(Kf, Ka, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, Xf, IA);
		TAKE_SNAPSHOT(windowJ.Deactivate());			EVENTS(Jf, Ja, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Xf, AA, BA);
		TAKE_SNAPSHOT(windowC.Deactivate());			EVENTS(Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.Deactivate());			EVENTS(Bf, Ba, AF);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa, XF);
		TAKE_SNAPSHOT(mainWindow.Deactivate());			EVENTS(Xf, Xa);

		wm.Stop();										EVENTS(Xo, Ao, Bo, Co, Io, Jo, Ko);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.UnregisterWindow(&windowI);
		wm.UnregisterWindow(&windowJ);
		wm.UnregisterWindow(&windowK);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Disabling windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetEnabled(false));		EVENTS(Ae);
		TAKE_SNAPSHOT(windowB.SetEnabled(false));		EVENTS(Be, Bf, Ba, XF);
		TAKE_SNAPSHOT(mainWindow.SetEnabled(false));	EVENTS(Xe, Xf, Xa);

		TAKE_SNAPSHOT(windowA.SetEnabled(true));		EVENTS(AE);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa);

		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA);
		TAKE_SNAPSHOT(mainWindow.SetEnabled(true));		EVENTS(XE, XA);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		DONT_TAKE_SNAPSHOT(windowA.Activate());			EVENTS(*);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa, XF);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Hiding windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		mainWindow.SetBounds(Bounds(0, 0, 8, 7));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));
		windowC.SetBounds(Bounds(3, 3, 4, 3));

		windowC.SetParent(&windowB);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		TEST_ASSERT(windowC.parent == &windowB);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowC.SetVisible(false));		EVENTS(Co, Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.SetVisible(false));		EVENTS(Bo, Bf, Ba, XF);
		TAKE_SNAPSHOT(windowA.SetVisible(false));		EVENTS(Ao);
		TAKE_SNAPSHOT(windowC.SetVisible(true));		EVENTS(CO);
		TAKE_SNAPSHOT(windowB.SetVisible(true));		EVENTS(BO);
		TAKE_SNAPSHOT(windowA.SetVisible(true));		EVENTS(AO);

		TAKE_SNAPSHOT(windowA.SetVisible(false));		EVENTS(Ao);
		TAKE_SNAPSHOT(windowA.SetVisible(true));		EVENTS(AO);
		TAKE_SNAPSHOT(windowC.SetVisible(false));		EVENTS(Co);
		TAKE_SNAPSHOT(windowC.SetVisible(true));		EVENTS(CO);
		TAKE_SNAPSHOT(windowB.SetVisible(false));		EVENTS(Bo);
		TAKE_SNAPSHOT(windowB.SetVisible(true));		EVENTS(BO);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo, Co);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Closing windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		mainWindow.SetBounds(Bounds(0, 0, 8, 7));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));
		windowC.SetBounds(Bounds(3, 3, 4, 3));

		windowB.SetParent(&windowA);
		windowC.SetParent(&windowA);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);
		TEST_ASSERT(windowC.parent == &windowA);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		windowC.Show();									EVENTS(CO, CF, CA, Bf, Ba);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(wm.UnregisterWindow(&windowA));	EVENTS(Ao, Aa);
		TEST_ASSERT(!windowA.visible);
		TEST_ASSERT(!windowA.active);
		TEST_ASSERT(!windowA.renderedAsActive);
		TEST_ASSERT(windowA.parent == nullptr);
		TEST_ASSERT(windowA.children.Count() == 0);
		TEST_ASSERT(!wm.registeredWindows.Keys().Contains(windowA.id));

		wm.Stop();										EVENTS(Xo, Xa, Bo, Co, Cf, Ca);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Changing topmost of windows")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		mainWindow.SetBounds(Bounds(0, 0, 8, 7));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));
		windowC.SetBounds(Bounds(3, 3, 4, 3));

		windowC.SetParent(&windowB);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		TEST_ASSERT(windowC.parent == &windowB);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af, Aa);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);

		TAKE_SNAPSHOT(windowB.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af, Aa);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);

		DONT_TAKE_SNAPSHOT(windowC.SetTopMost(true));	EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af, Aa);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);

		TAKE_SNAPSHOT(windowA.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af, Aa);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);

		TAKE_SNAPSHOT(windowB.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af, Aa);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);

		TAKE_SNAPSHOT(windowC.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af, Aa);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);

		wm.Stop();										EVENTS(Xo, Xa, Ao, Bo, Ba, Co, Cf, Ca);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Activing windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Bf, Ba);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Af, Aa);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Xf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Bf, Ba);
		TAKE_SNAPSHOT(windowA.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Xf);
		TAKE_SNAPSHOT(windowB.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowB.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Af, Aa);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Xf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Af, Aa);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Deactivating windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		DONT_TAKE_SNAPSHOT(windowA.Deactivate());		EVENTS(*);
		DONT_TAKE_SNAPSHOT(mainWindow.Deactivate());	EVENTS(*);
		TAKE_SNAPSHOT(windowB.Deactivate());			EVENTS(Bf, Ba, XF);
		TAKE_SNAPSHOT(mainWindow.Deactivate());			EVENTS(Xf, Xa);

		wm.Stop();										EVENTS(Xo, Ao, Bo);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Activing many windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		Window windowI(L'I');
		wm.RegisterWindow(&windowI);

		Window windowJ(L'J');
		wm.RegisterWindow(&windowJ);

		Window windowK(L'K');
		wm.RegisterWindow(&windowK);

		mainWindow.SetBounds(Bounds(0, 0, 12, 10));
		windowA.SetBounds(Bounds(1, 3, 6, 4));
		windowB.SetBounds(Bounds(2, 4, 6, 4));
		windowC.SetBounds(Bounds(3, 5, 6, 4));
		windowI.SetBounds(Bounds(3, 1, 6, 4));
		windowJ.SetBounds(Bounds(4, 2, 6, 4));
		windowK.SetBounds(Bounds(5, 3, 6, 4));

		windowB.SetParent(&windowA);
		windowC.SetParent(&windowB);
		windowI.SetParent(&mainWindow);
		windowJ.SetParent(&windowI);
		windowK.SetParent(&windowI);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);
		TEST_ASSERT(windowC.parent == &windowB);
		TEST_ASSERT(windowI.parent == &mainWindow);
		TEST_ASSERT(windowJ.parent == &windowI);
		TEST_ASSERT(windowK.parent == &windowI);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		windowI.Show();									EVENTS(IO, IF, IA, Aa, Ba, Cf, Ca);
		windowJ.Show();									EVENTS(JO, JF, JA, If);
		windowK.Show();									EVENTS(KO, KF, KA, Jf, Ja);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowI.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Ia, Kf, Ka);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Aa, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowI.Activate());				EVENTS(IF, IA, Xf);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, If);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, Jf, Ja);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Ia, Kf, Ka);

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Xf);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Aa, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowI.Activate());				EVENTS(IF, IA, Xf);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, If);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, Jf, Ja);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Ia, Kf, Ka);

		TAKE_SNAPSHOT(windowI.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Xf);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Aa, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowI.Activate());				EVENTS(IF, IA, Xf);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, If);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, Jf, Ja);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Ia, Kf, Ka);

		DONT_TAKE_SNAPSHOT(windowC.SetTopMost(true));	EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Xf);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Aa, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowI.Activate());				EVENTS(IF, IA, Xf);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, If);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, Jf, Ja);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Ia, Kf, Ka);

		TAKE_SNAPSHOT(windowA.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA, Xf);
		TAKE_SNAPSHOT(windowB.Activate());				EVENTS(BF, BA, Af);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Bf);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Aa, Ba, Cf, Ca);
		TAKE_SNAPSHOT(windowI.Activate());				EVENTS(IF, IA, Xf);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, If);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, Jf, Ja);
		TAKE_SNAPSHOT(mainWindow.Activate());			EVENTS(XF, Ia, Kf, Ka);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo, Co, Io, Jo, Ko);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.UnregisterWindow(&windowI);
		wm.UnregisterWindow(&windowJ);
		wm.UnregisterWindow(&windowK);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Deactivating many windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		Window windowI(L'I');
		wm.RegisterWindow(&windowI);

		Window windowJ(L'J');
		wm.RegisterWindow(&windowJ);

		Window windowK(L'K');
		wm.RegisterWindow(&windowK);

		mainWindow.SetBounds(Bounds(0, 0, 12, 10));
		windowA.SetBounds(Bounds(1, 3, 6, 4));
		windowB.SetBounds(Bounds(2, 4, 6, 4));
		windowC.SetBounds(Bounds(3, 5, 6, 4));
		windowI.SetBounds(Bounds(3, 1, 6, 4));
		windowJ.SetBounds(Bounds(4, 2, 6, 4));
		windowK.SetBounds(Bounds(5, 3, 6, 4));

		windowB.SetParent(&windowA);
		windowC.SetParent(&windowB);
		windowI.SetParent(&mainWindow);
		windowJ.SetParent(&windowI);
		windowK.SetParent(&windowI);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);
		TEST_ASSERT(windowC.parent == &windowB);
		TEST_ASSERT(windowI.parent == &mainWindow);
		TEST_ASSERT(windowJ.parent == &windowI);
		TEST_ASSERT(windowK.parent == &windowI);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		windowI.Show();									EVENTS(IO, IF, IA, Aa, Ba, Cf, Ca);
		windowJ.Show();									EVENTS(JO, JF, JA, If);
		windowK.Show();									EVENTS(KO, KF, KA, Jf, Ja);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowK.Deactivate());			EVENTS(Kf, Ka, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, Xf, IA);
		TAKE_SNAPSHOT(windowJ.Deactivate());			EVENTS(Jf, Ja, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Xf, AA, BA);
		TAKE_SNAPSHOT(windowC.Deactivate());			EVENTS(Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.Deactivate());			EVENTS(Bf, Ba, AF);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa, XF);
		TAKE_SNAPSHOT(mainWindow.Deactivate());			EVENTS(Xf, Xa);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, XA, IA);

		TAKE_SNAPSHOT(windowI.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowK.Deactivate());			EVENTS(Kf, Ka, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, Xf, IA);
		TAKE_SNAPSHOT(windowJ.Deactivate());			EVENTS(Jf, Ja, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Xf, AA, BA);
		TAKE_SNAPSHOT(windowC.Deactivate());			EVENTS(Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.Deactivate());			EVENTS(Bf, Ba, AF);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa, XF);
		TAKE_SNAPSHOT(mainWindow.Deactivate());			EVENTS(Xf, Xa);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, XA, IA);

		DONT_TAKE_SNAPSHOT(windowC.SetTopMost(true));	EVENTS(*);
		DONT_TAKE_SNAPSHOT(windowJ.SetTopMost(true));	EVENTS(*);
		TAKE_SNAPSHOT(windowA.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowI.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowK.Deactivate());			EVENTS(Kf, Ka, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowJ.Activate());				EVENTS(JF, JA, Xf, IA);
		TAKE_SNAPSHOT(windowJ.Deactivate());			EVENTS(Jf, Ja, IF);
		TAKE_SNAPSHOT(windowI.Deactivate());			EVENTS(If, Ia, XF);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Xf, AA, BA);
		TAKE_SNAPSHOT(windowC.Deactivate());			EVENTS(Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.Deactivate());			EVENTS(Bf, Ba, AF);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa, XF);
		TAKE_SNAPSHOT(mainWindow.Deactivate());			EVENTS(Xf, Xa);
		TAKE_SNAPSHOT(windowK.Activate());				EVENTS(KF, KA, XA, IA);

		wm.Stop();										EVENTS(Xo, Xa, Ao, Bo, Co, Io, Ia, Jo, Ko, Kf, Ka);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.UnregisterWindow(&windowI);
		wm.UnregisterWindow(&windowJ);
		wm.UnregisterWindow(&windowK);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Disabling windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		mainWindow.SetBounds(Bounds(0, 0, 7, 6));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.SetEnabled(false));		EVENTS(Ae);
		TAKE_SNAPSHOT(windowB.SetEnabled(false));		EVENTS(Be, Bf, Ba, XF);
		TAKE_SNAPSHOT(mainWindow.SetEnabled(false));	EVENTS(Xe, Xf, Xa);

		TAKE_SNAPSHOT(windowA.SetEnabled(true));		EVENTS(AE);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa);

		TAKE_SNAPSHOT(windowA.SetTopMost(false));		EVENTS(*);
		TAKE_SNAPSHOT(windowB.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.Activate());				EVENTS(AF, AA);
		TAKE_SNAPSHOT(mainWindow.SetEnabled(true));		EVENTS(XE, XA);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		DONT_TAKE_SNAPSHOT(windowA.Activate());			EVENTS(*);
		DONT_TAKE_SNAPSHOT(windowB.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowA.Deactivate());			EVENTS(Af, Aa, XF);

		wm.Stop();										EVENTS(Xo, Xf, Xa, Ao, Bo);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Hiding windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		mainWindow.SetBounds(Bounds(0, 0, 8, 7));
		windowA.SetBounds(Bounds(1, 1, 4, 3));
		windowB.SetBounds(Bounds(2, 2, 4, 3));
		windowC.SetBounds(Bounds(3, 3, 4, 3));

		windowC.SetParent(&windowB);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &mainWindow);
		TEST_ASSERT(windowC.parent == &windowB);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af, Aa);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowC.SetVisible(false));		EVENTS(Co, Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.SetVisible(false));		EVENTS(Bo, Bf, Ba, XF);
		TAKE_SNAPSHOT(windowA.SetVisible(false));		EVENTS(Ao);
		TAKE_SNAPSHOT(windowC.SetVisible(true));		EVENTS(CO);
		TAKE_SNAPSHOT(windowB.SetVisible(true));		EVENTS(BO);
		TAKE_SNAPSHOT(windowA.SetVisible(true));		EVENTS(AO);

		TAKE_SNAPSHOT(windowB.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowA.SetVisible(false));		EVENTS(Ao);
		TAKE_SNAPSHOT(windowA.SetVisible(true));		EVENTS(AO);
		TAKE_SNAPSHOT(windowC.SetVisible(false));		EVENTS(Co);
		TAKE_SNAPSHOT(windowC.SetVisible(true));		EVENTS(CO);
		TAKE_SNAPSHOT(windowB.SetVisible(false));		EVENTS(Bo);
		TAKE_SNAPSHOT(windowB.SetVisible(true));		EVENTS(BO);

		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(CF, CA, Xf, BA);
		TAKE_SNAPSHOT(windowB.SetVisible(false));		EVENTS(Bo, Ba);
		DONT_TAKE_SNAPSHOT(windowC.Activate());			EVENTS(*);
		TAKE_SNAPSHOT(windowB.SetVisible(true));		EVENTS(BO, BA);
		TAKE_SNAPSHOT(windowC.Activate());				EVENTS(*);
		TAKE_SNAPSHOT(windowC.Deactivate());			EVENTS(Cf, Ca, BF);
		TAKE_SNAPSHOT(windowB.BringToFront());			EVENTS(*);

		wm.Stop();										EVENTS(Xo, Xa, Ao, Bo, Bf, Ba, Co);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowA);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.EnsureCleanedUp();
	}});

	WM_TEST_CASE(L"Closing windows with topmost")
	{
		Window mainWindow(L'X');
		wm.RegisterWindow(&mainWindow);

		Window windowA(L'A');
		wm.RegisterWindow(&windowA);

		Window windowB(L'B');
		wm.RegisterWindow(&windowB);

		Window windowC(L'C');
		wm.RegisterWindow(&windowC);

		Window windowI(L'I');
		wm.RegisterWindow(&windowI);

		Window windowJ(L'J');
		wm.RegisterWindow(&windowJ);

		Window windowK(L'K');
		wm.RegisterWindow(&windowK);

		mainWindow.SetBounds(Bounds(0, 0, 12, 10));
		windowA.SetBounds(Bounds(1, 3, 6, 4));
		windowB.SetBounds(Bounds(2, 4, 6, 4));
		windowC.SetBounds(Bounds(3, 5, 6, 4));
		windowI.SetBounds(Bounds(3, 1, 6, 4));
		windowJ.SetBounds(Bounds(4, 2, 6, 4));
		windowK.SetBounds(Bounds(5, 3, 6, 4));

		windowB.SetParent(&windowA);
		windowC.SetParent(&windowB);
		windowI.SetParent(&mainWindow);
		windowJ.SetParent(&windowI);
		windowK.SetParent(&windowI);

		wm.Start(&mainWindow);
		TEST_ASSERT(windowA.parent == &mainWindow);
		TEST_ASSERT(windowB.parent == &windowA);
		TEST_ASSERT(windowC.parent == &windowB);
		TEST_ASSERT(windowI.parent == &mainWindow);
		TEST_ASSERT(windowJ.parent == &windowI);
		TEST_ASSERT(windowK.parent == &windowI);
		mainWindow.Show();								EVENTS(XO, XF, XA);
		windowA.Show();									EVENTS(AO, AF, AA, Xf);
		windowB.Show();									EVENTS(BO, BF, BA, Af);
		windowC.Show();									EVENTS(CO, CF, CA, Bf);
		windowI.Show();									EVENTS(IO, IF, IA, Aa, Ba, Cf, Ca);
		windowJ.Show();									EVENTS(JO, JF, JA, If);
		windowK.Show();									EVENTS(KO, KF, KA, Jf, Ja);
		TAKE_SNAPSHOT_INITIAL();

		TAKE_SNAPSHOT(windowA.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(windowI.SetTopMost(true));		EVENTS(*);
		TAKE_SNAPSHOT(wm.UnregisterWindow(&windowA));	EVENTS(Ao);
		TEST_ASSERT(!windowA.visible);
		TEST_ASSERT(!windowA.active);
		TEST_ASSERT(!windowA.renderedAsActive);
		TEST_ASSERT(windowA.parent == nullptr);
		TEST_ASSERT(windowA.children.Count() == 0);
		TEST_ASSERT(!wm.registeredWindows.Keys().Contains(windowA.id));

		wm.Stop();										EVENTS(Xo, Xa, Bo, Co, Io, Ia, Jo, Ko, Kf, Ka);
		wm.UnregisterWindow(&mainWindow);
		wm.UnregisterWindow(&windowB);
		wm.UnregisterWindow(&windowC);
		wm.UnregisterWindow(&windowI);
		wm.UnregisterWindow(&windowJ);
		wm.UnregisterWindow(&windowK);
		wm.EnsureCleanedUp();
	}});
}