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
		using namespace reflection;

		extern Ptr<GuiResourceFolder>					PrecompileAndWriteErrors(
															Ptr<GuiResource> resource,
															IGuiResourcePrecompileCallback* callback,
															collections::List<GuiResourceError>& errors,
															const filesystem::FilePath& errorPath);

		extern Ptr<GuiInstanceCompiledWorkflow>			WriteWorkflowScript(
															Ptr<GuiResourceFolder> precompiledFolder,
															const filesystem::FilePath& workflowPath);

		extern Ptr<workflow::cppcodegen::WfCppOutput>	WriteCppCodesToFile(
															Ptr<GuiInstanceCompiledWorkflow> compiled,
															Ptr<workflow::cppcodegen::WfCppInput> cppInput,
															const filesystem::FilePath& cppFolder);

		extern bool										WriteBinaryResource(
															Ptr<GuiResource> resource,
															bool compress,
															bool workflow,
															const filesystem::FilePath& filePath);

		extern bool										WriteEmbeddedResource(Ptr<GuiResource> resource,
															Ptr<workflow::cppcodegen::WfCppInput> cppInput,
															Ptr<workflow::cppcodegen::WfCppOutput> cppOutput,
															bool compress,
															const filesystem::FilePath& filePath);
	}
}

#endif
