#include "GacUI.Windows.h"
#include "DarkSkin.h"
#include "../UiaList/Source/UiaList.h"
#include "../UiaList/ViewModel/UiaListViewModel.h"
#include "../../../Test/GacUISrc/SharedArguments.h"

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;

WString SelectUserInterfaceLocale()
{
	ULONG count = 0;
	ULONG length = 0;
	if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &length))
	{
		throw Exception(L"GetUserPreferredUILanguages: " + itow(GetLastError()));
	}
	collections::Array<wchar_t> languages(length);
	if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, &languages[0], &length))
	{
		throw Exception(L"GetUserPreferredUILanguages: " + itow(GetLastError()));
	}
	for (auto p = &languages[0]; *p; p += wcslen(p) + 1)
	{
		auto name = wlower(WString(p));
		if (name == L"en" || (name.Length() >= 3 && name.Left(3) == L"en-")) return L"en-US";
		if (name == L"zh" || (name.Length() >= 3 && name.Left(3) == L"zh-")) return L"zh-CN";
		if (name == L"ja" || (name.Length() >= 3 && name.Left(3) == L"ja-")) return L"ja-JP";
	}
	return L"en-US";
}

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	auto localeName = SelectUserInterfaceLocale();
	GetApplication()->SetLocale(Locale(localeName));
	auto model = Ptr(new uialist::UiaListViewModel(uialist::Strings::Get(Locale(localeName)), localeName));
	{
		uialist::MainWindow window(model);
		window.ForceCalculateSizeImmediately();
		window.MoveToScreenCenter();
#ifdef _DEBUG
		windows::WindowsAutomationService automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		try
		{
			windows::StartWindowsHttpAutomationService(L"Automation/UiaListApp", gacui_test::automationArguments.port);
		}
		catch (const Error& error)
		{
			auto message = WString(L"StartWindowsHttpAutomationService: http://localhost:") + itow(gacui_test::automationArguments.port) + L"/Automation/UiaListApp/\r\n" + error.Description();
			OutputDebugStringW(message.Buffer());
			MessageBoxW(nullptr, message.Buffer(), L"UiaList", MB_OK | MB_ICONERROR);
			ExitProcess(1);
		}
#endif
		GetApplication()->Run(&window);
		model->RequestClose();
#ifdef _DEBUG
		windows::StopWindowsHttpAutomationService();
		automationService.Stop();
		GetNativeServiceSubstitution()->Unsubstitute(&automationService);
#endif
	}
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (!gacui_test::automationArguments.ParseWindowsCommandLine()) return 1;
	int result = 1;
	try
	{
		result = SetupWindowsDirect2DRenderer();
	}
	catch (const Exception& error)
	{
		OutputDebugStringW(error.Message().Buffer());
		MessageBoxW(nullptr, error.Message().Buffer(), L"UiaList", MB_OK | MB_ICONERROR);
		return 1;
	}
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
