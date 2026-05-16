#include "GuiRemoteProtocol_Channel_Async.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;
	using namespace vl::presentation::controls;

/***********************************************************************
GuiRemoteProtocolAsyncJsonChannelSerializer
***********************************************************************/

	bool GuiRemoteProtocolAsyncJsonChannelSerializer::AreCurrentPendingRequestGroupSatisfied(bool disconnected)
	{
		if (!pendingRequest) return false;
		if (disconnected) return true;
		for (vint requestId : pendingRequest->requestIds)
		{
			if (!queuedResponses.Keys().Contains(requestId))
			{
				return false;
			}
		}
		return true;
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::ScheduleProcessRemoteEvents()
	{
		bool shouldQueue = false;
		SPIN_LOCK(lockEvents)
		{
			if (!uiTaskQueued)
			{
				uiTaskQueued = true;
				shouldQueue = true;
			}
		}

		if (shouldQueue)
		{
			if (auto app = GetApplication())
			{
				app->InvokeInMainThread(app->GetMainWindow(), [this]()
				{
					ProcessChannelEvents();
				});
			}
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::ProcessChannelEvents()
	{
		List<ReceivedPackage> events;
		SPIN_LOCK(lockEvents)
		{
			events = std::move(queuedEvents);
			uiTaskQueued = false;
		}

		for (auto&& eventPackage : events)
		{
			ChannelPackageInfo info;
			Ptr<glr::json::JsonNode> jsonArguments;
			JsonChannelUnpack(eventPackage.package, info, jsonArguments);

			if (info.name == L"ControllerConnect")
			{
				SPIN_LOCK(lockConnection)
				{
					connectionCounter++;
					connectionAvailable = true;
				}
			}
			reader->OnRead(eventPackage.senderClientId, eventPackage.package);
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::ProcessRemoteEvents()
	{
		if (remoteEventProcessor)
		{
			remoteEventProcessor->ProcessRemoteEvents();
		}
		ProcessChannelEvents();
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::OnRead(vint senderClientId, const JsonPackage& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelSerializer::OnRead(vint, const JsonPackage&)#"
		ChannelPackageInfo info;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, info, jsonArguments);

		switch (info.semantic)
		{
		case ChannelPackageSemantic::Event:
			{
				ReceivedPackage receivedPackage;
				receivedPackage.senderClientId = senderClientId;
				receivedPackage.package = package;
				SPIN_LOCK(lockEvents)
				{
					queuedEvents.Add(receivedPackage);
				}
				ScheduleProcessRemoteEvents();
			}
			break;
		case ChannelPackageSemantic::Response:
			{
				ReceivedPackage receivedPackage;
				receivedPackage.senderClientId = senderClientId;
				receivedPackage.package = package;
				SPIN_LOCK(lockResponses)
				{
					queuedResponses.Add(info.id, receivedPackage);
					if (AreCurrentPendingRequestGroupSatisfied(false))
					{
						eventAutoResponses.Signal();
					}
				}
			}
			break;
		default:
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Only responses and events are expected.");
		}
#undef ERROR_MESSAGE_PREFIX
	}

	GuiRemoteProtocolAsyncJsonChannelSerializer::GuiRemoteProtocolAsyncJsonChannelSerializer(IJsonChannel* _channel, IGuiRemoteEventProcessor* _remoteEventProcessor)
		: channel(_channel)
		, remoteEventProcessor(_remoteEventProcessor)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelSerializer::GuiRemoteProtocolAsyncJsonChannelSerializer(IJsonChannel*, IGuiRemoteEventProcessor*)#"
		CHECK_ERROR(channel, ERROR_MESSAGE_PREFIX L"A valid channel is required.");
		CHECK_ERROR(eventAutoResponses.CreateAutoUnsignal(false), ERROR_MESSAGE_PREFIX L"Failed to initialize eventAutoResponses.");
#undef ERROR_MESSAGE_PREFIX
	}

	GuiRemoteProtocolAsyncJsonChannelSerializer::~GuiRemoteProtocolAsyncJsonChannelSerializer()
	{
	}

	const WString& GuiRemoteProtocolAsyncJsonChannelSerializer::GetChannelName()
	{
		return channel->GetChannelName();
	}

	IJsonChannelReader* GuiRemoteProtocolAsyncJsonChannelSerializer::GetReader()
	{
		return reader;
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::Initialize(IJsonChannelReader* _reader)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelSerializer::Initialize(IJsonChannelReader*)#"
		CHECK_ERROR(_reader, ERROR_MESSAGE_PREFIX L"A valid reader is required.");
		CHECK_ERROR(!reader, ERROR_MESSAGE_PREFIX L"The async channel cannot be initialized more than once.");
		reader = _reader;
		channel->Initialize(this);
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::SendToClient(vint senderClientId, vint receiverClientId, const JsonPackage& package)
	{
		QueuedPackage queuedPackage;
		queuedPackage.senderClientId = senderClientId;
		queuedPackage.receiverClientId = receiverClientId;
		queuedPackage.package = package;

		SPIN_LOCK(lockPendingPackages)
		{
			pendingPackages.Add(queuedPackage);
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::BroadcastFromClient(vint senderClientId, const JsonPackage& package)
	{
		QueuedPackage queuedPackage;
		queuedPackage.senderClientId = senderClientId;
		queuedPackage.package = package;

		SPIN_LOCK(lockPendingPackages)
		{
			pendingPackages.Add(queuedPackage);
		}
	}

	void GuiRemoteProtocolAsyncJsonChannelSerializer::BatchWrite(bool& disconnected)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelSerializer::BatchWrite(bool&)#"
		disconnected = false;

		List<QueuedPackage> packages;
		SPIN_LOCK(lockPendingPackages)
		{
			packages = std::move(pendingPackages);
		}

		if (packages.Count() == 0)
		{
			return;
		}

		vint currentConnectionCounter = -1;
		SPIN_LOCK(lockConnection)
		{
			currentConnectionCounter = connectionCounter;
			if (!connectionAvailable)
			{
				disconnected = true;
				return;
			}
		}

		auto requestGroup = Ptr(new PendingRequestGroup);
		requestGroup->connectionCounter = currentConnectionCounter;
		for (auto&& queuedPackage : packages)
		{
			ChannelPackageInfo info;
			Ptr<glr::json::JsonNode> jsonArguments;
			JsonChannelUnpack(queuedPackage.package, info, jsonArguments);

			if (info.semantic == ChannelPackageSemantic::Request)
			{
				requestGroup->requestIds.Add(info.id);
			}
		}

		SPIN_LOCK(lockResponses)
		{
			CHECK_ERROR(!pendingRequest, ERROR_MESSAGE_PREFIX L"Only one pending request group is expected.");
			pendingRequest = requestGroup;
		}

		for (auto&& queuedPackage : packages)
		{
			if (queuedPackage.receiverClientId)
			{
				channel->SendToClient(queuedPackage.senderClientId, queuedPackage.receiverClientId.Value(), queuedPackage.package);
			}
			else
			{
				channel->BroadcastFromClient(queuedPackage.senderClientId, queuedPackage.package);
			}
		}
		channel->BatchWrite(disconnected);

		if (disconnected || requestGroup->requestIds.Count() == 0)
		{
			eventAutoResponses.Signal();
		}

		eventAutoResponses.Wait();
		SPIN_LOCK(lockConnection)
		{
			if (requestGroup->connectionCounter != connectionCounter || !connectionAvailable)
			{
				disconnected = true;
			}
		}

		List<ReceivedPackage> responses;
		SPIN_LOCK(lockResponses)
		{
			if (!disconnected)
			{
				for (vint id : requestGroup->requestIds)
				{
					responses.Add(queuedResponses[id]);
					queuedResponses.Remove(id);
				}
			}
			pendingRequest = nullptr;
			queuedResponses.Clear();
		}

		for (auto&& response : responses)
		{
			reader->OnRead(response.senderClientId, response.package);
		}
#undef ERROR_MESSAGE_PREFIX
	}

	IGuiRemoteEventProcessor* GuiRemoteProtocolAsyncJsonChannelSerializer::GetRemoteEventProcessor()
	{
		return this;
	}
}
