#include "../Generated_TuiSkin/TuiSkinConfig.h"
#include "TuiControlTest.h"

using namespace vl;
using namespace vl::presentation;

void GuiMain()
{
	tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());
	theme::RegisterTheme(Ptr(new tuiskin::TuiTheme));
	tuidemo::TuiMainWindow window;
	window.ForceCalculateSizeImmediately();
	controls::GetApplication()->Run(&window);
}

int wmain()
{
	auto result = SetupTuiWindowsRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
