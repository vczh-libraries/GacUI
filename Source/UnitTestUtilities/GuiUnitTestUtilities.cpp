#include "GuiUnitTestUtilities.h"

namespace vl::presentation::controls
{
	extern bool GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES;
	extern bool GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD;
}

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::controls;

namespace vl::presentation::unittest
{
/***********************************************************************
UnitTestScreenConfig
***********************************************************************/

/***********************************************************************
UnitTestScreenConfig
***********************************************************************/

	void UnitTestScreenConfig::FastInitialize(vint width, vint height, vint taskBarHeight)
	{
		executablePath = WString::Unmanaged(L"/GacUI/Remote/Protocol/UnitTest.exe");
		customFramePadding = { 8,8,8,8 };

		fontConfig.defaultFont.fontFamily = WString::Unmanaged(L"GacUI Default Font");
		fontConfig.defaultFont.size = 12;
		fontConfig.supportedFonts = Ptr(new List<WString>());
		fontConfig.supportedFonts->Add(fontConfig.defaultFont.fontFamily);

		screenConfig.bounds = { 0,0,width,height };
		screenConfig.clientBounds = { 0,0,width,(height - taskBarHeight) };
		screenConfig.scalingX = 1;
		screenConfig.scalingY = 1;
	}

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/

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
	
	class UnitTestRemoteProtocol : public Object, public virtual IGuiRemoteProtocol
	{
	public:
		IGuiRemoteProtocolEvents*	events = nullptr;
		UnitTestScreenConfig		globalConfig;
		WindowSizingConfig			sizingConfig;
		WindowStyleConfig			styleConfig;
		NativeRect					lastRestoredSize;
		bool						capturing = false;
		List<GlobalShortcutKey>		globalShortcutKeys;
	
		UnitTestRemoteProtocol(UnitTestScreenConfig _globalConfig)
			: globalConfig(_globalConfig)
		{
			sizingConfig.bounds = { 0,0,0,0 };
			sizingConfig.clientBounds = { 0,0,0,0 };
			sizingConfig.customFramePadding = globalConfig.customFramePadding;
			sizingConfig.sizeState = INativeWindow::Restored;
		}

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		void Initialize(IGuiRemoteProtocolEvents* _events) override
		{
			events = _events;
		}

		void Submit() override
		{
		}

		void ProcessRemoteEvents() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	
		WString GetExecutablePath() override
		{
			return globalConfig.executablePath;
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Initialization)
***********************************************************************/
	
		void RequestControllerConnectionEstablished() override
		{
		}
	
		void RequestControllerConnectionStopped() override
		{
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Controller)
***********************************************************************/
	
		void RequestControllerGetFontConfig(vint id) override
		{
			events->RespondControllerGetFontConfig(id, globalConfig.fontConfig);
		}
	
		void RequestControllerGetScreenConfig(vint id) override
		{
			events->RespondControllerGetScreenConfig(id, globalConfig.screenConfig);
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Window)
***********************************************************************/
	
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
	
		void OnBoundsUpdated()
		{
			sizingConfig.clientBounds = sizingConfig.bounds;
			if (sizingConfig.sizeState == INativeWindow::Restored)
			{
				lastRestoredSize = sizingConfig.bounds;
			}
			events->OnWindowBoundsUpdated(sizingConfig);
		}
	
		void RequestWindowNotifySetBounds(const NativeRect& arguments) override
		{
			sizingConfig.bounds = arguments;
			OnBoundsUpdated();
		}
	
		void RequestWindowNotifySetClientSize(const NativeSize& arguments) override
		{
			sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
			OnBoundsUpdated();
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
			styleConfig.activated = arguments.activate;
			if (sizingConfig.sizeState != arguments.sizeState)
			{
				sizingConfig.sizeState = arguments.sizeState;
				switch (arguments.sizeState)
				{
				case INativeWindow::Maximized:
					sizingConfig.bounds = globalConfig.screenConfig.clientBounds;
					OnBoundsUpdated();
					break;
				case INativeWindow::Minimized:
					sizingConfig.bounds = NativeRect(
						{ globalConfig.screenConfig.bounds.x2,globalConfig.screenConfig.bounds.y2 },
						{ 1,1 }
					);
					OnBoundsUpdated();
					break;
				case INativeWindow::Restored:
					if (sizingConfig.bounds != lastRestoredSize)
					{
						sizingConfig.bounds = lastRestoredSize;
						OnBoundsUpdated();
					}
					else
					{
						events->OnWindowBoundsUpdated(sizingConfig);
					}
					break;
				}
			}
		}

/***********************************************************************
IGuiRemoteProtocolMessages (IO)
***********************************************************************/

		void RequestIOUpdateGlobalShortcutKey(const Ptr<List<GlobalShortcutKey>>& arguments) override
		{
			if (arguments)
			{
				CopyFrom(globalShortcutKeys, *arguments.Obj());
			}
			else
			{
				globalShortcutKeys.Clear();
			}
		}

		void RequestIORequireCapture() override
		{
			capturing = true;
		}

		void RequestIOReleaseCapture() override
		{
			capturing = false;
		}

		void RequestIOIsKeyPressing(vint id, const VKEY& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestIOIsKeyToggled(vint id, const VKEY& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering)
***********************************************************************/

		void RequestRendererCreated(const Ptr<List<RendererCreation>>& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererDestroyed(const Ptr<List<vint>>& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererBeginRendering() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererEndRendering(vint id) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererBeginBoundary(const ElementBoundary& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererEndBoundary() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererRenderElement(const ElementRendering& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SolidBorder(const ElementDesc_SolidBorder& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SinkBorder(const ElementDesc_SinkBorder& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SinkSplitter(const ElementDesc_SinkSplitter& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SolidBackground(const ElementDesc_SolidBackground& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_GradientBackground(const ElementDesc_GradientBackground& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_InnerShadow(const ElementDesc_InnerShadow& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_Polygon(const ElementDesc_Polygon& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - SolidLabel)
***********************************************************************/

		void RequestRendererUpdateElement_SolidLabel(const ElementDesc_SolidLabel& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - Image)
***********************************************************************/

		void RequestImageCreated(vint id, const ImageCreation& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestImageDestroyed(const vint& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_ImageFrame(const ElementDesc_ImageFrame& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};
}

using namespace vl::presentation::unittest;

class UnitTestContextEx : public UnitTestContext
{
	UnitTestRemoteProtocol*		protocol = nullptr;

public:
	UnitTestContextEx(UnitTestRemoteProtocol* _protocol)
		: protocol(_protocol)
	{
	}

	IGuiRemoteProtocolEvents*	GetEvents() override
	{
		return protocol ? protocol->events : nullptr;
	}
};

UnitTestMainFunc guiMainProxy;
UnitTestContextEx* guiMainUnitTestContext = nullptr;

void GacUIUnitTest_Initialize()
{
	GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES = true;
	GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD = true;

	GetGlobalTypeManager()->Load();
	GetPluginManager()->Load(true, false);
}

void GacUIUnitTest_Finalize()
{
	ResetGlobalTypeManager();
	GetPluginManager()->Unload(true, false);
	DestroyPluginManager();
	ThreadLocalStorage::DisposeStorages();

	GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES = false;
	GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD = false;
}

void GacUIUnitTest_SetGuiMainProxy(const UnitTestMainFunc& proxy)
{
	guiMainProxy = proxy;
}

void GacUIUnitTest_Start(vl::Nullable<UnitTestScreenConfig> config)
{
	UnitTestScreenConfig globalConfig;
	if (config)
	{
		globalConfig = config.Value();
	}
	else
	{
		globalConfig.FastInitialize(1024, 768);
	}

	UnitTestRemoteProtocol unitTestProtocol(globalConfig);
	repeatfiltering::GuiRemoteProtocolFilter filteredProtocol(&unitTestProtocol);
	repeatfiltering::GuiRemoteProtocolFilterVerifier verifierProtocol(&filteredProtocol);
	SetupRemoteNativeController(&verifierProtocol);
	GacUIUnitTest_SetGuiMainProxy({});
}

void GuiMain()
{
	guiMainProxy(guiMainUnitTestContext);
	guiMainUnitTestContext = nullptr;
}