#include "GuiRemoteProtocol_Channel_AsyncRenderer.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;

/***********************************************************************
GuiRemoteProtocolAsyncJsonChannelRenderer
***********************************************************************/

	void GuiRemoteProtocolAsyncJsonChannelRenderer::ScheduleProcessRemoteMessages()
	{
		IGuiRemoteProtocolAsyncRendererInvoker* invoker = nullptr;
		SPIN_LOCK(lockMessages)
		{
			if (invokeInMainThread && !uiTaskQueued && queuedMessages.Count() > 0)
			{
				uiTaskQueued = true;
				invoker = invokeInMainThread;
			}
		}

		if (invoker)
		{
			invoker->InvokeInMainThread([this]()
			{
				ProcessRemoteMessages();
			});
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::ProcessRemoteMessages()
	{
		while (true)
		{
			IJsonChannelReader* currentReader = nullptr;
			vint currentMessageVersion = -1;
			List<ReceivedPackage> messages;
			SPIN_LOCK(lockMessages)
			{
				currentReader = reader;
				currentMessageVersion = messageVersion;
				if (!currentReader)
				{
					queuedMessages.Clear();
					uiTaskQueued = false;
					return;
				}

				messages = std::move(queuedMessages);
				if (messages.Count() == 0)
				{
					uiTaskQueued = false;
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
					currentReader->OnRead(message.senderClientId, message.package);
				}
			}
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::OnRead(vint senderClientId, const JsonPackage& package)
	{
		ReceivedPackage receivedPackage;
		receivedPackage.senderClientId = senderClientId;
		receivedPackage.package = package;
		SPIN_LOCK(lockMessages)
		{
			if (!reader)
			{
				return;
			}
			receivedPackage.messageVersion = messageVersion;
			queuedMessages.Add(std::move(receivedPackage));
		}
		ScheduleProcessRemoteMessages();
	}

	GuiRemoteProtocolAsyncJsonChannelRenderer::GuiRemoteProtocolAsyncJsonChannelRenderer(IJsonChannel* _channel)
		: channel(_channel)
	{
	}

	GuiRemoteProtocolAsyncJsonChannelRenderer::~GuiRemoteProtocolAsyncJsonChannelRenderer()
	{
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
		return reader;
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

	void GuiRemoteProtocolAsyncJsonChannelRenderer::SetInvokeInMainThread(IGuiRemoteProtocolAsyncRendererInvoker* _invokeInMainThread)
	{
		SPIN_LOCK(lockMessages)
		{
			invokeInMainThread = _invokeInMainThread;
		}
		ScheduleProcessRemoteMessages();
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
