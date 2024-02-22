/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDASYNCSERVICE
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDASYNCSERVICE

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		/// <summary>
		/// A general <see cref="INativeAsyncService/> implementation.
		/// </summary>
		class SharedAsyncService : public INativeAsyncService
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
				DelayItem(SharedAsyncService* _service, const Func<void()>& _proc, bool _executeInMainThread, vint milliseconds);
				~DelayItem();

				SharedAsyncService*					service;
				Func<void()>						proc;
				ExecuteStatus						status;
				DateTime							executeUtcTime;
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
			SharedAsyncService();
			~SharedAsyncService();

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

#endif