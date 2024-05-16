#include "GuiUnitTestUtilities.h"
#include "../PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../Resources/GuiParserManager.h"
#include "../Reflection/GuiInstanceCompiledWorkflow.h"

namespace vl::presentation::controls
{
	extern bool GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES;
	extern bool GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD;
}

namespace vl::presentation::unittest
{
	const UnitTestFrameworkConfig*			globalUnitTestFrameworkConfig = nullptr;

	const UnitTestFrameworkConfig& GetUnitTestFrameworkConfig()
	{
		CHECK_ERROR(globalUnitTestFrameworkConfig, L"vl::presentation::unittest::GetUnitTestFrameworkConfig()#GacUIUnitTest_Initialize has not been called.");
		return *globalUnitTestFrameworkConfig;
	}
}

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::reflection::description;
using namespace vl::glr::json;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;

class UnitTestContextImpl : public Object, public virtual IUnitTestContext
{
	UnitTestRemoteProtocol*		protocol = nullptr;

public:
	UnitTestContextImpl(UnitTestRemoteProtocol* _protocol)
		: protocol(_protocol)
	{
	}

	UnitTestRemoteProtocol* GetProtocol()
	{
		return protocol;
	}
};

UnitTestMainFunc guiMainProxy;
UnitTestContextImpl* guiMainUnitTestContext = nullptr;

void GacUIUnitTest_Initialize(const UnitTestFrameworkConfig* config)
{
	CHECK_ERROR(config, L"GacUIUnitTest_Initialize()#Argument config should not be null.");
	globalUnitTestFrameworkConfig = config;

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
	globalUnitTestFrameworkConfig = nullptr;
}

void GacUIUnitTest_SetGuiMainProxy(const UnitTestMainFunc& proxy)
{
	guiMainProxy = proxy;
}

void GacUIUnitTest_Start(const WString& appName, Nullable<UnitTestScreenConfig> config)
{
#define ERROR_MESSAGE_PREFIX L"GacUIUnitTest_Start(const WString&, Nullable<UnitTestScreenConfig>)#"
	UnitTestScreenConfig globalConfig;
	if (config)
	{
		globalConfig = config.Value();
	}
	else
	{
		globalConfig.FastInitialize(1024, 768);
	}

	UnitTestRemoteProtocol unitTestProtocol(appName, globalConfig);
	repeatfiltering::GuiRemoteProtocolFilterVerifier verifierProtocol(&unitTestProtocol);
	repeatfiltering::GuiRemoteProtocolFilter filteredProtocol(&verifierProtocol);

	UnitTestContextImpl unitTestContext(&unitTestProtocol);
	guiMainUnitTestContext = &unitTestContext;
	SetupRemoteNativeController(&filteredProtocol);
	GacUIUnitTest_SetGuiMainProxy({});

	{
		Folder snapshotFolder = GetUnitTestFrameworkConfig().snapshotFolder;
		CHECK_ERROR(snapshotFolder.Exists(), ERROR_MESSAGE_PREFIX L"UnitTestFrameworkConfig::snapshotFolder does not point to an existing folder.");

		File snapshotFile = snapshotFolder.GetFilePath() / (appName + L".json");
		{
			auto pathPrefix = snapshotFolder.GetFilePath().GetFullPath() + WString::FromChar(FilePath::Delimiter);
			auto snapshotPath = snapshotFile.GetFilePath().GetFullPath();
			CHECK_ERROR(
				snapshotPath.Length() > pathPrefix.Length() && snapshotPath.Left(pathPrefix.Length()) == pathPrefix,
				ERROR_MESSAGE_PREFIX L"Argument appName should specify a file that is inside UnitTestFrameworkConfig::snapshotFolder"
				);
			Folder snapshotFileFolder = snapshotFile.GetFilePath().GetFolder();
			if (!snapshotFileFolder.Exists())
			{
				CHECK_ERROR(snapshotFileFolder.Create(true), ERROR_MESSAGE_PREFIX L"Failed to create the folder to contain the snapshot file specified by argument appName.");
			}
		}

		JsonFormatting formatting;
		formatting.spaceAfterColon = true;
		formatting.spaceAfterComma = true;
		formatting.crlf = true;
		formatting.compact = true;

		auto jsonLog = remoteprotocol::ConvertCustomTypeToJson(unitTestProtocol.GetLoggedTrace());
		auto textLog = JsonToString(jsonLog, formatting);
		{
			remoteprotocol::RenderingTrace deserialized;
			remoteprotocol::ConvertJsonToCustomType(jsonLog, deserialized);
			auto jsonLog2 = remoteprotocol::ConvertCustomTypeToJson(deserialized);
			auto textLog2 = JsonToString(jsonLog2, formatting);
			CHECK_ERROR(textLog == textLog2, ERROR_MESSAGE_PREFIX L"Serialization and deserialization doesn't match.");
		}
		bool succeeded = snapshotFile.WriteAllText(textLog, false, stream::BomEncoder::Utf8);
		CHECK_ERROR(succeeded, ERROR_MESSAGE_PREFIX L"Failed to write the snapshot file.");
	}
#undef ERROR_MESSAGE_PREFIX
}

Ptr<GuiResource> GacUIUnitTest_CompileAndLoad(const WString& xmlResource)
{
#define ERROR_MESSAGE_PREFIX L"GacUIUnitTest_CompileAndLoad(const WString&)#"
	auto resource = Ptr(new GuiResource);
	GuiResourceError::List errors;
	{
		auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
		auto xml = parser->Parse(
			{
				WString::Empty,
				(GetUnitTestFrameworkConfig().resourceFolder / L"Resource.xml").GetFullPath()
			},
			xmlResource,
			errors
			);
		CHECK_ERROR(xml && errors.Count() == 0, ERROR_MESSAGE_PREFIX L"Failed to parse XML resource.");
	}

	auto precompiledFolder = resource->Precompile(
#ifdef VCZH_64
		GuiResourceCpuArchitecture::X64,
#else
		GuiResourceCpuArchitecture::x86,
#endif
		nullptr,
		errors
		);
	CHECK_ERROR(precompiledFolder && errors.Count() == 0, ERROR_MESSAGE_PREFIX L"Failed to precompile XML resource.");

	auto compiledWorkflow = precompiledFolder->GetValueByPath(WString::Unmanaged(L"Workflow/InstanceClass")).Cast<GuiInstanceCompiledWorkflow>();
	CHECK_ERROR(compiledWorkflow, ERROR_MESSAGE_PREFIX L"Failed to compile generated Workflow script.");
	CHECK_ERROR(compiledWorkflow->assembly, ERROR_MESSAGE_PREFIX L"Failed to load Workflow assembly.");

	{
		WString text;
		auto& codes = compiledWorkflow->assembly->insAfterCodegen->moduleCodes;
		for (auto [code, codeIndex] : indexed(codes))
		{
			text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
			text += code + L"\r\n";
		}
		resource->CreateValueByPath(
			WString::Unmanaged(L"UnitTest/Workflow"),
			WString::Unmanaged(L"Text"),
			Ptr(new GuiTextData(text))
			);
	}

	return resource;
#undef ERROR_MESSAGE_PREFIX
}

void GuiMain()
{
	if (guiMainUnitTestContext)
	{
		guiMainProxy(guiMainUnitTestContext->GetProtocol(), guiMainUnitTestContext);
	}
	else
	{
		guiMainProxy(nullptr, nullptr);
	}
	guiMainUnitTestContext = nullptr;
}