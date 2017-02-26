#include "GuiInstanceSharedScript.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing::xml;

/***********************************************************************
GuiInstanceSharedScript
***********************************************************************/

		Ptr<GuiInstanceSharedScript> GuiInstanceSharedScript::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			if (xml->rootElement->subNodes.Count() == 1)
			{
				if (auto cdata = xml->rootElement->subNodes[0].Cast<XmlCData>())
				{
					auto script = MakePtr<GuiInstanceSharedScript>();
					script->language = xml->rootElement->name.value;
					script->code = cdata->content.value;
					script->codePosition = { {resource},cdata->codeRange.start };
					script->codePosition.column += 9; // <![CDATA[
					return script;
				}
			}
			errors.Add(GuiResourceError({ {resource},xml->rootElement->codeRange.start }, L"Script should be contained in a CDATA section."));
			return nullptr;
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