#include "GuiRemoteProtocol_Channel_Async.h"
#include "../../Application/Controls/GuiApplication.h"

namespace vl::presentation::remoteprotocol::channeling
{
	using namespace vl::collections;
	using namespace vl::presentation::controls;

/***********************************************************************
GuiRemoteProtocolAsyncJsonChannel
***********************************************************************/

	bool GuiRemoteProtocolAsyncJsonChannel::AreCurrentPendingRequestGroupSatisfied(bool disconnected)
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

	void GuiRemoteProtocolAsyncJsonChannel::ScheduleProcessRemoteEvents()
	{
		auto app = GetApplication();
		if (!app)
		{
			return;
		}

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
			app->InvokeInMainThread(app->GetMainWindow(), [this]()
			{
				ProcessChannelEvents();
			});
		}
	}

	void GuiRemoteProtocolAsyncJsonChannel::ProcessChannelEvents()
	{
		List<ReceivedPackage> events;
		SPIN_LOCK(lockEvents)
		{
			events = std::move(queuedEvents);
			uiTaskQueued = false;
		}

		auto processEvent = [this](const ReceivedPackage& eventPackage)
		{
			reader->OnRead(eventPackage.senderClientId, eventPackage.package);
		};

		for (auto&& eventPackage : events)
		{
			ChannelPackageInfo info;
			Ptr<glr::json::JsonNode> jsonArguments;
			JsonChannelUnpack(eventPackage.package, info, jsonArguments);

			vint currentConnectionClientId = -1;
			SPIN_LOCK(lockConnection)
			{
				currentConnectionClientId = connectionClientId;
			}

			if (info.name == L"ControllerConnect" && eventPackage.senderClientId == currentConnectionClientId)
			{
				processEvent(eventPackage);
			}
		}

		for (auto&& eventPackage : events)
		{
			ChannelPackageInfo info;
			Ptr<glr::json::JsonNode> jsonArguments;
			JsonChannelUnpack(eventPackage.package, info, jsonArguments);

			vint currentConnectionClientId = -1;
			SPIN_LOCK(lockConnection)
			{
				currentConnectionClientId = connectionClientId;
			}

			if (info.name != L"ControllerConnect" && eventPackage.senderClientId == currentConnectionClientId)
			{
				processEvent(eventPackage);
			}
		}
	}

	void GuiRemoteProtocolAsyncJsonChannel::ProcessRemoteEvents()
	{
		if (remoteEventProcessor)
		{
			remoteEventProcessor->ProcessRemoteEvents();
		}
		ProcessChannelEvents();
	}

	void GuiRemoteProtocolAsyncJsonChannel::OnRead(vint senderClientId, const JsonPackage& package)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannel::OnRead(vint, const JsonPackage&)#"
		ChannelPackageInfo info;
		Ptr<glr::json::JsonNode> jsonArguments;
		JsonChannelUnpack(package, info, jsonArguments);

		switch (info.semantic)
		{
		case ChannelPackageSemantic::Event:
			{
				if (info.name == L"ControllerConnect")
				{
					SPIN_LOCK(lockConnection)
					{
						connectionCounter++;
						connectionClientId = senderClientId;
						connectionAvailable = true;
					}

					SPIN_LOCK(lockResponses)
					{
						if (pendingRequest)
						{
							eventAutoResponses.Signal();
						}
					}
				}

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

	GuiRemoteProtocolAsyncJsonChannel::GuiRemoteProtocolAsyncJsonChannel(IJsonChannel* _channel, IGuiRemoteEventProcessor* _remoteEventProcessor)
		: channel(_channel)
		, remoteEventProcessor(_remoteEventProcessor)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannel::GuiRemoteProtocolAsyncJsonChannel(IJsonChannel*, IGuiRemoteEventProcessor*)#"
		CHECK_ERROR(channel, ERROR_MESSAGE_PREFIX L"A valid channel is required.");
		CHECK_ERROR(eventAutoResponses.CreateAutoUnsignal(false), ERROR_MESSAGE_PREFIX L"Failed to initialize eventAutoResponses.");
#undef ERROR_MESSAGE_PREFIX
	}

	GuiRemoteProtocolAsyncJsonChannel::~GuiRemoteProtocolAsyncJsonChannel()
	{
	}

	const WString& GuiRemoteProtocolAsyncJsonChannel::GetChannelName()
	{
		return channel->GetChannelName();
	}

	IJsonChannelReader* GuiRemoteProtocolAsyncJsonChannel::GetReader()
	{
		return reader;
	}

	void GuiRemoteProtocolAsyncJsonChannel::Initialize(IJsonChannelReader* _reader)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannel::Initialize(IJsonChannelReader*)#"
		CHECK_ERROR(_reader, ERROR_MESSAGE_PREFIX L"A valid reader is required.");
		CHECK_ERROR(!reader, ERROR_MESSAGE_PREFIX L"The async channel cannot be initialized more than once.");
		reader = _reader;
		channel->Initialize(this);
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteProtocolAsyncJsonChannel::SendToClient(vint receiverClientId, const JsonPackage& package)
	{
		QueuedPackage queuedPackage;
		queuedPackage.receiverClientId = receiverClientId;
		queuedPackage.package = package;

		SPIN_LOCK(lockPendingPackages)
		{
			pendingPackages.Add(queuedPackage);
		}
	}

	void GuiRemoteProtocolAsyncJsonChannel::BroadcastFromClient(const JsonPackage& package)
	{
		List<vint> blockedReceivers;
		BroadcastFromClient(package, blockedReceivers);
	}

	void GuiRemoteProtocolAsyncJsonChannel::BroadcastFromClient(const JsonPackage& package, const List<vint>& blockedReceivers)
	{
		QueuedPackage queuedPackage;
		queuedPackage.package = package;
		queuedPackage.blockedReceivers = Ptr(new List<vint>);
		CopyFrom(*queuedPackage.blockedReceivers.Obj(), blockedReceivers);

		SPIN_LOCK(lockPendingPackages)
		{
			pendingPackages.Add(queuedPackage);
		}
	}

	void GuiRemoteProtocolAsyncJsonChannel::BatchWrite(bool& disconnected)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannel::BatchWrite(bool&)#"
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
				channel->SendToClient(queuedPackage.receiverClientId.Value(), queuedPackage.package);
			}
			else
			{
				if (queuedPackage.blockedReceivers)
				{
					channel->BroadcastFromClient(queuedPackage.package, *queuedPackage.blockedReceivers.Obj());
				}
				else
				{
					channel->BroadcastFromClient(queuedPackage.package);
				}
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

	IGuiRemoteEventProcessor* GuiRemoteProtocolAsyncJsonChannel::GetRemoteEventProcessor()
	{
		return this;
	}
}
