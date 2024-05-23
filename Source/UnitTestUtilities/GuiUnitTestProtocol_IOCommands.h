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

		NativePoint LocationOf(compositions::GuiGraphicsComposition* composition, double ratioX = 0.5, double ratioY = 0.5, vint offsetX = 0, vint offsetY = 0)
		{
			INativeWindow* nativeWindow = composition->GetRelatedControlHost()->GetNativeWindow();
			Rect bounds = composition->GetGlobalBounds();
			NativeRect nativeBounds = nativeWindow->Convert(bounds);
			vint x = nativeBounds.x1 + (vint)(nativeBounds.Width().value * ratioX) + offsetX;
			vint y = nativeBounds.y1 + (vint)(nativeBounds.Height().value * ratioY) + offsetY;
			NativePoint windowLocation = nativeWindow->GetBounds().LeftTop();
			return { windowLocation.x.value + x,windowLocation.y.value + y };
		}

		NativePoint LocationOf(controls::GuiControl* control, double ratioX = 0.5, double ratioY = 0.5, vint offsetX = 0, vint offsetY = 0)
		{
			return LocationOf(control->GetBoundsComposition(), ratioX, ratioY, offsetX, offsetY);
		}
	};
}

#endif