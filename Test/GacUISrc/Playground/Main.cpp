#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "DarkSkinReflection.h"
#include "../GacUI_Compiler/ResourceCompiler.h"
#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection::description;

class Playground : public Object, public Description<Playground>
{
public:
	WString LoadBigJson()
	{
		auto exePath = FilePath(GetApplication()->GetExecutablePath()).GetFolder();
#ifdef VCZH_64
		auto jsonPath = exePath / L"../../Playground/Resources/BigJson.json";
#else
		auto jsonPath = exePath / L"../Playground/Resources/BigJson.json";
#endif
		return File(jsonPath).ReadAllTextByBom();
	}
};

namespace vl::reflection::description
{
#define PLAYGROUND_TYPE_LIST(F)	\
	F(Playground)\

	PLAYGROUND_TYPE_LIST(DECL_TYPE_INFO)
	PLAYGROUND_TYPE_LIST(IMPL_CPP_TYPE_INFO)

#define _ ,
	BEGIN_CLASS_MEMBER(Playground)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<Playground>(), NO_PARAMETER)
		CLASS_MEMBER_METHOD(LoadBigJson, NO_PARAMETER)
	END_CLASS_MEMBER(Playground)

#undef _

	class PlaygroundTypeLoader : public Object, public ITypeLoader
	{
	public:
		void Load(ITypeManager* manager)
		{
			PLAYGROUND_TYPE_LIST(ADD_TYPE_INFO)
		}

		void Unload(ITypeManager* manager)
		{
		}
	};

	bool LoadPlaygroundTypes()
	{
		if (auto manager = GetGlobalTypeManager())
		{
			return manager->AddTypeLoader(Ptr(new PlaygroundTypeLoader));
		}
		return false;
	}
}

namespace vl::presentation::description
{
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = SetupWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

void OpenMainWindow()
{
	{
		auto theme = Ptr(new darkskin::Theme);
		RegisterTheme(theme);
	}
	{
		//auto theme = UnboxValue<Ptr<ThemeTemplates>>(Value::Create(L"demo::TestTheme"));
		//RegisterTheme(theme);
	}
	{
		auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::TestWindow"));
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		GetApplication()->Run(window);
		delete window;
	}
}

void CopyWindowsKeyName()
{
	auto writer = GetCurrentController()->ClipboardService()->WriteClipboard();
	writer->SetText(GenerateToStream([](TextWriter& writer)
	{
#undef KEY_EXECUTE
#define RETRIEVE_KEY_NAME(NAME, CODE)\
		{\
			auto key =  GetCurrentController()->InputService()->GetKeyName(VKEY::KEY_ ## NAME);\
			if (key != L"?") writer.WriteLine(L"ITEM(" L ## #NAME L", L\"" + GetCurrentController()->InputService()->GetKeyName(VKEY::KEY_ ## NAME) + L"\")");\
		}\

		GUI_DEFINE_KEYBOARD_CODE_BASIC(RETRIEVE_KEY_NAME)
#undef RETRIEVE_KEY_NAME
	}));
	writer->Submit();
}

void TestWindowsKeyName()
{
	Array<const wchar_t*> keyNames((vint)VKEY::KEY_MAXIMUM);
	memset(&keyNames[0], 0, sizeof(const wchar_t*) * keyNames.Count());
#undef KEY_EXECUTE
#define RECORD_KEY_NAME(NAME, TEXT) keyNames[(vint)VKEY::KEY_ ## NAME] = TEXT;
	GUI_DEFINE_KEYBOARD_WINDOWS_NAME(RECORD_KEY_NAME)
#undef RECORD_KEY_NAME

#define TEST_KEY_NAME(NAME, CODE)\
	{\
		auto key = GetCurrentController()->InputService()->GetKeyName(VKEY::KEY_ ## NAME);\
		if (key == L"?") CHECK_ERROR(keyNames[(vint)VKEY::KEY_ ## NAME] == nullptr, L"Key name should not exist: " L ## #NAME);\
		else CHECK_ERROR(keyNames[(vint)VKEY::KEY_ ## NAME] == key, L"Key name does not match: " L ## #NAME);\
	}\

	GUI_DEFINE_KEYBOARD_CODE(TEST_KEY_NAME)
#undef TEST_KEY_NAME
}

void GuiMain()
{
	TestWindowsKeyName();
	LoadDarkSkinTypes();
	LoadPlaygroundTypes();

	List<WString> names;
	names.Add(L"ResourceBigJson");

	Group<WString, WString> deps;

	Dictionary<WString, FilePath> paths;
	for (auto name : names)
	{
		List<WString> dependencies;
		vint index = deps.Keys().IndexOf(name);
		if (index != -1)
		{
			CopyFrom(dependencies, deps.GetByIndex(index));
		}
		paths.Add(name, CompileResources(
			GuiResourceCpuArchitecture::Unspecified,
			name,
			L"",L"",L"", // Ignore arguments since skipped C++ codegen
			dependencies,
			L"Resources/" + name + L".xml",
			L"./",
			L"",
			false));
		LoadResource(paths[name]);
	}

	for (auto name : names)
	{
		GetResourceManager()->UnloadResource(name);
	}
	for (auto name : From(names).Reverse())
	{
		LoadResource(paths[name]);
	}

	names.Clear();
	GetResourceManager()->GetPendingResourceNames(names);
	CHECK_ERROR(names.Count() == 0, L"Error");

	OpenMainWindow();
}