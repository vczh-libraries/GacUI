#include "ProcessDiscovery.Windows.h"
#include <TlHelp32.h>
#include <dwmapi.h>

using namespace vl;
using namespace vl::collections;

namespace uialist::native
{
	bool IsCurrentWindow(WindowIdentity identity)
	{
		DWORD processId = 0;
		return IsWindow(identity.handle) && GetWindowThreadProcessId(identity.handle, &processId) && processId == identity.processId;
	}

	Ptr<ProcessSnapshot> DiscoverProcesses()
	{
		auto snapshot = Ptr(new ProcessSnapshot);
		auto handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (handle == INVALID_HANDLE_VALUE) throw Exception(L"CreateToolhelp32Snapshot: " + utow(GetLastError()));
		PROCESSENTRY32W entry = { sizeof(entry) };
		auto found = Process32FirstW(handle, &entry);
		while (found)
		{
			ProcessRecord record;
			record.processId = entry.th32ProcessID;
			record.parentId = entry.th32ParentProcessID;
			record.executable = entry.szExeFile;
			if (auto process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, record.processId))
			{
				FILETIME created, exited, kernel, user;
				if (GetProcessTimes(process, &created, &exited, &kernel, &user))
				{
					record.creationTime = (static_cast<ULONGLONG>(created.dwHighDateTime) << 32) | created.dwLowDateTime;
				}
				CloseHandle(process);
			}
			snapshot->processes.Add(std::move(record));
			found = Process32NextW(handle, &entry);
		}
		auto lastError = GetLastError();
		CloseHandle(handle);
		if (lastError != ERROR_NO_MORE_FILES) throw Exception(L"Process32NextW: " + utow(lastError));
		Dictionary<DWORD, vint> indexes;
		for (vint i = 0; i < snapshot->processes.Count(); i++) indexes.Add(snapshot->processes[i].processId, i);
		struct EnumContext
		{
			ProcessSnapshot* snapshot;
			Dictionary<DWORD, vint>* indexes;
		} context{ snapshot.Obj(), &indexes };
		SetLastError(ERROR_SUCCESS);
		auto enumerated = EnumWindows([](HWND window, LPARAM argument) -> BOOL
		{
			auto& context = *reinterpret_cast<EnumContext*>(argument);
			WindowRecord record;
			record.identity.handle = window;
			GetWindowThreadProcessId(window, &record.identity.processId);
			auto index = context.indexes->Keys().IndexOf(record.identity.processId);
			if (index == -1) return TRUE;
			Array<wchar_t> title(GetWindowTextLengthW(window) + 1);
			auto copied = GetWindowTextW(window, &title[0], static_cast<int>(title.Count()));
			record.title = WString::CopyFrom(&title[0], copied);
			wchar_t className[256] = {};
			GetClassNameW(window, className, 256);
			record.className = className;
			record.visible = IsWindowVisible(window) != FALSE;
			record.minimized = IsIconic(window) != FALSE;
			DWORD cloaked = 0;
			DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
			record.cloaked = cloaked != 0;
			RECT bounds = {};
			record.qualifies = record.visible && !record.cloaked && GetWindowRect(window, &bounds) && bounds.right > bounds.left && bounds.bottom > bounds.top;
			context.snapshot->processes[context.indexes->Values()[index]].windows.Add(std::move(record));
			return TRUE;
		}, reinterpret_cast<LPARAM>(&context));
		if (!enumerated) throw Exception(L"EnumWindows: " + utow(GetLastError()));

		// Resolve the complete forest before pruning, including PID reuse and malformed cycles.
		Array<vint> parents(snapshot->processes.Count());
		for (vint i = 0; i < parents.Count(); i++)
		{
			auto& child = snapshot->processes[i];
			auto index = indexes.Keys().IndexOf(child.parentId);
			parents[i] = index == -1 ? -1 : indexes.Values()[index];
			if (parents[i] == i) parents[i] = -1;
			if (parents[i] != -1)
			{
				auto& parent = snapshot->processes[parents[i]];
				if (parent.creationTime && child.creationTime && parent.creationTime.Value() > child.creationTime.Value()) parents[i] = -1;
			}
		}
		Array<vint> marks(parents.Count());
		for (vint i = 0; i < marks.Count(); i++) marks[i] = -1;
		for (vint i = 0; i < parents.Count(); i++)
		{
			auto current = i;
			while (current != -1 && marks[current] == -1)
			{
				marks[current] = i;
				current = parents[current];
			}
			if (current != -1 && marks[current] == i) parents[current] = -1;
		}
		for (vint i = 0; i < parents.Count(); i++)
		{
			if (parents[i] == -1) snapshot->roots.Add(i);
			else snapshot->processes[parents[i]].children.Add(i);
		}
		List<vint> preorder;
		CopyFrom(preorder, snapshot->roots);
		for (vint i = 0; i < preorder.Count(); i++)
		{
			for (auto child : snapshot->processes[preorder[i]].children) preorder.Add(child);
		}
		for (vint i = preorder.Count(); i > 0; i--)
		{
			auto index = preorder[i - 1];
			auto& process = snapshot->processes[index];
			for (auto&& window : process.windows) process.hasVisibleUI |= window.qualifies;
			if (process.hasVisibleUI && parents[index] != -1) snapshot->processes[parents[index]].hasVisibleUI = true;
		}
		auto sortProcesses = [&](List<vint>& values)
		{
			auto ordered = From(values).Where([&](vint i) { return snapshot->processes[i].hasVisibleUI; }).OrderBy([&](vint a, vint b)
			{
				auto order = snapshot->processes[a].executable <=> snapshot->processes[b].executable;
				return order != 0 ? order : snapshot->processes[a].processId <=> snapshot->processes[b].processId;
			}).Evaluate();
			CopyFrom(values, ordered);
		};
		sortProcesses(snapshot->roots);
		for (vint i = 0; i < snapshot->processes.Count(); i++)
		{
			auto& process = snapshot->processes[i];
			sortProcesses(process.children);
			if (process.windows.Count() > 1) Sort(&process.windows[0], process.windows.Count(), [](const WindowRecord& a, const WindowRecord& b)
			{
				auto order = a.title <=> b.title;
				return order != 0 ? order : reinterpret_cast<UINT_PTR>(a.identity.handle) <=> reinterpret_cast<UINT_PTR>(b.identity.handle);
			});
		}
		return snapshot;
	}
}
