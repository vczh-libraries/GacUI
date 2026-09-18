#include <Windows.h>
#include <CommCtrl.h>
#include <cstdio>
#include <UIAutomation.h>
#include <dwmapi.h>
#include <vector>
#include "Synthetic.h"

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Dwmapi.lib")

struct Fixture
{
	HWND window = nullptr;
	HWND stress = nullptr;
	std::vector<HWND> navigation;
	HANDLE log = INVALID_HANDLE_VALUE;
	int invoked = 0;

	void Record(const wchar_t* operation, int id, int code)
	{
		wchar_t line[512];
		auto length = swprintf_s(line, L"%llu\tPID=%lu\ttarget=%i\t%s\tcode=%i\tinvoked=%i\r\n", GetTickCount64(), GetCurrentProcessId(), id, operation, code, invoked);
		DWORD written = 0;
		if (!WriteFile(log, line, static_cast<DWORD>(length * sizeof(wchar_t)), &written, nullptr)) RaiseFailFastException(nullptr, nullptr, 0);
		FlushFileBuffers(log);
	}

	void CreateNavigationWindows()
	{
		for (auto handle : navigation) if (IsWindow(handle)) return;
		navigation.clear();
		WNDCLASSW type = {};
		type.hInstance = GetModuleHandleW(nullptr);
		type.lpszClassName = L"UiaNavigationFixture";
		type.lpfnWndProc = DefWindowProcW;
		type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		RegisterClassW(&type);
		const wchar_t* titles[] = { L"Duplicate navigation title", L"Duplicate navigation title", L"", L"Owned navigation dialog", L"Hidden navigation window", L"Cloaked navigation window", L"Minimized navigation window", L"Empty navigation bounds" };
		for (int i = 0; i < 8; i++)
		{
			auto handle = CreateWindowExW(0, type.lpszClassName, titles[i], i == 7 ? WS_POPUP : WS_OVERLAPPEDWINDOW, 120 + 20 * i, 120 + 20 * i, i == 7 ? 0 : 300, i == 7 ? 0 : 150, i == 3 ? window : nullptr, nullptr, type.hInstance, nullptr);
			navigation.push_back(handle);
			if (i != 4) ShowWindow(handle, i == 6 ? SW_SHOWMINNOACTIVE : SW_SHOWNOACTIVATE);
			if (i == 5) { BOOL cloak = TRUE; DwmSetWindowAttribute(handle, DWMWA_CLOAK, &cloak, sizeof(cloak)); }
		}
	}

	void CreateChildren()
	{
		auto create = [this](const wchar_t* kind, const wchar_t* text, DWORD style, int id, int x, int y, int width, int height)
		{
			return CreateWindowExW(0, kind, text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | style, x, y, width, height, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
		};
		create(L"BUTTON", L"Invoke counter", BS_PUSHBUTTON, 101, 20, 45, 180, 30);
		create(L"BUTTON", L"Toggle state", BS_AUTOCHECKBOX, 102, 220, 45, 180, 30);
		create(L"EDIT", L"Single line 日本語 中文", WS_BORDER | ES_AUTOHSCROLL, 103, 20, 95, 600, 30);
		create(L"EDIT", L"First line\r\n\r\n最後の行\r\n", WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 104, 20, 145, 600, 130);
		auto combo = create(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL, 105, 20, 300, 180, 150);
		SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"First"));
		SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Second"));
		SendMessageW(combo, CB_SETCURSEL, 0, 0);
		auto list = create(L"LISTBOX", L"", WS_BORDER | LBS_EXTENDEDSEL | LBS_NOTIFY, 106, 220, 300, 180, 140);
		SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Alpha"));
		SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Beta"));
		auto slider = create(TRACKBAR_CLASSW, L"Range value", TBS_AUTOTICKS, 107, 420, 300, 200, 40);
		SendMessageW(slider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
		SendMessageW(slider, TBM_SETPOS, TRUE, 25);
		create(L"STATIC", L"Bounds oracle: client origin (0,0), controls have fixed physical coordinates.", 0, 108, 20, 465, 640, 40);
		create(L"BUTTON", L"Open large/deep raw tree", BS_PUSHBUTTON, 109, 20, 410, 180, 30);
		create(L"BUTTON", L"Open navigation windows", BS_PUSHBUTTON, 110, 220, 410, 180, 30);
	}
};

LRESULT CALLBACK FixtureProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto fixture = reinterpret_cast<Fixture*>(GetWindowLongPtrW(window, GWLP_USERDATA));
	if (message == WM_NCCREATE)
	{
		fixture = static_cast<Fixture*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
		fixture->window = window;
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(fixture));
	}
	if (!fixture) return DefWindowProcW(window, message, wParam, lParam);
	switch (message)
	{
	case WM_CREATE: fixture->CreateChildren(); return 0;
	case WM_COMMAND:
		if (LOWORD(wParam) == 110 && HIWORD(wParam) == BN_CLICKED) fixture->CreateNavigationWindows();
		if (LOWORD(wParam) == 109 && HIWORD(wParam) == BN_CLICKED && !IsWindow(fixture->stress)) fixture->stress = CreateSyntheticFixture(GetModuleHandleW(nullptr), true);
		if (LOWORD(wParam) == 101 && HIWORD(wParam) == BN_CLICKED) ++fixture->invoked;
		fixture->Record(L"WM_COMMAND", LOWORD(wParam), HIWORD(wParam));
		return 0;
	case WM_HSCROLL: fixture->Record(L"WM_HSCROLL", GetDlgCtrlID(reinterpret_cast<HWND>(lParam)), LOWORD(wParam)); return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT paint;
			auto dc = BeginPaint(window, &paint);
			RECT bounds;
			GetClientRect(window, &bounds);
			SetBkMode(dc, TRANSPARENT);
			TextOutW(dc, 4, 4, L"TL (0,0)", 8);
			TextOutW(dc, bounds.right - 80, 4, L"TR", 2);
			TextOutW(dc, 4, bounds.bottom - 20, L"BL", 2);
			TextOutW(dc, bounds.right - 40, bounds.bottom - 20, L"BR", 2);
			EndPaint(window, &paint);
			return 0;
		}
	case WM_DESTROY:
		for (auto handle : fixture->navigation) if (IsWindow(handle)) DestroyWindow(handle);
		DestroySyntheticFixture(fixture->stress); fixture->Record(L"WM_DESTROY", 0, 0); PostQuitMessage(0); return 0;
	default: return DefWindowProcW(window, message, wParam, lParam);
	}
}

int ProbeLegacy(DWORD targetProcess)
{
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) return 1;
	struct Target { DWORD process; HWND window = nullptr; } target{ targetProcess };
	EnumWindows([](HWND window, LPARAM argument) -> BOOL
	{
		auto target = reinterpret_cast<Target*>(argument);
		DWORD process = 0;
		GetWindowThreadProcessId(window, &process);
		wchar_t name[100];
		GetClassNameW(window, name, 100);
		if (process == target->process && wcscmp(name, L"UiaFixture") == 0) target->window = window;
		return TRUE;
	}, reinterpret_cast<LPARAM>(&target));
	if (!target.window) return 2;
	IUIAutomation* automation = nullptr;
	if (FAILED(CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&automation)))) return 3;
	wchar_t path[100];
	swprintf_s(path, L"LegacyProbe-%lu.txt", targetProcess);
	FILE* log = nullptr;
	_wfopen_s(&log, path, L"w, ccs=UTF-8");
	if (!log) return 4;
	int failures = 0;
	const int controls[] = {101, 108};
	for (int id : controls)
	{
		IUIAutomationElement* element = nullptr;
		auto hr = automation->ElementFromHandle(GetDlgItem(target.window, id), &element);
		IUIAutomationLegacyIAccessiblePattern* legacy = nullptr;
		if (SUCCEEDED(hr)) hr = element->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId, IID_PPV_ARGS(&legacy));
		BSTR action = nullptr;
		DWORD state = 0;
		if (SUCCEEDED(hr)) hr = legacy->get_CurrentDefaultAction(&action);
		if (SUCCEEDED(hr)) hr = legacy->get_CurrentState(&state);
		if (FAILED(hr) || (id == 101 ? !action || !SysStringLen(action) : action && SysStringLen(action))) failures++;
		if (SUCCEEDED(hr)) hr = legacy->DoDefaultAction();
		fwprintf(log, L"target=%d defaultAction=%s state=0x%08lX DoDefaultAction=0x%08lX\n", id, action ? action : L"<null>", state, static_cast<unsigned long>(hr));
		if (id == 101 && FAILED(hr)) failures++;
		SysFreeString(action);
		if (legacy) legacy->Release();
		if (element) element->Release();
	}
	fclose(log);
	automation->Release();
	CoUninitialize();
	return failures ? 5 : 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR arguments, int show)
{
	unsigned long targetProcess = 0;
	if (sscanf_s(arguments, "/LegacyProbe:%lu", &targetProcess) == 1) return ProbeLegacy(targetProcess);
	if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	INITCOMMONCONTROLSEX common = {sizeof(common), ICC_WIN95_CLASSES};
	InitCommonControlsEx(&common);
	Fixture fixture;
	wchar_t logName[100];
	swprintf_s(logName, L"UiaFixture-%lu.calls.txt", GetCurrentProcessId());
	fixture.log = CreateFileW(logName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fixture.log == INVALID_HANDLE_VALUE) return 1;
	WNDCLASSW type = {};
	type.hInstance = instance;
	type.lpszClassName = L"UiaFixture";
	type.lpfnWndProc = FixtureProc;
	type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	type.hCursor = LoadCursor(nullptr, IDC_ARROW);
	if (!RegisterClassW(&type)) return 2;
	auto window = CreateWindowExW(0, type.lpszClassName, L"UiaList verification — native controls", WS_OVERLAPPEDWINDOW, 80, 80, 700, 600, nullptr, nullptr, instance, &fixture);
	if (!window) return 3;
	ShowWindow(window, show);
	auto synthetic = CreateSyntheticFixture(instance);
	MSG message;
	while (GetMessageW(&message, nullptr, 0, 0) > 0)
	{
		if (!IsDialogMessageW(window, &message)) { TranslateMessage(&message); DispatchMessageW(&message); }
	}
	DestroySyntheticFixture(synthetic);
	CloseHandle(fixture.log);
	CoUninitialize();
	return static_cast<int>(message.wParam);
}
