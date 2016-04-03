/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCECOMPILEDWORKFLOW
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCECOMPILEDWORKFLOW

#include "../Resources/GuiResource.h"
#include "../../Import/VlppWorkflow.h"

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
				ViewModel,
				Shared,
				InstanceCtor,
				InstanceClass,
				TemporaryClass,
			};
			
			collections::List<WString>						codes;
			collections::List<Ptr<workflow::WfModule>>		modules;
			Ptr<stream::MemoryStream>						binaryToLoad;

			AssemblyType									type = AssemblyType::Shared;
			Ptr<workflow::runtime::WfAssembly>				assembly;
			Ptr<workflow::runtime::WfRuntimeGlobalContext>	context;

			void											Initialize(bool initializeContext);
		};
	}
}

#endif