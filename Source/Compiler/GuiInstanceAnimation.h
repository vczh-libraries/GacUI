/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCEANIMATION
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCEANIMATION

#include "../Resources/GuiResource.h"
#include "../../Import/VlppWorkflowCompiler.h"

namespace vl
{
	namespace presentation
	{
		class GuiInstanceAnimation : public Object, public Description<GuiInstanceAnimation>
		{
		public:
		};

		class GuiInstanceGradientAnimation : public GuiInstanceAnimation, public Description<GuiInstanceGradientAnimation>
		{
		public:
			WString className;
			WString typeName;
			WString interpolation;

			GuiResourceTextPos tagPosition;
			GuiResourceTextPos classPosition;
			GuiResourceTextPos typePosition;
			GuiResourceTextPos interpolationPosition;

			static Ptr<GuiInstanceGradientAnimation>	LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();
			Ptr<workflow::WfModule>						Compile(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, bool generateImpl, GuiResourceError::List& errors);
		};
	}
}

#endif