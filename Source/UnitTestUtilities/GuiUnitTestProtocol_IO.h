/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_IO
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_IO

#include "GuiUnitTestProtocol_Shared.h"

namespace vl::presentation::unittest
{
	class UnitTestRemoteProtocol_IO : public virtual UnitTestRemoteProtocolBase
	{
		using GlobalShortcutKey = remoteprotocol::GlobalShortcutKey;
		using GlobalShortcutKeyList = collections::List<GlobalShortcutKey>;
	public:
		bool						capturing = false;
		GlobalShortcutKeyList		globalShortcutKeys;

		UnitTestRemoteProtocol_IO(const UnitTestScreenConfig& _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
		{
		}

	protected:

/***********************************************************************
IGuiRemoteProtocolMessages (IO)
***********************************************************************/

		void Impl_IOUpdateGlobalShortcutKey(const Ptr<GlobalShortcutKeyList>& arguments)
		{
			if (arguments)
			{
				CopyFrom(globalShortcutKeys, *arguments.Obj());
			}
			else
			{
				globalShortcutKeys.Clear();
			}
		}

		void Impl_IORequireCapture()
		{
			capturing = true;
		}

		void Impl_IOReleaseCapture()
		{
			capturing = false;
		}

		void Impl_IOIsKeyPressing(vint id, const VKEY& arguments)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void Impl_IOIsKeyToggled(vint id, const VKEY& arguments)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};
}

#endif