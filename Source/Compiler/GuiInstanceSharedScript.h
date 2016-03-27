/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESHAREDSCRIPT
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESHAREDSCRIPT

#include "../Resources/GuiResource.h"
#include "../../Import/VlppWorkflow.h"

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
	}
}

#endif