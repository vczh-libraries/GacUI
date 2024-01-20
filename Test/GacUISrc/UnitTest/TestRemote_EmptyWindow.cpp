#include "TestRemote.h"

class EmptyWindowProtocol : public NotImplementedProtocolBase
{
public:
	static EmptyWindowProtocol*	instance;
	IGuiRemoteProtocolEvents*	events = nullptr;
	bool						connectionEstablished = false;
	bool						connectionStopped = false;
	bool						connectionStoppedAndSubmitted = false;
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
		TEST_ASSERT(!connectionStoppedAndSubmitted);
		if (connectionStopped) connectionStoppedAndSubmitted = true;
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
		TEST_ASSERT(!connectionEstablished);
		connectionEstablished = true;
	}

	void RequestControllerConnectionStopped() override
	{
		TEST_ASSERT(!connectionStopped);
		connectionStopped = true;
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
		events->RespondWindowSetBounds(id, sizingConfig);
	}

	void RequestWindowSetClientSize(vint id, const ::vl::presentation::NativeSize& arguments) override
	{
		sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
		sizingConfig.clientBounds = sizingConfig.bounds;
		events->RespondWindowSetClientSize(id, sizingConfig);
	}

	void RequestWindowSetCustomFrameMode(vint id, const bool& arguments) override
	{
		events->RespondWindowSetCustomFrameMode(id, sizingConfig);
	}

	void RequestWindowSetMaximizedBox(vint id, const bool& arguments) override
	{
		events->RespondWindowSetMaximizedBox(id, sizingConfig);
	}

	void RequestWindowSetMinimizedBox(vint id, const bool& arguments) override
	{
		events->RespondWindowSetMinimizedBox(id, sizingConfig);
	}

	void RequestWindowSetBorder(vint id, const bool& arguments) override
	{
		events->RespondWindowSetBorder(id, sizingConfig);
	}

	void RequestWindowSetSizeBox(vint id, const bool& arguments) override
	{
		events->RespondWindowSetSizeBox(id, sizingConfig);
	}

	void RequestWindowSetIconVisible(vint id, const bool& arguments) override
	{
		events->RespondWindowSetIconVisible(id, sizingConfig);
	}

	void RequestWindowSetTitleBar(vint id, const bool& arguments) override
	{
		events->RespondWindowSetTitleBar(id, sizingConfig);
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
			TEST_CASE(L"Establish connection")
			{
				TEST_ASSERT(!EmptyWindowProtocol::instance->connectionEstablished);
				EmptyWindowProtocol::instance->events->OnControllerConnect();
				TEST_ASSERT(EmptyWindowProtocol::instance->connectionEstablished);
			});

			TEST_CASE(L"Create and destroy a window")
			{
				auto ws = GetCurrentController()->WindowService();
				auto window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->SetTitle(L"EmptyWindow");
				window->SetClientSize({ 100,200 });
				ws->Run(window);
				ws->DestroyNativeWindow(window);

				// TODO: find a place to test if window is shown with the expected size
			});
		});
		SetupRemoteNativeController(&protocol);
		SetGuiMainProxy(nullptr);
	});
}