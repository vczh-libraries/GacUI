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
		bool								capslockToggled = false;

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

		NativeWindowKeyInfo MakeKeyInfo(VKEY key, bool autoRepeatKeyDown = false)
		{
			NativeWindowKeyInfo info;
			info.code = key;
			info.ctrl = IsPressing(VKEY::KEY_CONTROL) || IsPressing(VKEY::KEY_LCONTROL) || IsPressing(VKEY::KEY_RCONTROL);
			info.shift = IsPressing(VKEY::KEY_SHIFT) || IsPressing(VKEY::KEY_LSHIFT) || IsPressing(VKEY::KEY_RSHIFT);
			info.alt = IsPressing(VKEY::KEY_MENU) || IsPressing(VKEY::KEY_LMENU) || IsPressing(VKEY::KEY_RMENU);
			info.capslock = capslockToggled;
			info.autoRepeatKeyDown = autoRepeatKeyDown;
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

		void _KeyDown(VKEY key)
		{
#define ERROR_MESSAGE_PREFIX CLASS_PREFIX L"_KeyDown(...)#"
			CHECK_ERROR(!pressingKeys.Contains(key), ERROR_MESSAGE_PREFIX L"The key is already being pressed.");
			pressingKeys.Add(key);
			if (key == VKEY::KEY_CAPITAL)
			{
				capslockToggled = !capslockToggled;
			}
			UseEvents().OnIOKeyDown(MakeKeyInfo(key, false));
#undef ERROR_MESSAGE_PREFIX
		}

		void _KeyDownRepeat(VKEY key)
		{
#define ERROR_MESSAGE_PREFIX CLASS_PREFIX L"_KeyDownRepeat(...)#"
			CHECK_ERROR(pressingKeys.Contains(key), ERROR_MESSAGE_PREFIX L"The key is not being pressed.");
			UseEvents().OnIOKeyDown(MakeKeyInfo(key, true));
#undef ERROR_MESSAGE_PREFIX
		}

		void _KeyUp(VKEY key)
		{
#define ERROR_MESSAGE_PREFIX CLASS_PREFIX L"_KeyUp(...)#"
			CHECK_ERROR(pressingKeys.Contains(key), ERROR_MESSAGE_PREFIX L"The key is not being pressed.");
			pressingKeys.Remove(key);
			UseEvents().OnIOKeyUp(MakeKeyInfo(key, false));
#undef ERROR_MESSAGE_PREFIX
		}

		void KeyPress(VKEY key)
		{
			_KeyDown(key);
			_KeyUp(key);
		}

		void KeyPress(VKEY key, bool ctrl, bool shift, bool alt)
		{
			if (ctrl) _KeyDown(VKEY::KEY_CONTROL);
			if (shift) _KeyDown(VKEY::KEY_SHIFT);
			if (alt) _KeyDown(VKEY::KEY_MENU);
			KeyPress(key);
			if (alt) _KeyUp(VKEY::KEY_MENU);
			if (shift) _KeyUp(VKEY::KEY_SHIFT);
			if (ctrl) _KeyUp(VKEY::KEY_CONTROL);
		}

/***********************************************************************
Mouse Move Events
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
Mouse Wheel Events
***********************************************************************/

/***********************************************************************
Mouse Click Events
***********************************************************************/

#define DEFINE_MOUSE_ACTIONS(PREFIX, LOWER, UPPER)\
		void _ ## PREFIX ## Down(Nullable<NativePoint> position = {})\
		{\
			if (position) MouseMove(position.Value());\
			CHECK_ERROR(!LOWER ## Pressing, CLASS_PREFIX L"_" L ## #PREFIX L"Down(...)#" L"The button should not be being pressed.");\
			LOWER ## Pressing = true;\
			UseEvents().OnIOButtonDown({ remoteprotocol::IOMouseButton::UPPER,MakeMouseInfo() });\
		}\
		void _ ## PREFIX ## Up(Nullable<NativePoint> position = {})\
		{\
			if (position) MouseMove(position.Value());\
			CHECK_ERROR(LOWER ## Pressing, CLASS_PREFIX L"_" L ## #PREFIX L"Up(...)#" L"The button should be being pressed.");\
			LOWER ## Pressing = false;\
			UseEvents().OnIOButtonUp({ remoteprotocol::IOMouseButton::UPPER,MakeMouseInfo() });\
		}\
		void _ ## PREFIX ## DBClick(Nullable<NativePoint> position = {})\
		{\
			if (position) MouseMove(position.Value());\
			CHECK_ERROR(!LOWER ## Pressing, CLASS_PREFIX L"_" L ## #PREFIX L"DBClick(...)#" L"The button should not be being pressed.");\
			LOWER ## Pressing = true;\
			UseEvents().OnIOButtonDoubleClick({ remoteprotocol::IOMouseButton::UPPER,MakeMouseInfo() });\
		}\
		void PREFIX ## Click(Nullable<NativePoint> position = {})\
		{\
			_ ## PREFIX ## Down(position);\
			_ ## PREFIX ## Up(position);\
		}\
		void PREFIX ## Click(Nullable<NativePoint> position, bool ctrl, bool shift, bool alt)\
		{\
			if (ctrl) _KeyDown(VKEY::KEY_CONTROL);\
			if (shift) _KeyDown(VKEY::KEY_SHIFT);\
			if (alt) _KeyDown(VKEY::KEY_MENU);\
			PREFIX ## Click(position);\
			if (alt) _KeyUp(VKEY::KEY_MENU);\
			if (shift) _KeyUp(VKEY::KEY_SHIFT);\
			if (ctrl) _KeyUp(VKEY::KEY_CONTROL);\
		}\
		void PREFIX ## DBClick(Nullable<NativePoint> position = {})\
		{\
			_ ## PREFIX ## Down(position);\
			_ ## PREFIX ## Up(position);\
			_ ## PREFIX ## DBClick(position);\
			_ ## PREFIX ## Up(position);\
		}\
		void PREFIX ## DBClick(Nullable<NativePoint> position, bool ctrl, bool shift, bool alt)\
		{\
			if (ctrl) _KeyDown(VKEY::KEY_CONTROL);\
			if (shift) _KeyDown(VKEY::KEY_SHIFT);\
			if (alt) _KeyDown(VKEY::KEY_MENU);\
			PREFIX ## DBClick(position);\
			if (alt) _KeyUp(VKEY::KEY_MENU);\
			if (shift) _KeyUp(VKEY::KEY_SHIFT);\
			if (ctrl) _KeyUp(VKEY::KEY_CONTROL);\
		}\

		DEFINE_MOUSE_ACTIONS(L, left, Left);
		DEFINE_MOUSE_ACTIONS(M, middle, Middle);
		DEFINE_MOUSE_ACTIONS(R, right, Right);

#undef DEFINE_MOUSE_ACTIONS

#undef CLASS_PREFIX
	};
}

#endif