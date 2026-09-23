#include "GacUI.h"
#include "TuiSkin.h"
#include "../GitView/Source/GitView.h"
#include "../GitView/ViewModel/GitViewModel.h"

#if defined VCZH_GCC && !defined VCZH_APPLE
#include "TUI/TuiWGacController.h"
#elif defined VCZH_GCC && defined VCZH_APPLE
#include "TUI/TuiCocoaController.h"
#endif

using namespace vl;
using namespace vl::presentation;

void GuiMain()
{
	theme::RegisterTheme(Ptr(new tuiskin::TuiTheme));
	auto model = Ptr(new gitview::GitViewModel(filesystem::FilePath(WString::Unmanaged(L"."))));
	gitview::MainWindow window(model);
	window.ForceCalculateSizeImmediately();
	controls::GetApplication()->Run(&window);
}

#ifdef VCZH_MSVC
int wmain()
#else
int main()
#endif
{
	vint result = 1;
	try
	{
#ifdef VCZH_MSVC
		result = SetupTuiWindowsRenderer();
#elif !defined VCZH_APPLE
		result = wayland::SetupTuiWaylandRenderer();
#else
		result = osx::SetupTuiCocoaRenderer();
#endif
	}
	catch (const Exception& error)
	{
		console::Console::WriteLine(error.Message());
	}
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return static_cast<int>(result);
}
