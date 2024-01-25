#include "TestRemote.h"

namespace remote_empty_window_tests
{
	struct WindowStyleConfig
	{
		WString						title;
		bool						enabled = true;
		bool						topMost = false;
		bool						showInTaskBar = true;

		bool						customFrameMode = false;
		bool						maximizedBox = true;
		bool						minimizedBox = true;
		bool						border = true;
		bool						sizeBox = true;
		bool						iconVisible = true;
		bool						titleBar = true;
		bool						activated = false;

		auto operator<=>(const WindowStyleConfig&) const = default;
	};

	class EmptyWindowProtocol : public NotImplementedProtocolBase
	{
	public:
		List<Func<void()>>			processRemoteEvents;
		vint						nextEventIndex = 0;

		bool						connectionEstablished = false;
		bool						connectionStopped = false;
		WindowSizingConfig			sizingConfig;
		WindowStyleConfig			styleConfig;
	
		EmptyWindowProtocol()
		{
			sizingConfig.bounds = { 0,0,0,0 };
			sizingConfig.clientBounds = { 0,0,0,0 };
			sizingConfig.customFramePadding = { 8,8,8,8 };
		}

		template<typename TCallback>
		void OnNextFrame(TCallback&& callback)
		{
			processRemoteEvents.Add(std::move(callback));
		}
	
		void Submit() override
		{
			CHECK_ERROR(!connectionStopped, L"IGuiRemoteProtocol::Submit is not allowed to call after connection stopped.");
		}
	
		void ProcessRemoteEvents() override
		{
			TEST_ASSERT(nextEventIndex < processRemoteEvents.Count());
			processRemoteEvents[nextEventIndex]();
			nextEventIndex++;
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
			styleConfig.title = arguments;
		}
	
		void RequestWindowNotifySetEnabled(const bool& arguments) override
		{
			styleConfig.enabled = arguments;
		}
	
		void RequestWindowNotifySetTopMost(const bool& arguments) override
		{
			styleConfig.topMost = arguments;
		}
	
		void RequestWindowNotifySetShowInTaskBar(const bool& arguments) override
		{
			styleConfig.showInTaskBar = arguments;
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
	
		void RequestWindowNotifySetCustomFrameMode(const bool& arguments) override	{ styleConfig.customFrameMode = arguments;	events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetMaximizedBox(const bool& arguments) override		{ styleConfig.maximizedBox = arguments;		events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetMinimizedBox(const bool& arguments) override		{ styleConfig.minimizedBox = arguments;		events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetBorder(const bool& arguments) override			{ styleConfig.border = arguments;			events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetSizeBox(const bool& arguments) override			{ styleConfig.sizeBox = arguments;			events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetIconVisible(const bool& arguments) override		{ styleConfig.iconVisible = arguments;		events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetTitleBar(const bool& arguments) override			{ styleConfig.titleBar = arguments;			events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifyActivate() override									{ styleConfig.activated = true; }
	
		void RequestWindowNotifyShow(const WindowShowing& arguments) override
		{
			bool changed = sizingConfig.sizeState == arguments.sizeState;
			sizingConfig.sizeState = arguments.sizeState;
			if(changed) events->OnWindowBoundsUpdated(sizingConfig);
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
			protocol.events->OnControllerRequestExit();
			listener.AssertCallbacks(
				L"BeforeClosing()"
			);
			subListener.AssertCallbacks();

			listener.blockClosing = false;
			protocol.events->OnControllerRequestExit();
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
			protocol.events->OnControllerForceExit();
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

	TEST_CATEGORY(L"Disconnect and connect during running")
	{
		EmptyWindowProtocol protocol;
		protocol.OnNextFrame([&]()
		{
			auto window = GetCurrentController()->WindowService()->GetMainWindow();
			TEST_ASSERT(window);
			TEST_ASSERT(window->GetBounds() == NativeRect(0, 0, 100, 200));
			TEST_ASSERT(window->GetClientSize() == NativeSize(100, 200));

			auto assertConfigs = [&]()
			{
				TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect(270, 120, 370, 320));
				TEST_ASSERT(protocol.sizingConfig.clientBounds == NativeRect(270, 120, 370, 320));
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

			assertConfigs();
			protocol.events->OnControllerDisconnect();
			assertConfigs();
			protocol.connectionEstablished = false;
			protocol.styleConfig = {};
			protocol.sizingConfig.bounds = {};
			protocol.sizingConfig.clientBounds = {};
			protocol.events->OnControllerConnect();
			assertConfigs();

			window->Hide(true);
		});
		SetGuiMainProxy([&]()
		{
			protocol.events->OnControllerConnect();
			TEST_CASE(L"Create and destroy a window")
			{
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
				TEST_ASSERT(protocol.styleConfig == WindowStyleConfig());
				TEST_ASSERT(protocol.sizingConfig.bounds == NativeRect());
				TEST_ASSERT(protocol.sizingConfig.clientBounds == NativeRect());
				ws->Run(window);
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
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

			{
				protocol.styleConfig.activated = false;
				protocol.events->OnWindowActivatedUpdated(false);
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
				TEST_ASSERT(protocol.styleConfig.activated == true);
			}
			{
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
					L"GotFocus()",
					L"RenderingAsActivated()",
					L"LostFocus()"
				);
				subListener.AssertCallbacks(
					L"GotFocus()",
					L"RenderingAsActivated()"
				);
				TEST_ASSERT(protocol.styleConfig.activated == true);
			}

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

			{
				protocol.styleConfig.activated = false;
				protocol.events->OnWindowActivatedUpdated(false);
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
				TEST_ASSERT(protocol.styleConfig.activated == true);
			}

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
			TEST_CASE(L"Create and destroy a window")
			{
				ws = GetCurrentController()->WindowService();
				window = ws->CreateNativeWindow(INativeWindow::Normal);
				window->InstallListener(&listener);
				ws->Run(window);
				listener.AssertCallbacks();
			});
		});
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Bidirectional control maximized/minimized/restore")
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
			// TODO: complete the test
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
			TEST_CASE(L"Create and destroy a window")
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
}