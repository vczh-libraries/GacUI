#include "GuiInstanceSharedScript.h"

namespace vl
{
	namespace presentation
	{
		using namespace glr::xml;

/***********************************************************************
GuiInstanceSharedScript
***********************************************************************/

		Ptr<GuiInstanceSharedScript> GuiInstanceSharedScript::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			if (xml->rootElement->subNodes.Count() == 1)
			{
				if (auto cdata = xml->rootElement->subNodes[0].Cast<XmlCData>())
				{
					auto script = Ptr(new GuiInstanceSharedScript);
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

		Ptr<glr::xml::XmlElement> GuiInstanceSharedScript::SaveToXml()
		{
			auto cdata = Ptr(new XmlCData);
			cdata->content.value = code;

			auto xml = Ptr(new XmlElement);
			xml->name.value = language;
			xml->subNodes.Add(cdata);

			return xml;
		}
	}
}