/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE

#include "../../NativeWindow/GuiNativeWindow.h"
#include "../../PlatformProviders/Remote/Protocol/Generated/GuiRemoteProtocolSchema.h"

namespace vl
{
	namespace presentation
	{
		/*
		* Schema:
		* {
		*   Title: string;
		*   Window: remoteprotocol::WindowSizingConfig;
		*   Dom: remoteprotocol::RenderingDom;
		*   Elements: [{
		*     Id: number;
		*     Type: remoteprotocol::RenderingType;
		*     Data: remoteprotocol::UnitTest_ElementDescVariant;
		*   }];
		* }
		*/
		extern Ptr<glr::json::JsonNode>			DumpRemoteProtocolRenderingDom(
													const WString& title,
													const remoteprotocol::WindowSizingConfig& windowSizingConfig,
													Ptr<remoteprotocol::RenderingDom> renderingDom,
													collections::Dictionary<vint, collections::Pair<remoteprotocol::RendererType, Nullable<remoteprotocol::UnitTest_ElementDescVariant>>>& elementData
													);

		extern WString							DumpJsonToString(Ptr<glr::json::JsonNode> json);

		/*
		* Predefined Commands:
		* !Type:<TEXT>
		*   Type <TEXT> to the focused control.
		* !Exit
		*   Try to quit the application, it could be blocked by the application itself.
		* (to be edited ...)
		*/
		extern WString							RunIOCommandOnNativeWindow(INativeController* nativeController, collections::List<INativeWindowListener*>& listeners, WString command);

		class AutomationServiceBase : public Object, public INativeAutomationService
		{
		protected:
			bool								stopped = false;

			virtual Nullable<WString>			GetNativeWindowId(INativeWindow* window) = 0;
			virtual INativeWindow*				GetNativeWindow(Nullable<WString> windowId) = 0;

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