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

extern void										GacUIUnitTest_Initialize(const vl::presentation::unittest::UnitTestFrameworkConfig* config);
extern void										GacUIUnitTest_Finalize();
extern void										GacUIUnitTest_SetGuiMainProxy(const vl::presentation::unittest::UnitTestMainFunc& proxy);
extern void										GacUIUnitTest_Start(const vl::WString& appName, vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config = {});
extern void										GacUIUnitTest_Start_WithResourceAsText(const vl::WString& appName, vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config, const vl::WString& resourceText);
extern vl::Ptr<vl::presentation::GuiResource>	GacUIUnitTest_CompileAndLoad(const vl::WString& xmlResource);

template<typename TTheme>
void GacUIUnitTest_StartFast_WithResourceAsText(const vl::WString& appName, const vl::WString& windowTypeFullName, vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config, const vl::WString& resourceText)
{
}

#endif