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
#include "../../PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"

namespace vl
{
	namespace presentation
	{
		/*
		* Schema of /Dom:
		* --------------------------------------------------------------------------------
		* {
		*   Title: string;
		*   Window: remoteprotocol::WindowSizingConfig;
		*   Dom: remoteprotocol::RenderingDom;
		*   Elements: [{
		*     Id: number;
		*     Type: remoteprotocol::RenderingType;
		*     Data: remoteprotocol::UnitTest_ElementDescVariant;
		*   }];
		*   fatalError?: string;
		* }
		* --------------------------------------------------------------------------------
		*/
		extern Ptr<glr::json::JsonNode>			DumpRemoteProtocolRenderingDom(
													const WString& title,
													const remoteprotocol::WindowSizingConfig& windowSizingConfig,
													Ptr<remoteprotocol::RenderingDom> renderingDom,
													collections::Dictionary<vint, collections::Pair<remoteprotocol::RendererType, Nullable<remoteprotocol::UnitTest_ElementDescVariant>>>& elementData
													);

		extern WString							DumpJsonToString(Ptr<glr::json::JsonNode> json);

		struct IoCommandState
		{
			Nullable<NativePoint>				mousePosition;
			collections::SortedList<VKEY>		pressingKeys;
			bool								leftPressing = false;
			bool								middlePressing = false;
			bool								rightPressing = false;
			bool								capslockToggled = false;
		};

		/*
		* Predefined Commands:
		* --------------------------------------------------------------------------------
		* !Type:<TEXT>
		*   Type <TEXT> to the focused control
		* !Exit
		*   Try to quit the application, it could be blocked by the application itself
		* !KeyDown:Key1+Key2+...+KeyN
		*   Key1 down, Key2 down, ..., KeyN down
		* !KeyUp:Key1+Key2+...+KeyN
		*   KeyN up, ..., Key2 up, Key1 up
		* !KeyPress:Key1+Key2+...+KeyN
		*   Key1 down, Key2 down, ..., KeyN down, KeyN up, ..., Key2 up, Key1 up
		* !MouseMove:X,Y(,ctrl)?(,shift)?(,alt)?
		* !(Left|Middle|Right)(Down|Up|Click|DbClick):X,Y(,ctrl)?(,shift)?(,alt)?
		*   Click means Down/Up
		*   DbClick means Down/Up/Down/DbClick/Up
		* !MouseWheel(Up|Down|Left|Right):ticks(,ctrl)?(,shift)?(,alt)?
		*   WindowMouseInfo_::wheel = ticks * 120 * direction (1 or -1)
		* 
		* --------------------------------------------------------------------------------
		* 
		* If the command satisfies the syntax, queue event handlers and then return "Queued"
		* Otherwise, return "Syntax Error!" followed by command descriptions in this comment
		* This function will crash if any event handler throws
		* Event handlers are queued with INativeAsyncService::InvokeInMainThread after the command is parsed
		*   therefore the returned "Queued" only means the command was accepted, not that it has finished executing
		* 
		* All coordinates are GuiCoordinate
		*   INativeWindow::Convert should be used to convert them to NativeCoordinate before calling the event handlers
		* 
		* During calling the event handlers
		*   ctrl/shift/alt should be set accordingly
		*   the state argument is for remembering whatever is needed
		*   RunIOCommandOnNativeWindow assume it is the only source of IO interactions
		*/
		extern WString							RunIOCommandOnNativeWindow(
													IoCommandState* state,
													INativeController* nativeController,
													INativeWindow* nativeWindow,
													collections::List<INativeWindowListener*>& listeners,
													WString command
													);

		class AutomationServiceBase : public Object, public INativeAutomationService
		{
		protected:
			IoCommandState						ioCommandState;
			bool								stopped = false;

			virtual Nullable<WString>			GetNativeWindowId(INativeWindow* window) = 0;
			virtual INativeWindow*				GetNativeWindow(Nullable<WString> windowId) = 0;

			virtual WString						DumpControlTreeInternal() { return WString::Empty; }
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

			WString DumpControlTree() override
			{
				return !stopped && CanDumpControlTree() ? DumpControlTreeInternal() : WString::Empty;
			}

			bool CanDumpDomTree() override
			{
				return false;
			}

			WString DumpDomTree() override
			{
				return !stopped && CanDumpDomTree() ? DumpDomTreeInternal() : WString::Empty;
			}

			IOCommandAvailability CanRunIOCommands() override
			{
				return IOCommandAvailability::Disabled;
			}

			WString RunIOCommand(Nullable<WString> windowId, const WString& ioCommand) override
			{
				auto availability = stopped ? IOCommandAvailability::Disabled : CanRunIOCommands();
				switch (availability)
				{
				case IOCommandAvailability::Enabled:
					return RunIOCommandInternal(windowId, ioCommand);
				case IOCommandAvailability::ExitOnly:
					return ioCommand == L"!Exit" ? RunIOCommandInternal(windowId, ioCommand) : WString::Unmanaged(L"!Application stopped responding.");
				default:
					return WString::Empty;
				}
			}
		};

		class AutomationServiceRenderer : public AutomationServiceBase
		{
		private:
			remote_renderer::GuiRemoteRendererSingle*	renderer = nullptr;
			SpinLock									lockFatalError; // covers fatalError
			Nullable<WString>							fatalError;

			Nullable<WString>							CopyFatalError();
			
		protected:
			Nullable<WString> GetNativeWindowId(INativeWindow* window) override
			{
				return {};
			}

			INativeWindow* GetNativeWindow(Nullable<WString> windowId) override
			{
				return GetCurrentController()->WindowService()->GetMainWindow();
			}

			WString DumpDomTreeInternal() override;

		public:
			AutomationServiceRenderer(remote_renderer::GuiRemoteRendererSingle* _renderer)
				:renderer(_renderer)
			{
			}

			bool CanDumpDomTree() override
			{
				return true;
			}

			void SetFatalError(Nullable<WString> value);
			IOCommandAvailability CanRunIOCommands() override;
		};
	}
}

#endif
