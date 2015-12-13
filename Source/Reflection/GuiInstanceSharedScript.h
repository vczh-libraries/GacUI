/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESHAREDSCRIPT
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESHAREDSCRIPT

#include "../Resources/GuiResource.h"
#include "../GacWorkflowReferences.h"

namespace vl
{
	namespace presentation
	{
		class GuiInstanceSharedScript :public Object, public Description<GuiInstanceSharedScript>
		{
		public:
			WString										language;
			WString										code;

			static Ptr<GuiInstanceSharedScript>			LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();
		};

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