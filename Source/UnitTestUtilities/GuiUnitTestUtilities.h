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
	using UnitTestLinkFunc = vl::Func<void(UnitTestRemoteProtocol*, IUnitTestContext*, const UnitTestMainFunc&)>;
}

extern void										GacUIUnitTest_Initialize(const vl::presentation::unittest::UnitTestFrameworkConfig* config);
extern void										GacUIUnitTest_Finalize();
extern void										GacUIUnitTest_SetGuiMainProxy(const vl::presentation::unittest::UnitTestMainFunc& proxy);
extern void										GacUIUnitTest_LinkGuiMainProxy(const vl::presentation::unittest::UnitTestLinkFunc& proxy);
extern void										GacUIUnitTest_Start(const vl::WString& appName, vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config = {});
extern void										GacUIUnitTest_Start_WithResourceAsText(const vl::WString& appName, vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config, const vl::WString& resourceText);
extern vl::Ptr<vl::presentation::GuiResource>	GacUIUnitTest_CompileAndLoad(const vl::WString& xmlResource);

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

template<typename TTheme>
void GacUIUnitTest_StartFast_WithResourceAsText(
	const vl::WString& appName,
	const vl::WString& windowTypeFullName,
	const vl::WString& resourceText, vl::Func<void(vl::presentation::controls::GuiWindow*)> installWindow,
	vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config
)
{
	GacUIUnitTest_LinkGuiMainProxy([=](
		vl::presentation::unittest::UnitTestRemoteProtocol* protocol,
		vl::presentation::unittest::IUnitTestContext* context,
		const vl::presentation::unittest::UnitTestMainFunc& previousMainProxy
		)
	{
		protocol->GetEvents()->OnControllerConnect();
		auto theme = vl::Ptr(new TTheme);
		vl::presentation::theme::RegisterTheme(theme);
		{
			auto windowValue = vl::reflection::description::Value::Create(windowTypeFullName);
			TEST_ASSERT(windowValue.GetRawPtr());

			auto window = vl::Ptr(windowValue.GetRawPtr()->SafeAggregationCast<vl::presentation::controls::GuiWindow>());
			TEST_ASSERT(window);

			if (installWindow)
			{
				installWindow(window.Obj());
			}
			window->MoveToScreenCenter();
			previousMainProxy(protocol, context);
			vl::presentation::controls::GetApplication()->Run(window.Obj());
		}
		vl::presentation::theme::UnregisterTheme(theme->Name);
	});
	GacUIUnitTest_Start_WithResourceAsText(appName, config, resourceText);
}

template<typename TTheme>
void GacUIUnitTest_StartFast_WithResourceAsText(
	const vl::WString& appName,
	const vl::WString& windowTypeFullName,
	const vl::WString& resourceText,
	vl::Nullable<vl::presentation::unittest::UnitTestScreenConfig> config
)
{
	GacUIUnitTest_StartFast_WithResourceAsText<TTheme>(appName, windowTypeFullName, resourceText, {}, config);
}

template<typename TTheme>
void GacUIUnitTest_StartFast_WithResourceAsText(
	const vl::WString& appName,
	const vl::WString& windowTypeFullName,
	const vl::WString& resourceText,
	vl::Func<void(vl::presentation::controls::GuiWindow*)> installWindow
)
{
	GacUIUnitTest_StartFast_WithResourceAsText<TTheme>(appName, windowTypeFullName, resourceText, installWindow, {});
}

template<typename TTheme>
void GacUIUnitTest_StartFast_WithResourceAsText(
	const vl::WString& appName,
	const vl::WString& windowTypeFullName,
	const vl::WString& resourceText
)
{
	GacUIUnitTest_StartFast_WithResourceAsText<TTheme>(appName, windowTypeFullName, resourceText, {}, {});
}

#endif

#endif