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
			};
			
			collections::List<WString>						codes;
			Ptr<stream::MemoryStream>						binaryToLoad;

			AssemblyType									type = AssemblyType::Shared;
			WString											classFullName;
			Ptr<workflow::runtime::WfAssembly>				assembly;
			Ptr<workflow::runtime::WfRuntimeGlobalContext>	context;

			void											Initialize(bool initializeContext);
		};
	}
}

#endif