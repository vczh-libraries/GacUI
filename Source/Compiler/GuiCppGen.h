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
		extern bool										WriteErrors(
															collections::List<GuiResourceError>& errors,
															const filesystem::FilePath& errorPath
															);

		extern Ptr<GuiResourceFolder>					PrecompileResource(
															Ptr<GuiResource> resource,
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
