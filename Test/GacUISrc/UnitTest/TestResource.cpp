#include "../../../Source/GacUI.h"
#include "../../../Source/Compiler/GuiInstanceRepresentation.h"
#include "../../../Source/Resources/GuiParserManager.h"
#include "../../../Source/Compiler/GuiCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;

namespace compiler_error_tests
{
	extern WString GetTestResourcePath();
	extern WString GetTestSharedBaselinePath();
	extern WString GetTestPlatformBaselinePath();
	extern WString GetTestOutputPath();
}
using namespace compiler_error_tests;

void WriteErrors(GuiResourceError::List& errors, const WString& resourceName)
{
	auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".txt");
	auto baselineShared = FilePath(GetTestSharedBaselinePath()) / (resourceName + L".txt");
	auto baselinePlatform = FilePath(GetTestPlatformBaselinePath()) / (resourceName + L".txt");

	bool baselineSharedExists = File(baselineShared).Exists();
	bool baselinePlatformExists = File(baselinePlatform).Exists();
	TEST_ASSERT(baselineSharedExists ^ baselinePlatformExists);

	List<WString> output;
	GuiResourceError::SortAndLog(errors, output, GetTestResourcePath());
	File(outputPath).WriteAllLines(output, false, BomEncoder::Utf8);

	List<WString> baseline;
	File(baselineSharedExists ? baselineShared : baselinePlatform).ReadAllLinesByBom(baseline);
	if (baseline.Count() > 0 && baseline[baseline.Count() - 1] == L"")
	{
		baseline.RemoveAt(baseline.Count() - 1);
	}

	vint count = output.Count();
	if (count < baseline.Count())
	{
		count = baseline.Count();
	}

	for (vint i = 0; i < count; i++)
	{
		TEST_ASSERT(output.Count() > i);
		TEST_ASSERT(baseline.Count() > i);
#if defined VCZH_MSVC
		TEST_ASSERT(output[i] == baseline[i]);
#elif defined VCZH_GCC
		auto posRes = INVLOC.FindFirst(output[i], L"Resource.", Locale::Normalization::None);
		if (posRes.key == -1)
		{
			TEST_ASSERT(output[i] == baseline[i]);
		}
		else
		{
			auto fixedOutput = output[i].Remove(posRes.key, posRes.value).Insert(posRes.key, L".\\Resource.");
			TEST_ASSERT(output[i] == baseline[i] || fixedOutput == baseline[i]);
		}
#endif
	}
}

#define PRINT_ERROR\
	do\
	{\
		if (errors.Count() > 0)\
		{\
			WriteErrors(errors, resourceName);\
			return;\
		}\
	}while(0)\

Ptr<GuiResource> LoadResource(const WString& resourceName)
{
	Ptr<GuiResource> resource;
	TEST_CASE(L"Compare compiler output for: " + resourceName)
	{
		auto inputPath = FilePath(GetTestResourcePath()) / resourceName;
		GuiResourceError::List errors;

		resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);
		PRINT_ERROR;
		resource->Precompile(GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
		PRINT_ERROR;
		TEST_ASSERT(false);
	});
	return resource;
}

#undef PRINT_ERROR

namespace easy_layout_xml_tests
{
	extern WString Resource(const WString& content);
}

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	SetGuiMainProxy([]()
	{
		TEST_CASE(L"Easy layout namespace is predefined independently of the default namespace")
		{
			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
			for (vint customized = 0; customized < 2; customized++)
			for (vint explicitEz = 0; explicitEz < 2; explicitEz++)
			{
				auto xml = parser->Parse({}, L"<Instance "
					+ WString(customized ? L"xmlns=\"presentation::controls::Gui*\" " : L"")
					+ WString(explicitEz ? L"xmlns:ez=\"presentation::compositions::Gui*Composition\" " : L"")
					+ L"><Window/></Instance>", errors);
				auto context = GuiInstanceContext::LoadFromXml(nullptr, xml, errors);
				for (vint round = 0; round < 2; round++)
				{
					TEST_ASSERT(errors.Count() == 0);
					TEST_ASSERT(context->namespaces.Keys().Contains(GlobalStringKey::Get(L"ez")));
					auto mapping = context->namespaces[GlobalStringKey::Get(L"ez")];
					TEST_ASSERT(mapping->namespaces.Count() == (explicitEz ? 1 : 2));
					TEST_ASSERT(mapping->namespaces[0]->prefix == (explicitEz ? L"presentation::compositions::Gui" : L"presentation::compositions::eazy_layout::GuiEasy"));
					context = GuiInstanceContext::LoadFromXml(nullptr, context->SaveToXml(), errors);
				}
			}
		});

		TEST_CASE(L"Compiler accepts easy splitter descriptors between fill tracks")
		{
			GuiResourceError::List errors;
			auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
			auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(L"<ez:Layout><ez:Fill/><ez:Splitter/><ez:Fill/></ez:Layout>"), errors);
			TEST_ASSERT(xml && errors.Count() == 0);
			auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
			TEST_ASSERT(resource);
			if (errors.Count() == 0) PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
			for (auto error : errors) TEST_PRINT(error.message);
			TEST_ASSERT(errors.Count() == 0);
		});

		TEST_CASE(L"Compiler rejects constant-property bindings, runtime struct fields and static child grammar with source positions")
		{
			collections::List<WString> cases;
			for (auto binder : { L"eval",L"ref",L"uri",L"bind",L"set" })
			for (vint element = 0; element < 2; element++)
			{
				auto percentage = element
					? L"<ez:Fill><att.Percentage-" + WString(binder) + L">1.0</att.Percentage-" + binder + L"></ez:Fill>"
					: L"<ez:Fill Percentage-" + WString(binder) + L"=\"1.0\"/>";
				auto option = element
					? L"<ez:Row><att.CellOption-" + WString(binder) + L">{composeType:MinSize}</att.CellOption-" + binder + L"><ez:Column/></ez:Row>"
					: L"<ez:Row CellOption-" + WString(binder) + L"=\"{composeType:MinSize}\"><ez:Column/></ez:Row>";
				cases.Add(L"<ez:Layout>" + percentage + L"</ez:Layout>");
				cases.Add(L"<ez:Layout>" + option + L"</ez:Layout>");
			}
			cases.Add(L"<ez:Layout><ez:Row CellOption=\"composeType:Absolute absolute:self.ClientSize.x\"><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><ez:Column><att.CellOption>composeType:Percentage percentage:(cast double self.ClientSize.x)</att.CellOption></ez:Column></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Top/><Button/></ez:Layout>");
			cases.Add(L"<ez:Layout><Button/><Button/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Top/><ez:Left/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Row><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><Button/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Column><ez:Top/></ez:Column></ez:Layout>");
			cases.Add(L"<ez:Layout><Cell/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill><StackItem/></ez:Fill></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Splitter/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Splitter/><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter/><ez:Splitter/><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter><Bounds/></ez:Splitter><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter><att.Composition><Bounds/></att.Composition></ez:Splitter><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Fill/><ez:Splitter><att.Layouts><ez:Fill/></att.Layouts></ez:Splitter><ez:Fill/></ez:Layout>");
			cases.Add(L"<ez:Layout><att.Layouts><ez:Splitter/><ez:Fill/></att.Layouts></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Row><ez:Splitter/><ez:Column/></ez:Row></ez:Layout>");
			cases.Add(L"<ez:Layout><ez:Column><att.Layouts><ez:Row/><ez:Splitter/><ez:Splitter/><ez:Row/></att.Layouts></ez:Column></ez:Layout>");
			for (auto content : cases)
			{
				GuiResourceError::List errors;
				auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
				auto xml = parser->Parse({}, easy_layout_xml_tests::Resource(content), errors);
				TEST_ASSERT(xml && errors.Count() == 0);
				auto resource = GuiResource::LoadFromXml(xml, L"TestControls_EasyLayout.xml", L".", errors);
				TEST_ASSERT(resource);
				if (errors.Count() == 0) PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
				TEST_ASSERT(errors.Count() > 0);
				bool positioned = false;
				for (auto error : errors)
				{
					TEST_PRINT(error.message);
					positioned |= error.position.row >= 0 && error.position.column >= 0;
				}
				TEST_ASSERT(positioned);
			}
		});

		LoadResource(L"Resource.NotExists.xml");
		LoadResource(L"Resource.WrongSyntax.xml");
		LoadResource(L"Resource.WrongSyntax2.xml");
		LoadResource(L"Resource.WrongDoc.xml");
		LoadResource(L"Resource.WrongInstanceStyle.xml");
		LoadResource(L"Resource.WrongInstance.xml");
		LoadResource(L"Resource.FailedInstance.Ctor1.xml");
		LoadResource(L"Resource.FailedInstance.Ctor2.xml");
		LoadResource(L"Resource.FailedInstance.Ctor2_r.xml");
		LoadResource(L"Resource.FailedInstance.Ctor3.xml");
		LoadResource(L"Resource.FailedInstance.Ctor4.xml");
		LoadResource(L"Resource.FailedInstance.Ctor5.xml");
		LoadResource(L"Resource.FailedInstance.Control.xml");
		LoadResource(L"Resource.FailedInstance.Inheriting1.xml");
		LoadResource(L"Resource.FailedInstance.Inheriting2.xml");
		LoadResource(L"Resource.FailedScript.Workflow.xml");
		LoadResource(L"Resource.FailedScript.Properties.xml");
		LoadResource(L"Resource.FailedScript.Animations.xml");
		LoadResource(L"Resource.FailedScript.Animations2.xml");
		LoadResource(L"Resource.FailedScript.Strings.xml");
		LoadResource(L"Resource.FailedScript.Strings2.xml");
		LoadResource(L"Resource.FailedScript.Strings3.xml");
		LoadResource(L"Resource.FailedScript.Strings4.xml");
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}
