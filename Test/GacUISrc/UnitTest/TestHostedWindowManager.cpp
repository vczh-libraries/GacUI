#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Hosted/GuiHostedWindowManager.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;

namespace hosted_window_manager_tests
{
	extern WString GetTestBaselinePath();
	extern WString GetTestOutputPath();
}
using namespace hosted_window_manager_tests;

#define UPDATE_SNAPSHOT

using Window = hosted_window_manager::Window<wchar_t>;

struct WindowManager : hosted_window_manager::WindowManager<wchar_t>
{
	WString						unitTestTitle;
	List<Pair<vint, WString>>	snapshots;

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

#pragma warning(push)
#pragma warning(disable: 4297)
	~WindowManager()
	{
		TEST_ASSERT(!mainWindow);
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
			WString baseline = GenerateToStream([&](StreamWriter& writer)
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
#pragma warning(pop)

	void CheckWindowStatus()
	{
		for (auto window : registeredWindows.Values())
		{
			TEST_ASSERT(window->active == (window == activeWindow));
			TEST_ASSERT(!window->active || window->enabled);
			TEST_ASSERT(!window->active || window->renderedAsActive);

			bool topMost = window->IsEventuallyTopMost();
			if (window->visible && topMost)
			{
				TEST_ASSERT(topMostedWindowsInOrder.Contains(window));
				auto current = window->parent;
				while (current && !current->visible)
				{
					current = current->parent;
				}
				if (current && current->IsEventuallyTopMost())
				{
					TEST_ASSERT(topMostedWindowsInOrder.IndexOf(window) < topMostedWindowsInOrder.IndexOf(current));
				}
			}
			else
			{
				TEST_ASSERT(!topMostedWindowsInOrder.Contains(window));
			}

			if (window->visible && !topMost)
			{
				TEST_ASSERT(ordinaryWindowsInOrder.Contains(window));
				auto current = window->parent;
				while (current && !current->visible)
				{
					current = current->parent;
				}
				if (current)
				{
					TEST_ASSERT(!current->IsEventuallyTopMost());
					TEST_ASSERT(ordinaryWindowsInOrder.IndexOf(window) < ordinaryWindowsInOrder.IndexOf(current));
				}
			}
			else
			{
				TEST_ASSERT(!ordinaryWindowsInOrder.Contains(window));
			}
		}
	}

	void TakeSnapshot()
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
		snapshots.Add({ h,snapshot });
	}
};

#define WM_TEST_CASE(NAME)				\
	TEST_CASE(NAME)						\
	{									\
		WindowManager wm(NAME);			\

#define TAKE_SNAPSHOT					\
	do{									\
		TEST_ASSERT(wm.needRefresh);	\
		wm.needRefresh = false;			\
		wm.TakeSnapshot();				\
	}while(false)						\

NativeRect Bounds(vint x, vint y, vint w, vint h)
{
	return { { {x},{y} },{ {w},{h} } };
}

TEST_FILE
{
	WM_TEST_CASE(L"Start and stop")
	{
		Window mainWindow(L'A', true);
		wm.RegisterWindow(&mainWindow);
		mainWindow.SetBounds(Bounds(0, 0, 6, 4));

		wm.Start(&mainWindow);
		mainWindow.Show();

		TAKE_SNAPSHOT;
		wm.TakeSnapshot();
		wm.Stop();
		wm.UnregisterWindow(&mainWindow);
	}});
}