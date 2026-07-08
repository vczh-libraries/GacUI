/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE_CONTROLS
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE_CONTROLS

#include "../../Application/Controls/GuiApplication.h"
#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

		class AutomationService : public AutomationServiceBase
		{
		protected:

			WString								DumpControlTreeInternal() override;

		public:
			AutomationService();
			~AutomationService();

			bool								CanDumpControlTree() override;
		};

		class AutomationServiceHosted : public AutomationServiceBase
		{
		protected:
			WString								windowManagement = WString::Unmanaged(L"Hosted");
			
			Nullable<WString>					GetNativeWindowId(INativeWindow* window) override;
			INativeWindow*						GetNativeWindow(Nullable<WString> windowId) override;
			WString								DumpControlTreeInternal() override;

		public:
			AutomationServiceHosted();
			~AutomationServiceHosted();

			bool								CanDumpControlTree() override;
		};

		class RemoteProtocolAutomationService : public AutomationServiceHosted
		{
		protected:
			WString								RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand) override;

		public:
			RemoteProtocolAutomationService();
			~RemoteProtocolAutomationService();

			bool								CanRunIOCommands() override;
		};

		/*
		* Schema of /Controls:
		* --------------------------------------------------------------------------------
		* {
		*   WindowManagement: "MultiWindow" | "Hosted" | "HostedRemoteProtocol";
		*   MainWindow: WindowDump;
		* 
		*   // sub windows are normal window
		*   // no ordering
		*   // available for "MultiWindow"
		*   // otherwise sub windows become child objects of the main window
		*   SubWindows?: WindowDump[];
		*
		*   // popups are usually dropdowns, tooltips or menus
		*   // no ordering
		*   // available for "MultiWindow"
		*   // otherwise popups become child objects of the main window
		*   Popups?: WindowDump[];
		* }
		* 
		* interface WindowDump
		* {
		*   // windowId is used to identify a window
		*   // for "MultiWindow"
		*   //   URL `.../IO/<windowId>` is used to send commands to a specific window
		*   // for other modes
		*   //   URL `.../IO` is used to send commands to the main window
		*   //   since sub windows and popups are all child objects of the main window
		*   //   the main window is responsible for window management, dispatching IO commands to the correct target
		*   // /IO parses the command synchronously and returns "Syntax Error!" or "Queued"
		*   //   "Queued" only means the command was accepted, not that it has finished executing
		*   windowId?: string;
		* 
		*   // bounds defines the valid coordinate space for IO commands
		*   // IO commands use the client area of a native window as the coordinate space
		*   // such native window is a OS native window
		*   // for a window that owns a OS native window, the valid coordinate space is the client area
		*   // for a window that doesn't own a OS native window, the valid coordinate space is the partial rectangle of the main window client area
		* 
		*   // (x1,y1) always (0,0) for main window or when "MultiWindow"
		*   bounds: { x1: number, y1: number, x2: number, y2: number };
		* 
		*   // available for main window in "Hosted" and "HostedRemoteProtocol"
		*   // ordered from bottom to top
		*   subWindowsInZOrder?: WindowDump[];
		* 
		*   title: string;
		* 
		*   // begins with GuiWindow::GetBoundsComposition()
		*   composition: CompositionDump;
		* }
		* 
		* interface CompositionDump
		* {
		*   // GuiGraphicsComposition::GetCachedBounds() converted to global bounds then offseted by "offset"
		*   bounds: { x1: number, y1: number, x2: number, y2: number };
		* 
		*   // available when the composition is or inherits from:
		*   //   GuiTableComposition			: "Table:rows*columns"
		*   //   GuiCellComposition				: "Cell:(row,column)*(rowSpan,columnSpan)"
		*   //     available only when its parent composition is GuiTableComposition
		*   //   GuiRowSplitterComposition		: "RowSplitter:rowsToTheTop"
		*   //   GuiColumnSplitterComposition	: "ColumnSplitter:columnsToTheLeft"
		*   //   GuiStackComposition			: "Stack"
		*   //   GuiStackItemComposition		: "StackItem:index"
		*   //     available only when its parent composition is GuiStackComposition
		*   //     index is defined by stack->GetStackItems().IndexOf(stackItem)
		*   //   GuiFlowComposition: "Flow"
		*   //   GuiFlowItemComposition			: "FlowItem:index"
		*   //     available only when its parent composition is GuiFlowComposition
		*   //     index is defined by flow->GetFlowItems().IndexOf(flowItem)
		*   layout?: string;
		*
		*   // available when GuiGraphicsComposition::GetAssociatedCursor is not null
		*   cursor?: string;
		* 
		*   // available when GuiGraphicsComposition::GetOwnedElement is not null and is or inherits from:
		*   //   GuiSolidBorderElement			: "Border:color,shape
		*   //   Gui3DBorderElement:			: "3DBorder:color1,color2"
		*   //   Gui3DSplitterElement:			: "3DSplitter:color1,color2,direction"
		*   //   GuiSolidBackground				: "Background:color,shape
		*   //   GuiGradientBackgroundElement	: "Gradient:color1,color2,direction,shape"
		*   //   GuiInnerShadowElement			: "InnerShadow:color,thickness"
		*   //   GuiSolidLabelElement			: "Label:color,fontProperties.fontFamily,fontProperties.size(,WrapLine)?(,Ellipse)?(,Multiline)?"
		*   //   GuiImageFrameElement			: "Image"
		*   //   GuiPolygonElement				: "Polygon"
		*   //   GuiDocumentElement				: "Document:Selection(caretBegin.row,caretBegin.column)-(caretEnd.row,caretEnd.column)(,PasswordChar=char)?(,WrapLine)?"
		*   //
		*   // Color is in #RRGGBBAA format
		*   // elementText is only available for GuiSolidLabelElement, storing its text
		*   // elementDocument is only availaboe for GuiDocumentElement, storing its document in XML representation
		*   //   The XML representation is done by calling GenerateToStream(XmlPrint(SaveToXml))
		*   element?: string;
		*   elementText?: string;
		*   elementDocument?: string;
		* 
		*   // available when GuiGraphicsComposition::GetAssociatedControl is not null
		*   // storing PrintControlThemeName(GuiControl::GetControlThemeName())
		*   control?: string;
		*
		*   children?: CompositionDump[];
		* }
		* --------------------------------------------------------------------------------
		* 
		* This function construct the WindowDump part (without subWindowsInZOrder)
		*/
		extern Ptr<glr::json::JsonNode>			DumpWindowClientArea(controls::GuiWindow* window, Nullable<WString> windowId, Point offset);
	}
}

#endif
