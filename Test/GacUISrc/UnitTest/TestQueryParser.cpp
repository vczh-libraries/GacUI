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

		TEST_CASE(L"Execute instance queries from the root and explicit inputs")
		{
			auto parser = GetParserManager()->GetParser<GuiIqQuery>(L"INSTANCE-QUERY");
			TEST_ASSERT(parser);
			auto parseQuery = [&](const WString& text)
			{
				List<glr::ParsingError> errors;
				auto query = parser->ParseInternal(text, errors);
				TEST_ASSERT(query);
				TEST_ASSERT(errors.Count() == 0);
				return query;
			};

			auto context = Ptr(new GuiInstanceContext);
			context->instance = Ptr(new GuiConstructorRepr);
			context->instance->typeName = GlobalStringKey::Get(L"Window");
			auto child = Ptr(new GuiConstructorRepr);
			child->typeName = GlobalStringKey::Get(L"Label");
			auto children = Ptr(new GuiAttSetterRepr::SetterValue);
			children->values.Add(child);
			context->instance->setters.Add(GlobalStringKey::Empty, children);

			List<Ptr<GuiConstructorRepr>> input, output;
			auto rootQuery = parseQuery(L"/Window");
			ExecuteQuery(rootQuery, context, output);
			TEST_ASSERT(output.Count() == 1);
			TEST_ASSERT(output[0] == context->instance);

			output.Clear();
			ExecuteQuery(rootQuery, context, input, output);
			TEST_ASSERT(output.Count() == 0);

			input.Add(context->instance);
			ExecuteQuery(parseQuery(L"/Label"), context, input, output);
			TEST_ASSERT(output.Count() == 1);
			TEST_ASSERT(output[0] == child);

			output.Clear();
			ExecuteQuery(parseQuery(L"/Window/Label"), context, output);
			TEST_ASSERT(output.Count() == 1);
			TEST_ASSERT(output[0] == child);

			output.Clear();
			ExecuteQuery(parseQuery(L"/Window + //Label"), context, output);
			TEST_ASSERT(output.Count() == 2);
			TEST_ASSERT(output.Contains(context->instance.Obj()));
			TEST_ASSERT(output.Contains(child.Obj()));
		});
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}
