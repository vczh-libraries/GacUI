#include "../../../Source/GacUI.h"
#include "../../../Source/Compiler/GuiCppGen.h"
#include "../../../Source/Resources/GuiParserManager.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::stream;
using namespace vl::unittest;
using namespace vl::workflow;
using namespace vl::workflow::analyzer;
using namespace vl::workflow::cppcodegen;
using namespace vl::presentation;

namespace compiler_error_tests
{
	extern WString GetTestResourcePath();
	extern WString GetTestOutputPath();
}
using namespace compiler_error_tests;

extern void SetGuiMainProxy(const Func<void()>& proxy);

class RpcPrecompileCallback : public Object, public IGuiResourcePrecompileCallback
{
public:
	List<vint>			orderedPasses;
	Array<vint>			perResourceCounts;

	RpcPrecompileCallback()
		:perResourceCounts(IGuiResourceTypeResolver_Precompile::Everything_Max + 1)
	{
		for (vint i = 0; i < perResourceCounts.Count(); i++)
		{
			perResourceCounts[i] = 0;
		}
	}

	void RecordPass(vint passIndex)
	{
		if (orderedPasses.Count() == 0 || orderedPasses[orderedPasses.Count() - 1] != passIndex)
		{
			orderedPasses.Add(passIndex);
		}
	}

	IWfCompilerCallback* GetCompilerCallback()override
	{
		return nullptr;
	}

	void OnPerPass(vint passIndex)override
	{
		RecordPass(passIndex);
	}

	void OnPerResource(vint passIndex, Ptr<GuiResourceItem> resource)override
	{
		RecordPass(passIndex);
		perResourceCounts[passIndex]++;
	}
};

Ptr<GuiResource> LoadRpcResourceText(const WString& text, GuiResourceError::List& errors)
{
	auto parser = GetParserManager()->GetParser<glr::xml::XmlDocument>(L"XML");
	auto xml = parser->Parse({}, text, errors);
	if (!xml)
	{
		return nullptr;
	}
	return GuiResource::LoadFromXml(xml, L"TestResource_Rpc.xml", L".", errors);
}

Ptr<GuiInstanceCompiledWorkflow> GetInstanceClass(Ptr<GuiResourceFolder> precompiledFolder)
{
	return precompiledFolder
		? precompiledFolder->GetValueByPath(L"Workflow/InstanceClass").Cast<GuiInstanceCompiledWorkflow>()
		: nullptr;
}

WString CombineCppFiles(Ptr<WfCppOutput> output)
{
	WString code;
	for (auto fileName : output->cppFiles.Keys())
	{
		code += output->cppFiles[fileName];
	}
	return code;
}

void AssertNoErrors(const GuiResourceError::List& errors)
{
	for (auto error : errors)
	{
		TEST_PRINT(error.message);
	}
	TEST_ASSERT(errors.Count() == 0);
}

TEST_FILE
{
	TEST_CASE(L"Workflow RPC metadata generation is the ninth and final resource precompile pass")
	{
		TEST_ASSERT(IGuiResourceTypeResolver_Precompile::Instance_GenerateRpcMetadata == 8);
		TEST_ASSERT(IGuiResourceTypeResolver_Precompile::Instance_Max == 8);
		TEST_ASSERT(IGuiResourceTypeResolver_Precompile::Everything_Max == 8);
	});

	SetGuiMainProxy([]()
	{
		TEST_CASE(L"Valid resource generates aggregate RPC metadata and an RPC-only C++ pair")
		{
			auto resourcePath = FilePath(GetTestResourcePath()).GetFolder() / L"App/RemoteViewModelTest/Resource.xml";
			GuiResourceError::List errors;
			auto resource = GuiResource::LoadFromXml(resourcePath.GetFullPath(), errors);
			AssertNoErrors(errors);
			TEST_ASSERT(resource);

			RpcPrecompileCallback callback;
			auto precompiledFolder = PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, &callback, errors);
			AssertNoErrors(errors);
			TEST_ASSERT(precompiledFolder);
			TEST_ASSERT(callback.orderedPasses.Count() == 9);
			for (vint i = 0; i < callback.orderedPasses.Count(); i++)
			{
				TEST_ASSERT(callback.orderedPasses[i] == i);
			}
			TEST_ASSERT(callback.perResourceCounts[IGuiResourceTypeResolver_Precompile::Instance_GenerateRpcMetadata] == 0);

			auto compiled = GetInstanceClass(precompiledFolder);
			TEST_ASSERT(compiled);
			TEST_ASSERT(compiled->modules.Count() >= 3);
			TEST_ASSERT(HasRpcMetadata(compiled));
			TEST_ASSERT(compiled->metadata->rpcMetadata->typeFullNames.Contains(L"rvmt::IViewModel"));

			auto normalInput = Ptr(new WfCppInput(L"RemoteViewModelTest"));
			normalInput->multiFile = WfCppFileSwitch::Enabled;
			normalInput->reflection = WfCppFileSwitch::Enabled;
			normalInput->normalIncludes.Add(L"GacUI.h");
			auto normalOutput = GenerateCppFiles(normalInput, compiled->metadata.Obj());
			TEST_ASSERT(normalOutput);
			TEST_ASSERT(compiled->metadata->errors.Count() == 0);
			auto normalCodeBefore = CombineCppFiles(normalOutput);
			auto rpcMetadataBefore = compiled->metadata->rpcMetadata;

			auto rpcOutput = GenerateRpcCppOutput(
				resource,
				compiled,
				normalOutput,
				L"RemoteViewModelTestRpc",
				L"Unit test RPC output",
				nullptr,
				errors
				);
			AssertNoErrors(errors);
			TEST_ASSERT(rpcOutput);
			TEST_ASSERT(compiled->metadata->rpcMetadata == rpcMetadataBefore);
			TEST_ASSERT(CombineCppFiles(normalOutput) == normalCodeBefore);
			TEST_ASSERT(rpcOutput->cppOutput->cppFiles.Count() == 2);
			TEST_ASSERT(rpcOutput->cppOutput->cppFiles.Keys().Contains(L"RemoteViewModelTestRpc.h"));
			TEST_ASSERT(rpcOutput->cppOutput->cppFiles.Keys().Contains(L"RemoteViewModelTestRpc.cpp"));
			TEST_ASSERT(!rpcOutput->cppOutput->cppFiles.Keys().Contains(L"RemoteViewModelTestRpcReflection.h"));
			TEST_ASSERT(!rpcOutput->cppOutput->cppFiles.Keys().Contains(L"RemoteViewModelTestRpcIncludes.h"));

			auto flatPosition = INVLOC.FindFirst(rpcOutput->workflowCode, L"1/2: RPC Wrapper", Locale::None).key;
			auto jsonPosition = INVLOC.FindFirst(rpcOutput->workflowCode, L"2/2: JSON RPC Wrapper", Locale::None).key;
			TEST_ASSERT(flatPosition != -1);
			TEST_ASSERT(jsonPosition > flatPosition);

			auto rpcCode = CombineCppFiles(rpcOutput->cppOutput);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"#include \"RemoteViewModelTestIncludes.h\"", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"namespace vl_workflow_global", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"RemoteViewModelTestRpc& RemoteViewModelTestRpc::Instance()", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpc_GetIds", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcwrapper_Create", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcwrapper_GetTypeId", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcjson_Serialize", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcops_IRpcSerializer", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcops_IRpcObjectOpsJson", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcops_IRpcObjectEventOpsJson", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"rpcops_IOps_CreateJson", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"InvokeMethod_rvmt__IViewModel_Translate", Locale::None).key != -1);
			TEST_ASSERT(INVLOC.FindFirst(rpcCode, L"class IViewModel", Locale::None).key == -1);

			auto outputFolder = FilePath(GetTestOutputPath()) / L"RpcCppOutput";
			Folder(outputFolder).Delete(true);
			TEST_ASSERT(Folder(outputFolder).Create(true));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpcReflection.h").WriteAllText(L"legacy"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpcReflection.cpp").WriteAllText(L"legacy"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpcIncludes.h").WriteAllText(L"legacy"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.backup").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").WriteAllText(L"stale transaction"));
			TEST_ASSERT(WriteRpcCppCodesToFile(resource, rpcOutput, L"RemoteViewModelTestRpc", outputFolder, errors));
			AssertNoErrors(errors);
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h").Exists());
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpcReflection.h").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpcReflection.cpp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpcIncludes.h").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.backup").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").Exists());

			auto headerBefore = File(outputFolder / L"RemoteViewModelTestRpc.h").ReadAllTextByBom();
			auto cppBefore = File(outputFolder / L"RemoteViewModelTestRpc.cpp").ReadAllTextByBom();
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.backup").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").WriteAllText(L"stale transaction"));
			TEST_ASSERT(WriteRpcCppCodesToFile(resource, rpcOutput, L"RemoteViewModelTestRpc", outputFolder, errors));
			AssertNoErrors(errors);
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h").ReadAllTextByBom() == headerBefore);
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp").ReadAllTextByBom() == cppBefore);
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.backup").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").Exists());
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.backup").WriteAllText(L"stale transaction"));
			TEST_ASSERT(File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").WriteAllText(L"stale transaction"));
			TEST_ASSERT(CleanRpcCppFiles(outputFolder, L"RemoteViewModelTestRpc"));
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.h.gacui.backup").Exists());
			TEST_ASSERT(!File(outputFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").Exists());
			TEST_ASSERT(Folder(outputFolder).Delete(true));

			auto rollbackFolder = FilePath(GetTestOutputPath()) / L"RpcCppRollbackOutput";
			Folder(rollbackFolder).Delete(true);
			TEST_ASSERT(Folder(rollbackFolder).Create(true));
			auto rollbackHeaderPath = rollbackFolder / L"RemoteViewModelTestRpc.h";
			auto rollbackCppPath = rollbackFolder / L"RemoteViewModelTestRpc.cpp";
			auto rollbackHeaderCode = rpcOutput->cppOutput->cppFiles[L"RemoteViewModelTestRpc.h"];
			TEST_ASSERT(File(rollbackHeaderPath).WriteAllText(rollbackHeaderCode, true, BomEncoder::Utf8));
			TEST_ASSERT(Folder(rollbackCppPath).Create(true));
			TEST_ASSERT(!WriteRpcCppCodesToFile(resource, rpcOutput, L"RemoteViewModelTestRpc", rollbackFolder, errors));
			TEST_ASSERT(errors.Count() == 1);
			TEST_ASSERT(INVLOC.FindFirst(errors[0].message, L"Unable to install the generated RPC source", Locale::None).key != -1);
			TEST_ASSERT(File(rollbackHeaderPath).ReadAllTextByBom() == rollbackHeaderCode);
			TEST_ASSERT(Folder(rollbackCppPath).Exists());
			TEST_ASSERT(!File(rollbackFolder / L"RemoteViewModelTestRpc.h.gacui.tmp").Exists());
			TEST_ASSERT(!File(rollbackFolder / L"RemoteViewModelTestRpc.cpp.gacui.tmp").Exists());
			TEST_ASSERT(!File(rollbackFolder / L"RemoteViewModelTestRpc.h.gacui.backup").Exists());
			TEST_ASSERT(!File(rollbackFolder / L"RemoteViewModelTestRpc.cpp.gacui.backup").Exists());
			errors.Clear();
			TEST_ASSERT(Folder(rollbackCppPath).Delete(true));
			TEST_ASSERT(Folder(rollbackFolder).Delete(true));

			auto multiPlatformFolder = FilePath(GetTestOutputPath()) / L"RpcCppMultiPlatformOutput";
			Folder(multiPlatformFolder).Delete(true);
			TEST_ASSERT(Folder(multiPlatformFolder).Create(true));
			TEST_ASSERT(WriteRpcCppCodesToFileMultiPlatform(
				resource,
				rpcOutput,
				rpcOutput,
				L"RemoteViewModelTestRpc",
				multiPlatformFolder,
				errors
				));
			AssertNoErrors(errors);
			TEST_ASSERT(File(multiPlatformFolder / L"RemoteViewModelTestRpc.h").Exists());
			TEST_ASSERT(File(multiPlatformFolder / L"RemoteViewModelTestRpc.cpp").Exists());
			TEST_ASSERT(Folder(multiPlatformFolder).Delete(true));
		});

		TEST_CASE(L"A sequential non-RPC compile has no leaked RPC metadata or type loader")
		{
			TEST_ASSERT(!reflection::description::GetTypeDescriptor(L"rvmt::IViewModel"));

			auto text = LR"RESOURCE(
<Resource>
  <Script name="PlainAfterRpc"><Workflow><![CDATA[
module plain_after_rpc;
using system::*;
namespace plain_after_rpc
{
  interface IPlain
  {
    func Echo(value : string) : string;
  }
}
  ]]></Workflow></Script>
</Resource>
)RESOURCE";
			GuiResourceError::List errors;
			auto resource = LoadRpcResourceText(text, errors);
			AssertNoErrors(errors);
			auto precompiledFolder = PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
			AssertNoErrors(errors);
			auto compiled = GetInstanceClass(precompiledFolder);
			TEST_ASSERT(compiled);
			TEST_ASSERT(!HasRpcMetadata(compiled));
			TEST_ASSERT(!compiled->metadata->rpcMetadata);
			TEST_ASSERT(reflection::description::GetTypeDescriptor(L"plain_after_rpc::IPlain"));

			compiled = nullptr;
			precompiledFolder = nullptr;
			resource = nullptr;
			TEST_ASSERT(!reflection::description::GetTypeDescriptor(L"plain_after_rpc::IPlain"));
		});

		TEST_CASE(L"Validated RPC metadata without GacGenConfig Cpp reports a configuration error and writes nothing")
		{
			auto text = LR"RESOURCE(
<Resource>
  <Script name="ConfiglessRpc"><Workflow><![CDATA[
module configless_rpc;
using system::*;
namespace configless_rpc
{
  @rpc:Interface
  @rpc:Ctor
  interface IService
  {
    func Echo(value : string) : string;
  }
}
  ]]></Workflow></Script>
</Resource>
)RESOURCE";
			GuiResourceError::List errors;
			auto resource = LoadRpcResourceText(text, errors);
			AssertNoErrors(errors);
			auto precompiledFolder = PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
			AssertNoErrors(errors);
			auto compiled = GetInstanceClass(precompiledFolder);
			TEST_ASSERT(HasRpcMetadata(compiled));

			auto outputFolder = FilePath(GetTestOutputPath()) / L"RpcMissingCppConfig";
			Folder(outputFolder).Delete(true);
			TEST_ASSERT(Folder(outputFolder).Create(true));
			TEST_ASSERT(!ValidateRpcCppGenerationConfiguration(resource, compiled, false, errors));
			TEST_ASSERT(errors.Count() == 1);
			TEST_ASSERT(INVLOC.FindFirst(errors[0].message, L"GacGenConfig/Cpp", Locale::None).key != -1);
			TEST_ASSERT(!File(outputFolder / L"WorkflowRpc.txt").Exists());
			TEST_ASSERT(!File(outputFolder / L"ConfiglessRpc.h").Exists());
			TEST_ASSERT(!File(outputFolder / L"ConfiglessRpc.cpp").Exists());
			TEST_ASSERT(Folder(outputFolder).Delete(true));
		});

		TEST_CASE(L"Two retained source modules are validated together in pass 8")
		{
			auto text = LR"RESOURCE(
<Resource>
  <Script name="One"><Workflow><![CDATA[
module rpc_one;
using system::*;
namespace rpc_multi
{
  @rpc:Interface
  @rpc:Ctor
  interface IOne
  {
    func First(value : string) : string;
  }
}
  ]]></Workflow></Script>
  <Script name="Two"><Workflow><![CDATA[
module rpc_two;
using system::*;
namespace rpc_multi
{
  @rpc:Interface
  @rpc:Ctor
  interface ITwo
  {
    func Second(value : string) : string;
  }
}
  ]]></Workflow></Script>
</Resource>
)RESOURCE";
			GuiResourceError::List errors;
			auto resource = LoadRpcResourceText(text, errors);
			AssertNoErrors(errors);
			auto precompiledFolder = PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
			AssertNoErrors(errors);
			auto compiled = GetInstanceClass(precompiledFolder);
			TEST_ASSERT(compiled);
			TEST_ASSERT(compiled->modules.Count() == 2);
			TEST_ASSERT(HasRpcMetadata(compiled));
			TEST_ASSERT(compiled->metadata->rpcMetadata->typeFullNames.Contains(L"rpc_multi::IOne"));
			TEST_ASSERT(compiled->metadata->rpcMetadata->typeFullNames.Contains(L"rpc_multi::ITwo"));

			auto input = Ptr(new WfCppInput(L"RpcMulti"));
			input->multiFile = WfCppFileSwitch::Disabled;
			input->reflection = WfCppFileSwitch::Enabled;
			auto normalOutput = GenerateCppFiles(input, compiled->metadata.Obj());
			TEST_ASSERT(normalOutput);
			auto rpcOutput = GenerateRpcCppOutput(resource, compiled, normalOutput, L"RpcMultiRpc", L"Unit test", nullptr, errors);
			AssertNoErrors(errors);
			TEST_ASSERT(rpcOutput);
			auto rpcCode = CombineCppFiles(rpcOutput->cppOutput);
			auto onePosition = INVLOC.FindFirst(rpcCode, L"rpc_multi::IOne", Locale::None).key;
			auto twoPosition = INVLOC.FindFirst(rpcCode, L"rpc_multi::ITwo", Locale::None).key;
			TEST_ASSERT(onePosition != -1);
			TEST_ASSERT(twoPosition > onePosition);
		});

		TEST_CASE(L"Non-RPC, unannotated, annotation-text, and empty resources do not produce metadata")
		{
			const wchar_t* resources[] =
			{
				LR"RESOURCE(
<Resource>
  <Script name="Plain"><Workflow><![CDATA[
module plain;
using system::*;
namespace plain
{
  interface IPlain
  {
    func Echo(value : string) : string;
  }
}
  ]]></Workflow></Script>
</Resource>
)RESOURCE",
				LR"RESOURCE(
<Resource>
  <Script name="AnnotationText"><Workflow><![CDATA[
module annotation_text;
using system::*;
/* @rpc:Interface @rpc:Ctor */
var annotation = "@rpc:Interface @rpc:Ctor";
namespace annotation_text
{
  interface IPlain
  {
    func Echo(value : string) : string;
  }
}
  ]]></Workflow></Script>
</Resource>
)RESOURCE",
				L"<Resource/>",
			};

			for (auto text : resources)
			{
				GuiResourceError::List errors;
				auto resource = LoadRpcResourceText(text, errors);
				AssertNoErrors(errors);
				auto precompiledFolder = PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, nullptr, errors);
				AssertNoErrors(errors);
				auto compiled = GetInstanceClass(precompiledFolder);
				TEST_ASSERT(compiled);
				TEST_ASSERT(!HasRpcMetadata(compiled));
				TEST_ASSERT(!compiled->metadata->rpcMetadata);
			}
		});

		TEST_CASE(L"Invalid shared-only RPC reports its original script position in pass 8")
		{
			auto text = LR"RESOURCE(
<Resource>
  <Script name="BadRpc"><Workflow><![CDATA[
module invalid_rpc;

@rpc:Interface
class Wrong
{
}
  ]]></Workflow></Script>
</Resource>
)RESOURCE";
			GuiResourceError::List errors;
			auto resource = LoadRpcResourceText(text, errors);
			AssertNoErrors(errors);
			RpcPrecompileCallback callback;
			auto precompiledFolder = PrecompileResource(resource, GuiResourceCpuArchitecture::Unspecified, &callback, errors);
			TEST_ASSERT(precompiledFolder);
			TEST_ASSERT(errors.Count() > 0);
			TEST_ASSERT(callback.orderedPasses[callback.orderedPasses.Count() - 1] == IGuiResourceTypeResolver_Precompile::Instance_GenerateRpcMetadata);
			TEST_ASSERT(callback.perResourceCounts[IGuiResourceTypeResolver_Precompile::Instance_GenerateRpcMetadata] == 0);

			bool foundRpcError = false;
			for (auto error : errors)
			{
				if (INVLOC.FindFirst(error.message, L"rpc:Interface", Locale::None).key != -1)
				{
					foundRpcError = true;
					TEST_ASSERT(error.position.originalLocation.resourcePath == L"BadRpc");
					TEST_ASSERT(error.position.row != glr::ParsingTextPos::UnknownValue);
				}
			}
			TEST_ASSERT(foundRpcError);
			TEST_ASSERT(!resource->GetFolder(L"Precompiled"));

			auto compiled = GetInstanceClass(precompiledFolder);
			TEST_ASSERT(compiled);
			TEST_ASSERT(compiled->modules.Count() == 1);
			TEST_ASSERT(compiled->modules[0].shared);
			TEST_ASSERT(!HasRpcMetadata(compiled));

			auto outputFolder = FilePath(GetTestOutputPath()) / L"InvalidRpcOutput";
			Folder(outputFolder).Delete(true);
			TEST_ASSERT(Folder(outputFolder).Create(true));
			TEST_ASSERT(File(outputFolder / L"InvalidRpc.h").WriteAllText(L"stale"));
			TEST_ASSERT(File(outputFolder / L"InvalidRpc.cpp").WriteAllText(L"stale"));
			TEST_ASSERT(File(outputFolder / L"InvalidRpcReflection.h").WriteAllText(L"stale"));
			TEST_ASSERT(File(outputFolder / L"InvalidRpcReflection.cpp").WriteAllText(L"stale"));
			TEST_ASSERT(File(outputFolder / L"InvalidRpcIncludes.h").WriteAllText(L"stale"));
			TEST_ASSERT(CleanRpcCppFiles(outputFolder, L"InvalidRpc"));
			TEST_ASSERT(!File(outputFolder / L"InvalidRpc.h").Exists());
			TEST_ASSERT(!File(outputFolder / L"InvalidRpc.cpp").Exists());
			TEST_ASSERT(!File(outputFolder / L"InvalidRpcReflection.h").Exists());
			TEST_ASSERT(!File(outputFolder / L"InvalidRpcReflection.cpp").Exists());
			TEST_ASSERT(!File(outputFolder / L"InvalidRpcIncludes.h").Exists());
			TEST_ASSERT(Folder(outputFolder).Delete(true));
		});
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}
