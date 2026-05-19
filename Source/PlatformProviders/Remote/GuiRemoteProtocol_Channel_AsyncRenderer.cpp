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
			List<ReceivedPackage> messages;
			SPIN_LOCK(lockMessages)
			{
				messages = std::move(queuedMessages);
				if (messages.Count() == 0)
				{
					uiTaskQueued = false;
					return;
				}
			}

			for (auto&& message : messages)
			{
				reader->OnRead(message.senderClientId, message.package);
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
		Initialize(nullptr);
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
		reader = _reader;
		channel->Initialize(_reader ? this : nullptr);
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::SendToClient(vint senderClientId, vint receiverClientId, const JsonPackage& package)
	{
		channel->SendToClient(senderClientId, receiverClientId, package);
	}

	void GuiRemoteProtocolAsyncJsonChannelRenderer::BroadcastFromClient(vint senderClientId, const JsonPackage& package)
	{
		channel->BroadcastFromClient(senderClientId, package);
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
}
