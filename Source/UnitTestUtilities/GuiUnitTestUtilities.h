/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTUTILITIES
#define VCZH_PRESENTATION_GUIUNITTESTUTILITIES

#include "../GacUI.h"
#include "../PlatformProviders/Remote/GuiRemoteController.h"

namespace vl::presentation::unittest
{
}

extern void InitializeGacUIUnitTest();
extern void FinalizeGacUIUnitTest();
extern void SetGuiMainProxy(const vl::Func<void()>& proxy);

#endif