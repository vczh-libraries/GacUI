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
		Nullable<NativePoint>				mousePosition;
		collections::SortedList<VKEY>		pressingKeys;
		bool								leftPressing = false;
		bool								middlePressing = false;
		bool								rightPressing = false;

		IGuiRemoteProtocolEvents& UseEvents()
		{
			return *this->GetEvents();
		}

		bool IsPressing(VKEY key)
		{
			return pressingKeys.Contains(key);
		}

		NativeWindowMouseInfo MakeMouseInfo(NativePoint position)
		{
			NativeWindowMouseInfo info;
			info.ctrl = IsPressing(VKEY::KEY_CONTROL) || IsPressing(VKEY::KEY_LCONTROL) || IsPressing(VKEY::KEY_RCONTROL);
			info.shift = IsPressing(VKEY::KEY_SHIFT) || IsPressing(VKEY::KEY_LSHIFT) || IsPressing(VKEY::KEY_RSHIFT);
			info.left = leftPressing;
			info.middle = middlePressing;
			info.right = rightPressing;
			info.x = position.x.value;
			info.y = position.y.value;
			info.wheel = 0;
			info.nonClient = false;
		}

	public:

		template<typename ...TArgs>
		UnitTestRemoteProtocol_IOCommands(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}

/***********************************************************************
Helper Functions
***********************************************************************/

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

/***********************************************************************
Keys
***********************************************************************/

/***********************************************************************
Mouse
***********************************************************************/

		void MouseMove(NativePoint position)
		{
			if (!mousePosition)
			{
				UseEvents().OnIOMouseEntered();
				goto DO_MOUSE_MOVE;
			}

			if (mousePosition.Value() == position) return;
		DO_MOUSE_MOVE:

			mousePosition = position;
			UseEvents().OnIOMouseMoving(MakeMouseInfo(position));
		}
	};
}

#endif