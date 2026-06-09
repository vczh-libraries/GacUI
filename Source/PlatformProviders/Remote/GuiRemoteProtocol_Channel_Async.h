/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteProtocolJsonChannelRenderer_Async

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_ASYNC
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_ASYNC

#include "GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
GuiRemoteProtocolJsonChannelRenderer_Async
***********************************************************************/

	class GuiRemoteProtocolJsonChannelRenderer_Async
		: public Object
		, public virtual IJsonChannel
		, protected virtual IJsonChannelReader
		, protected virtual IGuiRemoteEventProcessor
	{
	protected:
		struct QueuedPackage
		{
			Nullable<vint>									receiverClientId;
			Ptr<collections::List<vint>>						blockedReceivers;
			JsonPackage										package;
		};

		struct ReceivedPackage
		{
			vint											senderClientId = -1;
			JsonPackage										package;
		};

		struct PendingRequestGroup
		{
			vint											connectionCounter = -1;
			collections::List<vint>							requestIds;
		};

		IJsonChannel*										channel = nullptr;
		IJsonChannelReader*								reader = nullptr;
		IGuiRemoteEventProcessor*							remoteEventProcessor = nullptr;

		SpinLock											lockPendingPackages;
		collections::List<QueuedPackage>					pendingPackages;

		SpinLock											lockEvents;
		collections::List<ReceivedPackage>					queuedEvents;
		bool												uiTaskQueued = false;

		SpinLock											lockResponses;
		EventObject											eventAutoResponses;
		collections::Dictionary<vint, ReceivedPackage>		queuedResponses;
		Ptr<PendingRequestGroup>							pendingRequest;

		SpinLock											lockConnection;
		vint												connectionCounter = 0;
		vint												connectionClientId = -1;
		bool												connectionAvailable = false;

		bool												AreCurrentPendingRequestGroupSatisfied(bool disconnected);
		void												ScheduleProcessRemoteEvents();
		void												ProcessChannelEvents();
		void												ProcessRemoteEvents() override;

		void												OnRead(vint senderClientId, const JsonPackage& package) override;

	public:
		GuiRemoteProtocolJsonChannelRenderer_Async(IJsonChannel* _channel, IGuiRemoteEventProcessor* _remoteEventProcessor = nullptr);
		~GuiRemoteProtocolJsonChannelRenderer_Async();

		const WString&										GetChannelName() override;
		IJsonChannelReader*								GetReader() override;
		void												Initialize(IJsonChannelReader* _reader) override;
		void												SendToClient(vint receiverClientId, const JsonPackage& package) override;
		void												BroadcastFromClient(const JsonPackage& package) override;
		void												BroadcastFromClient(const JsonPackage& package, const collections::List<vint>& blockedReceivers) override;
		void												BatchWrite(bool& disconnected) override;

		IGuiRemoteEventProcessor*							GetRemoteEventProcessor();
	};
}

#endif
