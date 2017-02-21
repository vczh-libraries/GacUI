#include "../../../Source/GacUI.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;

extern WString GetTestResourcePath();
extern WString GetTestOutputPath();

void WriteErrors(GuiResourceError::List& errors, const WString& resourceName)
{
	auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".txt");
	auto baselinePath = FilePath(GetTestResourcePath()) / (resourceName + L".txt");
	List<WString> output;

	SortLambda(&errors[0], errors.Count(), [](const GuiResourceError& a, const GuiResourceError& b)
	{
		vint result = 0;
		if (result == 0) result = WString::Compare(a.resourcePath, b.resourcePath);
		if (result == 0) result = WString::Compare(a.filePath, b.filePath);
		if (result == 0) result = a.position.row - b.position.row;
		if (result == 0) result = a.position.column - b.position.column;
		return result;
	});

	FOREACH_INDEXER(GuiResourceError, error, index, errors)
	{
		bool needHeader = index == 0;
		if (index > 0)
		{
			auto previousError = errors[index - 1];
			if (error.resourcePath != previousError.resourcePath || error.filePath != previousError.filePath)
			{
				needHeader = true;
			}
		}

		if (needHeader)
		{
			output.Add(error.resourcePath + L" # " + FilePath(GetTestResourcePath()).GetRelativePathFor(error.filePath));
		}

		WString prefix = L"Failed to load file \"";
		WString postfix = L"\".";
		if (INVLOC.StartsWith(error.message, prefix, Locale::Normalization::None) && INVLOC.EndsWith(error.message, postfix, Locale::Normalization::None))
		{
			auto path = error.message.Sub(prefix.Length(), error.message.Length() - prefix.Length() - postfix.Length());
			path = FilePath(GetTestResourcePath()).GetRelativePathFor(path);
			error.message = prefix + path + postfix;
		}
		output.Add(L"(" + itow(error.position.row) + L", " + itow(error.position.column) + L"): " + error.message);
	}

	File(outputPath).WriteAllLines(output, false, BomEncoder::Utf8);

	List<WString> baseline;
	File(baselinePath).ReadAllLines(baseline);
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
		TEST_ASSERT(output[i] == baseline[i]);
	}
}

#define PRINT_ERROR\
	do\
	{\
		if (errors.Count() > 0)\
		{\
			WriteErrors(errors, resourceName);\
			return nullptr;\
		}\
	}while(0)\

Ptr<GuiResource> LoadResource(const WString& resourceName, bool requireErrors)
{
	auto inputPath = FilePath(GetTestResourcePath()) / resourceName;
	GuiResourceError::List errors;

	auto resource = GuiResource::LoadFromXml(inputPath.GetFullPath(), errors);
	PRINT_ERROR;
	resource->Precompile(nullptr, errors);
	PRINT_ERROR;
	TEST_ASSERT(!requireErrors);

	auto outputPath = FilePath(GetTestOutputPath()) / (resourceName + L".bin");
	FileStream stream(outputPath.GetFullPath(), FileStream::WriteOnly);
	resource->SavePrecompiledBinary(stream);
	return resource;
}

#undef PRINT_ERROR

TEST_CASE(TestResource_NotExists)
{
	LoadResource(L"Resource.NotExists.xml", true);
}

TEST_CASE(TestResource_WrongSyntax)
{
	LoadResource(L"Resource.WrongSyntax.xml", true);
}

TEST_CASE(TestResource_WrongSyntax2)
{
	LoadResource(L"Resource.WrongSyntax2.xml", true);
}

TEST_CASE(TestResource_WrongDoc)
{
	LoadResource(L"Resource.WrongDoc.xml", true);
}

TEST_CASE(TestResource_WrongInstanceStyle)
{
	LoadResource(L"Resource.WrongInstanceStyle.xml", true);
}