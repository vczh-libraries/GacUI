#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#include "../../../Source/Compiler/InstanceQuery/GuiInstanceQuery.h"
#include "../../../Source/Resources/GuiParserManager.h"

using namespace vl::collections;
using namespace vl::stream;
using namespace vl::presentation;

void UnitTestInGuiMain()
{
#define ASSERT(x) do{if(!(x))throw 0;}while(0)
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

		volatile vint* rc1 = ReferenceCounterOperator<GuiBoundsComposition>::CreateCounter(bounds);
		volatile vint* rc2 = ReferenceCounterOperator<GuiControl>::CreateCounter(control);

		ASSERT(*rc1 == 0);
		Ptr<GuiBoundsComposition> a1 = bounds;
		ASSERT(*rc1 == 1);
		Ptr<GuiBoundsComposition> a2 = bounds;
		ASSERT(*rc1 == 2);
		Ptr<DescriptableObject> a3 = a1.Cast<DescriptableObject>();
		ASSERT(*rc1 == 3);
		Ptr<DescriptableObject> a4 = a2;
		ASSERT(*rc1 == 4);

		ASSERT(*rc2 == 0);
		Ptr<GuiControl> b1 = control;
		ASSERT(*rc2 == 1);
		Ptr<GuiControl> b2 = control;
		ASSERT(*rc2 == 2);
		Ptr<DescriptableObject> b3 = b1.Cast<DescriptableObject>();
		ASSERT(*rc2 == 3);
		Ptr<DescriptableObject> b4 = b2;
		ASSERT(*rc2 == 4);

		b4 = b3 = b2 = b1 = 0; // not released yet
		ASSERT(*rc2 == 0);
		control->SetText(L"Text");

		a4 = a3 = a2 = a1 = 0; // all released
	}
	{
		auto parser = GetParserManager()->GetParser<GuiIqQuery>(L"INSTANCE-QUERY");
		ASSERT(parser);

		const wchar_t* queries[] = {
			L"/Label",
			L"/Label.labelMessage",
			L"/@:Label",
			L"/@:Label.labelMessage",
			L"/@Items:Label",
			L"/@Items:Label.labelMessage",
			L"/*",
			L"/*.labelMessage",
			L"/@:*",
			L"/@:*.labelMessage",
			L"/@Items:*",
			L"/@Items:*.labelMessage",
			L"//Label",
			L"//Label.labelMessage",
			L"//@:Label",
			L"//@:Label.labelMessage",
			L"//@Items:Label",
			L"//@Items:Label.labelMessage",
			L"//*",
			L"//*.labelMessage",
			L"//@:*",
			L"//@:*.labelMessage",
			L"//@Items:*",
			L"//@Items:*.labelMessage",

			L"//Cell/Label",
			L"//Cell/Label + //Cell/TextList",
			L"//Cell/Label - //Cell/TextList",
			L"//Cell/Label * //Cell/TextList",
			L"//Cell/Label ^ //Cell/TextList",
			L"//Cell(/Label + /TextList)"
			L"//Cell(/Label - /TextList)"
			L"//Cell(/Label * /TextList)"
			L"//Cell(/Label ^ /TextList)"
			L"//Cell(/Label + /TextList)//Items"
			L"//Cell(/Label - /TextList)//Items"
			L"//Cell(/Label * /TextList)//Items"
			L"//Cell(/Label ^ /TextList)//Items"
		};

		for (auto queryCode : queries)
		{
			List<glr::ParsingError> errors;
			auto query = parser->ParseInternal(queryCode, errors);
			ASSERT(query);
			ASSERT(errors.Count() == 0);

			MemoryStream stream;
			WString queryText;
			{
				StreamWriter writer(stream);
				GuiIqPrint(query, writer);
			}
			stream.SeekFromBegin(0);
			{
				StreamReader reader(stream);
				queryText = reader.ReadToEnd();
			}
			ASSERT(queryText == queryCode || queryText == L"(" + WString(queryCode) + L")");
		}
	}
}