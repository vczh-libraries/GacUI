#include "TestRemote.h"

namespace remote_empty_window_tests
{
	class EmptyWindowProtocol : public NotImplementedProtocolBase
	{
	public:
		Func<void()>				processRemoteEvents;
		bool						connectionEstablished = false;
		bool						connectionStopped = false;
		WindowSizingConfig			sizingConfig;
	
		EmptyWindowProtocol(Func<void()> _processRemoteEvents)
			: processRemoteEvents(_processRemoteEvents)
		{
			sizingConfig.bounds = { 0,0,0,0 };
			sizingConfig.clientBounds = { 0,0,0,0 };
			sizingConfig.customFramePadding = { 8,8,8,8 };
		}
	
		void Submit() override
		{
			CHECK_ERROR(!connectionStopped, L"IGuiRemoteProtocol::Submit is not allowed to call after connection stopped.");
		}
	
		void ProcessRemoteEvents() override
		{
			processRemoteEvents();
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
			response.clientBounds = { 0,0,640,440 };
			response.scalingX = 1;
			response.scalingY = 1;
			events->RespondControllerGetScreenConfig(id, response);
		}
	
		void RequestControllerConnectionEstablished() override
		{
			CHECK_ERROR(!connectionEstablished, L"IGuiRemoteProtocol::RequestControllerConnectionEstablished is not allowed to call twice.");
			connectionEstablished = true;
		}
	
		void RequestControllerConnectionStopped() override
		{
			CHECK_ERROR(!connectionStopped, L"IGuiRemoteProtocol::RequestControllerConnectionStopped is not allowed to call twice.");
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
	
		void RequestWindowNotifySetBounds(const NativeRect& arguments) override
		{
			sizingConfig.bounds = arguments;
			sizingConfig.clientBounds = sizingConfig.bounds;
			events->OnWindowBoundsUpdated(sizingConfig);
		}
	
		void RequestWindowNotifySetClientSize(const NativeSize& arguments) override
		{
			sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
			sizingConfig.clientBounds = sizingConfig.bounds;
			events->OnWindowBoundsUpdated(sizingConfig);
		}
	
		void RequestWindowNotifySetCustomFrameMode(const bool& arguments) override {}
		void RequestWindowNotifySetMaximizedBox(const bool& arguments) override {}
		void RequestWindowNotifySetMinimizedBox(const bool& arguments) override {}
		void RequestWindowNotifySetBorder(const bool& arguments) override {}
		void RequestWindowNotifySetSizeBox(const bool& arguments) override {}
		void RequestWindowNotifySetIconVisible(const bool& arguments) override {}
		void RequestWindowNotifySetTitleBar(const bool& arguments) override {}
		void RequestWindowNotifyActivate() override {}
	
		void RequestWindowNotifyShow(const WindowShowing& arguments) override
		{
			sizingConfig.sizeState = arguments.sizeState;
		}
	};
	
	class BlockClosingWindowListener : public LoggingWindowListener
	{
	public:
		bool			blockClosing = false;
	
		void BeforeClosing(bool& cancel) override
		{
			LoggingWindowListener::BeforeClosing(cancel);
			cancel = blockClosing;
		}
	};
}
using namespace remote_empty_window_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Create one window and exit immediately")
	{
		EmptyWindowProtocol protocol([&]()
		{
			auto window = GetCurrentController()->WindowService()->GetMainWindow();
			TEST_ASSERT(window);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(window->GetClientSize() == NativeSize(100, 200));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(270, 120, 370, 320));
			TEST_ASSERT(protocol.sizingConfig.clientBounds == NativeRect(270, 120, 370, 320));
			window->Hide(true);
		});
		SetGuiMainProxy([&]()
		{
			TEST_CASE(L"Establish connection")
			{
				TEST_ASSERT(!protocol.connectionEstablished);
				protocol.events->OnControllerConnect();
				TEST_ASSERT(protocol.connectionEstablished);
			});

			TEST_CASE(L"Create and destroy a window")
			{
				auto ws = GetCurrentController()->WindowService();
				auto window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->SetTitle(L"EmptyWindow");
				window->SetClientSize({ 100,200 });
				ws->Run(window);
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});

		TEST_CASE(L"Ensure stopped")
		{
			TEST_ASSERT(protocol.connectionStopped);
		});
	});

	TEST_CATEGORY(L"Close a window")
	{
		LoggingWindowListener listener;
		EmptyWindowProtocol protocol([&]()
		{
			listener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
			auto window = GetCurrentController()->WindowService()->GetMainWindow();
			window->Hide(true);
			listener.AssertCallbacks(
				L"BeforeClosing()",
				L"AfterClosing()",
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
		});
		SetGuiMainProxy([&]()
		{
			protocol.events->OnControllerConnect();
			TEST_CASE(L"Create and close a window")
			{
				auto ws = GetCurrentController()->WindowService();
				auto window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->InstallListener(&listener);
				window->SetTitle(L"EmptyWindow");
				ws->Run(window);
				listener.AssertCallbacks();
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Block closing the main window")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol([&]()
		{
			listener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
			auto window = GetCurrentController()->WindowService()->GetMainWindow();

			listener.blockClosing = true;
			window->Hide(true);
			listener.AssertCallbacks(
				L"BeforeClosing()"
			);

			listener.blockClosing = false;
			window->Hide(true);
			listener.AssertCallbacks(
				L"BeforeClosing()",
				L"AfterClosing()",
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
		});
		SetGuiMainProxy([&]()
		{
			protocol.events->OnControllerConnect();
			TEST_CASE(L"Create and close a window")
			{
				auto ws = GetCurrentController()->WindowService();
				auto window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->InstallListener(&listener);
				ws->Run(window);
				listener.AssertCallbacks();
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Block closing the non-main window (1)")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol([&]()
		{
			listener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);

			BlockClosingWindowListener subListener;
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->GetMainWindow();
			auto subWindow = ws->CreateNativeWindow(INativeWindow::Normal);
			subWindow->InstallListener(&subListener);
			subWindow->SetParent(window);
			listener.AssertCallbacks();
			subListener.AssertCallbacks();

			subWindow->Hide(true);
			listener.AssertCallbacks();
			subListener.AssertCallbacks();

			subWindow->Show();
			listener.AssertCallbacks(
				L"LostFocus()"
			);
			subListener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);

			subListener.blockClosing = true;
			subWindow->Hide(true);
			listener.AssertCallbacks();
			subListener.AssertCallbacks(
				L"BeforeClosing()"
			);

			subListener.blockClosing = false;
			subWindow->Hide(true);
			listener.AssertCallbacks(
				L"GotFocus()"
			);
			subListener.AssertCallbacks(
				L"BeforeClosing()",
				L"AfterClosing()",
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()"
			);

			listener.blockClosing = true;
			window->Hide(true);
			listener.AssertCallbacks(
				L"BeforeClosing()"
			);
			subListener.AssertCallbacks();

			listener.blockClosing = false;
			window->Hide(true);
			listener.AssertCallbacks(
				L"BeforeClosing()",
				L"AfterClosing()",
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
			subListener.AssertCallbacks(
				L"Destroying()",
				L"Destroyed()"
			);
		});
		SetGuiMainProxy([&]()
		{
			protocol.events->OnControllerConnect();
			TEST_CASE(L"Create and close a window")
			{
				auto ws = GetCurrentController()->WindowService();
				auto window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->InstallListener(&listener);
				ws->Run(window);
				listener.AssertCallbacks();
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	// TODO: test ControllerRequestExit with success and blocked
	// TODO: test ControllerForceExit and ensure it skipped INativeWindowListener::(Before|After)Closing
	// TODO: test ControllerDisconnect and ControllerConnect
	// TODO: test enabled/activate/showactivated from INativeWindow and event (bidirectional controlling)
	// TODO: test size status from INativeWindow and event (bidirectional controlling)
}