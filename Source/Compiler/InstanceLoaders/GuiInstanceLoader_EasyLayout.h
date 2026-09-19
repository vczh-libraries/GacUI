/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Resource Compiler
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPILER_GUIINSTANCELOADER_EASYLAYOUT
#define VCZH_PRESENTATION_COMPILER_GUIINSTANCELOADER_EASYLAYOUT

#include "../WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl::presentation
{
	extern bool IsEasyLayoutConstantProperty(const IGuiInstanceLoader::PropertyInfo& propertyInfo);
	extern void Workflow_ValidateEasyLayouts(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors);
	extern void Workflow_BuildEasyLayouts(types::ResolvingResult& resolvingResult, Ptr<workflow::WfBlockStatement> statements);

	namespace instance_loaders
	{
		extern void LoadEasyLayouts(IGuiInstanceLoaderManager* manager);
	}
}

#endif
