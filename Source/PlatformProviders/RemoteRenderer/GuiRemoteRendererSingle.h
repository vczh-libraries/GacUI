/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteRendererSingle

***********************************************************************/

#include "../Remote/GuiRemoteProtocol_Shared.h"
#include "../Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::remote_renderer
{
	class GuiRemoteRendererSingle
		: public Object
		, public virtual IGuiRemoteProtocol
		, protected virtual INativeWindowListener
	{
	protected:
		INativeWindow*							window = nullptr;
		INativeScreen*							screen = nullptr;
		IGuiRemoteProtocolEvents*				events = nullptr;

		bool									updatingBounds = false;
		remoteprotocol::WindowSizingConfig		windowSizingConfig;

		remoteprotocol::ScreenConfig			GetScreenConfig(INativeScreen* screen);
		remoteprotocol::WindowSizingConfig		GetWindowSizingConfig();
		void									UpdateConfigsIfNecessary();

		void									Opened() override;
		void									Moved() override;
		void									DpiChanged(bool preparing) override;

	protected:
		using ElementMap = collections::Dictionary<vint, Ptr<elements::IGuiGraphicsElement>>;
		using ImageMap = collections::Dictionary<vint, Ptr<INativeImage>>;
		using LabelMeasuringList = collections::List<collections::Pair<vint, remoteprotocol::ElementSolidLabelMeasuringRequest>>;

		remoteprotocol::ElementMeasurings		elementMeasurings;
		LabelMeasuringList						labelMeasurings;
		ElementMap								availableElements;
		ImageMap								availableImages;
		Ptr<remoteprotocol::RenderingDom>		renderingDom;
		remoteprotocol::DomIndex				renderingDomIndex;

		Alignment								GetAlignment(remoteprotocol::ElementHorizontalAlignment alignment);
		Alignment								GetAlignment(remoteprotocol::ElementVerticalAlignment alignment);

	public:
		GuiRemoteRendererSingle();
		~GuiRemoteRendererSingle();

		void			RegisterMainWindow(INativeWindow* _window);
		void			UnregisterMainWindow();
		WString			GetExecutablePath() override;
		void			Initialize(IGuiRemoteProtocolEvents* _events) override;
		void			Submit(bool& disconnected) override;
		void			ProcessRemoteEvents() override;


#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME() override;
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id) override;
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments) override;
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments) override;
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
	};
}