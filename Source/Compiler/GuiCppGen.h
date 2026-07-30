/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Loader

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUICPPGEN
#define VCZH_PRESENTATION_REFLECTION_GUICPPGEN

#include "../Reflection/GuiInstanceCompiledWorkflow.h"

namespace vl
{
	namespace presentation
	{
		class GuiResourceRpcCppOutput : public Object
		{
		public:
			Ptr<workflow::WfModule>						wrapperModule;
			Ptr<workflow::WfModule>						wrapperJsonModule;
			Ptr<workflow::cppcodegen::WfCppOutput>		cppOutput;
			WString										workflowCode;
		};

		extern bool										WriteErrors(
															collections::List<GuiResourceError>& errors,
															const filesystem::FilePath& errorPath
															);

		extern Ptr<GuiResourceFolder>					PrecompileResource(
															Ptr<GuiResource> resource,
															GuiResourceCpuArchitecture targetCpuArchitecture,
															IGuiResourcePrecompileCallback* callback,
															collections::List<GuiResourceError>& errors);

		extern Ptr<GuiInstanceCompiledWorkflow>			WriteWorkflowScript(
															Ptr<GuiResourceFolder> precompiledFolder,
															const WString& assemblyResourcePath,
															const filesystem::FilePath& workflowPath);

		extern Ptr<workflow::cppcodegen::WfCppOutput>	WriteCppCodesToFile(
															Ptr<GuiResource> resource,
															Ptr<GuiInstanceCompiledWorkflow> compiled,
															Ptr<workflow::cppcodegen::WfCppInput> cppInput,
															const filesystem::FilePath& cppFolder,
															collections::List<GuiResourceError>& errors);

		extern bool										HasRpcMetadata(
															Ptr<GuiInstanceCompiledWorkflow> compiled);

		extern bool										ValidateRpcCppGenerationConfiguration(
															Ptr<GuiResource> resource,
															Ptr<GuiInstanceCompiledWorkflow> compiled,
															bool hasCppConfiguration,
															collections::List<GuiResourceError>& errors);

		extern Ptr<GuiResourceRpcCppOutput>				GenerateRpcCppOutput(
															Ptr<GuiResource> resource,
															Ptr<GuiInstanceCompiledWorkflow> compiled,
															Ptr<workflow::cppcodegen::WfCppOutput> normalOutput,
															const WString& assemblyName,
															const WString& cppComment,
															workflow::IWfCompilerCallback* compilerCallback,
															collections::List<GuiResourceError>& errors);

		extern bool										WriteRpcWorkflowScript(
															Ptr<GuiResource> resource,
															Ptr<GuiResourceRpcCppOutput> rpcOutput,
															const filesystem::FilePath& workflowPath,
															collections::List<GuiResourceError>& errors);

		extern bool										WriteRpcCppCodesToFile(
															Ptr<GuiResource> resource,
															Ptr<GuiResourceRpcCppOutput> rpcOutput,
															const WString& assemblyName,
															const filesystem::FilePath& cppFolder,
															collections::List<GuiResourceError>& errors);

		extern bool										WriteRpcCppCodesToFileMultiPlatform(
															Ptr<GuiResource> resource,
															Ptr<GuiResourceRpcCppOutput> rpcOutput32,
															Ptr<GuiResourceRpcCppOutput> rpcOutput64,
															const WString& assemblyName,
															const filesystem::FilePath& cppFolder,
															collections::List<GuiResourceError>& errors);

		extern bool										CleanRpcCppFiles(
															const filesystem::FilePath& cppFolder,
															const WString& assemblyName);

		extern bool										WriteBinaryResource(
															Ptr<GuiResource> resource,
															bool compress,
															bool includeAssemblyInResource,
															Nullable<filesystem::FilePath> resourceOutput,
															Nullable<filesystem::FilePath> assemblyOutput);

		extern bool										WriteEmbeddedResource(Ptr<GuiResource> resource,
															Ptr<workflow::cppcodegen::WfCppInput> cppInput,
															Ptr<workflow::cppcodegen::WfCppOutput> cppOutput,
															bool compress,
															const filesystem::FilePath& filePath);
	}
}

#endif
