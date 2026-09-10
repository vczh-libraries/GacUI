#ifndef VCZH_PRESENTATION_DARKSKINCONFIG
#define VCZH_PRESENTATION_DARKSKINCONFIG

#include "../../../GacUI.h"

/* CodePack:BeginIgnore() */
#ifdef GACUI_SKIN_DEVELOPMENT
#ifdef VCZH_64
#include "../../../../Test/GacUISrc/Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../../../../Test/GacUISrc/Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif
#else
/* CodePack:EndIgnore() */
#include "../Source/DarkSkin.h"
/* CodePack:BeginIgnore() */
#endif
/* CodePack:EndIgnore() */

namespace darkskin
{
	extern ColorPackage CreateDefaultColorPackage();
	extern ColorPackage CreateAuroraColorPackage();
	extern ColorPackage CreateEmberColorPackage();
	extern ColorPackage CreateMoonstoneColorPackage();
	extern ColorPackage CreateLagoonColorPackage();
	extern ColorPackage CreateRosewoodColorPackage();
	extern void SetColorPackage(const ColorPackage& colors);
}

#endif
