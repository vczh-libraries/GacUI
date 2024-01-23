#include "../../../Source/GacUI.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;

namespace compiler_error_tests
{
	extern WString GetTestResourcePath();
	extern WString GetTestBaselinePath();
	extern WString GetTestOutputPath();
}
using namespace compiler_error_tests;

void WriteErrors(GuiResourceError::List& errors, const WString& resourceName)
{
	auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".txt");
	auto baselinePath = FilePath(GetTestBaselinePath()) / (resourceName + L".txt");

	List<WString> output;
	GuiResourceError::SortAndLog(errors, output, GetTestResourcePath());
	File(outputPath).WriteAllLines(output, false, BomEncoder::Utf8);

	List<WString> baseline;
	File(baselinePath).ReadAllLinesByBom(baseline);
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

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	SetGuiMainProxy([]()
	{
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