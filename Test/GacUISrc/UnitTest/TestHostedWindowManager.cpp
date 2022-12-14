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
	}
#pragma warning(pop)

	void TakeSnapshot()
	{
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
		mainWindow.SetVisible(true);
		mainWindow.Activate();

		wm.TakeSnapshot();
		wm.Stop();
	});}
}