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
		class CallbackState : public Object
		{
		public:
			SpinLock										lockOwner;
			GuiRemoteProtocolAsyncJsonChannelRenderer*		owner = nullptr;
		};

		struct PendingMessage
		{
			vint											senderClientId = -1;
			vint											messageVersion = -1;
			JsonPackage										package;
			Func<void()>									mainThreadTask;
		};

		IJsonChannel*										channel = nullptr;
		IJsonChannelReader*									reader = nullptr;

		// Covers reader, invokeInMainThread, queuedMessages, messageVersion, channelInitialized and uiTaskQueued.
		SpinLock											lockMessages;
		Ptr<IGuiRemoteProtocolAsyncRendererInvoker>			invokeInMainThread;
		Ptr<CallbackState>									callbackState;
		collections::List<PendingMessage>					queuedMessages;
		vint												messageVersion = 0;
		bool												channelInitialized = false;
		bool												uiTaskQueued = false;
		bool												processingMessages = false;

		void												ScheduleProcessPendingMessages();

		void												OnRead(vint senderClientId, const JsonPackage& package) override;

	public:
		GuiRemoteProtocolAsyncJsonChannelRenderer(IJsonChannel* _channel);
		~GuiRemoteProtocolAsyncJsonChannelRenderer();

		const WString&										GetChannelName() override;
		IJsonChannelReader*									GetReader() override;
		void												Initialize(IJsonChannelReader* _reader) override;
		void												SendToClient(vint receiverClientId, const JsonPackage& package) override;
		void												BroadcastFromClient(const JsonPackage& package) override;
		void												BroadcastFromClient(const JsonPackage& package, const collections::List<vint>& blockedReceivers) override;
		void												BatchWrite(bool& disconnected) override;

		void												SetInvokeInMainThread(Ptr<IGuiRemoteProtocolAsyncRendererInvoker> _invokeInMainThread);
		void												QueueMainThreadTask(const Func<void()>& task);
		void												ProcessPendingMessages();
		void												Detach();
	};
}

#endif
