#include "ResourceCompiler.h"
#ifdef VCZH_MSVC
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection::description;

namespace vl
{
	namespace presentation
	{
		void GuiInitializeUtilities() {}
		void GuiFinalizeUtilities() {}
	}
}

extern void UnitTestInGuiMain();

GuiResourceCpuArchitecture targetCpuArchitecture = GuiResourceCpuArchitecture::Unspecified;

const wchar_t* REFLECTION_BIN()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"Metadata/Reflection32.bin";
	case GuiResourceCpuArchitecture::x64: return L"Metadata/Reflection64.bin";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* DARKSKIN_BINARY_FOLDER()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"../GacUISrc/Generated_DarkSkin/Resource_x86/";
	case GuiResourceCpuArchitecture::x64: return L"../GacUISrc/Generated_DarkSkin/Resource_x64/";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* DARKSKIN_SOURCE_FOLDER()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"../GacUISrc/Generated_DarkSkin/Source_x86/";
	case GuiResourceCpuArchitecture::x64: return L"../GacUISrc/Generated_DarkSkin/Source_x64/";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* FULLCONTROLTEST_BINARY_FOLDER()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"../GacUISrc/Generated_FullControlTest/Resource_x86/";
	case GuiResourceCpuArchitecture::x64: return L"../GacUISrc/Generated_FullControlTest/Resource_x64/";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* FULLCONTROLTEST_SOURCE_FOLDER()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"../GacUISrc/Generated_FullControlTest/Source_x86/";
	case GuiResourceCpuArchitecture::x64: return L"../GacUISrc/Generated_FullControlTest/Source_x64/";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* DIALOGS_BINARY_FOLDER()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"../GacUISrc/Generated_Dialogs/Resource_x86/";
	case GuiResourceCpuArchitecture::x64: return L"../GacUISrc/Generated_Dialogs/Resource_x64/";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

const wchar_t* DIALOGS_SOURCE_FOLDER()
{
	switch (targetCpuArchitecture)
	{
	case GuiResourceCpuArchitecture::x86: return L"../GacUISrc/Generated_Dialogs/Source_x86/";
	case GuiResourceCpuArchitecture::x64: return L"../GacUISrc/Generated_Dialogs/Source_x64/";
	default: CHECK_FAIL(L"The target CPU architecture is unspecified.");
	}
}

#if defined VCZH_MSVC

int wmain(vint argc, wchar_t* argv[])
{
	targetCpuArchitecture = GuiResourceCpuArchitecture::x86;
	SetupGacGenNativeController();

	targetCpuArchitecture = GuiResourceCpuArchitecture::x64;
	SetupGacGenNativeController();

#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return 0;
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
		FileStream fileStream((GetResourcePath() / REFLECTION_BIN()).GetFullPath(), FileStream::ReadOnly);
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

	List<WString> dependencies;
	LoadResource(CompileResources(
		targetCpuArchitecture,
		L"GuiFakeDialogServiceUI",
		dependencies,
		(GetResourcePath() / L"../../Source/Utilities/FakeServices/Dialogs/Resource.xml"),
		(GetResourcePath() / DIALOGS_BINARY_FOLDER()),
		(GetResourcePath() / DIALOGS_SOURCE_FOLDER()),
		false
	));
	//LoadResource(CompileResources(
	//	targetCpuArchitecture,
	//	L"DarkSkin",
	//	dependencies,
	//	(GetResourcePath() / L"App/DarkSkin/Resource.xml"),
	//	(GetResourcePath() / DARKSKIN_BINARY_FOLDER()),
	//	(GetResourcePath() / DARKSKIN_SOURCE_FOLDER()),
	//	true
	//));
	//LoadResource(CompileResources(
	//	targetCpuArchitecture,
	//	L"Demo",
	//	dependencies,
	//	(GetResourcePath() / L"App/FullControlTest/Resource.xml"),
	//	(GetResourcePath() / FULLCONTROLTEST_BINARY_FOLDER()),
	//	(GetResourcePath() / FULLCONTROLTEST_SOURCE_FOLDER()),
	//	false
	//));
}