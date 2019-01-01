/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCECOMPILEDWORKFLOW
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCECOMPILEDWORKFLOW

#include "../Resources/GuiResource.h"
#include "../../Import/VlppWorkflowCompiler.h"

namespace vl
{
	namespace workflow
	{
		class WfModule;
	}

	namespace presentation
	{
		class GuiInstanceCompiledWorkflow : public Object, public Description<GuiInstanceCompiledWorkflow>
		{
		public:
			enum AssemblyType
			{
				Shared,
				InstanceClass,
				TemporaryClass,
			};

			struct ModuleRecord
			{
				Ptr<workflow::WfModule>							module;
				GuiResourceTextPos								position;
				bool											shared = false;
			};
			
			collections::List<ModuleRecord>						modules;
			Ptr<workflow::analyzer::WfLexicalScopeManager>		metadata;
			Ptr<stream::MemoryStream>							binaryToLoad;

			AssemblyType										type = AssemblyType::Shared;
			Ptr<workflow::runtime::WfAssembly>					assembly;
			Ptr<workflow::runtime::WfRuntimeGlobalContext>		context;

			bool												Initialize(bool initializeContext, workflow::runtime::WfAssemblyLoadErrors& loadErrors);
		};
	}
}

#endif