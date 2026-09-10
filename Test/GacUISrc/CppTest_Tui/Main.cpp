#include <Vlpp.h>

#if defined VCZH_MSVC
#include "../../../Source/Skins/TuiSkin/Config/TuiSkinConfig.h"
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
	window.PaletteSelected.Add([&window](vint preset)
	{
		controls::GetApplication()->InvokeInMainThread(&window, [preset]()
		{
			tuiskin::ColorPackage colors;
			switch (preset)
			{
			case 0: colors = tuiskin::CreatePinkColorPackage(); break;
			case 1: colors = tuiskin::CreateOrangeColorPackage(); break;
			case 2: colors = tuiskin::CreateGrassPackage(); break;
			case 3: colors = tuiskin::CreateEmeraldPackage(); break;
			case 4: colors = tuiskin::CreateSkyblueColorPackage(); break;
			case 5: colors = tuiskin::CreatePurplePackage(); break;
			default: return;
			}
			tuiskin::SetColorPackage(colors);
			controls::GetApplication()->RefreshThemes();
		});
	});
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
