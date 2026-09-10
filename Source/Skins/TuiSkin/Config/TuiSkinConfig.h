#ifndef VCZH_PRESENTATION_TUISKINCONFIG
#define VCZH_PRESENTATION_TUISKINCONFIG

#include "../../../GacUI.h"

/* CodePack:BeginIgnore() */
#ifdef GACUI_SKIN_DEVELOPMENT
#ifdef VCZH_64
#include "../../../../Test/GacUISrc/Generated_TuiSkin/Source_x64/TuiSkin.h"
#else
#include "../../../../Test/GacUISrc/Generated_TuiSkin/Source_x86/TuiSkin.h"
#endif
#else
/* CodePack:EndIgnore() */
#include "../Source/TuiSkin.h"
/* CodePack:BeginIgnore() */
#endif
/* CodePack:EndIgnore() */

namespace tuiskin
{
	/// <summary>Create the opaque terminal palette used by TuiSkin.</summary>
	extern ColorPackage CreateDefaultColorPackage();
	extern ColorPackage CreatePinkColorPackage();
	extern ColorPackage CreateOrangeColorPackage();
	extern ColorPackage CreateGrassPackage();
	extern ColorPackage CreateEmeraldPackage();
	extern ColorPackage CreateSkyblueColorPackage();
	extern ColorPackage CreatePurplePackage();
	/// <summary>Install colors for new templates. Refresh existing themed controls with GuiApplication::RefreshThemes.</summary>
	extern void SetColorPackage(const ColorPackage& colors);
}

#endif
