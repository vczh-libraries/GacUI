#include "GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
INativeWindowListener
***********************************************************************/

		INativeWindowListener::HitTestResult INativeWindowListener::HitTest(NativePoint location)
		{
			return INativeWindowListener::NoDecision;
		}

		void INativeWindowListener::Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder)
		{
		}

		void INativeWindowListener::Moved()
		{
		}

		void INativeWindowListener::DpiChanged(bool preparing)
		{
		}

		void INativeWindowListener::Enabled()
		{
		}

		void INativeWindowListener::Disabled()
		{
		}

		void INativeWindowListener::GotFocus()
		{
		}

		void INativeWindowListener::LostFocus()
		{
		}

		void INativeWindowListener::RenderingAsActivated()
		{
		}

		void INativeWindowListener::RenderingAsDeactivated()
		{
		}

		void INativeWindowListener::Opened()
		{
		}

		void INativeWindowListener::BeforeClosing(bool& cancel)
		{
		}

		void INativeWindowListener::AfterClosing()
		{
		}

		void INativeWindowListener::Closed()
		{
		}

		void INativeWindowListener::Paint()
		{
		}

		void INativeWindowListener::Destroying()
		{
		}

		void INativeWindowListener::Destroyed()
		{
		}

		void INativeWindowListener::LeftButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::LeftButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::RightButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::RightButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MiddleButtonDown(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MiddleButtonUp(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::HorizontalWheel(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::VerticalWheel(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MouseMoving(const NativeWindowMouseInfo& info)
		{
		}

		void INativeWindowListener::MouseEntered()
		{
		}

		void INativeWindowListener::MouseLeaved()
		{
		}

		void INativeWindowListener::KeyDown(const NativeWindowKeyInfo& info)
		{
		}

		void INativeWindowListener::KeyUp(const NativeWindowKeyInfo& info)
		{
		}

		void INativeWindowListener::SysKeyDown(const NativeWindowKeyInfo& info)
		{
		}

		void INativeWindowListener::SysKeyUp(const NativeWindowKeyInfo& info)
		{
		}

		void INativeWindowListener::Char(const NativeWindowCharInfo& info)
		{
		}

		bool INativeWindowListener::NeedRefresh()
		{
			return false;
		}

		void INativeWindowListener::ForceRefresh(bool handleFailure, bool& failureByResized, bool& failureByLostDevice)
		{
		}

		void INativeWindowListener::BecomeNonMainHostedWindow()
		{
		}

/***********************************************************************
INativeControllerListener
***********************************************************************/

		void INativeControllerListener::GlobalTimer()
		{
		}

		void INativeControllerListener::ClipboardUpdated()
		{
		}

		void INativeControllerListener::NativeWindowCreated(INativeWindow* window)
		{
		}

		void INativeControllerListener::NativeWindowDestroying(INativeWindow* window)
		{
		}

/***********************************************************************
Native Window Provider
***********************************************************************/

		class SubstitutableController;
		INativeController* nativeController = nullptr;
		SubstitutableController* substitutableController = nullptr;

		class SubstitutableController
			: public Object
			, public INativeController
			, public INativeServiceSubstitution
		{
		protected:
		public:
			WString GetExecutablePath() override
			{
				return nativeController->GetExecutablePath();
			}

			// INativeServiceSubstitution

#define SUBSTITUTE_SERVICE(NAME)															\
			void Substitute(INative##NAME##Service* service, bool optional) override		\
			{																				\
				CHECK_FAIL(L"Not Implemented!");											\
			}																				\
																							\
			void Unsubstitute(INative##NAME##Service* service) override						\
			{																				\
				CHECK_FAIL(L"Not Implemented!");											\
			}																				\

			GUI_SUBSTITUTABLE_SERVICES(SUBSTITUTE_SERVICE)
#undef SUBSTITUTE_SERVICE

			// INativeController

#define GET_SUBSTITUTABLE_SERVICE(NAME)\
			INative##NAME##Service* NAME##Service() override		\
			{														\
				return nativeController->NAME##Service();			\
			}														\

			GUI_SUBSTITUTABLE_SERVICES(GET_SUBSTITUTABLE_SERVICE)
#undef GET_SUBSTITUTABLE_SERVICE


#define GET_UNSUBSTITUTABLE_SERVICE(NAME)							\
			INative##NAME##Service* NAME##Service() override		\
			{														\
				return nativeController->NAME##Service();			\
			}														\

			GUI_UNSUBSTITUTABLE_SERVICES(GET_UNSUBSTITUTABLE_SERVICE)
#undef GET_UNSUBSTITUTABLE_SERVICE
		};

		INativeServiceSubstitution* GetNativeServiceSubstitution()
		{
			return substitutableController;
		}

		INativeController* GetCurrentController()
		{
			return substitutableController;
		}

		void SetNativeController(INativeController* controller)
		{
			nativeController = controller;

			if (nativeController)
			{
				if (!substitutableController)
				{
					substitutableController = new SubstitutableController();
				}
			}
			else
			{
				if (substitutableController)
				{
					delete substitutableController;
					substitutableController = 0;
				}
			}
		}

/***********************************************************************
Helper Functions
***********************************************************************/

		INativeCursor* GetCursorFromHitTest(INativeWindowListener::HitTestResult hitTestResult, INativeResourceService* resourceService)
		{
			switch (hitTestResult)
			{
			case INativeWindowListener::BorderLeft:
			case INativeWindowListener::BorderRight:
				return resourceService->GetSystemCursor(INativeCursor::SizeWE);
			case INativeWindowListener::BorderTop:
			case INativeWindowListener::BorderBottom:
				return resourceService->GetSystemCursor(INativeCursor::SizeNS);
			case INativeWindowListener::BorderLeftTop:
			case INativeWindowListener::BorderRightBottom:
				return resourceService->GetSystemCursor(INativeCursor::SizeNWSE);
			case INativeWindowListener::BorderRightTop:
			case INativeWindowListener::BorderLeftBottom:
				return resourceService->GetSystemCursor(INativeCursor::SizeNESW);
			default:
				return nullptr;
			}
		}
	}
}