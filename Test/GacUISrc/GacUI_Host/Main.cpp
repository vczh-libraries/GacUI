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
		BoxValue(window).AttachEvent(L"PaletteSelected", BoxParameter(Func<void(vint)>(&demo::OnPaletteSelected)));
		for (auto name : { L"editorRibbon", L"editorToolstrip" })
		{
			auto editor = window->GetNamedObject(name);
			auto root = editor.GetRawPtr()->SafeAggregationCast<GuiInstanceRootObject>();
			auto viewer = FindObjectByName<GuiDocumentViewer>(root, L"document");
			editor.AttachEvent(L"LoadAsPrivateFormat", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName) { demo::LoadFctDocument(viewer, fileName); })));
			editor.AttachEvent(L"SaveAsPrivateFormat", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName) { demo::SaveFctDocument(viewer, fileName); })));
			editor.AttachEvent(L"SaveAsRTF", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName) { demo::ExportFctDocumentRtf(viewer, fileName); })));
			editor.AttachEvent(L"SaveAsHTML", BoxParameter(Func<void(const WString&)>([viewer](const WString& fileName) { demo::ExportFctDocumentHtml(viewer, fileName); })));
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
