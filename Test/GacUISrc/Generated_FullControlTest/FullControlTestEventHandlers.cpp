#include "FullControlTestEventHandlers.h"
#ifdef GACUI_SKIN_DEVELOPMENT
#include "../../../Source/Skins/DarkSkin/Config/DarkSkinConfig.h"
#else
#include <Skins/DarkSkin/DarkSkin.h>
#endif

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;

namespace demo
{
	void InitializeFctSkinItems(const Func<void(const WString&, const Func<void()>&, bool)>& addItem)
	{
		addItem(L"Default", [] { darkskin::SetColorPackage(darkskin::CreateDefaultColorPackage()); }, true);
		addItem(L"Aurora", [] { darkskin::SetColorPackage(darkskin::CreateAuroraColorPackage()); }, false);
		addItem(L"Ember", [] { darkskin::SetColorPackage(darkskin::CreateEmberColorPackage()); }, false);
		addItem(L"Moonstone", [] { darkskin::SetColorPackage(darkskin::CreateMoonstoneColorPackage()); }, false);
		addItem(L"Lagoon", [] { darkskin::SetColorPackage(darkskin::CreateLagoonColorPackage()); }, false);
		addItem(L"Rosewood", [] { darkskin::SetColorPackage(darkskin::CreateRosewoodColorPackage()); }, false);
	}
}
