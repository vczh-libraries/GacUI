/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteProtocolAsyncJsonChannelRenderer

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_ASYNCRENDERER
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_ASYNCRENDERER

#include "GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
IGuiRemoteProtocolAsyncRendererInvoker
***********************************************************************/

	class IGuiRemoteProtocolAsyncRendererInvoker : public virtual Interface
	{
	public:
		virtual void										InvokeInMainThread(const Func<void()>& proc) = 0;
	};

/***********************************************************************
GuiRemoteProtocolAsyncJsonChannelRenderer
***********************************************************************/

	class GuiRemoteProtocolAsyncJsonChannelRenderer
		: public Object
		, public virtual IJsonChannel
		, protected virtual IJsonChannelReader
	{
	protected:
		struct ReceivedPackage
		{
			vint											senderClientId = -1;
			vint											messageVersion = -1;
			JsonPackage										package;
		};

		IJsonChannel*										channel = nullptr;
		IJsonChannelReader*								reader = nullptr;

		// Covers invokeInMainThread, queuedMessages and uiTaskQueued.
		SpinLock											lockMessages;
		IGuiRemoteProtocolAsyncRendererInvoker*			invokeInMainThread = nullptr;
		collections::List<ReceivedPackage>					queuedMessages;
		vint												messageVersion = 0;
		bool												channelInitialized = false;
		bool												uiTaskQueued = false;

		void												ScheduleProcessRemoteMessages();
		void												ProcessRemoteMessages();

		void												OnRead(vint senderClientId, const JsonPackage& package) override;

	public:
		GuiRemoteProtocolAsyncJsonChannelRenderer(IJsonChannel* _channel);
		~GuiRemoteProtocolAsyncJsonChannelRenderer();

		const WString&										GetChannelName() override;
		IJsonChannelReader*								GetReader() override;
		void												Initialize(IJsonChannelReader* _reader) override;
		void												SendToClient(vint senderClientId, vint receiverClientId, const JsonPackage& package) override;
		void												BroadcastFromClient(vint senderClientId, const JsonPackage& package) override;
		void												BatchWrite(bool& disconnected) override;

		void												SetInvokeInMainThread(IGuiRemoteProtocolAsyncRendererInvoker* _invokeInMainThread);
	};
}

#endif
