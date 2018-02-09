#include "GuiInstanceAnimation.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing::xml;

/***********************************************************************
GuiInstanceGradientAnimation
***********************************************************************/

		Ptr<GuiInstanceGradientAnimation> GuiInstanceGradientAnimation::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			auto animation = MakePtr<GuiInstanceGradientAnimation>();

			if (auto classAttr = XmlGetAttribute(xml->rootElement, L"ref.Class"))
			{
				animation->className = classAttr->value.value;
				animation->classPosition = { {resource},classAttr->value.codeRange.start };
				animation->classPosition.column += 1;
			}
			else
			{
				errors.Add({
					{{resource},xml->rootElement->codeRange.start},
					L"Precompile: Missing required attribute: \"ref.Class\"."
					});
			}

			if (auto typeAttr = XmlGetAttribute(xml->rootElement, L"Type"))
			{
				animation->typeName = typeAttr->value.value;
				animation->typePosition = { { resource },typeAttr->value.codeRange.start };
				animation->typePosition.column += 1;
			}
			else
			{
				errors.Add({
					{ { resource },xml->rootElement->codeRange.start },
					L"Precompile: Missing required attribute: \"Type\"."
					});
			}

			if (auto interpolationElement = XmlGetElement(xml->rootElement, L"Interpolation"))
			{
				if (auto cdata = interpolationElement->subNodes[0].Cast<XmlCData>())
				{
					animation->interpolation = cdata->content.value;
					animation->interpolationPosition = { { resource },cdata->codeRange.start };
					animation->interpolationPosition.column += 9; // <![CDATA[
				}
				else
				{
					errors.Add({
						{ { resource },interpolationElement->codeRange.start },
						L"Precompile: Interpolation function should be contained in a CDATA section."
						});
				}
			}

			return animation;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceGradientAnimation::SaveToXml()
		{
			auto gradientElement = MakePtr<XmlElement>();
			{
				auto classAttr = MakePtr<XmlAttribute>();
				classAttr->name.value = L"ref.Class";
				classAttr->value.value = className;
				gradientElement->attributes.Add(classAttr);
			}
			{
				auto typeAttr = MakePtr<XmlAttribute>();
				typeAttr->name.value = L"Type";
				typeAttr->value.value = typeName;
				gradientElement->attributes.Add(typeAttr);
			}
			if (interpolation != L"")
			{
				auto interpolationElement = MakePtr<XmlElement>();
				interpolationElement->name.value = L"Interpolation";
				gradientElement->subNodes.Add(interpolationElement);

				auto cdata = MakePtr<XmlCData>();
				cdata->content.value = interpolation;
				interpolationElement->subNodes.Add(cdata);
			}
			return gradientElement;
		}
	}
}