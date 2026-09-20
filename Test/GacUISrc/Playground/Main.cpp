#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "DarkSkinReflection.h"
#include "../GacUI_Compiler/ResourceCompiler.h"
#include "../../../Source/GacUI.h"
#include "../../../Source/Utilities/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include "../../../Source/PlatformProviders/Windows/UIAutomation/WindowsUIAutomation.Windows.h"
#include "../SharedArguments.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::reflection::description;

FilePath GetPlaygroundResourceFolder()
{
	auto exePath = FilePath(GetApplication()->GetExecutablePath()).GetFolder();
#ifdef VCZH_64
	return exePath / L"../../Playground/Resources";
#else
	return exePath / L"../Playground/Resources";
#endif
}

class Playground : public Object, public Description<Playground>
{
public:
	WString LoadBigJson()
	{
		auto jsonPath = GetPlaygroundResourceFolder() / L"BigJson.json";
		return File(jsonPath).ReadAllTextByBom();
	}
};

class PlaygroundUia : public Object, public Description<PlaygroundUia>
{
protected:
	HANDLE busyStarted;
	HANDLE busyRelease;

public:
	PlaygroundUia()
	{
		auto eventName = L"Local\\GacUI.UiaBusy." + itow(GetCurrentProcessId());
		busyStarted = CreateEvent(nullptr, TRUE, FALSE, (eventName + L".Started").Buffer());
		busyRelease = CreateEvent(nullptr, TRUE, FALSE, (eventName + L".Release").Buffer());
	}

	~PlaygroundUia()
	{
		CloseHandle(busyStarted);
		CloseHandle(busyRelease);
	}

	void WaitUntilReleased()
	{
#define ERROR_MESSAGE_PREFIX L"PlaygroundUia::WaitUntilReleased()#"
		SetEvent(busyStarted);
		CHECK_ERROR(WaitForSingleObject(busyRelease, 5000) == WAIT_OBJECT_0, ERROR_MESSAGE_PREFIX L"UIA busy fixture timed out.");
#undef ERROR_MESSAGE_PREFIX
	}

	static void SetName(GuiControl* control, const WString& name)
	{
		windows::SetWindowsUIAutomationName(control, name);
	}

	static void SetLabel(GuiControl* control, GuiControl* label)
	{
		windows::SetWindowsUIAutomationLabel(control, label);
	}

	static void SetId(GuiControl* control, const WString& id)
	{
		windows::SetWindowsUIAutomationId(control, id);
	}

	static void SetText(GuiControl* control, const WString& key, const WString& text)
	{
		windows::SetWindowsUIAutomationText(control, key, text);
	}
};

class PlaygroundIndependentGroup : public GuiSelectableButton::GroupController, public Description<PlaygroundIndependentGroup>
{
public:
	void OnSelectedChanged(GuiSelectableButton*)override
	{
	}
};

namespace vl::reflection::description
{
#define PLAYGROUND_TYPE_LIST(F)	\
	F(Playground)\
	F(PlaygroundUia)\
	F(PlaygroundIndependentGroup)\

	PLAYGROUND_TYPE_LIST(DECL_TYPE_INFO)
	PLAYGROUND_TYPE_LIST(IMPL_CPP_TYPE_INFO)

#define _ ,
	BEGIN_CLASS_MEMBER(Playground)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<Playground>(), NO_PARAMETER)
		CLASS_MEMBER_METHOD(LoadBigJson, NO_PARAMETER)
	END_CLASS_MEMBER(Playground)

	BEGIN_CLASS_MEMBER(PlaygroundUia)
		CLASS_MEMBER_CONSTRUCTOR(Ptr<PlaygroundUia>(), NO_PARAMETER)
		CLASS_MEMBER_METHOD(WaitUntilReleased, NO_PARAMETER)
		CLASS_MEMBER_STATIC_METHOD(SetName, { L"control" _ L"name" })
		CLASS_MEMBER_STATIC_METHOD(SetId, { L"control" _ L"id" })
		CLASS_MEMBER_STATIC_METHOD(SetLabel, { L"control" _ L"label" })
		CLASS_MEMBER_STATIC_METHOD(SetText, { L"control" _ L"key" _ L"text" })
	END_CLASS_MEMBER(PlaygroundUia)

	BEGIN_CLASS_MEMBER(PlaygroundIndependentGroup)
		CLASS_MEMBER_BASE(GuiSelectableButton::GroupController)
		CLASS_MEMBER_CONSTRUCTOR(PlaygroundIndependentGroup*(), NO_PARAMETER)
	END_CLASS_MEMBER(PlaygroundIndependentGroup)

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

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	if (!gacui_test::automationArguments.ParseWindowsCommandLine()) return 1;
	bool hosted = wcsstr(GetCommandLineW(), L"/UiaHosted") != nullptr;
	bool gdi = wcsstr(GetCommandLineW(), L"/UiaGdi") != nullptr;
	int result = gdi
		? (hosted ? SetupHostedWindowsGDIRenderer() : SetupWindowsGDIRenderer())
		: (hosted ? SetupHostedWindowsDirect2DRenderer() : SetupWindowsDirect2DRenderer());
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

		Ptr<INativeAutomationService> automationService;
		if (wcsstr(GetCommandLineW(), L"/UiaHosted"))
		{
			automationService = Ptr(new windows::WindowsAutomationServiceHosted);
		}
		else
		{
			automationService = Ptr(new windows::WindowsAutomationService);
		}
		GetNativeServiceSubstitution()->Substitute(automationService.Obj(), false);
		windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/Playground"), gacui_test::automationArguments.port);
		GetApplication()->Run(window);
		windows::StopWindowsHttpAutomationService();
		automationService->Stop();
		GetNativeServiceSubstitution()->Unsubstitute(automationService.Obj());
		delete window;
	}
}

void GuiMain()
{
	LoadDarkSkinTypes();
	LoadPlaygroundTypes();

	List<WString> names;
	names.Add(L"ResourceUiaReview");

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
			GetPlaygroundResourceFolder() / (name + L".xml"),
			GetPlaygroundResourceFolder().GetFolder(),
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
