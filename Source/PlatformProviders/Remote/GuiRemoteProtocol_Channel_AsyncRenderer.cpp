#include "GuiRemoteProtocol_Channel_AsyncRenderer.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;

/***********************************************************************
GuiRemoteProtocolAsyncJsonChannelRenderer
***********************************************************************/

	void GuiRemoteProtocolAsyncJsonChannelRenderer::ScheduleProcessPendingMessages()
	{
		Ptr<IGuiRemoteProtocolAsyncRendererInvoker> invoker;
		Ptr<CallbackState> state;
		SPIN_LOCK(lockMessages)
		{
			if (invokeInMainThread && !uiTaskQueued && queuedMessages.Count() > 0)
			{
				uiTaskQueued = true;
				invoker = invokeInMainThread;
				state = callbackState;
			}
		}

		if (invoker)
		{
			invoker->InvokeInMainThread([state]()
			{
				SPIN_LOCK(state->lockOwner)
				{
					if (state->owner)
					{
						state->owner->ProcessPendingMessages();
					}
				}
			});
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::ProcessPendingMessages()
	{
		SPIN_LOCK(lockMessages)
		{
			if (processingMessages)
			{
				return;
			}
			processingMessages = true;
		}

		try
		{
			while (true)
			{
				IJsonChannelReader* currentReader = nullptr;
				vint currentMessageVersion = -1;
				List<PendingMessage> messages;
				SPIN_LOCK(lockMessages)
				{
					currentReader = reader;
					currentMessageVersion = messageVersion;
					if (!currentReader)
					{
						queuedMessages.Clear();
						uiTaskQueued = false;
						processingMessages = false;
						return;
					}

					messages = std::move(queuedMessages);
					if (messages.Count() == 0)
					{
						uiTaskQueued = false;
						processingMessages = false;
						return;
					}
				}

				for (auto&& message : messages)
				{
					bool shouldProcess = false;
					SPIN_LOCK(lockMessages)
					{
						shouldProcess = reader == currentReader && message.messageVersion == currentMessageVersion;
					}

					if (shouldProcess)
					{
						if (message.mainThreadTask)
						{
							message.mainThreadTask();
						}
						else
						{
							currentReader->OnRead(message.senderClientId, message.package);
						}
					}
				}
			}
		}
		catch (...)
		{
			SPIN_LOCK(lockMessages)
			{
				uiTaskQueued = false;
				processingMessages = false;
			}
			throw;
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::OnRead(vint senderClientId, const JsonPackage& package)
	{
		PendingMessage pendingMessage;
		pendingMessage.senderClientId = senderClientId;
		pendingMessage.package = package;
		SPIN_LOCK(lockMessages)
		{
			if (!reader)
			{
				return;
			}
			pendingMessage.messageVersion = messageVersion;
			queuedMessages.Add(std::move(pendingMessage));
		}
		ScheduleProcessPendingMessages();
	}

	GuiRemoteProtocolAsyncJsonChannelRenderer::GuiRemoteProtocolAsyncJsonChannelRenderer(IJsonChannel* _channel)
		: channel(_channel)
	{
		callbackState = Ptr(new CallbackState);
		callbackState->owner = this;
	}

	GuiRemoteProtocolAsyncJsonChannelRenderer::~GuiRemoteProtocolAsyncJsonChannelRenderer()
	{
		SPIN_LOCK(callbackState->lockOwner)
		{
			callbackState->owner = nullptr;
		}
		SPIN_LOCK(lockMessages)
		{
			invokeInMainThread = nullptr;
			queuedMessages.Clear();
			uiTaskQueued = false;
		}
	}

	const WString& GuiRemoteProtocolAsyncJsonChannelRenderer::GetChannelName()
	{
		return channel->GetChannelName();
	}

	IJsonChannelReader* GuiRemoteProtocolAsyncJsonChannelRenderer::GetReader()
	{
		IJsonChannelReader* currentReader = nullptr;
		SPIN_LOCK(lockMessages)
		{
			currentReader = reader;
		}
		return currentReader;
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::Initialize(IJsonChannelReader* _reader)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelRenderer::Initialize(IJsonChannelReader*)#"
		CHECK_ERROR(_reader, ERROR_MESSAGE_PREFIX L"The reader must not be null. Call Detach() to clear the reader.");
		bool initializeChannel = false;
		SPIN_LOCK(lockMessages)
		{
			reader = _reader;
			if (reader && !channelInitialized)
			{
				channelInitialized = true;
				initializeChannel = true;
			}
		}
		if (initializeChannel)
		{
			channel->Initialize(this);
		}
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::SendToClient(vint receiverClientId, const JsonPackage& package)
	{
		channel->SendToClient(receiverClientId, package);
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::BroadcastFromClient(const JsonPackage& package)
	{
		channel->BroadcastFromClient(package);
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::BroadcastFromClient(const JsonPackage& package, const List<vint>& blockedReceivers)
	{
		channel->BroadcastFromClient(package, blockedReceivers);
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::BatchWrite(bool& disconnected)
	{
		channel->BatchWrite(disconnected);
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::SetInvokeInMainThread(Ptr<IGuiRemoteProtocolAsyncRendererInvoker> _invokeInMainThread)
	{
		SPIN_LOCK(lockMessages)
		{
			invokeInMainThread = _invokeInMainThread;
		}
		ScheduleProcessPendingMessages();
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::QueueMainThreadTask(const Func<void()>& task)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelRenderer::QueueMainThreadTask(const Func<void()>&)#"
		CHECK_ERROR(task, ERROR_MESSAGE_PREFIX L"The task must not be empty.");
		PendingMessage pendingMessage;
		pendingMessage.mainThreadTask = task;
		SPIN_LOCK(lockMessages)
		{
			if (!reader)
			{
				return;
			}
			pendingMessage.messageVersion = messageVersion;
			queuedMessages.Add(std::move(pendingMessage));
		}
		ScheduleProcessPendingMessages();
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::Detach()
	{
		SPIN_LOCK(lockMessages)
		{
			reader = nullptr;
			messageVersion++;
			queuedMessages.Clear();
			uiTaskQueued = false;
		}
	}
}
