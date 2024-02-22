#include "GuiSharedAsyncService.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
SharedAsyncService::TaskItem
***********************************************************************/

		SharedAsyncService::TaskItem::TaskItem()
			:semaphore(0)
		{
		}

		SharedAsyncService::TaskItem::TaskItem(Semaphore* _semaphore, const Func<void()>& _proc)
			:semaphore(_semaphore)
			,proc(_proc)
		{
		}

		SharedAsyncService::TaskItem::~TaskItem()
		{
		}

/***********************************************************************
SharedAsyncService::DelayItem
***********************************************************************/

		SharedAsyncService::DelayItem::DelayItem(SharedAsyncService* _service, const Func<void()>& _proc, bool _executeInMainThread, vint milliseconds)
			:service(_service)
			,proc(_proc)
			,status(INativeDelay::Pending)
			, executeUtcTime(DateTime::UtcTime().Forward(milliseconds))
			,executeInMainThread(_executeInMainThread)
		{
		}

		SharedAsyncService::DelayItem::~DelayItem()
		{
		}

		INativeDelay::ExecuteStatus SharedAsyncService::DelayItem::GetStatus()
		{
			return status;
		}

		bool SharedAsyncService::DelayItem::Delay(vint milliseconds)
		{
			SPIN_LOCK(service->taskListLock)
			{
				if(status==INativeDelay::Pending)
				{
					executeUtcTime =DateTime::UtcTime().Forward(milliseconds);
					return true;
				}
			}
			return false;
		}

		bool SharedAsyncService::DelayItem::Cancel()
		{
			SPIN_LOCK(service->taskListLock)
			{
				if(status==INativeDelay::Pending)
				{
					if(service->delayItems.Remove(this))
					{
						status=INativeDelay::Canceled;
						return true;
					}
				}
			}
			return false;
		}

/***********************************************************************
SharedAsyncService
***********************************************************************/

		SharedAsyncService::SharedAsyncService()
			:mainThreadId(Thread::GetCurrentThreadId())
		{
		}

		SharedAsyncService::~SharedAsyncService()
		{
		}

		void SharedAsyncService::ExecuteAsyncTasks()
		{
			auto now=DateTime::UtcTime();
			Array<TaskItem> items;
			List<Ptr<DelayItem>> executableDelayItems;

			SPIN_LOCK(taskListLock)
			{
				CopyFrom(items, taskItems);
				taskItems.RemoveRange(0, items.Count());
				// TODO: (enumerable) foreach:indexed(alterable(reversed))
				for(vint i=delayItems.Count()-1;i>=0;i--)
				{
					Ptr<DelayItem> item=delayItems[i];
					if(now >= item->executeUtcTime)
					{
						item->status=INativeDelay::Executing;
						executableDelayItems.Add(item);
						delayItems.RemoveAt(i);
					}
				}
			}

			for (auto item : items)
			{
				item.proc();
				if(item.semaphore)
				{
					item.semaphore->Release();
				}
			}
			for (auto item : executableDelayItems)
			{
				if(item->executeInMainThread)
				{
					item->proc();
					item->status=INativeDelay::Executed;
				}
				else
				{
					InvokeAsync([=]()
					{
						item->proc();
						item->status=INativeDelay::Executed;
					});
				}
			}
		}

		bool SharedAsyncService::IsInMainThread(INativeWindow* window)
		{
			return Thread::GetCurrentThreadId()==mainThreadId;
		}

		void SharedAsyncService::InvokeAsync(const Func<void()>& proc)
		{
			ThreadPoolLite::Queue(proc);
		}

		void SharedAsyncService::InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)
		{
			SPIN_LOCK(taskListLock)
			{
				TaskItem item(0, proc);
				taskItems.Add(item);
			}
		}

		bool SharedAsyncService::InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds)
		{
			Semaphore semaphore;
			semaphore.Create(0, 1);

			SPIN_LOCK(taskListLock)
			{
				TaskItem item(&semaphore, proc);
				taskItems.Add(item);
			}

#ifdef VCZH_MSVC
			if(milliseconds<0)
			{
				return semaphore.Wait();
			}
			else
			{
				return semaphore.WaitForTime(milliseconds);
			}
#else
			return semaphore.Wait();
#endif
		}

		Ptr<INativeDelay> SharedAsyncService::DelayExecute(const Func<void()>& proc, vint milliseconds)
		{
			Ptr<DelayItem> delay;
			SPIN_LOCK(taskListLock)
			{
				delay=Ptr(new DelayItem(this, proc, false, milliseconds));
				delayItems.Add(delay);
			}
			return delay;
		}

		Ptr<INativeDelay> SharedAsyncService::DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)
		{
			Ptr<DelayItem> delay;
			SPIN_LOCK(taskListLock)
			{
				delay=Ptr(new DelayItem(this, proc, true, milliseconds));
				delayItems.Add(delay);
			}
			return delay;
		}
	}
}