#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../../../Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUIReflection.h"
#include "../../../Source/Utilities/FakeServices/TuiDialogs/Source/TuiFakeDialogServiceUIReflection.h"
#include "../../../Source/Utilities/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include "../SharedArguments.h"
#include "DarkSkinReflection.h"
#include "../Generated_FullControlTest/FullControlTestEventHandlers.h"
#include "../Generated_FullControlTest/FullControlTestRawElement.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::reflection::description;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	if (!gacui_test::automationArguments.ParseWindowsCommandLine()) return 1;
	int result = gacui_test::automationArguments.gdi ? SetupWindowsGDIRenderer() : SetupWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

extern void GuiMain_GrammarIntellisense();

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
#ifdef VCZH_64
#define FULLCONTROLTEST_BINARY	L"../GacUISrc/Generated_FullControlTest/Resource_x64/FullControlTest.UI.bin"
#else
#define FULLCONTROLTEST_BINARY	L"../GacUISrc/Generated_FullControlTest/Resource_x86/FullControlTest.UI.bin"
#endif
	LoadGuiFakeDialogServiceUITypes();
	LoadTuiFakeDialogServiceUITypes();
	LoadDarkSkinTypes();
	{
		FileStream fileStream(GetResourcePath() + FULLCONTROLTEST_BINARY, FileStream::ReadOnly);
		GetResourceManager()->LoadResourceOrPending(fileStream, GuiResourceUsage::InstanceClass);
	}
	{
		auto theme = UnboxValue<Ptr<ThemeTemplates>>(Value::Create(L"darkskin::Theme"));
		RegisterTheme(theme);
	}
	{
		auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow"));
		auto reflectedWindow = BoxValue(window);
		auto skinItems = UnboxValue<Ptr<IValueObservableList>>(reflectedWindow.GetProperty(L"SkinItems"));
		demo::InitializeFctSkinItems([&](const WString& name, const Func<void()>& action, bool selected)
		{
			Array<Value> arguments(2);
			arguments[0] = BoxValue(name);
			arguments[1] = BoxParameter(action);
			auto item = Value::Create(L"demo::FctSkinItem", arguments);
			if (selected) reflectedWindow.SetProperty(L"SelectedSkinItem", item);
			skinItems->Add(item);
		});
		for (auto name : { L"editorRibbon", L"editorToolstrip" })
		{
			auto editor = window->GetNamedObject(name);
			auto root = editor.GetRawPtr()->SafeAggregationCast<GuiInstanceRootObject>();
			auto viewer = FindObjectByName<GuiDocumentViewer>(root, L"document");
			editor.AttachEvent(L"LoadAsPrivateFormat", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName) { viewer->SetDocument(gacui_demo::LoadDocument(fileName)); })));
			editor.AttachEvent(L"SaveAsPrivateFormat", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName) { gacui_demo::SaveDocument(viewer->GetDocument(), fileName); })));
			editor.AttachEvent(L"SaveAsRTF", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName)
			{
				viewer->SelectAll();
				gacui_demo::ExportDocumentRtf(viewer->GetSelectionModel(), fileName);
			})));
			editor.AttachEvent(L"SaveAsHTML", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName)
			{
				viewer->SelectAll();
				gacui_demo::ExportDocumentHtml(viewer->GetSelectionModel(), fileName);
			})));
		}
		auto rawClockAttachment = demo::AttachFullControlTestRawElement(UnboxValue<Ptr<GuiRawElementBase>>(BoxValue(window).GetProperty(L"ClockElement")));
		auto touchingAttachment = demo::AttachFullControlTestTouchingElement(UnboxValue<Ptr<GuiRawElementBase>>(BoxValue(window).GetProperty(L"TouchingElement")));
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();

		windows::WindowsAutomationService automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/GacUI_Host"), gacui_test::automationArguments.port);
		GetApplication()->Run(window);
		windows::StopWindowsHttpAutomationService();
		automationService.Stop();
		GetNativeServiceSubstitution()->Unsubstitute(&automationService);
		rawClockAttachment = nullptr;
		touchingAttachment = nullptr;
		delete window;
	}
}
