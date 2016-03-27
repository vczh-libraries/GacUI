#include "GuiInstanceSharedScript.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing::xml;
		using namespace workflow::runtime;

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

/***********************************************************************
GuiInstanceSharedScript
***********************************************************************/

		void GuiInstanceCompiledWorkflow::Initialize(bool initializeContext)
		{
			if (binaryToLoad)
			{
				assembly = new WfAssembly(*binaryToLoad.Obj());
				context = nullptr;
				binaryToLoad = nullptr;
			}

			if (initializeContext && !context)
			{
				context = new WfRuntimeGlobalContext(assembly);
				LoadFunction<void()>(context, L"<initialize>")();
			}
		}
	}
}