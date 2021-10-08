#include "ResourceCompiler.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::reflection::description;

int wmain(vint argc, wchar_t* argv[])
{
	int result = SetupWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

extern void UnitTestInGuiMain();

#ifdef VCZH_64
#define REFLECTION_BIN L"Metadata/Reflection64.bin"
#else
#define REFLECTION_BIN L"Metadata/Reflection32.bin"
#endif

WString GetResourcePath()
{
#ifdef _WIN64
	return GetApplication()->GetExecutableFolder() + L"../../../Resources/";
#else
	return GetApplication()->GetExecutableFolder() + L"../../Resources/";
#endif
}

void GuiMain()
{
	UnitTestInGuiMain();
	{
#define INSTALL_SERIALIZABLE_TYPE(TYPE) serializableTypes.Add(TypeInfo<TYPE>::content.typeName, MakePtr<SerializableType<TYPE>>());
		collections::Dictionary<WString, Ptr<ISerializableType>> serializableTypes;
		REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_SERIALIZABLE_TYPE)
		INSTALL_SERIALIZABLE_TYPE(Color)
		INSTALL_SERIALIZABLE_TYPE(GlobalStringKey)
		INSTALL_SERIALIZABLE_TYPE(DocumentFontSize)
		FileStream fileStream(GetResourcePath() + REFLECTION_BIN, FileStream::ReadOnly);
		auto typeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);
		auto tm = GetGlobalTypeManager();
		tm->AddTypeLoader(typeLoader);
		tm->Load();
#undef INSTALL_SERIALIZABLE_TYPE
	}

#define DARKSKIN_PATH			L"App/DarkSkin/Resource.xml"
#define FULLCONTROLTEST_PATH	L"App/FullControlTest/Resource.xml"
#define BINARY_FOLDER			L"../GacUISrc/TestCppCodegen/"
#define SOURCE_FOLDER			L"../GacUISrc/TestCppCodegen/Source/"

	List<WString> dependencies;
	LoadResource(CompileResources(
		L"DarkSkin",
		dependencies,
		(GetResourcePath() + DARKSKIN_PATH),
		(GetResourcePath() + BINARY_FOLDER),
		(GetResourcePath() + SOURCE_FOLDER),
		true
	));
	LoadResource(CompileResources(
		L"Demo",
		dependencies,
		(GetResourcePath() + FULLCONTROLTEST_PATH),
		(GetResourcePath() + BINARY_FOLDER),
		(GetResourcePath() + SOURCE_FOLDER),
		false
	));
}