#include "GuiInstanceSharedScript.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing::xml;

/***********************************************************************
GuiInstanceSharedScript
***********************************************************************/

		Ptr<GuiInstanceSharedScript> GuiInstanceSharedScript::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors)
		{
			auto script = MakePtr<GuiInstanceSharedScript>();
			script->language = xml->rootElement->name.value;
			script->code = XmlGetValue(xml->rootElement);
			return script;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceSharedScript::SaveToXml()
		{
			auto cdata = MakePtr<XmlCData>();
			cdata->content.value = code;

			auto xml = MakePtr<XmlElement>();
			xml->name.value = language;
			xml->subNodes.Add(cdata);

			return xml;
		}
	}
}