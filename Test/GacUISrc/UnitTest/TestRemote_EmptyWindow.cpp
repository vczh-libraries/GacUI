#include "TestRemote.h"

class EmptyWindowProtocol : public NotImplementedProtocolBase
{
public:
	static EmptyWindowProtocol*	instance;
	IGuiRemoteProtocolEvents*	events = nullptr;
	WindowSizingConfig			sizingConfig;

	EmptyWindowProtocol()
	{
		CHECK_ERROR(instance == nullptr, L"EmptyWindowProtocol can only have one instance");
		instance = this;

		sizingConfig.bounds = { 0,0,0,0 };
		sizingConfig.clientBounds = { 0,0,0,0 };
		sizingConfig.customFramePadding = { 8,8,8,8 };
	}

	~EmptyWindowProtocol()
	{
		instance = nullptr;
	}

	void Initialize(IGuiRemoteProtocolEvents* _events) override
	{
		events = _events;
	}

	void Submit() override
	{
		// respond to messages immediately in this test
		// no need to submit
	}

	WString GetExecutablePath() override
	{
		return L"/EmptyWindow/Protocol.exe";
	}

	void RequestControllerGetFontConfig(vint id) override
	{
		FontConfig response;
		response.defaultFont.fontFamily = L"One";
		response.supportedFonts = Ptr(new List<WString>());
		response.supportedFonts->Add(L"One");
		response.supportedFonts->Add(L"Two");
		response.supportedFonts->Add(L"Three");
		events->RespondControllerGetFontConfig(id, response);
	}

	void RequestControllerGetScreenConfig(vint id) override
	{
		ScreenConfig response;
		response.bounds = { 0,0,640,480};
		response.clientBounds = { 10,10,630,470 };
		response.scalingX = 1;
		response.scalingY = 1;
		events->RespondControllerGetScreenConfig(id, response);
	}

	void RequestControllerConnectionEstablished() override
	{
	}

	void RequestWindowGetBounds(vint id) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowNotifySetTitle(const ::vl::WString& arguments) override
	{
	}

	void RequestWindowNotifySetEnabled(const bool& arguments) override
	{
	}

	void RequestWindowNotifySetTopMost(const bool& arguments) override
	{
	}

	void RequestWindowNotifySetShowInTaskBar(const bool& arguments) override
	{
	}

	void RequestWindowSetBounds(vint id, const ::vl::presentation::NativeRect& arguments) override
	{
		sizingConfig.bounds = arguments;
		sizingConfig.clientBounds = sizingConfig.bounds;
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetClientSize(vint id, const ::vl::presentation::NativeSize& arguments) override
	{
		sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
		sizingConfig.clientBounds = sizingConfig.bounds;
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetCustomFrameMode(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetMaximizedBox(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetMinimizedBox(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetBorder(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetSizeBox(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetIconVisible(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowSetTitleBar(vint id, const bool& arguments) override
	{
		events->RespondWindowGetBounds(id, sizingConfig);
	}

	void RequestWindowNotifyActivate() override
	{
	}

	void RequestWindowNotifyShowActivated() override
	{
	}

	void RequestWindowNotifyShowDeactivated() override
	{
	}

	void RequestWindowNotifyShowMaximized() override
	{
	}

	void RequestWindowNotifyShowMinimized() override
	{
	}
};
EmptyWindowProtocol* EmptyWindowProtocol::instance = nullptr;

TEST_FILE
{
	TEST_CATEGORY(L"Create one window and exit immediately")
	{
		EmptyWindowProtocol protocol;
		SetGuiMainProxy([]()
		{
			EmptyWindowProtocol::instance->events->OnControllerConnect();

			TEST_CASE(L"Create and destroy a window")
			{
				auto ws = GetCurrentController()->WindowService();
				auto window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->SetTitle(L"EmptyWindow");
				window->SetClientSize({ 100,200 });
				ws->Run(window);
				ws->DestroyNativeWindow(window);
			});

			EmptyWindowProtocol::instance->events->OnControllerDisconnect();
			EmptyWindowProtocol::instance->events->OnControllerExit();
		});
		SetupRemoteNativeController(&protocol);
		SetGuiMainProxy(nullptr);
	});
}