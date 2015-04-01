#include "GuiInstanceSchemaRepresentation.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing::xml;

/***********************************************************************
GuiInstancePropertySchame
***********************************************************************/

		Ptr<GuiInstancePropertySchame> GuiInstancePropertySchame::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			auto schema = MakePtr<GuiInstancePropertySchame>();
			if (auto attName = XmlGetAttribute(xml, L"Name"))
			{
				schema->name = attName->value.value;
			}
			else
			{
				errors.Add(L"Missing attribute \"Name\" in <" + xml->name.value + L">.");
			}
			if (auto attName = XmlGetAttribute(xml, L"Type"))
			{
				schema->typeName = attName->value.value;
			}
			else
			{
				errors.Add(L"Missing attribute \"Type\" in <" + xml->name.value + L">.");
			}
			if (auto attReadonly = XmlGetAttribute(xml, L"Readonly"))
			{
				schema->readonly = attReadonly->value.value == L"true";
			}
			if (auto attObservable = XmlGetAttribute(xml, L"Observable"))
			{
				schema->observable = attObservable->value.value == L"true";
			}
			return schema;
		}

		Ptr<parsing::xml::XmlElement> GuiInstancePropertySchame::SaveToXml()
		{
			auto xmlProperty = MakePtr<XmlElement>();
			xmlProperty->name.value = L"Property";
			
			auto attName = MakePtr<XmlAttribute>();
			attName->name.value = L"Name";
			attName->value.value = name;
			xmlProperty->attributes.Add(attName);
			
			auto attType = MakePtr<XmlAttribute>();
			attType->name.value = L"Type";
			attType->value.value = typeName;
			xmlProperty->attributes.Add(attType);
			
			auto attReadonly = MakePtr<XmlAttribute>();
			attReadonly->name.value = L"Readonly";
			attReadonly->value.value = readonly ? L"true" : L"false";
			xmlProperty->attributes.Add(attReadonly);
			
			auto attObservable = MakePtr<XmlAttribute>();
			attObservable->name.value = L"Observable";
			attObservable->value.value = observable ? L"true" : L"false";
			xmlProperty->attributes.Add(attObservable);

			return xmlProperty;
		}

/***********************************************************************
GuiInstanceTypeSchema
***********************************************************************/

		void GuiInstanceTypeSchema::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			if (auto attName = XmlGetAttribute(xml, L"ref.Class"))
			{
				typeName = attName->value.value;
			}
			else
			{
				errors.Add(L"Missing attribute \"ref.Class\" in <" + xml->name.value + L">.");
			}
			if (auto attParent = XmlGetAttribute(xml, L"Parent"))
			{
				parentType = attParent->value.value;
			}
			FOREACH(Ptr<XmlElement>, memberElement, XmlGetElements(xml, L"Property"))
			{
				auto prop = GuiInstancePropertySchame::LoadFromXml(memberElement, errors);
				properties.Add(prop);
			}
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceTypeSchema::SaveToXml()
		{
			auto xmlType = MakePtr<XmlElement>();
			
			auto attClass = MakePtr<XmlAttribute>();
			attClass->name.value = L"ref.Class";
			attClass->value.value = typeName;
			xmlType->attributes.Add(attClass);

			if (parentType != L"")
			{
				auto attParent = MakePtr<XmlAttribute>();
				attParent->name.value = L"Parent";
				attParent->value.value = parentType;
				xmlType->attributes.Add(attParent);
			}

			FOREACH(Ptr<GuiInstancePropertySchame>, prop, properties)
			{
				xmlType->subNodes.Add(prop->SaveToXml());
			}

			return xmlType;
		}

/***********************************************************************
GuiInstanceDataSchema
***********************************************************************/

		Ptr<GuiInstanceDataSchema> GuiInstanceDataSchema::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			auto schema = MakePtr<GuiInstanceDataSchema>();
			schema->GuiInstanceTypeSchema::LoadFromXml(xml, errors);
			schema->referenceType = xml->name.value == L"Class";

			if (!schema->referenceType)
			{
				if (schema->parentType != L"")
				{
					errors.Add(
						L"Struct \"" + schema->parentType +
						L"\" should not have a parent type."
						);
				}
			}

			FOREACH(Ptr<GuiInstancePropertySchame>, prop, schema->properties)
			{
				if (prop->readonly)
				{
					errors.Add(
						L"Property \"" + prop->name +
						L"\" should not be readonly in data type \"" + schema->typeName +
						L"\"."
						);
				}
				if (prop->observable)
				{
					errors.Add(
						L"Property \"" + prop->name +
						L"\" should not be observable in data type \"" + schema->typeName +
						L"\"."
						);
				}
			}
			return schema;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceDataSchema::SaveToXml()
		{
			auto xmlType = GuiInstanceTypeSchema::SaveToXml();
			xmlType->name.value = referenceType ? L"Class" : L"Struct";
			return xmlType;
		}

/***********************************************************************
GuiInstanceInterfaceSchema
***********************************************************************/

		Ptr<GuiInstanceMethodSchema> GuiInstanceMethodSchema::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			auto schema = MakePtr<GuiInstanceMethodSchema>();
			if (auto attName = XmlGetAttribute(xml, L"Name"))
			{
				schema->name = attName->value.value;
			}
			else
			{
				errors.Add(L"Missing attribute \"Name\" in <" + xml->name.value + L">.");
			}
			if (auto attName = XmlGetAttribute(xml, L"Type"))
			{
				schema->returnType = attName->value.value;
			}
			else
			{
				errors.Add(L"Missing attribute \"Type\" in <" + xml->name.value + L">.");
			}
			FOREACH(Ptr<XmlElement>, memberElement, XmlGetElements(xml, L"Argument"))
			{
				auto prop = GuiInstancePropertySchame::LoadFromXml(memberElement, errors);
				schema->arguments.Add(prop);
			}
			return schema;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceMethodSchema::SaveToXml()
		{
			auto xmlMethod = MakePtr<XmlElement>();
			xmlMethod->name.value = L"Method";
			
			auto attName = MakePtr<XmlAttribute>();
			attName->name.value = L"Name";
			attName->value.value = name;
			xmlMethod->attributes.Add(attName);
			
			auto attType = MakePtr<XmlAttribute>();
			attType->name.value = L"Type";
			attType->value.value = returnType;
			xmlMethod->attributes.Add(attType);

			FOREACH(Ptr<GuiInstancePropertySchame>, prop, arguments)
			{
				xmlMethod->subNodes.Add(prop->SaveToXml());
			}

			return xmlMethod;
		}

/***********************************************************************
GuiInstanceInterfaceSchema
***********************************************************************/

		Ptr<GuiInstanceInterfaceSchema> GuiInstanceInterfaceSchema::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			auto schema = MakePtr<GuiInstanceInterfaceSchema>();
			schema->GuiInstanceTypeSchema::LoadFromXml(xml, errors);
			FOREACH(Ptr<XmlElement>, memberElement, XmlGetElements(xml, L"Method"))
			{
				auto method = GuiInstanceMethodSchema::LoadFromXml(memberElement, errors);
				schema->methods.Add(method);
			}
			return schema;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceInterfaceSchema::SaveToXml()
		{
			auto xmlType = GuiInstanceTypeSchema::SaveToXml();
			xmlType->name.value = L"Interface";

			FOREACH(Ptr<GuiInstanceMethodSchema>, method, methods)
			{
				xmlType->subNodes.Add(method->SaveToXml());
			}

			return xmlType;
		}

/***********************************************************************
GuiInstanceSchema
***********************************************************************/

		Ptr<GuiInstanceSchema> GuiInstanceSchema::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors)
		{
			auto schema = MakePtr<GuiInstanceSchema>();
			FOREACH(Ptr<XmlElement>, schemaElement, XmlGetElements(xml->rootElement))
			{
				if (schemaElement->name.value == L"Struct" || schemaElement->name.value == L"Class")
				{
					schema->schemas.Add(GuiInstanceDataSchema::LoadFromXml(schemaElement, errors));
				}
				else if (schemaElement->name.value == L"Interface")
				{
					schema->schemas.Add(GuiInstanceInterfaceSchema::LoadFromXml(schemaElement, errors));
				}
				else
				{
					errors.Add(L"Unknown schema type \"" + schemaElement->name.value + L"\".");
				}
			}
			return schema;
		}

		Ptr<parsing::xml::XmlDocument> GuiInstanceSchema::SaveToXml()
		{
			auto xmlElement = MakePtr<XmlElement>();
			xmlElement->name.value = L"InstanceSchema";

			FOREACH(Ptr<GuiInstanceTypeSchema>, type, schemas)
			{
				xmlElement->subNodes.Add(type->SaveToXml());
			}

			auto doc = MakePtr<XmlDocument>();
			doc->rootElement = xmlElement;
			return doc;
		}
	}
}