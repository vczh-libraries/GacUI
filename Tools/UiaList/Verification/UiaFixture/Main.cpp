#include <Windows.h>
#include <CommCtrl.h>
#include <cstdio>
#include "Synthetic.h"

#pragma comment(lib, "Comctl32.lib")

struct Fixture
{
	HWND window = nullptr;
	HWND stress = nullptr;
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
	case WM_DESTROY: DestroySyntheticFixture(fixture->stress); fixture->Record(L"WM_DESTROY", 0, 0); PostQuitMessage(0); return 0;
	default: return DefWindowProcW(window, message, wParam, lParam);
	}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show)
{
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
