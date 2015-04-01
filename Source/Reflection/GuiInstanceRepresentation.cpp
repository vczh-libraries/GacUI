#include "GuiInstanceRepresentation.h"
#include "GuiInstanceLoader.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "../Resources/GuiParserManager.h"
#include "../Controls/Templates/GuiControlTemplateStyles.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing;
		using namespace parsing::xml;
		using namespace templates;
		using namespace stream;

/***********************************************************************
GuiValueRepr
***********************************************************************/

		Ptr<GuiValueRepr> GuiValueRepr::LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys)
		{
			stream::internal::Reader reader(stream);
			vint key = -1;
			reader << key;
			switch (key)
			{
			case GuiTextRepr::BinaryKey:
				return GuiTextRepr::LoadPrecompiledBinary(stream, keys);
			case GuiAttSetterRepr::BinaryKey:
				return GuiAttSetterRepr::LoadPrecompiledBinary(stream, keys);
			case GuiConstructorRepr::BinaryKey:
				return GuiConstructorRepr::LoadPrecompiledBinary(stream, keys);
			default:
				CHECK_FAIL(L"GuiValueRepr::LoadPrecompiledBinary(stream::IStream&, collections::List<presentation::GlobalStringKey>&)#Internal Error.");
			}
		}

/***********************************************************************
GuiTextRepr
***********************************************************************/

		Ptr<GuiValueRepr> GuiTextRepr::Clone()
		{
			auto repr = MakePtr<GuiTextRepr>();
			repr->fromStyle = fromStyle;
			repr->text = text;
			return repr;
		}

		void GuiTextRepr::FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource)
		{
			if (!fromStyle || serializePrecompiledResource)
			{
				auto xmlText = MakePtr<XmlText>();
				xmlText->content.value = text;
				xml->subNodes.Add(xmlText);
			}
		}

		void GuiTextRepr::CollectUsedKey(collections::List<GlobalStringKey>& keys)
		{
		}

		void GuiTextRepr::SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey)
		{
			stream::internal::Writer writer(stream);
			if (saveKey)
			{
				vint key = BinaryKey;
				writer << key;
			}
			writer << text;
		}

		Ptr<GuiTextRepr> GuiTextRepr::LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys, Ptr<GuiTextRepr> repr)
		{
			stream::internal::Reader reader(stream);
			if (!repr)
			{
				repr = MakePtr<GuiTextRepr>();
			}
			reader << repr->text;
			return repr;
		}

/***********************************************************************
GuiAttSetterRepr
***********************************************************************/

		void GuiAttSetterRepr::CloneBody(Ptr<GuiAttSetterRepr> repr)
		{
			CopyFrom(repr->eventHandlers, eventHandlers);
			FOREACH_INDEXER(GlobalStringKey, name, index, setters.Keys())
			{
				Ptr<SetterValue> src = setters.Values()[index];
				Ptr<SetterValue> dst = new SetterValue;
				dst->binding = src->binding;
				FOREACH(Ptr<GuiValueRepr>, value, src->values)
				{
					dst->values.Add(value->Clone());
				}
				repr->setters.Add(name, dst);
			}
			repr->instanceName = instanceName;
		}

		Ptr<GuiValueRepr> GuiAttSetterRepr::Clone()
		{
			auto repr = MakePtr<GuiAttSetterRepr>();
			repr->fromStyle = fromStyle;
			CloneBody(repr);
			return repr;
		}

		void GuiAttSetterRepr::FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource)
		{
			if (!fromStyle || serializePrecompiledResource)
			{
				if (instanceName != GlobalStringKey::Empty)
				{
					auto attName = MakePtr<XmlAttribute>();
					attName->name.value = L"ref.Name";
					attName->value.value = instanceName.ToString();
					xml->attributes.Add(attName);
				}

				for (vint i = 0; i < setters.Count(); i++)
				{
					auto key = setters.Keys()[i];
					auto value = setters.Values()[i];
					if (key == GlobalStringKey::Empty)
					{
						FOREACH(Ptr<GuiValueRepr>, repr, value->values)
						{
							repr->FillXml(xml, serializePrecompiledResource);
						}
					}
					else
					{
						bool containsElement = false;
						FOREACH(Ptr<GuiValueRepr>, repr, value->values)
						{
							if (!repr.Cast<GuiTextRepr>())
							{
								containsElement = true;
								break;
							}
						}

						if (containsElement)
						{
							auto xmlProp = MakePtr<XmlElement>();
							xmlProp->name.value = L"att." + key.ToString();
							if (value->binding != GlobalStringKey::Empty)
							{
								xmlProp->name.value += L"-" + value->binding.ToString();
							}

							FOREACH(Ptr<GuiValueRepr>, repr, value->values)
							{
								if (!repr.Cast<GuiTextRepr>())
								{
									repr->FillXml(xmlProp, serializePrecompiledResource);
								}
							}
							xml->subNodes.Add(xmlProp);
						}
						else if (value->values.Count() > 0)
						{
							auto att = MakePtr<XmlAttribute>();
							att->name.value = key.ToString();
							if (value->binding != GlobalStringKey::Empty)
							{
								att->name.value += L"-" + value->binding.ToString();
							}
							att->value.value = value->values[0].Cast<GuiTextRepr>()->text;
							xml->attributes.Add(att);
						}
					}
				}

				for (vint i = 0; i < eventHandlers.Count(); i++)
				{
					auto key = eventHandlers.Keys()[i];
					auto value = eventHandlers.Values()[i];

					auto xmlEvent = MakePtr<XmlElement>();
					xmlEvent->name.value = L"ev." + key.ToString();
					if (value->binding != GlobalStringKey::Empty)
					{
						xmlEvent->name.value += L"-" + value->binding.ToString();
					}
					xml->subNodes.Add(xmlEvent);

					auto xmlText = MakePtr<XmlText>();
					xmlText->content.value = value->value;
					xmlEvent->subNodes.Add(xmlText);
				}
			}
		}

		void GuiAttSetterRepr::CollectUsedKey(collections::List<GlobalStringKey>& keys)
		{
			keys.Add(instanceName);

			for (vint i = 0; i < setters.Count(); i++)
			{
				keys.Add(setters.Keys()[i]);
				auto value = setters.Values()[i];
				keys.Add(value->binding);
				for (vint j = 0; j < value->values.Count(); j++)
				{
					value->values[j]->CollectUsedKey(keys);
				}
			}
			
			for (vint i = 0; i < eventHandlers.Count(); i++)
			{
				keys.Add(eventHandlers.Keys()[i]);
				keys.Add(eventHandlers.Values()[i]->binding);
			}
		}

		void GuiAttSetterRepr::SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey)
		{
			stream::internal::Writer writer(stream);
			if (saveKey)
			{
				vint key = BinaryKey;
				writer << key;
			}
			{
				vint count = setters.Count();
				writer << count;
				for (vint i = 0; i < count; i++)
				{
					auto keyIndex = keys.IndexOf(setters.Keys()[i]);
					auto value = setters.Values()[i];
					auto bindingIndex = keys.IndexOf(value->binding);
					CHECK_ERROR(keyIndex != -1 && bindingIndex != -1, L"GuiAttSetterRepr::SavePrecompiledBinary(stream::IStream&, collections::SortedList<presentation::GlobalStringKey>&)#Internal Error.");
					writer << keyIndex << bindingIndex;

					vint valueCount = value->values.Count();
					writer << valueCount;
					for (vint j = 0; j < valueCount; j++)
					{
						value->values[j]->SavePrecompiledBinary(stream, keys, true);
					}
				}
			}
			{
				vint count = eventHandlers.Count();
				writer << count;
				for (vint i = 0; i < count; i++)
				{
					auto keyIndex = keys.IndexOf(eventHandlers.Keys()[i]);
					auto value = eventHandlers.Values()[i];
					auto bindingIndex = keys.IndexOf(value->binding);
					CHECK_ERROR(keyIndex != -1 && bindingIndex != -1, L"GuiAttSetterRepr::SavePrecompiledBinary(stream::IStream&, collections::SortedList<presentation::GlobalStringKey>&)#Internal Error.");
					writer << keyIndex << bindingIndex << value->value;
				}
			}
			{
				vint instanceNameIndex = keys.IndexOf(instanceName);
				CHECK_ERROR(instanceNameIndex != -1, L"GuiAttSetterRepr::SavePrecompiledBinary(stream::IStream&, collections::SortedList<presentation::GlobalStringKey>&)#Internal Error.");
				writer << instanceNameIndex;
			}
		}

		Ptr<GuiAttSetterRepr> GuiAttSetterRepr::LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys, Ptr<GuiAttSetterRepr> repr)
		{
			stream::internal::Reader reader(stream);
			if (!repr)
			{
				repr = MakePtr<GuiAttSetterRepr>();
			}
			{
				vint count = -1;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					vint keyIndex = -1;
					vint bindingIndex = -1;
					auto value = MakePtr<SetterValue>();
					reader << keyIndex << bindingIndex;
					auto key = keys[keyIndex];
					value->binding = keys[bindingIndex];
					repr->setters.Add(key, value);

					vint valueCount = -1;
					reader << valueCount;
					for (vint j = 0; j < valueCount; j++)
					{
						auto repr = GuiValueRepr::LoadPrecompiledBinary(stream, keys);
						value->values.Add(repr);
					}
				}
			}
			{
				vint count = -1;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					vint keyIndex = -1;
					vint bindingIndex = -1;
					auto value = MakePtr<EventValue>();
					reader << keyIndex << bindingIndex << value->value;
					auto key = keys[keyIndex];
					value->binding = keys[bindingIndex];
					repr->eventHandlers.Add(key, value);
				}
			}
			{
				vint instanceNameIndex = -1;
				reader << instanceNameIndex;
				repr->instanceName = keys[instanceNameIndex];
			}
			return repr;
		}

/***********************************************************************
GuiConstructorRepr
***********************************************************************/

		Ptr<GuiValueRepr> GuiConstructorRepr::Clone()
		{
			auto repr = MakePtr<GuiConstructorRepr>();
			repr->fromStyle = fromStyle;
			repr->typeNamespace = typeNamespace;
			repr->typeName = typeName;
			repr->styleName = styleName;
			CloneBody(repr);
			return repr;
		}

		void GuiConstructorRepr::FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource)
		{
			if (!fromStyle || serializePrecompiledResource)
			{
				auto xmlCtor = MakePtr<XmlElement>();
				if (typeNamespace == GlobalStringKey::Empty)
				{
					xmlCtor->name.value = typeName.ToString();
				}
				else
				{
					xmlCtor->name.value = typeNamespace.ToString() + L":" + typeName.ToString();
				}

				if (styleName)
				{
					auto attStyle = MakePtr<XmlAttribute>();
					attStyle->name.value = L"ref.Style";
					attStyle->value.value = styleName.Value();
					xml->attributes.Add(attStyle);
				}

				GuiAttSetterRepr::FillXml(xmlCtor, serializePrecompiledResource);
				xml->subNodes.Add(xmlCtor);
			}
		}

		void GuiConstructorRepr::CollectUsedKey(collections::List<GlobalStringKey>& keys)
		{
			GuiAttSetterRepr::CollectUsedKey(keys);
			keys.Add(typeNamespace);
			keys.Add(typeName);
		}

		void GuiConstructorRepr::SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey)
		{
			stream::internal::Writer writer(stream);
			if (saveKey)
			{
				vint key = BinaryKey;
				writer << key;
			}
			vint typeNamespaceIndex = keys.IndexOf(typeNamespace);
			vint typeNameIndex = keys.IndexOf(typeName);
			CHECK_ERROR(typeNamespaceIndex != -1 && typeNameIndex != -1, L"GuiConstructorRepr::SavePrecompiledBinary(stream::IStream&, collections::SortedList<presentation::GlobalStringKey>&)#Internal Error.");
			writer << typeNamespaceIndex << typeNameIndex << styleName;
			GuiAttSetterRepr::SavePrecompiledBinary(stream, keys, false);
		}

		Ptr<GuiConstructorRepr> GuiConstructorRepr::LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys, Ptr<GuiConstructorRepr> repr)
		{
			stream::internal::Reader reader(stream);
			if (!repr)
			{
				repr = MakePtr<GuiConstructorRepr>();
			}
			vint typeNamespaceIndex = -1;
			vint typeNameIndex = -1;
			reader << typeNamespaceIndex << typeNameIndex << repr->styleName;
			repr->typeNamespace = keys[typeNamespaceIndex];
			repr->typeName = keys[typeNameIndex];
			GuiAttSetterRepr::LoadPrecompiledBinary(stream, keys, repr);
			return repr;
		}

/***********************************************************************
GuiInstanceContext
***********************************************************************/

		void GuiInstanceContext::CollectDefaultAttributes(GuiAttSetterRepr::ValueList& values, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			if(auto parser=GetParserManager()->GetParser<ElementName>(L"INSTANCE-ELEMENT-NAME"))
			{
				// test if there is only one text value in the xml
				if(xml->subNodes.Count()==1)
				{
					if(Ptr<XmlText> text=xml->subNodes[0].Cast<XmlText>())
					{
						Ptr<GuiTextRepr> value=new GuiTextRepr;
						value->text=text->content.value;
						values.Add(value);
					}
					else if(Ptr<XmlCData> text=xml->subNodes[0].Cast<XmlCData>())
					{
						Ptr<GuiTextRepr> value=new GuiTextRepr;
						value->text=text->content.value;
						values.Add(value);
					}
				}

				// collect default attributes
				FOREACH(Ptr<XmlElement>, element, XmlGetElements(xml))
				{
					if(auto name=parser->TypedParse(element->name.value, errors))
					{
						if(name->IsCtorName())
						{
							// collect constructor values in the default attribute setter
							auto ctor=LoadCtor(element, errors);
							if(ctor)
							{
								values.Add(ctor);
							}
						}
					}
				}
			}
		}

		void GuiInstanceContext::CollectAttributes(GuiAttSetterRepr::SetteValuerMap& setters, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			if(auto parser=GetParserManager()->GetParser<ElementName>(L"INSTANCE-ELEMENT-NAME"))
			{
				Ptr<GuiAttSetterRepr::SetterValue> defaultValue=new GuiAttSetterRepr::SetterValue;

				// collect default attributes
				CollectDefaultAttributes(defaultValue->values, xml, errors);
				if(defaultValue->values.Count()>0)
				{
					setters.Add(GlobalStringKey::Empty, defaultValue);
				}

				// collect values
				FOREACH(Ptr<XmlElement>, element, XmlGetElements(xml))
				{
					if (auto name = parser->TypedParse(element->name.value, errors))
					{
						if(name->IsPropertyElementName())
						{
							// collect a value as a new attribute setter
							if (setters.Keys().Contains(GlobalStringKey::Get(name->name)))
							{
								errors.Add(L"Duplicated attribute name \"" + name->name + L"\".");
							}
							else
							{
								Ptr<GuiAttSetterRepr::SetterValue> sv=new GuiAttSetterRepr::SetterValue;
								sv->binding = GlobalStringKey::Get(name->binding);

								if(name->binding==L"set")
								{
									// if the binding is "set", it means that this element is a complete setter element
									Ptr<GuiAttSetterRepr> setter=new GuiAttSetterRepr;
									FillAttSetter(setter, element, errors);
									sv->values.Add(setter);
								}
								else
								{
									// if the binding is not "set", then this is a single-value attribute or a colection attribute
									// fill all data into this attribute
									CollectDefaultAttributes(sv->values, element, errors);
								}

								if(sv->values.Count()>0)
								{
									setters.Add(GlobalStringKey::Get(name->name), sv);
								}
							}
						}
					}
				}
			}
		}

		void GuiInstanceContext::CollectEvents(GuiAttSetterRepr::EventHandlerMap& eventHandlers, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			if(auto parser=GetParserManager()->GetParser<ElementName>(L"INSTANCE-ELEMENT-NAME"))
			{
				// collect values
				FOREACH(Ptr<XmlElement>, element, XmlGetElements(xml))
				{
					if(auto name=parser->TypedParse(element->name.value, errors))
					{
						if(name->IsEventElementName())
						{
							// collect a value as a new attribute setter
							if (eventHandlers.Keys().Contains(GlobalStringKey::Get(name->name)))
							{
								errors.Add(L"Duplicated event name \"" + name->name + L"\".");
							}
							else
							{
								// test if there is only one text value in the xml
								if(element->subNodes.Count()==1)
								{
									if(Ptr<XmlText> text=element->subNodes[0].Cast<XmlText>())
									{
										auto value = MakePtr<GuiAttSetterRepr::EventValue>();
										value->binding = GlobalStringKey::Get(name->binding);
										value->value = text->content.value;
										eventHandlers.Add(GlobalStringKey::Get(name->name), value);
									}
									else if(Ptr<XmlCData> text=element->subNodes[0].Cast<XmlCData>())
									{
										auto value = MakePtr<GuiAttSetterRepr::EventValue>();
										value->binding = GlobalStringKey::Get(name->binding);
										value->value = text->content.value;
										eventHandlers.Add(GlobalStringKey::Get(name->name), value);
									}
								}
							}
						}
					}
				}
			}
		}

		void GuiInstanceContext::FillAttSetter(Ptr<GuiAttSetterRepr> setter, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			if(auto parser=GetParserManager()->GetParser<ElementName>(L"INSTANCE-ELEMENT-NAME"))
			{
				// collect attributes as setters
				FOREACH(Ptr<XmlAttribute>, att, xml->attributes)
				{
					if (auto name = parser->TypedParse(att->name.value, errors))
					{
						if(name->IsReferenceAttributeName())
						{
							// collect reference attributes
							if (name->name == L"Name")
							{
								setter->instanceName = GlobalStringKey::Get(att->value.value);
							}
						}
						else if(name->IsPropertyAttributeName())
						{
							// collect attributes setters
							if (setter->setters.Keys().Contains(GlobalStringKey::Get(name->name)))
							{
								errors.Add(L"Duplicated attribute name \"" + name->name + L"\".");
							}
							else
							{
								Ptr<GuiAttSetterRepr::SetterValue> sv=new GuiAttSetterRepr::SetterValue;
								sv->binding=GlobalStringKey::Get(name->binding);
								setter->setters.Add(GlobalStringKey::Get(name->name), sv);

								Ptr<GuiTextRepr> value=new GuiTextRepr;
								value->text=att->value.value;
								sv->values.Add(value);
							}
						}
						else if (name->IsEventAttributeName())
						{
							// collect event setters
							if (!setter->eventHandlers.Keys().Contains(GlobalStringKey::Get(name->name)))
							{
								auto value = MakePtr<GuiAttSetterRepr::EventValue>();
								value->binding = GlobalStringKey::Get(name->binding);
								value->value = att->value.value;
								setter->eventHandlers.Add(GlobalStringKey::Get(name->name), value);
							}
						}
					}
				}

				// collect attributes and events
				CollectAttributes(setter->setters, xml, errors);
				CollectEvents(setter->eventHandlers, xml, errors);
			}
		}

		Ptr<GuiConstructorRepr> GuiInstanceContext::LoadCtor(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			if (auto parser = GetParserManager()->GetParser<ElementName>(L"INSTANCE-ELEMENT-NAME"))
			{
				if (auto name = parser->TypedParse(xml->name.value, errors))
				{
					if(name->IsCtorName())
					{
						Ptr<GuiConstructorRepr> ctor=new GuiConstructorRepr;
						ctor->typeNamespace = GlobalStringKey::Get(name->namespaceName);
						ctor->typeName = GlobalStringKey::Get(name->name);
						// collect attributes as setters
						FOREACH(Ptr<XmlAttribute>, att, xml->attributes)
						{
							if(auto name=parser->TypedParse(att->name.value, errors))
							if(name->IsReferenceAttributeName())
							{
								if (name->name == L"Style")
								{
									ctor->styleName = att->value.value;
								}
							}
						}
						FillAttSetter(ctor, xml, errors);
						return ctor;
					}
					else
					{
						errors.Add(L"Wrong constructor name \"" + xml->name.value + L"\".");
					}
				}
			}
			return 0;
		}

		Ptr<GuiInstanceContext> GuiInstanceContext::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors)
		{
			Ptr<GuiInstanceContext> context=new GuiInstanceContext;
			if(xml->rootElement->name.value==L"Instance")
			{
				if (auto codeBehindAttr = XmlGetAttribute(xml->rootElement, L"ref.CodeBehind"))
				{
					context->codeBehind = codeBehindAttr->value.value == L"true";
				}

				// load type name
				if (auto classAttr = XmlGetAttribute(xml->rootElement, L"ref.Class"))
				{
					context->className = classAttr->value.value;
				}

				// load style names
				if (auto styleAttr = XmlGetAttribute(xml->rootElement, L"ref.Styles"))
				{
					SplitBySemicolon(styleAttr->value.value, context->stylePaths);
				}

				// load namespaces
				List<Ptr<XmlAttribute>> namespaceAttributes;
				CopyFrom(namespaceAttributes, xml->rootElement->attributes);
				if(!XmlGetAttribute(xml->rootElement, L"xmlns"))
				{
					Ptr<XmlAttribute> att=new XmlAttribute;
					att->name.value=L"xmlns";
					att->value.value =
						L"presentation::controls::Gui*;"
						L"presentation::elements::Gui*Element;"
						L"presentation::compositions::Gui*Composition;"
						L"presentation::compositions::Gui*;"
						L"presentation::templates::Gui*;"
						L"system::*;"
						L"system::reflection::*;"
						L"presentation::*;"
						L"presentation::Gui*;"
						L"presentation::controls::*;"
						L"presentation::controls::list::*;"
						L"presentation::controls::tree::*;"
						L"presentation::elements::*;"
						L"presentation::elements::Gui*;"
						L"presentation::elements::text*;"
						L"presentation::compositions::*;"
						L"presentation::templates::*";
					namespaceAttributes.Add(att);
				}
				FOREACH(Ptr<XmlAttribute>, att, namespaceAttributes)
				{
					// check if the attribute defines a namespace
					WString attName=att->name.value;
					if(attName.Length()>=5 && attName.Left(5)==L"xmlns")
					{
						GlobalStringKey ns;
						if(attName.Length()>6)
						{
							if(attName.Left(6)==L"xmlns:")
							{
								ns = GlobalStringKey::Get(attName.Sub(6, attName.Length() - 6));
							}
							else
							{
								continue;
							}
						}

						// create a data structure for the namespace
						Ptr<NamespaceInfo> info;
						vint index=context->namespaces.Keys().IndexOf(ns);
						if(index==-1)
						{
							info=new NamespaceInfo;
							info->name=ns;
							context->namespaces.Add(ns, info);
						}
						else
						{
							info=context->namespaces.Values()[index];
						}

						// extract all patterns in the namespace, split the value by ';'
						List<WString> patterns;
						SplitBySemicolon(att->value.value, patterns);
						FOREACH(WString, pattern, patterns)
						{
							// add the pattern to the namespace
							Ptr<GuiInstanceNamespace> ns=new GuiInstanceNamespace;
							Pair<vint, vint> star=INVLOC.FindFirst(pattern, L"*", Locale::None);
							if(star.key==-1)
							{
								ns->prefix=pattern;
							}
							else
							{
								ns->prefix=pattern.Sub(0, star.key);
								ns->postfix=pattern.Sub(star.key+star.value, pattern.Length()-star.key-star.value);
							}
							info->namespaces.Add(ns);
						}
					}
				}

				// load instance
				FOREACH(Ptr<XmlElement>, element, XmlGetElements(xml->rootElement))
				{
					if (element->name.value == L"ref.Parameter")
					{
						auto attName = XmlGetAttribute(element, L"Name");
						auto attClass = XmlGetAttribute(element, L"Class");
						if (attName && attClass)
						{
							auto parameter = MakePtr<GuiInstanceParameter>();
							parameter->name = GlobalStringKey::Get(attName->value.value);
							parameter->className = GlobalStringKey::Get(attClass->value.value);
							context->parameters.Add(parameter);
						}
					}
					else if (element->name.value == L"ref.Property")
					{
						auto attName = XmlGetAttribute(element, L"Name");
						auto attType = XmlGetAttribute(element, L"Type");
						auto attReadonly = XmlGetAttribute(element, L"Readonly");
						if (attName && attType)
						{
							auto prop = MakePtr<GuiInstanceProperty>();
							prop->name = GlobalStringKey::Get(attName->value.value);
							prop->typeName = attType->value.value;
							if (attReadonly)
							{
								prop->readonly = attReadonly->value.value == L"true";
							}
							context->properties.Add(prop);
						}
					}
					else if (element->name.value == L"ref.State")
					{
						auto attName = XmlGetAttribute(element, L"Name");
						auto attType = XmlGetAttribute(element, L"Type");
						if (attName && attType)
						{
							auto state = MakePtr<GuiInstanceState>();
							state->name = GlobalStringKey::Get(attName->value.value);
							state->typeName = attType->value.value;
							context->states.Add(state);
						}
					}
					else if (element->name.value == L"ref.Cache")
					{
						auto attName = XmlGetAttribute(element, L"Name");
						auto attType = XmlGetAttribute(element, L"Type");
						if (attName && attType)
						{
							auto resolver = GetInstanceLoaderManager()->GetInstanceCacheResolver(GlobalStringKey::Get(attType->value.value));

							MemoryStream stream;
							HexToBinary(stream, XmlGetValue(element));
							stream.SeekFromBegin(0);

							auto cache = resolver->Deserialize(stream);
							context->precompiledCaches.Add(GlobalStringKey::Get(attName->value.value), cache);
						}
					}
					else if (!context->instance)
					{
						context->instance=LoadCtor(element, errors);
					}
				}
			}

			return context->instance ? context : nullptr;
		}

		Ptr<parsing::xml::XmlDocument> GuiInstanceContext::SaveToXml(bool serializePrecompiledResource)
		{
			auto xmlInstance = MakePtr<XmlElement>();
			xmlInstance->name.value = L"Instance";

			{
				auto attCodeBehind = MakePtr<XmlAttribute>();
				attCodeBehind->name.value = L"ref.CodeBehind";
				attCodeBehind->value.value = codeBehind ? L"true" : L"false";
				xmlInstance->attributes.Add(attCodeBehind);
			}
			
			if (className)
			{
				auto attClass = MakePtr<XmlAttribute>();
				attClass->name.value = L"ref.Class";
				attClass->value.value = className.Value();
				xmlInstance->attributes.Add(attClass);
			}

			for (vint i = 0; i < namespaces.Count(); i++)
			{
				auto key = namespaces.Keys()[i];
				auto value = namespaces.Values()[i];

				auto xmlns = MakePtr<XmlAttribute>();
				xmlns->name.value = L"xmlns";
				if (key != GlobalStringKey::Empty)
				{
					xmlns->name.value += L":" + key.ToString();
				}
				xmlInstance->attributes.Add(xmlns);

				for (vint j = 0; j < value->namespaces.Count(); j++)
				{
					auto ns = value->namespaces[j];
					if (j != 0)
					{
						xmlns->value.value += L";";
					}
					xmlns->value.value += ns->prefix + L"*" + ns->postfix;
				}
			}

			FOREACH(Ptr<GuiInstanceParameter>, parameter, parameters)
			{
				auto xmlParameter = MakePtr<XmlElement>();
				xmlParameter->name.value = L"ref.Parameter";
				xmlInstance->subNodes.Add(xmlParameter);

				auto attName = MakePtr<XmlAttribute>();
				attName->name.value = L"Name";
				attName->value.value = parameter->name.ToString();
				xmlParameter->attributes.Add(attName);

				auto attClass = MakePtr<XmlAttribute>();
				attClass->name.value = L"Class";
				attClass->value.value = parameter->className.ToString();
				xmlParameter->attributes.Add(attClass);
			}

			FOREACH(Ptr<GuiInstanceProperty>, prop, properties)
			{
				auto xmlProperty = MakePtr<XmlElement>();
				xmlProperty->name.value = L"ref.Property";
				xmlInstance->subNodes.Add(xmlProperty);

				auto attName = MakePtr<XmlAttribute>();
				attName->name.value = L"Name";
				attName->value.value = prop->name.ToString();
				xmlProperty->attributes.Add(attName);

				auto attType = MakePtr<XmlAttribute>();
				attType->name.value = L"Type";
				attType->value.value = prop->typeName;
				xmlProperty->attributes.Add(attType);

				auto attReadonly = MakePtr<XmlAttribute>();
				attReadonly->name.value = L"Readonly";
				attReadonly->value.value = prop->readonly ? L"true" : L"false";
				xmlProperty->attributes.Add(attReadonly);
			}

			FOREACH(Ptr<GuiInstanceState>, state, states)
			{
				auto xmlState = MakePtr<XmlElement>();
				xmlState->name.value = L"ref.State";
				xmlInstance->subNodes.Add(xmlState);

				auto attName = MakePtr<XmlAttribute>();
				attName->name.value = L"Name";
				attName->value.value = state->name.ToString();
				xmlState->attributes.Add(attName);

				auto attType = MakePtr<XmlAttribute>();
				attType->name.value = L"Type";
				attType->value.value = state->typeName;
				xmlState->attributes.Add(attType);
			}

			if (!serializePrecompiledResource && stylePaths.Count() > 0)
			{
				auto attStyles = MakePtr<XmlAttribute>();
				attStyles->name.value = L"ref.Styles";
				xmlInstance->attributes.Add(attStyles);

				for (vint j = 0; j < stylePaths.Count(); j++)
				{
					if (j != 0)
					{
						attStyles->value.value += L";";
					}
					attStyles->value.value += stylePaths[j];
				}
			}

			if (serializePrecompiledResource)
			{
				for (vint i = 0; i < precompiledCaches.Count(); i++)
				{
					auto key = precompiledCaches.Keys()[i];
					auto value = precompiledCaches.Values()[i];
					auto resolver = GetInstanceLoaderManager()->GetInstanceCacheResolver(value->GetCacheTypeName());

					MemoryStream stream;
					resolver->Serialize(value, stream);
					stream.SeekFromBegin(0);
					auto hex = BinaryToHex(stream);
					
					auto xmlCache = MakePtr<XmlElement>();
					xmlCache->name.value = L"ref.Cache";
					xmlInstance->subNodes.Add(xmlCache);

					auto attName = MakePtr<XmlAttribute>();
					attName->name.value = L"Name";
					attName->value.value = key.ToString();
					xmlCache->attributes.Add(attName);

					auto attType = MakePtr<XmlAttribute>();
					attType->name.value = L"Type";
					attType->value.value = value->GetCacheTypeName().ToString();
					xmlCache->attributes.Add(attType);

					auto xmlContent = MakePtr<XmlCData>();
					xmlContent->content.value = hex;
					xmlCache->subNodes.Add(xmlContent);
				}
			}

			instance->FillXml(xmlInstance, serializePrecompiledResource);

			auto doc = MakePtr<XmlDocument>();
			doc->rootElement = xmlInstance;
			return doc;
		}

		Ptr<GuiInstanceContext> GuiInstanceContext::LoadPrecompiledBinary(stream::IStream& stream, collections::List<WString>& errors)
		{
			stream::internal::Reader reader(stream);
			List<GlobalStringKey> sortedKeys;
			{
				vint count = 0;
				reader << count;

				for (vint i = 0; i < count; i++)
				{
					WString keyString;
					reader << keyString;
					sortedKeys.Add(GlobalStringKey::Get(keyString));
				}
			}

			auto context = MakePtr<GuiInstanceContext>();
			context->appliedStyles = true;
			{
				context->instance = GuiConstructorRepr::LoadPrecompiledBinary(stream, sortedKeys);
			}
			{
				vint count = -1;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					vint keyIndex = -1;
					vint valueNameIndex = -1;
					reader << keyIndex << valueNameIndex;

					auto key = sortedKeys[keyIndex];
					auto ni = MakePtr<NamespaceInfo>();
					ni->name = sortedKeys[valueNameIndex];
					context->namespaces.Add(key, ni);

					vint valueCount = -1;
					reader << valueCount;
					for (vint j = 0; j < valueCount; j++)
					{
						auto ns = MakePtr<GuiInstanceNamespace>();
						reader << ns->prefix << ns->postfix;
						ni->namespaces.Add(ns);
					}
				}
			}
			{
				reader << context->codeBehind << context->className;
			}
			{
				vint count = -1;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					vint nameIndex = -1;
					vint classNameIndex = -1;
					reader << nameIndex << classNameIndex;

					auto parameter = MakePtr<GuiInstanceParameter>();
					parameter->name = sortedKeys[nameIndex];
					parameter->className = sortedKeys[classNameIndex];
					context->parameters.Add(parameter);
				}
			}
			{
				vint count = -1;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					vint nameIndex = -1;
					WString typeName;
					bool readonly = false;
					reader << nameIndex << typeName << readonly;

					auto prop = MakePtr<GuiInstanceProperty>();
					prop->name = sortedKeys[nameIndex];
					prop->typeName = typeName;
					prop->readonly = readonly;
					context->properties.Add(prop);
				}
			}
			{
				vint count = -1;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					vint nameIndex = -1;
					WString typeName;
					reader << nameIndex << typeName;

					auto state = MakePtr<GuiInstanceState>();
					state->name = sortedKeys[nameIndex];
					state->typeName = typeName;
					context->states.Add(state);
				}
			}
			{
				vint count = 0;
				reader << count;

				for (vint i = 0; i < count; i++)
				{
					vint keyIndex = -1;
					vint nameIndex = -1;
					stream::MemoryStream stream;
					reader << keyIndex << nameIndex << (stream::IStream&)stream;

					auto key = sortedKeys[keyIndex];
					auto name = sortedKeys[nameIndex];
					if (auto resolver = GetInstanceLoaderManager()->GetInstanceCacheResolver(name))
					{
						if (auto cache = resolver->Deserialize(stream))
						{
							context->precompiledCaches.Add(key, cache);
						}
					}
				}
			}

			return context;
		}

		void GuiInstanceContext::SavePrecompiledBinary(stream::IStream& stream)
		{
			stream::internal::Writer writer(stream);
			SortedList<GlobalStringKey> sortedKeys;
			{
				List<GlobalStringKey> keys;
				CollectUsedKey(keys);
				CopyFrom(sortedKeys, From(keys).Distinct());

				vint count = sortedKeys.Count();
				writer << count;
				FOREACH(GlobalStringKey, key, sortedKeys)
				{
					WString keyString = key.ToString();
					writer << keyString;
				}
			}
			{
				instance->SavePrecompiledBinary(stream, sortedKeys, false);
			}
			{
				vint count = namespaces.Count();
				writer << count;
				for (vint i = 0; i < count; i++)
				{
					auto keyIndex = sortedKeys.IndexOf(namespaces.Keys()[i]);
					auto value = namespaces.Values()[i];
					auto valueNameIndex = sortedKeys.IndexOf(value->name);
					CHECK_ERROR(keyIndex != -1 && valueNameIndex != -1, L"GuiInstanceContext::SavePrecompiledBinary(stream::IStream&)#Internal Error.");
					writer << keyIndex << valueNameIndex;

					vint valueCount = value->namespaces.Count();
					writer << valueCount;
					FOREACH(Ptr<GuiInstanceNamespace>, ns, value->namespaces)
					{
						writer << ns->prefix << ns->postfix;
					}
				}
			}
			{
				writer << codeBehind << className;
			}
			{
				vint count = parameters.Count();
				writer << count;
				FOREACH(Ptr<GuiInstanceParameter>, parameter, parameters)
				{
					vint nameIndex = sortedKeys.IndexOf(parameter->name);
					vint classNameIndex = sortedKeys.IndexOf(parameter->className);
					CHECK_ERROR(nameIndex != -1 && classNameIndex != -1, L"GuiInstanceContext::SavePrecompiledBinary(stream::IStream&)#Internal Error.");
					writer << nameIndex << classNameIndex;
				}
			}
			{
				vint count = properties.Count();
				writer << count;
				FOREACH(Ptr<GuiInstanceProperty>, prop, properties)
				{
					vint nameIndex = sortedKeys.IndexOf(prop->name);
					WString typeName = prop->typeName;
					bool readonly = prop->readonly;
					CHECK_ERROR(nameIndex != -1, L"GuiInstanceContext::SavePrecompiledBinary(stream::IStream&)#Internal Error.");
					writer << nameIndex << typeName << readonly;
				}
			}
			{
				vint count = states.Count();
				writer << count;
				FOREACH(Ptr<GuiInstanceState>, state, states)
				{
					vint nameIndex = sortedKeys.IndexOf(state->name);
					WString typeName = state->typeName;
					CHECK_ERROR(nameIndex != -1, L"GuiInstanceContext::SavePrecompiledBinary(stream::IStream&)#Internal Error.");
					writer << nameIndex << typeName;
				}
			}
			{
				vint count = precompiledCaches.Count();
				writer << count;
				for (vint i = 0; i < count; i++)
				{
					auto keyIndex = sortedKeys.IndexOf(precompiledCaches.Keys()[i]);
					auto cache = precompiledCaches.Values()[i];
					auto name = cache->GetCacheTypeName();
					vint nameIndex = sortedKeys.IndexOf(name);
					CHECK_ERROR(keyIndex != -1 && nameIndex != -1, L"GuiInstanceContext::SavePrecompiledBinary(stream::IStream&)#Internal Error.");

					stream::MemoryStream stream;

					if (auto resolver = GetInstanceLoaderManager()->GetInstanceCacheResolver(name))
					{
						resolver->Serialize(cache, stream);
					}
					writer << keyIndex << nameIndex << (stream::IStream&)stream;
				}
			}
		}

		void GuiInstanceContext::CollectUsedKey(collections::List<GlobalStringKey>& keys)
		{
			instance->CollectUsedKey(keys);
			
			for (vint i = 0; i < namespaces.Count(); i++)
			{
				keys.Add(namespaces.Keys()[i]);
				keys.Add(namespaces.Values()[i]->name);
			}

			for (vint i = 0; i < parameters.Count(); i++)
			{
				keys.Add(parameters[i]->name);
				keys.Add(parameters[i]->className);
			}

			for (vint i = 0; i < precompiledCaches.Count(); i++)
			{
				keys.Add(precompiledCaches.Keys()[i]);
				keys.Add(precompiledCaches.Values()[i]->GetCacheTypeName());
			}
		}

		bool GuiInstanceContext::ApplyStyles(Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)
		{
			if (!appliedStyles)
			{
				appliedStyles = true;

				List<Ptr<GuiInstanceStyle>> styles;
				FOREACH(WString, uri, stylePaths)
				{
					WString protocol, path;
					if (IsResourceUrl(uri, protocol, path))
					{
						if (auto styleContext = resolver->ResolveResource(protocol, path).Cast<GuiInstanceStyleContext>())
						{
							CopyFrom(styles, styleContext->styles, true);
						}
						else
						{
							errors.Add(L"Failed to find the style referred in attribute \"ref.Styles\": \"" + uri + L"\".");
						}
					}
					else
					{
						errors.Add(L"Invalid path in attribute \"ref.Styles\": \"" + uri + L"\".");
					}
				}

				FOREACH(Ptr<GuiInstanceStyle>, style, styles)
				{
					List<Ptr<GuiConstructorRepr>> output;
					ExecuteQuery(style->query, this, output);
					FOREACH(Ptr<GuiConstructorRepr>, ctor, output)
					{
						ApplyStyle(style, ctor);
					}
				}

				return true;
			}
			else
			{
				return false;
			}
		}

/***********************************************************************
GuiInstanceStyle
***********************************************************************/

		namespace visitors
		{
			class SetStyleMarkVisitor : public Object, public GuiValueRepr::IVisitor
			{
			public:
				void Visit(GuiTextRepr* repr)override
				{
					repr->fromStyle = true;
				}

				void Visit(GuiAttSetterRepr* repr)override
				{
					repr->fromStyle = true;
					FOREACH(Ptr<GuiAttSetterRepr::SetterValue>, value, repr->setters.Values())
					{
						FOREACH(Ptr<GuiValueRepr>, subValue, value->values)
						{
							subValue->Accept(this);
						}
					}
					FOREACH(Ptr<GuiAttSetterRepr::EventValue>, value, repr->eventHandlers.Values())
					{
						value->fromStyle = true;
					}
				}

				void Visit(GuiConstructorRepr* repr)override
				{
					Visit((GuiAttSetterRepr*)repr);
				}
			};
		}
		using namespace visitors;

		Ptr<GuiInstanceStyle> GuiInstanceStyle::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors)
		{
			auto style = MakePtr<GuiInstanceStyle>();
			if (auto pathAttr = XmlGetAttribute(xml, L"ref.Path"))
			{
				auto parser = GetParserManager()->GetParser<GuiIqQuery>(L"INSTANCE-QUERY");
				if (auto query = parser->TypedParse(pathAttr->value.value, errors))
				{
					style->query = query;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				errors.Add(L"Missing attribute \"ref.Path\" in <Style>.");
			}
			style->setter = MakePtr<GuiAttSetterRepr>();
			GuiInstanceContext::FillAttSetter(style->setter, xml, errors);

			SetStyleMarkVisitor visitor;
			style->setter->Accept(&visitor);
			return style;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceStyle::SaveToXml()
		{
			auto xmlStyle = MakePtr<XmlElement>();
			xmlStyle->name.value = L"Style";

			auto attPath = MakePtr<XmlAttribute>();
			attPath->name.value = L"ref.Path";
			{
				MemoryStream stream;
				{
					StreamWriter writer(stream);
					GuiIqPrint(query, writer);
				}
				stream.SeekFromBegin(0);
				{
					StreamReader reader(stream);
					attPath->value.value = reader.ReadToEnd();
				}
			}
			xmlStyle->attributes.Add(attPath);

			setter->FillXml(xmlStyle, true);

			return xmlStyle;
		}

/***********************************************************************
GuiInstanceStyleContext
***********************************************************************/

		Ptr<GuiInstanceStyleContext> GuiInstanceStyleContext::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors)
		{
			auto context = MakePtr<GuiInstanceStyleContext>();
			FOREACH(Ptr<XmlElement>, styleElement, XmlGetElements(xml->rootElement))
			{
				if (styleElement->name.value == L"Style")
				{
					if (auto style = GuiInstanceStyle::LoadFromXml(styleElement, errors))
					{
						context->styles.Add(style);
					}
				}
				else
				{
					errors.Add(L"Unknown style type \"" + styleElement->name.value + L"\".");
				}
			}
			return context;
		}

		Ptr<parsing::xml::XmlDocument> GuiInstanceStyleContext::SaveToXml()
		{
			auto xmlStyles = MakePtr<XmlElement>();
			xmlStyles->name.value = L"Styles";

			FOREACH(Ptr<GuiInstanceStyle>, style, styles)
			{
				xmlStyles->subNodes.Add(style->SaveToXml());
			}

			auto doc = MakePtr<XmlDocument>();
			doc->rootElement = xmlStyles;
			return doc;
		}
	}
}