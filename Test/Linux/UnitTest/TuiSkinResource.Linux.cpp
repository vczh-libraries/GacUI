#include "../../../Import/Vlpp.h"

#if defined VCZH_GCC || defined VCZH_WASM
#ifdef VCZH_64
#include "../../GacUISrc/Generated_TuiSkin/Source_x64/TuiSkinResource.cpp"
#else
#include "../../GacUISrc/Generated_TuiSkin/Source_x86/TuiSkinResource.cpp"
#endif
#endif
