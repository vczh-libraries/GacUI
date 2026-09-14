#ifndef VCZH_UIALIST_PROCESSDISCOVERY_WINDOWS
#define VCZH_UIALIST_PROCESSDISCOVERY_WINDOWS

#include "GacUI.h"
#include <Windows.h>

namespace uialist::native
{
	struct WindowIdentity
	{
		HWND						handle = nullptr;
		DWORD						processId = 0;
		auto operator<=>(const WindowIdentity&) const = default;
	};

	struct WindowRecord
	{
		WindowIdentity				identity;
		vl::WString					title;
		vl::WString					className;
		bool						visible = false;
		bool						cloaked = false;
		bool						minimized = false;
		bool						qualifies = false;
	};

	struct ProcessRecord
	{
		DWORD						processId = 0;
		DWORD						parentId = 0;
		vl::WString					executable;
		vl::Nullable<ULONGLONG>		creationTime;
		vl::collections::List<WindowRecord> windows;
		vl::collections::List<vl::vint> children;
		bool						hasVisibleUI = false;
	};

	struct ProcessSnapshot
	{
		vl::collections::List<ProcessRecord> processes;
		vl::collections::List<vl::vint> roots;
	};

	extern vl::Ptr<ProcessSnapshot> DiscoverProcesses();
	extern bool IsCurrentWindow(WindowIdentity identity);
}

#endif
