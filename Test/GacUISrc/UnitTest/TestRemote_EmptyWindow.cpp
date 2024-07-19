#include "TestRemote.h"

namespace remote_empty_window_tests
{
	class EmptyWindowProtocol : public SingleScreenProtocol
	{
	public:
		bool						connectionEstablished = false;
		bool						connectionStopped = false;
		bool						connectionStoppedSubmitted = false;

		static UnitTestScreenConfig MakeUnitTestScreenConfig()
		{
			UnitTestScreenConfig config;

			config.customFramePadding = { 8,8,8,8 };

			config.fontConfig.defaultFont.fontFamily = L"One";

			config.screenConfig.bounds = { 0,0,640,480 };
			config.screenConfig.clientBounds = { 0,0,640,440 };
			config.screenConfig.scalingX = 1;
			config.screenConfig.scalingY = 1;

			return config;
		}

		EmptyWindowProtocol()
			: SingleScreenProtocol(MakeUnitTestScreenConfig())
		{
		}

		void Submit() override
		{
			CHECK_ERROR(!connectionStoppedSubmitted, L"IGuiRemoteProtocol::Submit is not allowed to call after connection stopped.");
			if (connectionStopped)
			{
				connectionStoppedSubmitted = true;
			}
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
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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
				protocol.GetEvents()->OnControllerConnect();
				TEST_ASSERT(protocol.connectionEstablished);
			});

			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->SetTitle(L"EmptyWindow");
			window->SetClientSize({ 100,200 });
			ws->Run(window);
		});
		StartRemoteControllerTest(protocol);

		TEST_CASE(L"Ensure stopped")
		{
			TEST_ASSERT(protocol.connectionStopped);
		});
	});

	TEST_CATEGORY(L"Close a window")
	{
		LoggingWindowListener listener;
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			window->SetTitle(L"EmptyWindow");
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Block closing the main window")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Block closing the non-main window (1)")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Block closing the non-main window (2)")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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

			listener.blockClosing = true;
			subListener.blockClosing = true;
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
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
			subListener.AssertCallbacks(
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
		});
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"ControllerRequestExit event")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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

			subWindow->Show();
			listener.AssertCallbacks(
				L"LostFocus()"
			);
			subListener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);

			listener.blockClosing = true;
			protocol.GetEvents()->OnControllerRequestExit();
			listener.AssertCallbacks(
				L"BeforeClosing()"
			);
			subListener.AssertCallbacks();

			listener.blockClosing = false;
			protocol.GetEvents()->OnControllerRequestExit();
			listener.AssertCallbacks(
				L"BeforeClosing()",
				L"AfterClosing()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
			subListener.AssertCallbacks(
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
		});
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"ControllerForceExit event")
	{
		BlockClosingWindowListener listener;
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
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

			subWindow->Show();
			listener.AssertCallbacks(
				L"LostFocus()"
			);
			subListener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);

			listener.blockClosing = true;
			protocol.GetEvents()->OnControllerForceExit();
			listener.AssertCallbacks(
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
			subListener.AssertCallbacks(
				L"LostFocus()",
				L"RenderingAsDeactivated()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
		});
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Disconnect and connect during running")
	{
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
		{
			auto window = GetCurrentController()->WindowService()->GetMainWindow();
			TEST_ASSERT(window);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(window->GetClientSize() == NativeSize(100, 200));

			auto assertConfigs = [&](NativeRect bounds)
			{
				TEST_ASSERT(protocol.sizingConfig.bounds == bounds);
				TEST_ASSERT(protocol.sizingConfig.clientBounds == bounds);
				TEST_ASSERT(protocol.styleConfig.title == L"EmptyWindow");
				TEST_ASSERT(protocol.styleConfig.enabled == true); // GuiHostedWindow won't really disable the main native window
				TEST_ASSERT(protocol.styleConfig.topMost == false);
				TEST_ASSERT(protocol.styleConfig.showInTaskBar == false);
				TEST_ASSERT(protocol.styleConfig.customFrameMode == true);
				TEST_ASSERT(protocol.styleConfig.maximizedBox == false);
				TEST_ASSERT(protocol.styleConfig.minimizedBox == false);
				TEST_ASSERT(protocol.styleConfig.border == false);
				TEST_ASSERT(protocol.styleConfig.sizeBox == false);
				TEST_ASSERT(protocol.styleConfig.iconVisible == false);
				TEST_ASSERT(protocol.styleConfig.titleBar == false);
			};

			assertConfigs({ 270, 120, 370, 320 });
			protocol.GetEvents()->OnControllerDisconnect();
			assertConfigs({ 270, 120, 370, 320 });
			protocol.connectionEstablished = false;
			protocol.styleConfig = {};
			protocol.sizingConfig.bounds = { 270, 130, 370, 330 };
			protocol.sizingConfig.clientBounds = { 270, 130, 370, 330 };
			protocol.GetEvents()->OnControllerConnect();
			assertConfigs({ 270, 130, 370, 330 });

			window->Hide(true);
		});
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			auto window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->SetTitle(L"EmptyWindow");
			window->SetClientSize({ 100,200 });
			window->Disable();
			window->SetTopMost(false);
			window->HideInTaskBar();
			window->EnableCustomFrameMode();
			window->SetMaximizedBox(false);
			window->SetMinimizedBox(false);
			window->SetBorder(false);
			window->SetSizeBox(false);
			window->SetIconVisible(false);
			window->SetTitleBar(false);
			TEST_CASE_ASSERT(protocol.styleConfig == WindowStyleConfig());
			TEST_CASE_ASSERT(protocol.sizingConfig.bounds == NativeRect());
			TEST_CASE_ASSERT(protocol.sizingConfig.clientBounds == NativeRect());
			ws->Run(window);
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Bidirectional controlling enabled/activated/focused")
	{
		LoggingWindowListener listener;
		LoggingWindowListener subListener;
		INativeWindowService* ws = nullptr;
		INativeWindow* window = nullptr;
		INativeWindow* subWindow = nullptr;
		EmptyWindowProtocol protocol;

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.styleConfig.activated == true);
			listener.AssertCallbacks(
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);

			subWindow = ws->CreateNativeWindow(INativeWindow::Normal);
			subWindow->InstallListener(&subListener);
			subWindow->SetParent(window);
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
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.styleConfig.activated == true);
			protocol.styleConfig.activated = false;
			protocol.GetEvents()->OnWindowActivatedUpdated(false);
			listener.AssertCallbacks(
				L"GotFocus()",
				L"LostFocus()",
				L"RenderingAsDeactivated()"
			);
			subListener.AssertCallbacks(
				L"LostFocus()",
				L"RenderingAsDeactivated()"
			);

			window->SetActivate();
			listener.AssertCallbacks(
				L"GotFocus()",
				L"RenderingAsActivated()",
				L"LostFocus()"
			);
			subListener.AssertCallbacks(
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.styleConfig.activated == true);
			protocol.styleConfig.activated = false;
			protocol.GetEvents()->OnWindowActivatedUpdated(false);
			listener.AssertCallbacks(
				L"GotFocus()",
				L"LostFocus()",
				L"RenderingAsDeactivated()"
			);
			subListener.AssertCallbacks(
				L"LostFocus()",
				L"RenderingAsDeactivated()"
			);

			subWindow->SetActivate();
			listener.AssertCallbacks(
				L"RenderingAsActivated()"
			);
			subListener.AssertCallbacks(
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.styleConfig.activated == true);
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
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.styleConfig.activated == true);
			protocol.styleConfig.activated = false;
			protocol.GetEvents()->OnWindowActivatedUpdated(false);
			listener.AssertCallbacks(
				L"LostFocus()",
				L"RenderingAsDeactivated()"
			);
			subListener.AssertCallbacks();

			subWindow->SetActivate();
			listener.AssertCallbacks();
			subListener.AssertCallbacks();

			window->SetActivate();
			listener.AssertCallbacks(
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
			subListener.AssertCallbacks();
		});

		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.styleConfig.activated == true);
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
			protocol.GetEvents()->OnControllerConnect();
			ws = GetCurrentController()->WindowService();
			window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Bidirectional control maximized/minimized/restore (1)")
	{
		LoggingWindowListener listener;
		listener.logMoved = true;
		EmptyWindowProtocol protocol;
		INativeWindow* window = nullptr;

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()",
				L"Moved()",
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(270, 120, 370, 320));
			window->ShowMaximized();
		});

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 640, 440));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(0, 0, 640, 440));
			window->ShowMinimized();
		});

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 1, 1));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(640, 480, 641, 481));
			window->ShowRestored();
		});

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(270, 120, 370, 320));
		});

		protocol.OnNextFrame([&]()
		{
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
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			window->SetClientSize({ 100,200 });
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Bidirectional control maximized/minimized/restore (2)")
	{
		LoggingWindowListener listener;
		listener.logMoved = true;
		EmptyWindowProtocol protocol;
		INativeWindow* window = nullptr;

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()",
				L"Moved()",
				L"Opened()",
				L"GotFocus()",
				L"RenderingAsActivated()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(270, 120, 370, 320));
			protocol.GetProtocol()->RequestWindowNotifyShow({ true,INativeWindow::Maximized });
		});

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 640, 440));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(0, 0, 640, 440));
			protocol.GetProtocol()->RequestWindowNotifyShow({ true,INativeWindow::Minimized });
		});

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 1, 1));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(640, 480, 641, 481));
			protocol.GetProtocol()->RequestWindowNotifyShow({ false,INativeWindow::Restored });
			protocol.GetEvents()->OnWindowActivatedUpdated(false);
		});

		protocol.OnNextFrame([&]()
		{
			listener.AssertCallbacks(
				L"Moved()",
				L"LostFocus()",
				L"RenderingAsDeactivated()"
			);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(270, 120, 370, 320));
		});

		protocol.OnNextFrame([&]()
		{
			protocol.GetEvents()->OnControllerRequestExit();
			listener.AssertCallbacks(
				L"BeforeClosing()",
				L"AfterClosing()",
				L"Closed()",
				L"Destroying()",
				L"Destroyed()"
			);
		});

		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();
			auto ws = GetCurrentController()->WindowService();
			window = ws->CreateNativeWindow(INativeWindow::Normal);
			window->InstallListener(&listener);
			window->SetClientSize({ 100,200 });
			ws->Run(window);
			TEST_CASE(L"Test if no more events are generated after existing")
			{
				listener.AssertCallbacks();
			});
		});
		StartRemoteControllerTest(protocol);
	});
}