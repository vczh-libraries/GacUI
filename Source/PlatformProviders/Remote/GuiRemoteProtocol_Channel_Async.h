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

	template<typename TPackage>
	class GuiRemoteProtocolAsyncChannelSerializer
		: public Object
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
		using TCallbackProc = Func<void()>;

	protected:
		IGuiRemoteProtocolChannel<TPackage>*						channel = nullptr;
		IGuiRemoteProtocolChannelReceiver<TPackage>*				receiver = nullptr;
		TStartingProc												proc_starting;
		TStoppingProc												proc_stopping;
		TUIMainProc													proc_ui;
		volatile bool												started = false;
		volatile bool												stopped = false;

		void UIThreadProc()
		{
			CHECK_FAIL(L"Not Implemented!");
			// Signal and wait for ChannelThreadProc to finish
			// All remaining queued callbacks should be executed
			// Call OnStopped after stopped.
		}

		void ChannelThreadProc()
		{
			// TODO:
			//   The current version always start a channel thread
			//   So that it does not matter whether the underlying IO is sync or asyc
			//   But async IO does not need a channel thread
			//   Refactor and optimize the channel thread to be optional in the future

			// All members of "_channel" argument to Start is called in this thread
			// So that the implementation does not need to care about thread safety
			CHECK_FAIL(L"Not Implemented!");
			// The thread stopped after receiving a signal from UIThreadProc
			// All remaining queued callbacks should be executed
		}

		void OnStopped()
		{
			proc_starting = {};
			proc_stopping = {};
			proc_ui = {};
		}

	protected:

		void OnReceive(const TPackage& package) override
		{
			// Called from any thread
			CHECK_FAIL(L"Not Implemented!");
		}

	public:

		void Write(const TPackage& package) override
		{
			// Called from UI thread
			CHECK_FAIL(L"Not Implemented!");
		}

		void Submit() override
		{
			// Called from UI thread
			// Block until the response of the top request is received
			// Re-entrance recursively is possible
			CHECK_FAIL(L"Not Implemented!");
		}

		void ProcessRemoteEvents() override
		{
			// Called from UI thread
			CHECK_FAIL(L"Not Implemented!");
		}

		void QueueToChannelThread(TCallbackProc callback)
		{
			// Called from any thread
			// The callback will be executed in ChannelThreadProc
			// Throw when the thread has stopped
		}

		void QueueToUIThread(TCallbackProc callback)
		{
			// Called from any thread
			// The callback will be executed in ProcessRemoteEvents
			// Throw when the thread has stopped
		}

	public:

		/// <summary>
		/// Start the async channel.
		/// </summary>
		/// <param name="_channel">
		/// A channel object that runs in the <see cref="TChannelThreadProc"/> argument offered to startingProc.
		/// </param>
		/// <param name="uiProc">
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
		/// <param name="stoppingProc">
		/// A callback executed in the current thread, that responsible to clean up after arguments to startingProc are all ended.
		/// </param>
		void Start(
			IGuiRemoteProtocolChannel<TPackage>* _channel,
			TUIMainProc uiProc,
			TStartingProc startingProc,
			TStoppingProc stoppingProc
		)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::channeling::GuiRemoteProtocolAsyncChannelSerializer<TPackage>::Start(...)#"
			CHECK_ERROR(!started, ERROR_MESSAGE_PREFIX L"This function can only be called once.");

			channel = _channel;
			proc_starting = startingProc;
			proc_stopping = stoppingProc;
			proc_ui = uiProc;

			TChannelThreadProc thread_channel = [this]()
			{
				ChannelThreadProc();
			};

			TUIThreadProc thread_ui = [this]()
			{
				UIThreadProc();
			};

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
			CHECK_FAIL(L"Not Implemented!");
		}

	public:

		GuiRemoteProtocolAsyncChannelSerializer()
		{
		}

		void Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* _receiver) override
		{
			// Called from UI thread
			receiver = _receiver;
			channel->Initialize(this);
		}

		IGuiRemoteProtocolChannelReceiver<TPackage>* GetReceiver() override
		{
			// Called from UI thread
			return receiver;
		}

		WString GetExecutablePath() override
		{
			// Called from UI thread
			CHECK_FAIL(L"Not Implemented!");
		}
	};
}

#endif