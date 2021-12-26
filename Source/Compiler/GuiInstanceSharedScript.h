/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESHAREDSCRIPT
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESHAREDSCRIPT

#include "../Resources/GuiResource.h"

namespace vl
{
	namespace presentation
	{
		class GuiInstanceSharedScript :public Object, public Description<GuiInstanceSharedScript>
		{
		public:
			WString										language;
			WString										code;
			GuiResourceTextPos							codePosition;

			static Ptr<GuiInstanceSharedScript>			LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<glr::xml::XmlElement>					SaveToXml();
		};
	}
}

#endif