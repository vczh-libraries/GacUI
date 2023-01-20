#include "ResourceCompiler.h"
#ifdef VCZH_MSVC
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection::description;

extern void UnitTestInGuiMain();

#ifdef VCZH_64
#define REFLECTION_BIN L"Metadata/Reflection64.bin"
#else
#define REFLECTION_BIN L"Metadata/Reflection32.bin"
#endif

#if defined VCZH_MSVC

int wmain(vint argc, wchar_t* argv[])
{
	int result = SetupGacGenNativeController();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

FilePath GetResourcePath()
{
	Array<wchar_t> buffer(65536);
	GetModuleFileName(NULL, &buffer[0], (DWORD)buffer.Count());
	auto folder = FilePath(WString::Unmanaged(&buffer[0])).GetFolder();
#ifdef _WIN64
	return folder / L"../../../Resources";
#else
	return folder / L"../../Resources";
#endif
}

#elif defined VCZH_GCC

int main(int argc, char* argv[])
{
	return SetupGacGenNativeController();
}

FilePath GetResourcePath()
{
	return FilePath(WString::Unmanaged(L"../../Resources"));
}

#endif

class GuiReflectionPlugin : public Object, public IGuiPlugin
{
public:

	GUI_PLUGIN_NAME(GacUI_Instance_Reflection)
	{
	}

	void Load()override
	{
#define INSTALL_SERIALIZABLE_TYPE(TYPE) serializableTypes.Add(TypeInfo<TYPE>::content.typeName, Ptr(new SerializableType<TYPE>));
		collections::Dictionary<WString, Ptr<ISerializableType>> serializableTypes;
		REFLECTION_PREDEFINED_SERIALIZABLE_TYPES(INSTALL_SERIALIZABLE_TYPE)
		INSTALL_SERIALIZABLE_TYPE(Color)
		INSTALL_SERIALIZABLE_TYPE(GlobalStringKey)
		INSTALL_SERIALIZABLE_TYPE(DocumentFontSize)
		FileStream fileStream((GetResourcePath() / REFLECTION_BIN).GetFullPath(), FileStream::ReadOnly);
		auto typeLoader = LoadMetaonlyTypes(fileStream, serializableTypes);
		auto tm = GetGlobalTypeManager();
		tm->AddTypeLoader(typeLoader);
#undef INSTALL_SERIALIZABLE_TYPE
	}

	void Unload()override
	{
	}
};
GUI_REGISTER_PLUGIN(GuiReflectionPlugin)

void GuiMain()
{
	UnitTestInGuiMain();
#define DARKSKIN_PATH					L"App/DarkSkin/Resource.xml"
#define FULLCONTROLTEST_PATH			L"App/FullControlTest/Resource.xml"

#ifdef VCZH_64
#define DARKSKIN_BINARY_FOLDER			L"../GacUISrc/Generated_DarkSkin/Resource_x64/"
#define DARKSKIN_SOURCE_FOLDER			L"../GacUISrc/Generated_DarkSkin/Source_x64/"
#define FULLCONTROLTEST_BINARY_FOLDER	L"../GacUISrc/Generated_FullControlTest/Resource_x64/"
#define FULLCONTROLTEST_SOURCE_FOLDER	L"../GacUISrc/Generated_FullControlTest/Source_x64/"
#else
#define DARKSKIN_BINARY_FOLDER			L"../GacUISrc/Generated_DarkSkin/Resource_x86/"
#define DARKSKIN_SOURCE_FOLDER			L"../GacUISrc/Generated_DarkSkin/Source_x86/"
#define FULLCONTROLTEST_BINARY_FOLDER	L"../GacUISrc/Generated_FullControlTest/Resource_x86/"
#define FULLCONTROLTEST_SOURCE_FOLDER	L"../GacUISrc/Generated_FullControlTest/Source_x86/"
#endif

	List<WString> dependencies;
	LoadResource(CompileResources(
		L"DarkSkin",
		dependencies,
		(GetResourcePath() / DARKSKIN_PATH),
		(GetResourcePath() / DARKSKIN_BINARY_FOLDER),
		(GetResourcePath() / DARKSKIN_SOURCE_FOLDER),
		true
	));
	LoadResource(CompileResources(
		L"Demo",
		dependencies,
		(GetResourcePath() / FULLCONTROLTEST_PATH),
		(GetResourcePath() / FULLCONTROLTEST_BINARY_FOLDER),
		(GetResourcePath() / FULLCONTROLTEST_SOURCE_FOLDER),
		false
	));
}