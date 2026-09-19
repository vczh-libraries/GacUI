#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "DarkSkinReflection.h"
#include "../GacUI_Compiler/ResourceCompiler.h"
#include "../../../Source/GacUI.h"
#include "../../../Source/Utilities/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include "../SharedArguments.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
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
	if (!gacui_test::automationArguments.ParseWindowsCommandLine()) return 1;
	int result = wcsstr(GetCommandLineW(), L"/UiaHosted") ? SetupHostedWindowsDirect2DRenderer() : SetupWindowsDirect2DRenderer();
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

		windows::WindowsAutomationService automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/Playground"), gacui_test::automationArguments.port);
		GetApplication()->Run(window);
		windows::StopWindowsHttpAutomationService();
		automationService.Stop();
		GetNativeServiceSubstitution()->Unsubstitute(&automationService);
		delete window;
	}
}

void RunUiaReview()
{
	RegisterTheme(Ptr(new darkskin::Theme));
	GuiWindow window(theme::ThemeName::Window);
	window.SetText(L"UIA Review Fixture");
	window.SetClientSize(Size(700, 420));
	auto add = [&](GuiControl* control, Rect bounds)
	{
		control->GetBoundsComposition()->SetExpectedBounds(bounds);
		window.GetContainerComposition()->AddChild(control->GetBoundsComposition());
	};
	auto edit = new GuiSinglelineTextBox(theme::ThemeName::SinglelineTextBox);
	edit->SetText(L"public-before-password");
	add(edit, Rect(10, 10, 450, 45));
	auto button = [&](const WString& name, vint x, vint y, const Func<void()>& action)
	{
		auto control = new GuiButton(theme::ThemeName::Button);
		control->SetText(name);
		control->Clicked.AttachLambda([action](GuiGraphicsComposition*, GuiEventArgs&) { action(); });
		add(control, Rect(x, y, x + 140, y + 35));
	};
	button(L"Password on", 10, 60, [=]() { edit->SetPasswordChar(L'*'); });
	button(L"Password off", 160, 60, [=]() { edit->SetPasswordChar(0); });
	button(L"Read only", 310, 60, [=]() { edit->SetEditMode(GuiDocumentEditMode::Selectable); });
	button(L"Editable", 460, 60, [=]() { edit->SetEditMode(GuiDocumentEditMode::Editable); });
	button(L"Disabled", 10, 105, [=]() { edit->SetEnabled(false); });
	button(L"Enabled", 160, 105, [=]() { edit->SetEnabled(true); });
	auto list = new GuiTextList(theme::ThemeName::TextList);
	list->SetMultiSelect(true);
	for (vint i = 0; i < 3; i++) list->GetItems().Add(Ptr(new list::TextItem(itow(i))));
	add(list, Rect(10, 160, 250, 310));
	button(L"Select second", 270, 160, [=]() { list->SetSelected(1, true); });
	button(L"Remove first", 270, 205, [=]() { list->SetSelected(0, false); });
	button(L"Clear selection", 270, 250, [=]() { list->ClearSelection(); });
	class IndependentGroup : public GuiSelectableButton::GroupController
	{
	public:
		void OnSelectedChanged(GuiSelectableButton*)override {}
	};
	auto group = new IndependentGroup;
	window.AddComponent(group);
	for (vint i = 0; i < 2; i++)
	{
		auto check = new GuiSelectableButton(theme::ThemeName::CheckBox);
		check->SetText(L"Independent " + itow(i));
		check->SetGroupController(group);
		add(check, Rect(470, 160 + i * 45, 670, 195 + i * 45));
	}
	auto progress = new GuiScroll(theme::ThemeName::ProgressBar);
	progress->SetTotalSize(100);
	progress->SetPosition(40);
	add(progress, Rect(10, 330, 450, 360));
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
	if (wcsstr(GetCommandLineW(), L"/UiaHosted"))
	{
		windows::WindowsAutomationServiceHosted automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		windows::StartWindowsHttpAutomationService(L"Automation/Playground", gacui_test::automationArguments.port);
		GetApplication()->Run(&window);
		windows::StopWindowsHttpAutomationService();
		automationService.Stop();
		GetNativeServiceSubstitution()->Unsubstitute(&automationService);
	}
	else
	{
		windows::WindowsAutomationService automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		windows::StartWindowsHttpAutomationService(L"Automation/Playground", gacui_test::automationArguments.port);
		GetApplication()->Run(&window);
		windows::StopWindowsHttpAutomationService();
		automationService.Stop();
		GetNativeServiceSubstitution()->Unsubstitute(&automationService);
	}
}

void GuiMain()
{
	LoadDarkSkinTypes();
	LoadPlaygroundTypes();
	if (wcsstr(GetCommandLineW(), L"/UiaReview"))
	{
		RunUiaReview();
		return;
	}

	List<WString> names;
	names.Add(L"ResourceDocument");

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
