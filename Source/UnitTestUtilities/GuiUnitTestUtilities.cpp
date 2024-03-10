#include "GuiUnitTestUtilities.h"
#include "../PlatformProviders/Remote/GuiRemoteProtocol.h"

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
using namespace vl::presentation::unittest;

class UnitTestContextEx : public UnitTestContext
{
	UnitTestRemoteProtocol*		protocol = nullptr;

public:
	UnitTestContextEx(UnitTestRemoteProtocol* _protocol)
		: protocol(_protocol)
	{
	}

	IGuiRemoteProtocolEvents* GetEvents() override
	{
		return protocol ? protocol->GetEvents() : nullptr;
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