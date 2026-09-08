#ifndef VCZH_PRESENTATION_TUISKINCONFIG
#define VCZH_PRESENTATION_TUISKINCONFIG

#include "../../../GacUI.h"

#ifdef VCZH_64
#include "TuiSkin.h"
#else
#include "TuiSkin.h"
#endif

namespace tuiskin
{
	/// <summary>Create the opaque terminal palette used by TuiSkin.</summary>
	extern ColorPackage CreateDefaultColorPackage();
	/// <summary>Install colors before constructing the theme and its controls.</summary>
	extern void SetColorPackage(const ColorPackage& colors);
}

#endif
