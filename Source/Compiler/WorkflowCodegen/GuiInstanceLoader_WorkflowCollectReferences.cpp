#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection::description;
		using namespace collections;
		using namespace parsing;
		using namespace workflow::analyzer;

/***********************************************************************
WorkflowReferenceNamesVisitor
***********************************************************************/

		class WorkflowReferenceNamesVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			types::ResolvingResult&				resolvingResult;
			vint&								generatedNameCount;
			GuiResourceError::List&				errors;

			List<types::PropertyResolving>&		candidatePropertyTypeInfos;
			IGuiInstanceLoader::TypeInfo		resolvedTypeInfo;
			vint								selectedPropertyTypeInfo = -1;

			WorkflowReferenceNamesVisitor(types::ResolvingResult& _resolvingResult, List<types::PropertyResolving>& _candidatePropertyTypeInfos, vint& _generatedNameCount, GuiResourceError::List& _errors)
				:resolvingResult(_resolvingResult)
				, candidatePropertyTypeInfos(_candidatePropertyTypeInfos)
				, generatedNameCount(_generatedNameCount)
				, errors(_errors)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
				if (selectedPropertyTypeInfo == -1)
				{
					selectedPropertyTypeInfo = 0;
				}
				
				auto candidate = candidatePropertyTypeInfos[selectedPropertyTypeInfo];
				auto propertyInfo = candidate.propertyInfo;
				auto td = candidate.info->acceptableTypes[0];

				if (auto st = td->GetSerializableType())
				{
					Value value;
					if (st->Deserialize(repr->text, value))
					{
						resolvingResult.propertyResolvings.Add(repr, candidate);
					}
					else
					{
						auto error
							= L"Precompile: Property \""
							+ propertyInfo.propertyName.ToString()
							+ L"\" of type \""
							+ propertyInfo.typeInfo.typeName.ToString()
							+ L"\" does not accept a value of text \""
							+ repr->text
							+ L"\" because it is not in a correct format of the serializable type \""
							+ td->GetTypeName()
							+ L"\".";
						errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition, error));
					}
				}
				else
				{
					switch (td->GetTypeDescriptorFlags())
					{
					case TypeDescriptorFlags::FlagEnum:
					case TypeDescriptorFlags::NormalEnum:
					case TypeDescriptorFlags::Struct:
						{
							List<Ptr<ParsingError>> parsingErrors;
							if (auto expression = Workflow_ParseTextValue(td, repr->text, parsingErrors))
							{
								resolvingResult.propertyResolvings.Add(repr, candidate);
							}
							GuiResourceError::Transform({ resolvingResult.resource }, errors, parsingErrors, repr->tagPosition);
						}
						break;
					default:
						{
							auto error
								= L"Precompile: Property \""
								+ propertyInfo.propertyName.ToString()
								+ L"\" of type \""
								+ propertyInfo.typeInfo.typeName.ToString()
								+ L"\" does not accept a value of text \""
								+ repr->text
								+ L"\" because its type \""
								+ td->GetTypeName()
								+ L"\" is not serializable.";
							errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition, error));
						}
					}
				}
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				if (candidatePropertyTypeInfos.Count() > 0)
				{
					resolvingResult.propertyResolvings.Add(repr, candidatePropertyTypeInfos[selectedPropertyTypeInfo]);
				}

				bool isReferenceType = (resolvedTypeInfo.typeDescriptor->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined;
				if (repr->instanceName == GlobalStringKey::Empty)
				{
					if (isReferenceType)
					{
						auto name = GlobalStringKey::Get(L"<precompile>" + itow(generatedNameCount++));
						repr->instanceName = name;
						resolvingResult.typeInfos.Add(name, resolvedTypeInfo);
					}
				}
				else if (resolvingResult.typeInfos.Keys().Contains(repr->instanceName))
				{
					auto error
						= L"Precompile: Referece name \""
						+ repr->instanceName.ToString()
						+ L"\" conflict with an existing named object.";
					errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition, error));
				}
				else if (!isReferenceType)
				{
					auto error
						= L"Precompile: Reference name \""
						+ repr->instanceName.ToString()
						+ L"\" cannot be added to a non-reference instance of type \""
						+ resolvedTypeInfo.typeName.ToString()
						+ L"\".";
					errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition, error));
				}
				else
				{
					resolvingResult.referenceNames.Add(repr->instanceName);
					resolvingResult.typeInfos.Add(repr->instanceName, resolvedTypeInfo);
				}
			
				auto loader = GetInstanceLoaderManager()->GetLoader(resolvedTypeInfo.typeName);

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					List<types::PropertyResolving> possibleInfos;
					auto prop = repr->setters.Keys()[index];
					IGuiInstanceLoader::PropertyInfo propertyInfo(resolvedTypeInfo, prop);

					auto errorPrefix = L"Precompile: Property \"" + propertyInfo.propertyName.ToString() + L"\" of type \"" + resolvedTypeInfo.typeName.ToString() + L"\"";
					{
						auto currentLoader = loader;

						while (currentLoader)
						{
							if (auto propertyTypeInfo = currentLoader->GetPropertyType(propertyInfo))
							{
								if (propertyTypeInfo->support == GuiInstancePropertyInfo::NotSupport)
								{
									errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" is not supported."));
									break;
								}
								else
								{
									types::PropertyResolving resolving;
									resolving.loader = currentLoader;
									resolving.propertyInfo = propertyInfo;
									resolving.info = propertyTypeInfo;
									possibleInfos.Add(resolving);

									if (setter->binding == GlobalStringKey::_Set)
									{
										break;
									}
								}

								if (!propertyTypeInfo->tryParent)
								{
									break;
								}
							}
							currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
						}
					}

					if (possibleInfos.Count() == 0)
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" does not exist."));
					}
					else
					{
						if (setter->binding == GlobalStringKey::Empty)
						{
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								WorkflowReferenceNamesVisitor visitor(resolvingResult, possibleInfos, generatedNameCount, errors);
								value->Accept(&visitor);
							}
						}
						else  if (setter->binding == GlobalStringKey::_Set)
						{
							if (possibleInfos[0].info->support == GuiInstancePropertyInfo::SupportSet)
							{
								auto setTarget = dynamic_cast<GuiAttSetterRepr*>(setter->values[0].Obj());

								WorkflowReferenceNamesVisitor visitor(resolvingResult, possibleInfos, generatedNameCount, errors);
								auto td = possibleInfos[0].info->acceptableTypes[0];
								visitor.selectedPropertyTypeInfo = 0;
								visitor.resolvedTypeInfo.typeDescriptor = td;
								visitor.resolvedTypeInfo.typeName = GlobalStringKey::Get(td->GetTypeName());
								setTarget->Accept(&visitor);

								if (auto propInfo = resolvedTypeInfo.typeDescriptor->GetPropertyByName(prop.ToString(), true))
								{
									auto propType = propInfo->GetReturn();
									if (propType->GetTypeDescriptor() == td)
									{
										resolvingResult.typeOverrides.Add(setTarget->instanceName, CopyTypeInfo(propInfo->GetReturn()));
									}
									else
									{
										switch (propType->GetDecorator())
										{
										case ITypeInfo::Nullable:
											{
												auto elementType = MakePtr<TypeDescriptorTypeInfo>(td, TypeInfoHint::Normal);
												auto decoratedType = MakePtr<NullableTypeInfo>(elementType);
												resolvingResult.typeOverrides.Add(setTarget->instanceName, decoratedType);
											}
											break;
										case ITypeInfo::RawPtr:
											{
												auto elementType = MakePtr<TypeDescriptorTypeInfo>(td, TypeInfoHint::Normal);
												auto decoratedType = MakePtr<RawPtrTypeInfo>(elementType);
												resolvingResult.typeOverrides.Add(setTarget->instanceName, decoratedType);
											}
											break;
										case ITypeInfo::SharedPtr:
											{
												auto elementType = MakePtr<TypeDescriptorTypeInfo>(td, TypeInfoHint::Normal);
												auto decoratedType = MakePtr<SharedPtrTypeInfo>(elementType);
												resolvingResult.typeOverrides.Add(setTarget->instanceName, decoratedType);
											}
											break;
										default:
											resolvingResult.typeOverrides.Add(setTarget->instanceName, CopyTypeInfo(propInfo->GetReturn()));
										}
									}
								}
							}
							else
							{
								errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, L"[INTERNAL-ERROR] " + errorPrefix + L" does not support the \"-set\" binding."));
							}
						}
						else if (setter->binding != GlobalStringKey::Empty)
						{
							auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding);
							if (binder)
							{
								if (possibleInfos[0].info->scope == GuiInstancePropertyInfo::Constructor)
								{
									if (!possibleInfos[0].info->bindable)
									{
										errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition,
											errorPrefix +
											L" cannot be assigned using binding \"-" +
											setter->binding.ToString() +
											L"\". Because it is a non-bindable constructor argument."));
									}
									else if (!binder->ApplicableToConstructorArgument())
									{
										errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition,
											errorPrefix +
											L" cannot be assigned using binding \"-" +
											setter->binding.ToString() +
											L"\". Because it is a constructor argument, and this binding does not apply to any constructor argument."));
									}
								}
							}
							else
							{
								errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition,
									errorPrefix +
									L" cannot be assigned using an unexisting binding \"-" +
									setter->binding.ToString() +
									L"\"."));
							}

							if (setter->values.Count() == 1 && setter->values[0].Cast<GuiTextRepr>())
							{
								resolvingResult.propertyResolvings.Add(setter->values[0].Obj(), possibleInfos[0]);
							}
							else
							{
								errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition,
									L"Precompile: Binder \"" +
									setter->binding.ToString() +
									L"\" requires the text value of property \"" +
									propertyInfo.propertyName.ToString() +
									L"\"."));
							}
						}
					}
				}

				Group<GlobalStringKey, IGuiInstanceLoader*> properties;
				CopyFrom(
					properties,
					From(repr->setters)
						.SelectMany([=](Pair<GlobalStringKey, Ptr<GuiAttSetterRepr::SetterValue>> item)
						{
							return From(item.value->values)
								.Where([=](Ptr<GuiValueRepr> value)
								{
									return resolvingResult.propertyResolvings.Keys().Contains(value.Obj());
								})
								.Select([=](Ptr<GuiValueRepr> value)
								{
									auto loader = resolvingResult.propertyResolvings[value.Obj()].loader;
									return Pair<GlobalStringKey, IGuiInstanceLoader*>(item.key, loader);
								});
						})
						.Distinct()
					);

				if (resolvingResult.context->instance.Obj() != repr)
				{
					List<GlobalStringKey> ctorProps;
					loader->GetConstructorParameters(resolvedTypeInfo, ctorProps);
					FOREACH(GlobalStringKey, prop, ctorProps)
					{
						auto info = loader->GetPropertyType(IGuiInstanceLoader::PropertyInfo(resolvedTypeInfo, prop));
						if (info->required && !properties.Contains(prop, loader))
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
								L"Precompile: Missing constructor argument \"" +
								prop.ToString() +
								L"\" of type \"" +
								resolvedTypeInfo.typeName.ToString() +
								L"\"."));
						}
					}
				}

				while (properties.Count() > 0)
				{
					auto prop = properties.Keys()[0];
					auto loader = properties.GetByIndex(0)[0];
					IGuiInstanceLoader::PropertyInfo propertyInfo(resolvedTypeInfo, prop);

					List<GlobalStringKey> pairProps;
					loader->GetPairedProperties(propertyInfo, pairProps);
					if (pairProps.Count() > 0)
					{
						List<GlobalStringKey> missingProps;
						FOREACH(GlobalStringKey, key, pairProps)
						{
							if (!properties.Contains(key, loader))
							{
								missingProps.Add(key);
							}
						}

						if (missingProps.Count() > 0)
						{
							WString error
								= L"Precompile: When you assign to property \""
								+ prop.ToString()
								+ L"\" of type \""
								+ resolvedTypeInfo.typeName.ToString()
								+ L"\", the following missing properties are required: ";
							FOREACH_INDEXER(GlobalStringKey, key, index, missingProps)
							{
								if (index > 0)error += L", ";
								error += L"\"" + key.ToString() + L"\"";
							}
							error += L".";
							errors.Add(GuiResourceError({ resolvingResult.resource }, repr->setters[prop]->attPosition, error));
						}
						
						FOREACH(GlobalStringKey, key, pairProps)
						{
							properties.Remove(key, loader);
						}
					}
					else
					{
						properties.Remove(prop, loader);
					}
				}

				FOREACH(Ptr<GuiAttSetterRepr::EventValue>, handler, repr->eventHandlers.Values())
				{
					if (handler->binding != GlobalStringKey::Empty)
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceEventBinder(handler->binding);
						if (!binder)
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, handler->attPosition,
								L"The appropriate IGuiInstanceEventBinder of binding \"-" +
								handler->binding.ToString() +
								L"\" cannot be found."));
						}
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				bool found = false;

				if (repr == resolvingResult.context->instance.Obj())
				{
					auto fullName = GlobalStringKey::Get(resolvingResult.context->className);
					auto td = GetInstanceLoaderManager()->GetTypeDescriptorForType(fullName);
					if (td)
					{
						found = true;
						resolvedTypeInfo.typeName = fullName;
						resolvedTypeInfo.typeDescriptor = td;
					}
				}

				if (!found)
				{
					auto source = FindInstanceLoadingSource(resolvingResult.context, repr);
					resolvedTypeInfo.typeName = source.typeName;
					resolvedTypeInfo.typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				}

				if (!found)
				{
					if (repr->typeNamespace == GlobalStringKey::Empty && repr->typeName.ToString() == L"Int")
					{
						resolvedTypeInfo.typeDescriptor = description::GetTypeDescriptor<vint>();
						resolvedTypeInfo.typeName = GlobalStringKey::Get(resolvedTypeInfo.typeDescriptor->GetTypeName());
					}
				}

				if (resolvedTypeInfo.typeDescriptor)
				{
					for (vint i = 0; i < candidatePropertyTypeInfos.Count(); i++)
					{
						const auto& tds = candidatePropertyTypeInfos[i].info->acceptableTypes;
						for (vint j = 0; j < tds.Count(); j++)
						{
							if (resolvedTypeInfo.typeDescriptor->CanConvertTo(tds[j]))
							{
								selectedPropertyTypeInfo = i;
								goto FINISH_MATCHING;
							}
						}
					}
				FINISH_MATCHING:

					if (selectedPropertyTypeInfo == -1 && candidatePropertyTypeInfos.Count() > 0)
					{
						auto propertyInfo = candidatePropertyTypeInfos[0].propertyInfo;
						auto error 
							= L"Precompile: Property \""
							+ propertyInfo.propertyName.ToString()
							+ L"\" of type \""
							+ propertyInfo.typeInfo.typeName.ToString()
							+ L"\" does not accept a value of type \""
							+ resolvedTypeInfo.typeName.ToString()
							+ L"\" because it only accepts value of the following types: ";
						
						for (vint i = 0; i < candidatePropertyTypeInfos.Count(); i++)
						{
							const auto& tds = candidatePropertyTypeInfos[i].info->acceptableTypes;
							for (vint j = 0; j < tds.Count(); j++)
							{
								if (i != 0 || j != 0)
								{
									error += L", ";
								}
								error += L"\"" + tds[j]->GetTypeName() + L"\"";
							}
						}

						error += L".";
						errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition, error));
					}
					else
					{
						if (repr->setters.Count() == 1 && repr->setters.Keys()[0] == GlobalStringKey::Empty)
						{
							auto setter = repr->setters.Values()[0];
							if (setter->values.Count() == 1)
							{
								if (auto text = setter->values[0].Cast<GuiTextRepr>())
								{
									if (candidatePropertyTypeInfos.Count() == 0)
									{
										errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
											L"Precompile: Type \"" +
											resolvedTypeInfo.typeName.ToString() +
											L"\" cannot be used to create an instance."));
									}
									else
									{
										Visit(text.Obj());
										auto index = resolvingResult.propertyResolvings.Keys().IndexOf(text.Obj());
										if (index != -1)
										{
											auto value = resolvingResult.propertyResolvings.Values()[index];
											resolvingResult.propertyResolvings.Remove(text.Obj());
											resolvingResult.propertyResolvings.Add(repr, value);
										}
									}
									return;
								}
							}
						}

						if (resolvingResult.context->instance.Obj() != repr)
						{
							auto loader = GetInstanceLoaderManager()->GetLoader(resolvedTypeInfo.typeName);
							while (loader)
							{
								if (loader->CanCreate(resolvedTypeInfo))
								{
									break;
								}
								loader = GetInstanceLoaderManager()->GetParentLoader(loader);
							}
							if (loader)
							{
								if (repr == resolvingResult.context->instance.Obj())
								{
									List<GlobalStringKey> propertyNames;
									loader->GetConstructorParameters(resolvedTypeInfo, propertyNames);
									if (propertyNames.Count() == 1)
									{
										if (propertyNames[0] != GlobalStringKey::_ControlTemplate)
										{
											errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
												L"Precompile: Type \"" +
												resolvedTypeInfo.typeName.ToString() +
												L"\" cannot be used to create a root instance, because its only constructor parameter is not for a the control template."));
										}
									}
									else if (propertyNames.Count() > 1)
									{
										errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
											L"Precompile: Type \"" +
											resolvedTypeInfo.typeName.ToString() +
											L"\" cannot be used to create a root instance, because it has more than one constructor parameters. A root instance type can only have one constructor parameter, which is for the control template."));
									}
								}
							}
							else
							{
								errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
									L"Precompile: Type \"" +
									resolvedTypeInfo.typeName.ToString() +
									L"\" cannot be used to create an instance."));
							}
						}
						Visit((GuiAttSetterRepr*)repr);
					}
				}
				else
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
						L"[INTERNAL-ERROR] Precompile: Failed to find type \"" +
						(repr->typeNamespace == GlobalStringKey::Empty
							? repr->typeName.ToString()
							: repr->typeNamespace.ToString() + L":" + repr->typeName.ToString()
							) +
						L"\"."));
				}
			}
		};

		ITypeDescriptor* Workflow_CollectReferences(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors)
		{
			FOREACH(Ptr<GuiInstanceParameter>, parameter, resolvingResult.context->parameters)
			{
				auto type = GetTypeDescriptor(parameter->className.ToString());
				if (!type)
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, parameter->classPosition,
						L"Precompile: Cannot find type \"" +
						parameter->className.ToString() +
						L"\"."));
				}
				else if (resolvingResult.typeInfos.Keys().Contains(parameter->name))
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, parameter->classPosition,
						L"[INTERNAL-ERROR] Precompile: Parameter \"" +
						parameter->name.ToString() +
						L"\" conflict with an existing named object."));
				}
				else
				{
					{
						IGuiInstanceLoader::TypeInfo typeInfo;
						typeInfo.typeDescriptor = type;
						typeInfo.typeName = GlobalStringKey::Get(type->GetTypeName());
						resolvingResult.typeInfos.Add(parameter->name, typeInfo);
					}
					{
						auto elementType = MakePtr<TypeDescriptorTypeInfo>(type, TypeInfoHint::Normal);
						auto pointerType = MakePtr<SharedPtrTypeInfo>(elementType);

						resolvingResult.typeOverrides.Add(parameter->name, pointerType);
					}
				}
			}
			
			List<types::PropertyResolving> infos;
			vint generatedNameCount = 0;
			WorkflowReferenceNamesVisitor visitor(resolvingResult, infos, generatedNameCount, errors);
			resolvingResult.context->instance->Accept(&visitor);
			return visitor.resolvedTypeInfo.typeDescriptor;
		}
	}
}