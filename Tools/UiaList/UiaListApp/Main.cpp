#include "GacUI.Windows.h"
#undef GetRoleText
#include "DarkSkin.h"
#include "../UiaList/Source/UiaList.h"
#include "../UiaList/ViewModel/UiaListViewModel.h"
#include <Shellapi.h>
#pragma comment(lib, "Shell32.lib")

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;

class AutomationArguments
{
	bool specified = false;
public:
	vint port = 8888;

	// 0: another option; 1: consumed; -1: invalid automation option.
	vint Consume(const WString& argument)
	{
		if (argument.Length() < 7 || argument.Left(7) != L"/AsPort") return 0;
		if (specified || argument.Length() <= 8 || argument[7] != L':') return -1;
		vint value = 0;
		for (vint i = 8; i < argument.Length(); i++)
		{
			if (argument[i] < L'0' || argument[i] > L'9') return -1;
			value = value * 10 + argument[i] - L'0';
			if (value > 65535) return -1;
		}
		if (!value) return -1;
		specified = true;
		port = value;
		return 1;
	}

	bool ParseWindowsCommandLine()
	{
		int count = 0;
		auto arguments = CommandLineToArgvW(GetCommandLineW(), &count);
		if (!arguments) return false;
		bool valid = true;
		for (int i = 1; i < count; i++) if (Consume(WString(arguments[i])) < 0) valid = false;
		LocalFree(arguments);
		return valid;
	}
};

vint automationPort = 0;

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
			windows::StartWindowsHttpAutomationService(L"Automation/UiaListApp", automationPort);
		}
		catch (const Error& error)
		{
			auto message = WString(L"StartWindowsHttpAutomationService: http://localhost:") + itow(automationPort) + L"/Automation/UiaListApp/\r\n" + error.Description();
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
	AutomationArguments automationArguments;
	if (!automationArguments.ParseWindowsCommandLine()) return 1;
	automationPort = automationArguments.port;
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
