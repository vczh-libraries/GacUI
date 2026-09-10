#include "FullControlTestPalette.h"
#include "../../../Source/Skins/DarkSkin/Config/DarkSkinConfig.h"

using namespace vl;
using namespace vl::presentation::controls;

namespace demo
{
	void OnPaletteSelected(vint preset)
	{
		GetApplication()->InvokeInMainThread(GetApplication()->GetMainWindow(), [preset]()
		{
#define ERROR_MESSAGE_PREFIX L"demo::OnPaletteSelected()#"
			darkskin::ColorPackage colors;
			switch (preset)
			{
			case 0: colors = darkskin::CreateDefaultColorPackage(); break;
			case 1: colors = darkskin::CreateAuroraColorPackage(); break;
			case 2: colors = darkskin::CreateEmberColorPackage(); break;
			case 3: colors = darkskin::CreateMoonstoneColorPackage(); break;
			case 4: colors = darkskin::CreateLagoonColorPackage(); break;
			case 5: colors = darkskin::CreateRosewoodColorPackage(); break;
			default: CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unknown palette.");
			}
			darkskin::SetColorPackage(colors);
			GetApplication()->RefreshThemes();
#undef ERROR_MESSAGE_PREFIX
		});
	}
}
