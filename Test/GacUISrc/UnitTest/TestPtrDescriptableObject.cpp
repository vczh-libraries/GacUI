#include "../../../Source/GacUI.h"
#include "../../../Source/Resources/GuiParserManager.h"
#include "../../../Source/Compiler/InstanceQuery/GuiInstanceQuery.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::templates;

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	SetGuiMainProxy([]()
	{
		TEST_CASE(L"Test Ptr<DescriptableObject>")
		{
			auto bounds = new GuiBoundsComposition;
			auto control = new GuiControl(theme::ThemeName::CustomControl);
			control->SetControlTemplate([](const description::Value&)
				{
					auto style = new GuiControlTemplate;
					style->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					return style;
				});
			bounds->AddChild(control->GetBoundsComposition());

			auto rc1 = ReferenceCounterOperator<GuiBoundsComposition>::CreateCounter(bounds);
			auto rc2 = ReferenceCounterOperator<GuiControl>::CreateCounter(control);

			TEST_ASSERT(*rc1 == 0);
			auto a1 = Ptr(bounds);
			TEST_ASSERT(*rc1 == 1);
			auto a2 = Ptr(bounds);
			TEST_ASSERT(*rc1 == 2);
			Ptr<DescriptableObject> a3 = a1.Cast<DescriptableObject>();
			TEST_ASSERT(*rc1 == 3);
			Ptr<DescriptableObject> a4 = a2;
			TEST_ASSERT(*rc1 == 4);

			TEST_ASSERT(*rc2 == 0);
			auto b1 = Ptr(control);
			TEST_ASSERT(*rc2 == 1);
			auto b2 = Ptr(control);
			TEST_ASSERT(*rc2 == 2);
			Ptr<DescriptableObject> b3 = b1.Cast<DescriptableObject>();
			TEST_ASSERT(*rc2 == 3);
			Ptr<DescriptableObject> b4 = b2;
			TEST_ASSERT(*rc2 == 4);

			b4 = b3 = b2 = b1 = 0; // not released yet
			TEST_ASSERT(*rc2 == 0);
			control->SetText(L"Text");

			a4 = a3 = a2 = a1 = 0; // all released
		});
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}