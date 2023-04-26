#define GAC_HEADER_USE_NAMESPACE

#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../../../Source/GacUI.h"
#include <VlppReflection.h>
#include <VlppWorkflowCompiler.h>
#ifdef VCZH_MSVC
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::workflow::analyzer;

#if defined VCZH_MSVC
WString GetExePath()
{
	wchar_t buffer[65536];
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
	vint pos = -1;
	vint index = 0;
	while (buffer[index])
	{
		if (buffer[index] == L'\\' || buffer[index] == L'/')
		{
			pos = index;
		}
		index++;
	}
	return WString::CopyFrom(buffer, pos + 1);
}

WString GetTestOutputPath()
{
#ifdef _WIN64
	return GetExePath() + L"../../../Resources/Metadata/";
#else
	return GetExePath() + L"../../Resources/Metadata/";
#endif
}
#elif defined VCZH_GCC
WString GetTestOutputPath()
{
	return L"../../Resources/Metadata/";
}
#endif

WfCpuArchitecture targetCpuArchitecture = WfCpuArchitecture::x64;
bool coreTypes = false;

const wchar_t* REFLECTION_BIN()
{
	switch (targetCpuArchitecture)
	{
	case WfCpuArchitecture::x86: return coreTypes ? L"ReflectionCore32.bin" : L"Reflection32.bin";
	case WfCpuArchitecture::x64: return coreTypes ? L"ReflectionCore64.bin" : L"Reflection64.bin";
	default:
		CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* REFLECTION_BASELINE()
{
	switch (targetCpuArchitecture)
	{
	case WfCpuArchitecture::x86: return coreTypes ? L"ReflectionCore32.txt" : L"Reflection32.txt";
	case WfCpuArchitecture::x64: return coreTypes ? L"ReflectionCore64.txt" : L"Reflection64.txt";
	default:
		CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* REFLECTION_OUTPUT()
{
	switch (targetCpuArchitecture)
	{
	case WfCpuArchitecture::x86: return coreTypes ? L"ReflectionCore32[2].txt" : L"Reflection32[2].txt";
	case WfCpuArchitecture::x64: return coreTypes ? L"ReflectionCore64[2].txt" : L"Reflection64[2].txt";
	default:
		CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

#define INSTALL_SERIALIZABLE_TYPE(TYPE)\
	serializableTypes.Add(TypeInfo<TYPE>::content.typeName, Ptr(new SerializableType<TYPE>));

void GuiMain()
{
}

void TestMetaonlyReflection()
{
	{
		collections::Dictionary<WString, Ptr<ISerializableType>> serializableTypes;
		REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_SERIALIZABLE_TYPE)
		INSTALL_SERIALIZABLE_TYPE(Color)
		INSTALL_SERIALIZABLE_TYPE(GlobalStringKey)
		INSTALL_SERIALIZABLE_TYPE(DocumentFontSize)
		FileStream fileStream(GetTestOutputPath() + REFLECTION_BIN(), FileStream::ReadOnly);
		auto typeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);
		auto tm = GetGlobalTypeManager();
		tm->AddTypeLoader(typeLoader);
		tm->Load();
	}
	{
		FileStream fileStream(GetTestOutputPath() + REFLECTION_OUTPUT(), FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf8);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		LogTypeManager(writer);
	}
	{
		List<WString> firstLines;
		List<WString> secondLines;
		File(GetTestOutputPath() + REFLECTION_BASELINE()).ReadAllLinesByBom(firstLines);
		File(GetTestOutputPath() + REFLECTION_OUTPUT()).ReadAllLinesByBom(secondLines);
		CHECK_ERROR(firstLines.Count() == secondLines.Count(), L"Metadata is not properly loaded!");
		for (auto [first, index] : indexed(firstLines))
		{
			auto second = secondLines[index];
			CHECK_ERROR(first == second, L"Metadata is not properly loaded!");
		}
	}
	{
		GetGlobalTypeManager()->Unload();
		ResetGlobalTypeManager();
	}
}

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#elif defined VCZH_GCC
int main(int argc, char* argv[])
#endif
{
	coreTypes = true;
	targetCpuArchitecture = WfCpuArchitecture::x86;
	TestMetaonlyReflection();

	coreTypes = true;
	targetCpuArchitecture = WfCpuArchitecture::x64;
	TestMetaonlyReflection();

	coreTypes = false;
	targetCpuArchitecture = WfCpuArchitecture::x86;
	TestMetaonlyReflection();

	coreTypes = false;
	targetCpuArchitecture = WfCpuArchitecture::x64;
	TestMetaonlyReflection();

	return 0;
}