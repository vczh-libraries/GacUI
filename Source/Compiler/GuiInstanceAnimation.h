/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCEANIMATION
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCEANIMATION

#include "../Resources/GuiResource.h"

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

			GuiResourceTextPos classPosition;
			GuiResourceTextPos typePosition;
			GuiResourceTextPos interpolationPosition;

			static Ptr<GuiInstanceGradientAnimation>	LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();
		};
	}
}

#endif