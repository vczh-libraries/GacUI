#include "TestRemote.h"

namespace remote_startup_tests
{
	class StartUpProtocol : public NotImplementedProtocolBase
	{
	public:
		StartUpProtocol()
			: NotImplementedProtocolBase({})
		{
		}

		WString GetExecutablePath() override
		{
			return L"/StartUp/Protocol.exe";
		}
	
		void RequestControllerGetFontConfig(vint id) override
		{
			FontConfig response;
			response.defaultFont.fontFamily = L"StartUpDefault";
			response.defaultFont.bold = true;
			response.supportedFonts = Ptr(new List<WString>());
			response.supportedFonts->Add(L"StartUpDefault");
			response.supportedFonts->Add(L"Another");
			response.supportedFonts->Add(L"YetAnother");
			events->RespondControllerGetFontConfig(id, response);
		}
	
		void RequestControllerGetScreenConfig(vint id) override
		{
			ScreenConfig response;
			response.bounds = { 0,0,30,20 };
			response.clientBounds = { 1,1,29,19 };
			response.scalingX = 1;
			response.scalingY = 1;
			events->RespondControllerGetScreenConfig(id, response);
		}
	
		void RequestControllerConnectionEstablished() override
		{
		}
	
		void RequestControllerConnectionStopped() override
		{
		}
	
		void RequestWindowGetBounds(vint id) override
		{
			WindowSizingConfig response;
			response.bounds = { 0,0,50,40 };
			response.clientBounds = { 1,1,49,39 };
			response.customFramePadding = { 8,8,8,8 };
			response.sizeState = INativeWindow::Restored;
			events->RespondWindowGetBounds(id, response);
		}

		void RequestIOUpdateGlobalShortcutKey(const Ptr<List<GlobalShortcutKey>>& arguments) override
		{
		}
	};
}
using namespace remote_startup_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Start and Stop")
	{
		StartUpProtocol protocol;
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect();

			auto rs = GetCurrentController()->ResourceService();
			auto ss = GetCurrentController()->ScreenService();

			TEST_CASE(L"Assert font config")
			{
				TEST_ASSERT(GetCurrentController()->GetExecutablePath() == L"/StartUp/Protocol.exe");
				TEST_ASSERT(rs->GetDefaultFont().fontFamily == L"StartUpDefault");
				TEST_ASSERT(rs->GetDefaultFont().bold == true);
				TEST_ASSERT(rs->GetDefaultFont().italic == false);

				List<WString> fonts;
				rs->EnumerateFonts(fonts);
				TEST_ASSERT(fonts.Count() == 3);
				TEST_ASSERT(fonts[0] == L"StartUpDefault");
				TEST_ASSERT(fonts[1] == L"Another");
				TEST_ASSERT(fonts[2] == L"YetAnother");
			});

			TEST_CASE(L"Assert screen config")
			{
				TEST_ASSERT(ss->GetScreenCount() == 1);
				auto screen = ss->GetScreen((vint)0);
				TEST_ASSERT(screen->IsPrimary() == true);
				TEST_ASSERT(screen->GetBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetClientBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetScalingX() == 1);
				TEST_ASSERT(screen->GetScalingY() == 1);
			});

			TEST_CASE(L"Assert updated screen config")
			{
				{
					ScreenConfig response;
					response.bounds = { 1,1,19, 29 };
					response.clientBounds = { 2,2,18,28 };
					response.scalingX = 1.2;
					response.scalingY = 1.5;
					protocol.GetEvents()->OnControllerScreenUpdated(response);
				}
				TEST_ASSERT(ss->GetScreenCount() == 1);
				auto screen = ss->GetScreen((vint)0);
				TEST_ASSERT(screen->IsPrimary() == true);
				TEST_ASSERT(screen->GetBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetClientBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetScalingX() == 1.2);
				TEST_ASSERT(screen->GetScalingY() == 1.5);
			});

			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(protocol.GetProtocol());
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Create windows without calling INativeWindowService::Run")
	{
		StartUpProtocol protocol;
		SetGuiMainProxy([&]()
		{
			// by not calling INativeWindowService::Run
			// non of them will be connected to the native window
			// so no interaction with the native window will happen
			protocol.GetEvents()->OnControllerConnect();

			TEST_CASE(L"Create and destroy a window")
			{
				auto window = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
				GetCurrentController()->WindowService()->DestroyNativeWindow(window);
			});

			TEST_CASE(L"Create and destroy two windows")
			{
				auto window1 = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
				auto window2 = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
				GetCurrentController()->WindowService()->DestroyNativeWindow(window1);
				GetCurrentController()->WindowService()->DestroyNativeWindow(window2);
			});

			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(protocol.GetProtocol());
		SetGuiMainProxy({});
	});
}