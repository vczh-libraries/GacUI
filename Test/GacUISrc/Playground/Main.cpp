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
	GuiWindow secondary(theme::ThemeName::Window);
	secondary.SetText(L"Review secondary");
	secondary.SetClientSize(Size(250, 140));
	GuiPopup plainPopup(theme::ThemeName::Window);
	plainPopup.SetText(L"Review plain popup");
	plainPopup.SetClientSize(Size(200, 100));
	auto eventName = L"Local\\GacUI.UiaBusy." + itow(GetCurrentProcessId());
	HANDLE busyStarted = CreateEvent(nullptr, TRUE, FALSE, (eventName + L".Started").Buffer());
	HANDLE busyRelease = CreateEvent(nullptr, TRUE, FALSE, (eventName + L".Release").Buffer());
	if (wcsstr(GetCommandLineW(), L"/UiaReview2"))
	{
		window.SetClientSize(Size(1050, 760));
		auto label = new GuiLabel(theme::ThemeName::Label);
		label->SetText(L"Account name");
		add(label, Rect(470, 10, 670, 45));
		windows::SetWindowsUIAutomationLabel(edit, label);
		auto tooltip = new GuiLabel(theme::ThemeName::Label);
		tooltip->SetText(L"Your account identifier");
		edit->SetTooltipControl(tooltip);
		button(L"Show tooltip", 470, 330, [=]() { edit->DisplayTooltip(Point(10, 10)); });
		button(L"Hide tooltip", 620, 330, [=]() { edit->CloseTooltip(); });
		auto scroll = new GuiScrollContainer(theme::ThemeName::ScrollView);
		windows::SetWindowsUIAutomationName(scroll, L"Review scroll");
		add(scroll, Rect(700, 10, 1020, 150));
		auto offscreen = new GuiButton(theme::ThemeName::Button);
		offscreen->SetText(L"Far button");
		offscreen->GetBoundsComposition()->SetExpectedBounds(Rect(600, 350, 740, 385));
		scroll->GetContainerComposition()->AddChild(offscreen->GetBoundsComposition());
		auto groupA = new GuiSelectableButton::MutexGroupController;
		auto groupB = new GuiSelectableButton::MutexGroupController;
		window.AddComponent(groupA);
		window.AddComponent(groupB);
		auto radioParent = new GuiCustomControl(theme::ThemeName::CustomControl);
		add(radioParent, Rect(700, 230, 1020, 275));
		for (vint i = 0; i < 4; i++)
		{
			auto radio = new GuiSelectableButton(theme::ThemeName::RadioButton);
			radio->SetText(L"Radio " + itow(i));
			radio->SetGroupController(i < 2 ? groupA : groupB);
			if (i == 1)
			{
				radio->GetBoundsComposition()->SetExpectedBounds(Rect(0, 0, 140, 35));
				radioParent->GetContainerComposition()->AddChild(radio->GetBoundsComposition());
			}
			else add(radio, Rect(700 + (i % 2) * 150, 160 + (i / 2) * 35, 840 + (i % 2) * 150, 195 + (i / 2) * 35));
			if (i % 2 == 0) radio->SetSelected(true);
		}
		auto document = new GuiDocumentLabel(theme::ThemeName::DocumentLabel);
		document->SetEditMode(GuiDocumentEditMode::Selectable);
		windows::SetWindowsUIAutomationName(document, L"Review objects");
		add(document, Rect(10, 385, 650, 460));
		auto model = Ptr(new DocumentModel);
		auto paragraph = Ptr(new DocumentParagraphRun);
		model->paragraphs.Add(paragraph);
		auto link = Ptr(new DocumentHyperlinkRun);
		link->reference = L"review://link";
		auto linkText = Ptr(new DocumentTextRun);
		linkText->text = L"Review link";
		link->runs.Add(linkText);
		paragraph->runs.Add(link);
		for (vint i = 0; i < 2; i++)
		{
			auto image = Ptr(new DocumentImageRun);
			image->source = L"Review image " + itow(i);
			image->sizeOverride = Size(35, 35);
			paragraph->runs.Add(image);
		}
		document->SetDocument(model);
		document->ActiveHyperlinkExecuted.AttachLambda([document, label](GuiGraphicsComposition*, GuiEventArgs&) { label->SetText(document->GetActiveHyperlinkReference()); });
		button(L"Replace document", 10, 480, [=]() { document->SetDocument(model); });
		button(L"Update paragraph", 160, 480, [=]()
		{
			auto text = Ptr(new DocumentTextRun);
			text->text = L"updated";
			document->GetDocument()->paragraphs[0]->runs.Add(text);
			document->NotifyParagraphUpdated(0, 1, 1, true);
		});
		button(L"Open secondary", 310, 480, [&]() { secondary.ShowWithOwner(&window); });
		button(L"Disable secondary", 460, 480, [&]() { secondary.SetEnabled(false); });
		button(L"Enable secondary", 610, 480, [&]() { secondary.SetEnabled(true); });
		button(L"Busy", 760, 480, [=]() { SetEvent(busyStarted); CHECK_ERROR(WaitForSingleObject(busyRelease, 5000) == WAIT_OBJECT_0, L"UIA busy fixture timed out."); });
		auto views = new GuiListView(theme::ThemeName::ListView);
		add(views, Rect(700, 385, 1020, 460));
		windows::SetWindowsUIAutomationText(views, L"BigIcon", L"Grandes icones");
		windows::SetWindowsUIAutomationText(views, L"ColumnHeaders", L"En-tetes");
		auto comboLabel = new GuiLabel(theme::ThemeName::Label);
		comboLabel->SetText(L"Review choices");
		add(comboLabel, Rect(10, 540, 160, 570));
		auto comboList = new GuiTextList(theme::ThemeName::TextList);
		comboList->GetItems().Add(Ptr(new list::TextItem(L"First choice")));
		comboList->GetItems().Add(Ptr(new list::TextItem(L"Second choice")));
		auto combo = new GuiComboBoxListControl(theme::ThemeName::ComboBox, comboList);
		combo->SetSelectedIndex(0);
		windows::SetWindowsUIAutomationLabel(combo, comboLabel);
		add(combo, Rect(170, 540, 350, 575));
		auto namedEdit = new GuiSinglelineTextBox(theme::ThemeName::SinglelineTextBox);
		windows::SetWindowsUIAutomationName(namedEdit, L"Explicit editor name");
		add(namedEdit, Rect(370, 540, 650, 575));
		auto interactiveTooltip = new GuiButton(theme::ThemeName::Button);
		interactiveTooltip->SetText(L"Tooltip command");
		namedEdit->SetTooltipControl(interactiveTooltip);
		button(L"Interactive tooltip", 10, 590, [=]() { namedEdit->DisplayTooltip(Point(10, 10)); });
		button(L"Hide interactive", 160, 590, [=]() { namedEdit->CloseTooltip(); });
		button(L"Plain popup", 310, 590, [&]() { plainPopup.ShowPopup(&window, Point(200, 100)); });
		button(L"Hide popup", 460, 590, [&]() { plainPopup.Hide(); });
		button(L"Open modal", 610, 590, [&]() { secondary.ShowModal(&window, []() {}); });
		auto nestedDocument = new GuiDocumentLabel(theme::ThemeName::DocumentLabel);
		windows::SetWindowsUIAutomationName(nestedDocument, L"Nested review objects");
		add(nestedDocument, Rect(10, 650, 650, 725));
		auto nestedModel = Ptr(new DocumentModel);
		auto nestedParagraph = Ptr(new DocumentParagraphRun);
		auto nestedLink = Ptr(new DocumentHyperlinkRun);
		nestedLink->reference = L"review://nested";
		auto nestedText = Ptr(new DocumentTextRun);
		nestedText->text = L"Nested link ";
		nestedLink->runs.Add(nestedText);
		auto nestedImage = Ptr(new DocumentImageRun);
		nestedImage->source = L"Nested image";
		nestedImage->sizeOverride = Size(35, 35);
		nestedLink->runs.Add(nestedImage);
		auto embedded = Ptr(new DocumentEmbeddedObjectRun);
		embedded->name = L"nested-button";
		nestedLink->runs.Add(embedded);
		nestedParagraph->runs.Add(nestedLink);
		nestedModel->paragraphs.Add(nestedParagraph);
		auto nestedItem = Ptr(new GuiDocumentItem(embedded->name));
		auto nestedButton = new GuiButton(theme::ThemeName::Button);
		nestedButton->SetText(L"Nested button");
		nestedButton->GetBoundsComposition()->SetPreferredMinSize(Size(120, 35));
		nestedItem->GetContainer()->AddChild(nestedButton->GetBoundsComposition());
		nestedDocument->AddDocumentItem(nestedItem);
		nestedDocument->SetDocument(nestedModel);
	}
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
	CloseHandle(busyStarted);
	CloseHandle(busyRelease);
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
