#include "WindowsAsyncService.h"

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace collections;

/***********************************************************************
WindowsAsyncService::TaskItem
***********************************************************************/

			WindowsAsyncService::TaskItem::TaskItem()
				:semaphore(0)
			{
			}

			WindowsAsyncService::TaskItem::TaskItem(Semaphore* _semaphore, const Func<void()>& _proc)
				:semaphore(_semaphore)
				,proc(_proc)
			{
			}

			WindowsAsyncService::TaskItem::~TaskItem()
			{
			}

/***********************************************************************
WindowsAsyncService::DelayItem
***********************************************************************/

			WindowsAsyncService::DelayItem::DelayItem(WindowsAsyncService* _service, const Func<void()>& _proc, bool _executeInMainThread, vint milliseconds)
				:service(_service)
				,proc(_proc)
				,status(INativeDelay::Pending)
				,executeTime(DateTime::LocalTime().Forward(milliseconds))
				,executeInMainThread(_executeInMainThread)
			{
			}

			WindowsAsyncService::DelayItem::~DelayItem()
			{
			}

			INativeDelay::ExecuteStatus WindowsAsyncService::DelayItem::GetStatus()
			{
				return status;
			}

			bool WindowsAsyncService::DelayItem::Delay(vint milliseconds)
			{
				SPIN_LOCK(service->taskListLock)
				{
					if(status==INativeDelay::Pending)
					{
						executeTime=DateTime::LocalTime().Forward(milliseconds);
						return true;
					}
				}
				return false;
			}

			bool WindowsAsyncService::DelayItem::Cancel()
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
WindowsAsyncService
***********************************************************************/

			WindowsAsyncService::WindowsAsyncService()
				:mainThreadId(Thread::GetCurrentThreadId())
			{
			}

			WindowsAsyncService::~WindowsAsyncService()
			{
			}

			void WindowsAsyncService::ExecuteAsyncTasks()
			{
				DateTime now=DateTime::LocalTime();
				Array<TaskItem> items;
				List<Ptr<DelayItem>> executableDelayItems;

				SPIN_LOCK(taskListLock)
				{
					CopyFrom(items, taskItems);
					taskItems.RemoveRange(0, items.Count());
					for(vint i=delayItems.Count()-1;i>=0;i--)
					{
						Ptr<DelayItem> item=delayItems[i];
						if(now.filetime>=item->executeTime.filetime)
						{
							item->status=INativeDelay::Executing;
							executableDelayItems.Add(item);
							delayItems.RemoveAt(i);
						}
					}
				}

				FOREACH(TaskItem, item, items)
				{
					item.proc();
					if(item.semaphore)
					{
						item.semaphore->Release();
					}
				}
				FOREACH(Ptr<DelayItem>, item, executableDelayItems)
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

			bool WindowsAsyncService::IsInMainThread(INativeWindow* window)
			{
				return Thread::GetCurrentThreadId()==mainThreadId;
			}

			void WindowsAsyncService::InvokeAsync(const Func<void()>& proc)
			{
				ThreadPoolLite::Queue(proc);
			}

			void WindowsAsyncService::InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)
			{
				SPIN_LOCK(taskListLock)
				{
					TaskItem item(0, proc);
					taskItems.Add(item);
				}
			}

			bool WindowsAsyncService::InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds)
			{
				Semaphore semaphore;
				semaphore.Create(0, 1);

				SPIN_LOCK(taskListLock)
				{
					TaskItem item(&semaphore, proc);
					taskItems.Add(item);
				}

				if(milliseconds<0)
				{
					return semaphore.Wait();
				}
				else
				{
					return semaphore.WaitForTime(milliseconds);
				}
			}

			Ptr<INativeDelay> WindowsAsyncService::DelayExecute(const Func<void()>& proc, vint milliseconds)
			{
				Ptr<DelayItem> delay;
				SPIN_LOCK(taskListLock)
				{
					delay=new DelayItem(this, proc, false, milliseconds);
					delayItems.Add(delay);
				}
				return delay;
			}

			Ptr<INativeDelay> WindowsAsyncService::DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)
			{
				Ptr<DelayItem> delay;
				SPIN_LOCK(taskListLock)
				{
					delay=new DelayItem(this, proc, true, milliseconds);
					delayItems.Add(delay);
				}
				return delay;
			}
		}
	}
}