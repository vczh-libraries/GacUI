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
	template<typename TProtocol>
	class UnitTestRemoteProtocol_IO : public TProtocol
	{
		using GlobalShortcutKey = remoteprotocol::GlobalShortcutKey;
		using GlobalShortcutKeyList = collections::List<GlobalShortcutKey>;
	public:
		bool						capturing = false;
		GlobalShortcutKeyList		globalShortcutKeys;

		template<typename ...TArgs>
		UnitTestRemoteProtocol_IO(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}

	protected:

/***********************************************************************
IGuiRemoteProtocolMessages (IO)
***********************************************************************/

		void RequestIOUpdateGlobalShortcutKey(const Ptr<GlobalShortcutKeyList>& arguments) override
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

		void RequestIORequireCapture() override
		{
			capturing = true;
		}

		void RequestIOReleaseCapture() override
		{
			capturing = false;
		}

		void RequestIOIsKeyPressing(vint id, const VKEY& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestIOIsKeyToggled(vint id, const VKEY& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};
}

#endif