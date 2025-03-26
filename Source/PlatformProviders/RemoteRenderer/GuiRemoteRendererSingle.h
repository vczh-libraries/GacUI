/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteRendererSingle

***********************************************************************/

#include "../Remote/GuiRemoteProtocol_Shared.h"

namespace vl::presentation::remote_renderer
{
	class GuiRemoteRendererSingle
		: public Object
		, public virtual IGuiRemoteProtocol
		, protected virtual INativeWindowListener
	{
	protected:
		INativeWindow*				window = nullptr;
		IGuiRemoteProtocolEvents*	events = nullptr;

		void			Opened() override;

	public:
		GuiRemoteRendererSingle();
		~GuiRemoteRendererSingle();

		void			RegisterMainWindow(INativeWindow* _window);
		void			UnregisterMainWindow();
		WString			GetExecutablePath() override;
		void			Initialize(IGuiRemoteProtocolEvents* _events) override;
		void			Submit(bool& disconnected) override;
		void			ProcessRemoteEvents() override;

		/***********************************************************************
		Controller
		***********************************************************************/

		void			RequestControllerGetFontConfig(vint id) override;
		void			RequestControllerGetScreenConfig(vint id) override;
		void			RequestControllerConnectionEstablished() override;
		void			RequestControllerConnectionStopped() override;

		/***********************************************************************
		MainWindow
		***********************************************************************/
	
		void			RequestWindowGetBounds(vint id) override;
		void			RequestWindowNotifySetTitle(const WString& arguments) override;
		void			RequestWindowNotifySetEnabled(const bool& arguments) override;
		void			RequestWindowNotifySetTopMost(const bool& arguments) override;
		void			RequestWindowNotifySetShowInTaskBar(const bool& arguments) override;
		void			RequestWindowNotifySetBounds(const NativeRect& arguments) override;
		void			RequestWindowNotifySetClientSize(const NativeSize& arguments) override;
		void			RequestWindowNotifySetCustomFrameMode(const bool& arguments) override;
		void			RequestWindowNotifySetMaximizedBox(const bool& arguments) override;
		void			RequestWindowNotifySetMinimizedBox(const bool& arguments) override;
		void			RequestWindowNotifySetBorder(const bool& arguments) override;
		void			RequestWindowNotifySetSizeBox(const bool& arguments) override;
		void			RequestWindowNotifySetIconVisible(const bool& arguments) override;
		void			RequestWindowNotifySetTitleBar(const bool& arguments) override;
		void			RequestWindowNotifyActivate() override;
		void			RequestWindowNotifyShow(const remoteprotocol::WindowShowing& arguments) override;

		/***********************************************************************
		IO
		***********************************************************************/

		void			RequestIOUpdateGlobalShortcutKey(const Ptr<collections::List<remoteprotocol::GlobalShortcutKey>>& arguments) override;
		void			RequestIORequireCapture() override;
		void			RequestIOReleaseCapture() override;
		void			RequestIOIsKeyPressing(vint id, const VKEY& arguments) override;
		void			RequestIOIsKeyToggled(vint id, const VKEY& arguments) override;

		/***********************************************************************
		Rendering
		***********************************************************************/

		void			RequestRendererCreated(const Ptr<collections::List<remoteprotocol::RendererCreation>>& arguments) override;
		void			RequestRendererDestroyed(const Ptr<collections::List<vint>>& arguments) override;
		void			RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments) override;
		void			RequestRendererEndRendering(vint id) override;
		void			RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments) override;
		void			RequestRendererEndBoundary() override;
		void			RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments) override;

		/***********************************************************************
		Rendering - Elements
		***********************************************************************/

		void			RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments) override;
		void			RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments) override;
		void			RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments) override;
		void			RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments) override;
		void			RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments) override;
		void			RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments) override;
		void			RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments) override;
		void			RequestRendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments) override;
		void			RequestImageCreated(vint id, const remoteprotocol::ImageCreation& arguments) override;
		void			RequestImageDestroyed(const vint& arguments) override;
		void			RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments) override;

		/***********************************************************************
		Rendering - SyncDom
		***********************************************************************/

		void			RequestRendererRenderDom(const Ptr<remoteprotocol::RenderingDom>& arguments) override;
		void			RequestRendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments) override;
	};
}