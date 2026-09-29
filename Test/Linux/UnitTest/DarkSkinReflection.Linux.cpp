#include "../../../Import/Vlpp.h"

#if defined VCZH_GCC || defined VCZH_WASM
#ifdef VCZH_64
#include "../../GacUISrc/Generated_DarkSkin/Source_x64/DarkSkinReflection.cpp"
#else
#include "../../GacUISrc/Generated_DarkSkin/Source_x86/DarkSkinReflection.cpp"
#endif
#endif
