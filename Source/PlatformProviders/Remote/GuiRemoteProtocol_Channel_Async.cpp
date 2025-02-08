#include "GuiRemoteProtocol_Channel_Async.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;

/***********************************************************************
GuiRemoteProtocolAsyncChannelSerializerBase
***********************************************************************/

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueTask(SpinLock& lock, collections::List<TTaskProc>& tasks, TTaskProc task)
	{
		SPIN_LOCK(lock)
		{
			tasks.Add(task);
		}
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueTaskAndWait(SpinLock& lock, collections::List<TTaskProc>& tasks, TTaskProc task)
	{
		auto taskEvent = Ptr(new vl::EventObject);
		taskEvent->CreateAutoUnsignal(false);
		auto taskWithEvent = [=]()
		{
			task();
			taskEvent->Signal();
		};

		QueueTask(lock, tasks, taskWithEvent);
		taskEvent->Wait();
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::FetchTasks(SpinLock& lock, collections::List<TTaskProc>& tasks, collections::List<TTaskProc>& results)
	{
		SPIN_LOCK(lock)
		{
			results = std::move(tasks);
		}
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::FetchAndExecuteTasks(SpinLock& lock, collections::List<TTaskProc>& tasks)
	{
		List<TTaskProc> results;
		FetchTasks(lock, tasks, results);
		for (auto&& task : results)
		{
			task();
		}
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::FetchAndExecuteChannelTasks()
	{
		FetchAndExecuteTasks(channelThreadLock, channelThreadTasks);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::FetchAndExecuteUITasks()
	{
		FetchAndExecuteTasks(uiThreadLock, uiThreadTasks);
	}

	GuiRemoteProtocolAsyncChannelSerializerBase::GuiRemoteProtocolAsyncChannelSerializerBase()
	{
	}

	GuiRemoteProtocolAsyncChannelSerializerBase::~GuiRemoteProtocolAsyncChannelSerializerBase()
	{
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToChannelThread(TTaskProc task)
	{
		QueueTask(channelThreadLock, channelThreadTasks, task);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToChannelThreadAndWait(TTaskProc task)
	{
		QueueTaskAndWait(channelThreadLock, channelThreadTasks, task);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToUIThread(TTaskProc task)
	{
		QueueTask(uiThreadLock, uiThreadTasks, task);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToUIThreadAndWait(TTaskProc task)
	{
		QueueTaskAndWait(uiThreadLock, uiThreadTasks, task);
	}
}