/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocolChannel<T>

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_ASYNC
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL_ASYNC

#include "GuiRemoteProtocol_Channel_Shared.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
Metadata
***********************************************************************/

	enum class ChannelPackageSemantic
	{
		Message,
		Request,
		Response,
		Event,
		Unknown,
	};

	enum class ChannelAsyncState
	{
		Ready,
		Running,
		Stopped,
	};

/***********************************************************************
Async
  A certain package type could run in async mode
  if the following function is defined
  and accessible via argument-dependent lookup

void ChannelPackageSemanticUnpack(
  const T& package,
  ChannelPackageSemantic& semantic,
  vint& id,
  WString& name
  );
***********************************************************************/

	class GuiRemoteProtocolAsyncChannelSerializerBase : public Object
	{
	public:
		using TTaskProc = Func<void()>;

	private:
		collections::List<TTaskProc>								channelThreadTasks;
		SpinLock													channelThreadLock;
		collections::List<TTaskProc>								uiThreadTasks;
		SpinLock													uiThreadLock;

	protected:
		void			QueueTask(SpinLock& lock, collections::List<TTaskProc>& tasks, TTaskProc task, EventObject* signalAfterQueue);
		void			QueueTaskAndWait(SpinLock& lock, collections::List<TTaskProc>& tasks, TTaskProc task, EventObject* signalAfterQueue);
		void			FetchTasks(SpinLock& lock, collections::List<TTaskProc>& tasks, collections::List<TTaskProc>& results);
		void			FetchAndExecuteTasks(SpinLock& lock, collections::List<TTaskProc>& tasks);

		void			FetchAndExecuteChannelTasks();
		void			FetchAndExecuteUITasks();

		void			QueueToChannelThread(TTaskProc task, EventObject* signalAfterQueue);
		void			QueueToChannelThreadAndWait(TTaskProc task, EventObject* signalAfterQueue);
		void			QueueToUIThread(TTaskProc task, EventObject* signalAfterQueue);
		void			QueueToUIThreadAndWait(TTaskProc task, EventObject* signalAfterQueue);

	public:
		GuiRemoteProtocolAsyncChannelSerializerBase();
		~GuiRemoteProtocolAsyncChannelSerializerBase();
	};

	template<typename TPackage>
	class GuiRemoteProtocolAsyncChannelSerializer
		: public GuiRemoteProtocolAsyncChannelSerializerBase
		, public virtual IGuiRemoteProtocolChannel<TPackage>
		, protected virtual IGuiRemoteProtocolChannelReceiver<TPackage>
	{
		static_assert(
			std::is_same_v<void, decltype(ChannelPackageSemanticUnpack(
				std::declval<const TPackage&>(),
				std::declval<ChannelPackageSemantic&>(),
				std::declval<vint&>(),
				std::declval<WString&>()
				))>,
			"ChannelPackageSemanticUnpack must be defined for this TPackage"
			);

	public:
		using TChannelThreadProc = Func<void()>;
		using TUIThreadProc = Func<void()>;
		using TStartingProc = Func<void(TChannelThreadProc, TUIThreadProc)>;
		using TStoppingProc = Func<void()>;
		using TUIMainProc = Func<void(GuiRemoteProtocolAsyncChannelSerializer<TPackage>*)>;

	protected:

		IGuiRemoteProtocolChannel<TPackage>*						channel = nullptr;
		IGuiRemoteProtocolChannelReceiver<TPackage>*				receiver = nullptr;
		TUIMainProc													uiMainProc;
		collections::List<TPackage>									uiPendingPackages;

		volatile bool												started = false;
		volatile bool												stopping = false;
		volatile bool												stopped = false;
		Nullable<WString>											executablePath; 

		EventObject													eventChannelTaskQueued;
		EventObject													eventChannelThreadStopped;
		EventObject													eventUIThreadStopped;

		void UIThreadProc()
		{
			uiMainProc(this);
			uiMainProc = {};

			// Signal and wait for ChannelThreadProc to finish
			stopping = true;
			eventChannelTaskQueued.Signal();
			eventChannelThreadStopped.Wait();

			// All remaining queued callbacks should be executed
			FetchAndExecuteUITasks();
		}

		void ChannelThreadProc()
		{
			// TODO:
			//   The current version always start a channel thread
			//   So that it does not matter whether the underlying IO is sync or async
			//   But async IO does not need a channel thread
			//   Refactor and optimize the channel thread to be optional in the future

			// All members of "_channel" argument to Start is called in this thread
			// So that the implementation does not need to care about thread safety

			// The thread stopped after receiving a signal from UIThreadProc
			while (!stopping)
			{
				eventChannelTaskQueued.Wait();
				FetchAndExecuteChannelTasks();
			}

			// All remaining queued callbacks should be executed
			FetchAndExecuteChannelTasks();
		}

	protected:

		void OnReceive(const TPackage& package) override
		{
			// Called from any thread, very likely the channel thread
			// If it is a response, unblock Submit()
			// If it is an event, send to ProcessRemoteEvents()
			CHECK_FAIL(L"Not Implemented!");
		}

	public:

		void Write(const TPackage& package) override
		{
			// Called from UI thread
			uiPendingPackages.Add(package);
		}

		void Submit() override
		{
			// TODO: Assert if there is at most only one request
			CHECK_FAIL(L"Not Implemented!");

			// Called from UI thread
			QueueToChannelThread([this, packages = std::move(uiPendingPackages)]()
			{
				for (auto&& package : packages)
				{
					channel->Write(package);
				}
				channel->Submit();
			}, &eventChannelTaskQueued);

			// Block until the response of the top request is received
			// Re-entrance recursively is possible
			CHECK_FAIL(L"Not Implemented!");
		}

		void ProcessRemoteEvents() override
		{
			QueueToChannelThread([this]()
			{
				channel->ProcessRemoteEvents();
			}, &eventChannelTaskQueued);

			// Called from UI thread
			FetchAndExecuteUITasks();

			// Process of queued events from channel
			CHECK_FAIL(L"Not Implemented!");
		}

	public:

		/// <summary>
		/// Start the async channel.
		/// </summary>
		/// <param name="_channel">
		/// A channel object that runs in the <see cref="TChannelThreadProc"/> argument offered to startingProc.
		/// </param>
		/// <param name="_uiMainProc">
		/// A callback that runs in the <see cref="TUIThreadProc"/> argument offered to startingProc, which is supposed to call <see cref="SetupRemoteNativeController"/>.
		/// An example of argument to <see cref="SetupRemoteNativeController"/> would be
		///   <see cref="GuiRemoteProtocolDomDiffConverter"/> over
		///   <see cref="repeatfiltering::GuiRemoteProtocolFilter"/> over
		///   <see cref="GuiRemoteProtocolFromJsonChannel"/> over
		///   <see cref="GuiRemoteProtocolAsyncChannelSerializer`1/> (which is an argument to uiProc)
		/// </param>
		/// <param name="startingProc">
		/// A callback executed in the current thread, that responsible to start two threads for arguments <see cref="TChannelThreadProc"/> and <see cref="TUIThreadProc"/>.
		/// </param>
		void Start(
			IGuiRemoteProtocolChannel<TPackage>* _channel,
			TUIMainProc _uiMainProc,
			TStartingProc startingProc
		)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncChannelSerializer<TPackage>::Start(...)#"
			CHECK_ERROR(!started, ERROR_MESSAGE_PREFIX L"This function can only be called once.");

			channel = _channel;
			uiMainProc = _uiMainProc;

			TChannelThreadProc thread_channel = [this]()
			{
				ChannelThreadProc();
			};

			TUIThreadProc thread_ui = [this]()
			{
				UIThreadProc();
			};

			eventChannelTaskQueued.CreateAutoUnsignal(false);
			eventChannelThreadStopped.CreateManualUnsignal(false);
			eventUIThreadStopped.CreateManualUnsignal(false);
			startingProc(thread_channel, thread_ui);
			started = true;

#undef ERROR_MESSAGE_PREFIX
		}

		ChannelAsyncState GetAsyncStateUnsafe()
		{
			if (started)
			{
				if (stopped)
				{
					return ChannelAsyncState::Stopped;
				}
				else
				{
					return ChannelAsyncState::Running;
				}
			}
			else
			{
				return ChannelAsyncState::Ready;
			}
		}

		void WaitForStopped()
		{
			eventUIThreadStopped.Wait();
		}

	public:

		GuiRemoteProtocolAsyncChannelSerializer() = default;
		~GuiRemoteProtocolAsyncChannelSerializer() = default;

		void Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* _receiver) override
		{
			// Called from UI thread
			receiver = _receiver;
			QueueToChannelThreadAndWait([this]()
			{
				channel->Initialize(this);
			}, &eventChannelTaskQueued);
		}

		IGuiRemoteProtocolChannelReceiver<TPackage>* GetReceiver() override
		{
			// Called from UI thread
			return receiver;
		}

		WString GetExecutablePath() override
		{
			// Called from UI thread
			if (!executablePath)
			{
				QueueToChannelThreadAndWait([this]()
				{
					executablePath = channel->GetExecutablePath();
				}, &eventChannelTaskQueued);
			}
			return executablePath.Value();
		}
	};
}

#endif