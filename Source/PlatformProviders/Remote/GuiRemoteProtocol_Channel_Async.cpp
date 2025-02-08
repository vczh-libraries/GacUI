#include "GuiRemoteProtocol_Channel_Async.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;

/***********************************************************************
GuiRemoteProtocolAsyncChannelSerializerBase
***********************************************************************/

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueTask(SpinLock& lock, collections::List<TTaskProc>& tasks, TTaskProc task, EventObject* signalAfterQueue)
	{
		SPIN_LOCK(lock)
		{
			tasks.Add(task);
		}

		if (signalAfterQueue)
		{
			signalAfterQueue->Signal();
		}
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueTaskAndWait(SpinLock& lock, collections::List<TTaskProc>& tasks, TTaskProc task, EventObject* signalAfterQueue)
	{
		auto taskEvent = Ptr(new vl::EventObject);
		taskEvent->CreateAutoUnsignal(false);
		auto taskWithEvent = [=]()
		{
			task();
			taskEvent->Signal();
		};

		QueueTask(lock, tasks, taskWithEvent, signalAfterQueue);
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

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToChannelThread(TTaskProc task, EventObject* signalAfterQueue)
	{
		QueueTask(channelThreadLock, channelThreadTasks, task, signalAfterQueue);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToChannelThreadAndWait(TTaskProc task, EventObject* signalAfterQueue)
	{
		QueueTaskAndWait(channelThreadLock, channelThreadTasks, task, signalAfterQueue);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToUIThread(TTaskProc task, EventObject* signalAfterQueue)
	{
		QueueTask(uiThreadLock, uiThreadTasks, task, signalAfterQueue);
	}

	void GuiRemoteProtocolAsyncChannelSerializerBase::QueueToUIThreadAndWait(TTaskProc task, EventObject* signalAfterQueue)
	{
		QueueTaskAndWait(uiThreadLock, uiThreadTasks, task, signalAfterQueue);
	}
}