/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTUTILITIES
#define VCZH_PRESENTATION_GUIUNITTESTUTILITIES

#include "GuiUnitTestProtocol.h"

namespace vl::presentation::unittest
{
	class IUnitTestContext : public virtual Interface
	{
	public:
	};

	using UnitTestMainFunc = vl::Func<void(UnitTestRemoteProtocol*, IUnitTestContext*)>;
}

extern void GacUIUnitTest_Initialize();
extern void GacUIUnitTest_Finalize();
extern void GacUIUnitTest_SetGuiMainProxy(const vl::presentation::unittest::UnitTestMainFunc& proxy);
extern void GacUIUnitTest_Start(vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config = {});

#endif