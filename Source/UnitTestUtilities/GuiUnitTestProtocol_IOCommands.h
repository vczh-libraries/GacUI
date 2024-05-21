/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_IOCOMMANDS
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_IOCOMMANDS

#include "GuiUnitTestProtocol_Shared.h"

namespace vl::presentation::unittest
{

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/
	
	template<typename TProtocol>
	class UnitTestRemoteProtocol_IOCommands : public TProtocol
	{
	protected:
	public:

		template<typename ...TArgs>
		UnitTestRemoteProtocol_IOCommands(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}
	};
}

#endif