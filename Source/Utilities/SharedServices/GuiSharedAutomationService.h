/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE

#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

		/*
		* Schema:
		*/
		extern Ptr<glr::json::JsonNode>			DumpWindowClientArea(controls::GuiWindow* window, bool withCompositionsAndElements, Nullable<WString> windowId, Point offsetLogical, double scaleX, double scaleY);

		/*
		* Predefined Commands:
		*/
		extern WString							RunIOCommandOnNativeWindow(INativeController* nativeController, collections::List<INativeWindowListener*>& listeners, WString command);

		class AutomationServiceBase : public Object, public INativeAutomationService
		{
		protected:
			bool								stopped = false;

			virtual WString						DumpControlTreeInternal(bool withCompositionsAndElements) { return WString::Empty; }
			virtual WString						DumpDomTreeInternal() { return WString::Empty; }
			virtual WString						RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) { return WString::Empty; }
		public:
			AutomationServiceBase() = default;
			~AutomationServiceBase() = default;

			bool Available() override
			{
				return true;
			}

			void Stop() override
			{
				stopped = true;
			}

			bool CanDumpControlTree() override
			{
				return false;
			}

			WString DumpControlTree(bool withCompositionsAndElements) override
			{
				return !stopped && CanDumpControlTree() ? DumpControlTreeInternal(withCompositionsAndElements) : WString::Empty;
			}

			bool CanDumpDomTree() override
			{
				return false;
			}

			WString DumpDomTree() override
			{
				return !stopped && CanDumpDomTree() ? DumpDomTreeInternal() : WString::Empty;
			}

			bool CanRunIOCommands() override
			{
				return false;
			}

			WString RunIOCommand(Nullable<WString> windowId, const WString& ioCommand) override
			{
				return !stopped && CanRunIOCommands() ? RunIOCommandInternal(windowId, ioCommand) : WString::Empty;
			}
		};
	}
}

#endif