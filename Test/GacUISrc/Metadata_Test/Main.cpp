#define GAC_HEADER_USE_NAMESPACE

#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../../../Source/GacUI.h"
#include <VlppReflection.h>
#include <Windows.h>

using namespace vl;
using namespace vl::filesystem;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;

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
	return WString(buffer, pos + 1);
}

WString GetTestOutputPath()
{
#ifdef _WIN64
	return GetExePath() + L"../../../Resources/Metadata/";
#else
	return GetExePath() + L"../../Resources/Metadata/";
#endif
}

#ifdef VCZH_64
#define REFLECTION_BIN L"Reflection64.bin"
#define REFLECTION_OUTPUT L"Reflection64[2].txt"
#define REFLECTION_BASELINE L"Reflection64.txt"
#else
#define REFLECTION_BIN L"Reflection32.bin"
#define REFLECTION_OUTPUT L"Reflection32[2].txt"
#define REFLECTION_BASELINE L"Reflection32.txt"
#endif

#define INSTALL_SERIALIZABLE_TYPE(TYPE)\
	serializableTypes.Add(TypeInfo<TYPE>::content.typeName, MakePtr<SerializableType<TYPE>>());

void GuiMain()
{
}

int wmain(vint argc, wchar_t* argv[])
{
	{
		collections::Dictionary<WString, Ptr<ISerializableType>> serializableTypes;
		REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_SERIALIZABLE_TYPE)
		INSTALL_SERIALIZABLE_TYPE(Color)
		INSTALL_SERIALIZABLE_TYPE(GlobalStringKey)
		INSTALL_SERIALIZABLE_TYPE(DocumentFontSize)
		FileStream fileStream(GetTestOutputPath() + REFLECTION_BIN, FileStream::ReadOnly);
		auto typeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);
		auto tm = GetGlobalTypeManager();
		tm->AddTypeLoader(typeLoader);
		tm->Load();
	}
	{
		FileStream fileStream(GetTestOutputPath() + REFLECTION_OUTPUT, FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf8);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		LogTypeManager(writer);
	}
	{
		auto first = File(GetTestOutputPath() + REFLECTION_BASELINE).ReadAllTextByBom();
		auto second = File(GetTestOutputPath() + REFLECTION_OUTPUT).ReadAllTextByBom();
		CHECK_ERROR(first == second, L"Metadata is not properly loaded!");
	}
	return 0;
}

bool LoadPredefinedTypesForTestCase()
{
	auto result = LoadPredefinedTypes();
	GetGlobalTypeManager()->Load();
	return result;
}