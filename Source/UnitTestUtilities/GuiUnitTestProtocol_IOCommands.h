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

		NativeWindowMouseInfo MakeMouseInfo()
		{
			NativeWindowMouseInfo info;
			info.ctrl = IsPressing(VKEY::KEY_CONTROL) || IsPressing(VKEY::KEY_LCONTROL) || IsPressing(VKEY::KEY_RCONTROL);
			info.shift = IsPressing(VKEY::KEY_SHIFT) || IsPressing(VKEY::KEY_LSHIFT) || IsPressing(VKEY::KEY_RSHIFT);
			info.left = leftPressing;
			info.middle = middlePressing;
			info.right = rightPressing;
			info.x = mousePosition.Value().x.value;
			info.y = mousePosition.Value().y.value;
			info.wheel = 0;
			info.nonClient = false;
			return info;
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
			NativeRect nativeBounds = { nativeWindow->Convert(bounds.LeftTop()),nativeWindow->Convert(bounds.GetSize()) };
			vint x = nativeBounds.x1.value + (vint)(nativeBounds.Width().value * ratioX) + offsetX;
			vint y = nativeBounds.y1.value + (vint)(nativeBounds.Height().value * ratioY) + offsetY;
			NativePoint windowLocation = nativeWindow->GetBounds().LeftTop();
			return { windowLocation.x.value + x,windowLocation.y.value + y };
		}

		NativePoint LocationOf(controls::GuiControl* control, double ratioX = 0.5, double ratioY = 0.5, vint offsetX = 0, vint offsetY = 0)
		{
			return LocationOf(control->GetBoundsComposition(), ratioX, ratioY, offsetX, offsetY);
		}

#define CLASS_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_IOCommands<TProtocol>::"

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
			UseEvents().OnIOMouseMoving(MakeMouseInfo());
		}

/***********************************************************************
Mouse (Left)
***********************************************************************/

		void _LDown(Nullable<NativePoint> position = {})
		{
#define ERROR_MESSAGE_PREFIX CLASS_PREFIX L"_LDown(...)#"
			if (position) MouseMove(position.Value());
			CHECK_ERROR(!leftPressing, ERROR_MESSAGE_PREFIX L"The button should not be being pressed.");
			leftPressing = true;
			UseEvents().OnIOButtonDown({ remoteprotocol::IOMouseButton::Left,MakeMouseInfo() });
#undef ERROR_MESSAGE_PREFIX
		}

		void _LUp(Nullable<NativePoint> position = {})
		{
#define ERROR_MESSAGE_PREFIX CLASS_PREFIX L"_LUp(...)#"
			if (position) MouseMove(position.Value());
			CHECK_ERROR(leftPressing, ERROR_MESSAGE_PREFIX L"The button should be being pressed.");
			leftPressing = false;
			UseEvents().OnIOButtonUp({ remoteprotocol::IOMouseButton::Left,MakeMouseInfo() });
#undef ERROR_MESSAGE_PREFIX
		}

		void _LDBClick(Nullable<NativePoint> position = {})
		{
#define ERROR_MESSAGE_PREFIX CLASS_PREFIX L"_LDBClick(...)#"
			if (position) MouseMove(position.Value());
			CHECK_ERROR(!leftPressing, ERROR_MESSAGE_PREFIX L"The button should not be being pressed.");
			leftPressing = true;
			UseEvents().OnIOButtonDoubleClick({ remoteprotocol::IOMouseButton::Left,MakeMouseInfo() });
#undef ERROR_MESSAGE_PREFIX
		}

		void LClick(Nullable<NativePoint> position = {})
		{
			_LDown(position);
			_LUp(position);
		}

		void LDBClick(Nullable<NativePoint> position = {})
		{
			_LDown(position);
			_LUp(position);
			_LDBClick(position);
			_LUp(position);
		}

#undef CLASS_PREFIX
	};
}

#endif