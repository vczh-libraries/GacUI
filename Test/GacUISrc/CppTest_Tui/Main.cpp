#include <Vlpp.h>

#if defined VCZH_MSVC
#include "../Generated_TuiSkin/TuiSkinConfig.h"
#else
#include "Skins/TuiSkin/TuiSkin.h"
#endif
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

#if defined VCZH_MSVC
int wmain()
{
	auto result = SetupTuiWindowsRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
#endif
