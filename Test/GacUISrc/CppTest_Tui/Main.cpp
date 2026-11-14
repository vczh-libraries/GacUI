#include <Vlpp.h>

#if defined VCZH_MSVC
#include "../../../Source/Skins/TuiSkin/Config/TuiSkinConfig.h"
#elif defined VCZH_GCC || defined VCZH_WASM
#include "Skins/TuiSkin/TuiSkin.h"
#endif
#include "TuiControlTest.h"
#include "TuiControlTestEventHandlers.h"

using namespace vl;
using namespace vl::presentation;

void GuiMain()
{
	theme::RegisterTheme(Ptr(new tuiskin::TuiTheme));
	tuidemo::TuiMainWindow window;
	tuidemo::AttachTuiEvents<tuidemo::TuiDocumentEditorBase, tuidemo::TuiSkinItem>(&window);
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
