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
	protected:
		IGuiRemoteProtocolChannel<TPackage>*						channel = nullptr;
		IGuiRemoteProtocolChannelReceiver<TPackage>*				receiver = nullptr;

		void OnReceive(const TPackage& package) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

	public:

		void Write(const TPackage& package) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

	public:

		using TUIMainProc = Func<void()>;
		using TStartingProc = Func<void(TUIMainProc uiMainProc)>;
		using TStoppingProc = Func<void()>;

		void Start(TStartingProc startingProc, TStoppingProc stoppingProc)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool IsStopped()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void WaitForStopped()
		{
			CHECK_FAIL(L"Not Implemented!");
		}

	public:
		GuiRemoteProtocolAsyncChannelSerializer(IGuiRemoteProtocolChannel<TPackage>* _channel)
			: channel(_channel)
		{
		}

		void Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* _receiver) override
		{
			receiver = _receiver;
			channel->Initialize(this);
		}

		IGuiRemoteProtocolChannelReceiver<TPackage>* GetReceiver() override
		{
			return receiver;
		}

		WString GetExecutablePath() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void Submit() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void ProcessRemoteEvents() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};
}

#endif