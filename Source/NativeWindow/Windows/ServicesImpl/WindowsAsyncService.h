/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Windows Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSASYNCSERVICE
#define VCZH_PRESENTATION_WINDOWS_SERVICESIMPL_WINDOWSASYNCSERVICE

#include "..\..\GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			class WindowsAsyncService : public INativeAsyncService
			{
			protected:
				struct TaskItem
				{
					Semaphore*							semaphore;
					Func<void()>						proc;

					TaskItem();
					TaskItem(Semaphore* _semaphore, const Func<void()>& _proc);
					~TaskItem();
				};

				class DelayItem : public Object, public INativeDelay
				{
				public:
					DelayItem(WindowsAsyncService* _service, const Func<void()>& _proc, bool _executeInMainThread, vint milliseconds);
					~DelayItem();

					WindowsAsyncService*				service;
					Func<void()>						proc;
					ExecuteStatus						status;
					DateTime							executeTime;
					bool								executeInMainThread;

					ExecuteStatus						GetStatus()override;
					bool								Delay(vint milliseconds)override;
					bool								Cancel()override;
				};
			protected:
				vint									mainThreadId;
				SpinLock								taskListLock;
				collections::List<TaskItem>				taskItems;
				collections::List<Ptr<DelayItem>>		delayItems;
			public:
				WindowsAsyncService();
				~WindowsAsyncService();

				void									ExecuteAsyncTasks();
				bool									IsInMainThread(INativeWindow* window)override;
				void									InvokeAsync(const Func<void()>& proc)override;
				void									InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)override;
				bool									InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds)override;
				Ptr<INativeDelay>						DelayExecute(const Func<void()>& proc, vint milliseconds)override;
				Ptr<INativeDelay>						DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)override;
			};
		}
	}
}

#endif