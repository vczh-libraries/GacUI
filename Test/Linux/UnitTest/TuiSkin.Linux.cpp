#include "../../../Import/Vlpp.h"

#if defined VCZH_GCC || defined VCZH_WASM
#ifdef VCZH_64
#include "../../GacUISrc/Generated_TuiSkin/Source_x64/TuiSkin.cpp"
#else
#include "../../GacUISrc/Generated_TuiSkin/Source_x86/TuiSkin.cpp"
#endif
#endif
