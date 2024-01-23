#include "../../../Source/GacUI.h"
#include "../../../Source/Resources/GuiParserManager.h"
#include "../../../Source/Compiler/InstanceQuery/GuiInstanceQuery.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::presentation;
using namespace vl::presentation::instancequery;

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	SetGuiMainProxy([]()
	{
		TEST_CASE(L"Test INSTANCE_QUERY Parser")
		{
			auto parser = GetParserManager()->GetParser<GuiIqQuery>(L"INSTANCE-QUERY");
			TEST_ASSERT(parser);

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
				TEST_ASSERT(query);
				TEST_ASSERT(errors.Count() == 0);

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
				TEST_ASSERT(queryText == queryCode || queryText == L"(" + WString(queryCode) + L")");
			}
		});
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}