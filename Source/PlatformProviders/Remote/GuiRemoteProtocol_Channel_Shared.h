/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_CHANNEL

#include "GuiRemoteProtocol_Shared.h"

namespace vl::presentation::remoteprotocol::channeling
{

/***********************************************************************
IGuiRemoteProtocolChannel<T>
***********************************************************************/

	template<typename TPackage>
	class IGuiRemoteProtocolChannelReceiver : public virtual Interface
	{
	public:
		virtual void											OnReceive(const TPackage& package) = 0;
	};

	template<typename TPackage>
	class IGuiRemoteProtocolChannel : public virtual Interface
	{
	public:
		virtual void											Initialize(IGuiRemoteProtocolChannelReceiver<TPackage>* receiver) = 0;
		virtual IGuiRemoteProtocolChannelReceiver<TPackage>*	GetReceiver() = 0;
		virtual void											Write(const TPackage& package) = 0;
		virtual WString											GetExecutablePath() = 0;
		virtual void											Submit() = 0;
		virtual void											ProcessRemoteEvents() = 0;
	};
}

#endif