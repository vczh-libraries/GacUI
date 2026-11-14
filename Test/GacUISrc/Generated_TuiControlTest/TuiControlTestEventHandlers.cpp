#include "TuiControlTestEventHandlers.h"
#ifdef GACUI_SKIN_DEVELOPMENT
#include "../../../Source/Skins/TuiSkin/Config/TuiSkinConfig.h"
#else
#include <Skins/TuiSkin/TuiSkin.h>
#endif

using namespace vl;

namespace tuidemo
{
	void InitializeTuiSkinItems(const Func<void(const WString&, const Func<void()>&, bool)>& addItem)
	{
		addItem(L"Pink", [] { tuiskin::SetColorPackage(tuiskin::CreatePinkColorPackage()); }, false);
		addItem(L"Orange", [] { tuiskin::SetColorPackage(tuiskin::CreateOrangeColorPackage()); }, false);
		addItem(L"Grass", [] { tuiskin::SetColorPackage(tuiskin::CreateGrassPackage()); }, false);
		addItem(L"Emerald", [] { tuiskin::SetColorPackage(tuiskin::CreateEmeraldPackage()); }, false);
		addItem(L"SkyBlue (default)", [] { tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage()); }, true);
		addItem(L"Purple", [] { tuiskin::SetColorPackage(tuiskin::CreatePurplePackage()); }, false);
	}
}
