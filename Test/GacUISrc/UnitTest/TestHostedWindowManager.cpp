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
	const wchar_t*				unitTestTitle = nullptr;
	List<Pair<vint, WString>>	snapshots;

	WindowManager(const wchar_t* _unitTestTitle)
		:unitTestTitle(_unitTestTitle)
	{
	}

#pragma warning(push)
#pragma warning(disable: 4297)
	~WindowManager()
	{
		TEST_ASSERT(!mainWindow);
		auto snapshotPath = FilePath(GetTestBaselinePath()) / (WString::Unmanaged(unitTestTitle) + L".txt");
#ifdef UPDATE_SNAPSHOT
		FileStream fileStream(snapshotPath.GetFullPath(), FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf8);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		for (auto [lines, snapshot] : snapshots)
		{
			writer.WriteLine(snapshot);
			writer.WriteLine(L"");
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
				writer.WriteChar(L']');
			}
		});
		snapshots.Add({ h,snapshot });
	}
};

#define WM_TEST_CASE(NAME) { WindowManager wm(NAME); TEST_CASE(NAME)

NativeRect Bounds(vint x, vint y, vint w, vint h)
{
	return { { {x},{y} },{ {w},{h} } };
}

TEST_FILE
{
	WM_TEST_CASE(L"Start and stop")
	{
		Window mainWindow(L'A', true);
		mainWindow.SetBounds(Bounds(0, 0, 6, 4));

		wm.Start(&mainWindow);
		mainWindow.Show();

		wm.TakeSnapshot();
		wm.Stop();
	});}
}