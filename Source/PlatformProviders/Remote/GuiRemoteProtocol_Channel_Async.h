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
		using TPendingRequestGroup = collections::List<vint>;
		using TPendingRequestStack = collections::List<Ptr<TPendingRequestGroup>>;

		IGuiRemoteProtocolChannel<TPackage>*						channel = nullptr;
		IGuiRemoteProtocolChannelReceiver<TPackage>*				receiver = nullptr;
		TUIMainProc													uiMainProc;
		collections::List<TPackage>									uiPendingPackages;

		SpinLock													lockEvents;
		collections::List<TPackage>									queuedEvents;

		SpinLock													lockResponses;
		EventObject													eventManualResponses;
		collections::Dictionary<vint, TPackage>						queuedResponses;
		TPendingRequestStack										pendingRequests;

		volatile bool												started = false;
		volatile bool												stopping = false;
		volatile bool												stopped = false;
		Nullable<WString>											executablePath; 

		EventObject													eventAutoChannelTaskQueued;
		EventObject													eventManualChannelThreadStopped;
		EventObject													eventManualUIThreadStopped;

		void UIThreadProc()
		{
			uiMainProc(this);
			uiMainProc = {};

			// Signal and wait for ChannelThreadProc to finish
			stopping = true;
			eventAutoChannelTaskQueued.Signal();
			eventManualChannelThreadStopped.Wait();

			// All remaining queued callbacks should be executed
			FetchAndExecuteUITasks();
			eventManualUIThreadStopped.Signal();
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
				eventAutoChannelTaskQueued.Wait();
				FetchAndExecuteChannelTasks();
			}

			// All remaining queued callbacks should be executed
			FetchAndExecuteChannelTasks();
			eventManualChannelThreadStopped.Signal();
		}

	protected:

		bool AreCurrentPendingRequestGroupSatisfied()
		{
			if (pendingRequests.Count() == 0) return false;
			for (vint requestId : *pendingRequests[pendingRequests.Count() - 1].Obj())
			{
				if (!queuedResponses.Keys().Contains(requestId))
				{
					return false;
				}
			}
			return true;
		}

		void OnReceive(const TPackage& package) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncChannelSerializer<TPackage>::OnReceive(...)#"
			// Called from any thread, very likely the channel thread
			// If it is a response, unblock Submit()
			// If it is an event, send to ProcessRemoteEvents()

			auto semantic = ChannelPackageSemantic::Unknown;
			vint id = -1;
			WString name;
			ChannelPackageSemanticUnpack(package, semantic, id, name);

			switch (semantic)
			{
			case ChannelPackageSemantic::Event:
				{
					SPIN_LOCK(lockEvents)
					{
						queuedEvents.Add(package);
					}
				}
				break;
			case ChannelPackageSemantic::Response:
				{
					SPIN_LOCK(lockResponses)
					{
						queuedResponses.Add(id, package);
						if (AreCurrentPendingRequestGroupSatisfied())
						{
							eventManualResponses.Signal();
						}
					}
				}
				break;
			default:
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Only responses and events are expected.");
			}

#undef ERROR_MESSAGE_PREFIX
		}

	public:

		void Write(const TPackage& package) override
		{
			// Called from UI thread
			uiPendingPackages.Add(package);
		}

		void Submit(bool& disconnected) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncChannelSerializer<TPackage>::Submit(...)#"

			// Group all pending requests into a group
			Ptr<TPendingRequestGroup> requestGroup;
			for (auto&& package : uiPendingPackages)
			{
				auto semantic = ChannelPackageSemantic::Unknown;
				vint id = -1;
				WString name;
				ChannelPackageSemanticUnpack(package, semantic, id, name);

				if (semantic == ChannelPackageSemantic::Request)
				{
					if (!requestGroup)
					{
						requestGroup = Ptr(new TPendingRequestGroup);
					}
					requestGroup->Add(id);
				}
			}
			if (requestGroup)
			{
				SPIN_LOCK(lockResponses)
				{
					pendingRequests.Add(requestGroup);
				}
			}

			// Called from UI thread
			QueueToChannelThread([this, packages = std::move(uiPendingPackages)]()
			{
				for (auto&& package : packages)
				{
					channel->Write(package);
				}
				bool disconnected = false;
				channel->Submit(disconnected);
				// TODO: do not ignore diconnected
			}, &eventAutoChannelTaskQueued);

			// Block until the all responses of the top request group are received
			// Re-entrance recursively is possible
			if (requestGroup)
			{
				eventManualResponses.Wait();
				collections::List<TPackage> responses;
				SPIN_LOCK(lockResponses)
				{
					for (vint id : *requestGroup.Obj())
					{
						responses.Add(queuedResponses[id]);
						queuedResponses.Remove(id);
					}
					pendingRequests.RemoveAt(pendingRequests.Count() - 1);

					if (!AreCurrentPendingRequestGroupSatisfied())
					{
						eventManualResponses.Unsignal();
					}
				}

				for (auto&& response : responses)
				{
					receiver->OnReceive(response);
				}
			}

#undef ERROR_MESSAGE_PREFIX
		}

		void ProcessRemoteEvents() override
		{
			QueueToChannelThread([this]()
			{
				channel->ProcessRemoteEvents();
			}, &eventAutoChannelTaskQueued);

			// Called from UI thread
			FetchAndExecuteUITasks();

			// Process of queued events from channel
			collections::List<TPackage> events;
			SPIN_LOCK(lockEvents)
			{
				events = std::move(queuedEvents);
			}

			for (auto&& event : events)
			{
				receiver->OnReceive(event);
			}
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

			eventManualResponses.CreateManualUnsignal(false);
			eventAutoChannelTaskQueued.CreateAutoUnsignal(false);
			eventManualChannelThreadStopped.CreateManualUnsignal(false);
			eventManualUIThreadStopped.CreateManualUnsignal(false);
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
			eventManualUIThreadStopped.Wait();
		}

	public:

		GuiRemoteProtocolAsyncChannelSerializer() = default;
		~GuiRemoteProtocolAsyncChannelSerializer() = default;

		void ExecuteInChannelThread(TTaskProc task)
		{
			QueueToChannelThread(task, &eventAutoChannelTaskQueued);
		}

		void Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* _receiver) override
		{
			// Called from UI thread
			receiver = _receiver;
			QueueToChannelThreadAndWait([this]()
			{
				channel->Initialize(this);
			}, &eventAutoChannelTaskQueued);
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
				}, &eventAutoChannelTaskQueued);
			}
			return executablePath.Value();
		}
	};
}

#endif