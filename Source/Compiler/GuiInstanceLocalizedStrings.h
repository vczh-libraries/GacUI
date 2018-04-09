/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Shared Script

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOCALIZEDSTRINGS
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOCALIZEDSTRINGS

#include "../Resources/GuiResource.h"

namespace vl
{
	namespace presentation
	{
		class GuiInstanceLocalizedStrings : public Object, public Description<GuiInstanceLocalizedStrings>
		{
		public:
			class Strings : public Object
			{
			public:
				collections::List<WString>				locales;
			};

			WString										defaultLocale;
			collections::List<Ptr<Strings>>				strings;

			static Ptr<GuiInstanceLocalizedStrings>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();
		};
	}
}

#endif