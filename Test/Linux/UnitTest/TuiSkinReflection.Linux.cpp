#include "../../../Import/Vlpp.h"

#if defined VCZH_GCC || defined VCZH_WASM
#ifdef VCZH_64
#include "../../GacUISrc/Generated_TuiSkin/Source_x64/TuiSkinReflection.cpp"
#else
#include "../../GacUISrc/Generated_TuiSkin/Source_x86/TuiSkinReflection.cpp"
#endif
#endif
