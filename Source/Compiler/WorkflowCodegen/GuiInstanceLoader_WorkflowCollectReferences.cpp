#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../GuiInstanceLocalizedStrings.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection::description;
		using namespace collections;
		using namespace parsing;
		using namespace workflow::analyzer;

/***********************************************************************
Workflow_AdjustPropertySearchType
***********************************************************************/

		IGuiInstanceLoader::TypeInfo Workflow_AdjustPropertySearchType(types::ResolvingResult& resolvingResult, IGuiInstanceLoader::TypeInfo resolvedTypeInfo, GlobalStringKey prop)
		{
			if (resolvedTypeInfo.typeName.ToString() == resolvingResult.context->className)
			{
				if (auto propTd = resolvedTypeInfo.typeInfo->GetTypeDescriptor())
				{
					vint baseCount = propTd->GetBaseTypeDescriptorCount();
					for (vint i = 0; i < baseCount; i++)
					{
						auto baseTd = propTd->GetBaseTypeDescriptor(i);
						if (auto ctorGroup = baseTd->GetConstructorGroup())
						{
							if (ctorGroup->GetMethodCount() == 1)
							{
								auto ctor = ctorGroup->GetMethod(0);
								auto propertyName = prop.ToString();
								auto ctorArgumentName = L"<ctor-parameter>" + propertyName;
								vint paramCount = ctor->GetParameterCount();
								for (vint j = 0; j < paramCount; j++)
								{
									auto parameterInfo = ctor->GetParameter(j);
									if (parameterInfo->GetName() == ctorArgumentName)
									{
										if (baseTd->GetPropertyByName(propertyName, false))
										{
											resolvedTypeInfo.typeInfo = CopyTypeInfo(ctor->GetReturn());
											resolvedTypeInfo.typeName = GlobalStringKey::Get(baseTd->GetTypeName());
											return resolvedTypeInfo;
										}
										break;
									}
								}
							}
						}
					}
				}
			}
			return resolvedTypeInfo;
		}

/***********************************************************************
Workflow_GetPropertyTypes
***********************************************************************/

		bool Workflow_GetPropertyTypes(WString& errorPrefix, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, IGuiInstanceLoader::TypeInfo resolvedTypeInfo, GlobalStringKey prop, Ptr<GuiAttSetterRepr::SetterValue> setter, collections::List<types::PropertyResolving>& possibleInfos, GuiResourceError::List& errors)
		{
			resolvedTypeInfo = Workflow_AdjustPropertySearchType(resolvingResult, resolvedTypeInfo, prop);
			bool reportedNotSupported = false;
			IGuiInstanceLoader::PropertyInfo propertyInfo(resolvedTypeInfo, prop);

			errorPrefix = L"Precompile: Property \"" + propertyInfo.propertyName.ToString() + L"\" of type \"" + resolvedTypeInfo.typeName.ToString() + L"\"";
			{
				auto currentLoader = loader;

				while (currentLoader)
				{
					if (auto propertyTypeInfo = currentLoader->GetPropertyType(propertyInfo))
					{
						if (propertyTypeInfo->support == GuiInstancePropertyInfo::NotSupport)
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" is not supported."));
							reportedNotSupported = true;
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

						if (propertyTypeInfo->mergability == GuiInstancePropertyInfo::NotMerge)
						{
							break;
						}
					}
					currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
				}
			}

			if (possibleInfos.Count() == 0)
			{
				if (!reportedNotSupported)
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" does not exist."));
				}
				return false;
			}
			else
			{
				return true;
			}
		}

/***********************************************************************
WorkflowReferenceNamesVisitor
***********************************************************************/

		class WorkflowReferenceNamesVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			GuiResourcePrecompileContext&		precompileContext;
			types::ResolvingResult&				resolvingResult;
			vint&								generatedNameCount;
			GuiResourceError::List&				errors;

			List<types::PropertyResolving>&		candidatePropertyTypeInfos;
			IGuiInstanceLoader::TypeInfo		resolvedTypeInfo;
			vint								selectedPropertyTypeInfo = -1;

			WorkflowReferenceNamesVisitor(GuiResourcePrecompileContext& _precompileContext, types::ResolvingResult& _resolvingResult, List<types::PropertyResolving>& _candidatePropertyTypeInfos, vint& _generatedNameCount, GuiResourceError::List& _errors)
				:precompileContext(_precompileContext)
				, resolvingResult(_resolvingResult)
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
				ITypeDescriptor* td = nullptr;
				{
					auto typeInfo = candidate.info->acceptableTypes[0];
					if (auto deserializer = GetInstanceLoaderManager()->GetInstanceDeserializer(propertyInfo, typeInfo.Obj()))
					{
						td = deserializer->DeserializeAs(propertyInfo, typeInfo.Obj())->GetTypeDescriptor();
					}
					else
					{
						td = typeInfo->GetTypeDescriptor();
					}
				}

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
							if (auto expression = Workflow_ParseTextValue(precompileContext, td, { resolvingResult.resource }, repr->text, repr->tagPosition, errors))
							{
								resolvingResult.propertyResolvings.Add(repr, candidate);
							}
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

				bool isReferenceType = (resolvedTypeInfo.typeInfo->GetTypeDescriptor()->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined;
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

					WString errorPrefix;
					if (Workflow_GetPropertyTypes(errorPrefix, resolvingResult, loader, resolvedTypeInfo, prop, setter, possibleInfos, errors))
					{
						if (setter->binding == GlobalStringKey::Empty)
						{
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								WorkflowReferenceNamesVisitor visitor(precompileContext, resolvingResult, possibleInfos, generatedNameCount, errors);
								value->Accept(&visitor);
							}
						}
						else  if (setter->binding == GlobalStringKey::_Set)
						{
							if (possibleInfos[0].info->support == GuiInstancePropertyInfo::SupportSet)
							{
								auto setTarget = dynamic_cast<GuiAttSetterRepr*>(setter->values[0].Obj());

								WorkflowReferenceNamesVisitor visitor(precompileContext, resolvingResult, possibleInfos, generatedNameCount, errors);
								auto typeInfo = possibleInfos[0].info->acceptableTypes[0];
								visitor.selectedPropertyTypeInfo = 0;
								visitor.resolvedTypeInfo.typeName = GlobalStringKey::Get(typeInfo->GetTypeDescriptor()->GetTypeName());
								visitor.resolvedTypeInfo.typeInfo = typeInfo;
								setTarget->Accept(&visitor);
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
								if (possibleInfos[0].info->usage == GuiInstancePropertyInfo::ConstructorArgument)
								{
									if (possibleInfos[0].info->bindability == GuiInstancePropertyInfo::NotBindable)
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
									prop.ToString() +
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
					List<GlobalStringKey> requiredProps;
					{
						auto currentLoader = loader;
						while (currentLoader)
						{
							currentLoader->GetRequiredPropertyNames(resolvedTypeInfo, requiredProps);
							currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
						}
					}
					FOREACH(GlobalStringKey, prop, From(requiredProps).Distinct())
					{
						if (!properties.Keys().Contains(prop))
						{
							Ptr<GuiInstancePropertyInfo> info;
							{
								auto currentLoader = loader;
								while (currentLoader && !info)
								{
									info = currentLoader->GetPropertyType({ resolvedTypeInfo, prop });
									currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
								}
							}

							errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
								L"Precompile: Missing required " +
								WString(info->usage == GuiInstancePropertyInfo::ConstructorArgument ? L"constructor argument" : L"property") +
								L" \"" +
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

				bool inferType = repr->typeNamespace == GlobalStringKey::Empty && repr->typeName == GlobalStringKey::_InferType;
				if (inferType)
				{
					if (candidatePropertyTypeInfos.Count() == 1)
					{
						auto info = candidatePropertyTypeInfos[0].info;
						if (info->acceptableTypes.Count() == 1)
						{
							auto typeInfo = info->acceptableTypes[0];
							resolvedTypeInfo.typeName = GlobalStringKey::Get(typeInfo->GetTypeDescriptor()->GetTypeName());
							resolvedTypeInfo.typeInfo = typeInfo;
						}
					}
				}
				else
				{
					if (repr == resolvingResult.context->instance.Obj())
					{
						auto fullName = GlobalStringKey::Get(resolvingResult.context->className);
						if (auto typeInfo = GetInstanceLoaderManager()->GetTypeInfoForType(fullName))
						{
							resolvedTypeInfo.typeName = fullName;
							resolvedTypeInfo.typeInfo = typeInfo;
							found = true;
						}
					}

					if (!found)
					{
						auto source = FindInstanceLoadingSource(resolvingResult.context, repr);
						resolvedTypeInfo.typeName = source.typeName;
						resolvedTypeInfo.typeInfo = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName);
					}
				}

				if (resolvingResult.context->instance == repr)
				{
					static const wchar_t Prefix[] = L"<ctor-parameter>";
					static const vint PrefixLength = (vint)sizeof(Prefix) / sizeof(*Prefix) - 1;

					auto source = FindInstanceLoadingSource(resolvingResult.context, repr);
					if (auto baseTd = description::GetTypeDescriptor(source.typeName.ToString()))
					{
						if (auto ctorGroup = baseTd->GetConstructorGroup())
						{
							if (ctorGroup->GetMethodCount() == 1)
							{
								auto ctor = ctorGroup->GetMethod(0);
								vint paramCount = ctor->GetParameterCount();
								for (vint i = 0; i < paramCount; i++)
								{
									auto parameterInfo = ctor->GetParameter(i);
									auto ctorArg = parameterInfo->GetName();
									if (ctorArg.Length() > PrefixLength && ctorArg.Left(PrefixLength) == Prefix)
									{
										auto propName = ctorArg.Right(ctorArg.Length() - PrefixLength);
										if (baseTd->GetPropertyByName(propName, false))
										{
											if (!repr->setters.Keys().Contains(GlobalStringKey::Get(propName)))
											{
												errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
													L"Precompile: Missing required property \"" +
													propName +
													L"\" of type \"" +
													resolvedTypeInfo.typeName.ToString() +
													L"\" for its base type \"" +
													baseTd->GetTypeName() +
													L"\"."));
											}
										}
									}
								}
							}
						}
					}
				}

				if (resolvedTypeInfo.typeInfo)
				{
					for (vint i = 0; i < candidatePropertyTypeInfos.Count(); i++)
					{
						const auto& typeInfos = candidatePropertyTypeInfos[i].info->acceptableTypes;
						for (vint j = 0; j < typeInfos.Count(); j++)
						{
							if (resolvedTypeInfo.typeInfo->GetTypeDescriptor()->CanConvertTo(typeInfos[j]->GetTypeDescriptor()))
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
							const auto& typeInfos = candidatePropertyTypeInfos[i].info->acceptableTypes;
							for (vint j = 0; j < typeInfos.Count(); j++)
							{
								if (i != 0 || j != 0)
								{
									error += L", ";
								}
								error += L"\"" + typeInfos[j]->GetTypeFriendlyName() + L"\"";
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
									loader->GetPropertyNames(resolvedTypeInfo, propertyNames);
									for (vint i = propertyNames.Count() - 1; i >= 0; i--)
									{
										auto info = loader->GetPropertyType({ resolvedTypeInfo, propertyNames[i] });
										if (!info || info->usage == GuiInstancePropertyInfo::Property)
										{
											propertyNames.RemoveAt(i);
										}
									}

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

		Ptr<reflection::description::ITypeInfo> Workflow_GetSuggestedParameterType(reflection::description::ITypeDescriptor* typeDescriptor)
		{
			auto elementType = MakePtr<TypeDescriptorTypeInfo>(typeDescriptor, TypeInfoHint::Normal);
			if ((typeDescriptor->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined)
			{
				bool isShared = false;
				bool isRaw = false;
				if (auto ctorGroup = typeDescriptor->GetConstructorGroup())
				{
					vint count = ctorGroup->GetMethodCount();
					for (vint i = 0; i < count; i++)
					{
						auto returnType = ctorGroup->GetMethod(i)->GetReturn();
						switch (returnType->GetDecorator())
						{
						case ITypeInfo::RawPtr: isRaw = true; break;
						case ITypeInfo::SharedPtr: isShared = true; break;
						default:;
						}
					}
				}
				if (!isShared && !isRaw)
				{
					return MakePtr<SharedPtrTypeInfo>(elementType);
				}
				else if (isShared)
				{
					return MakePtr<SharedPtrTypeInfo>(elementType);
				}
				else
				{
					return MakePtr<RawPtrTypeInfo>(elementType);
				}
			}
			else
			{
				return elementType;
			}
		}

		IGuiInstanceLoader::TypeInfo Workflow_CollectReferences(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors)
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
					auto referenceType = Workflow_GetSuggestedParameterType(type);
					resolvingResult.typeInfos.Add(parameter->name, { GlobalStringKey::Get(type->GetTypeName()),referenceType });
				}
			}
			
			List<types::PropertyResolving> infos;
			vint generatedNameCount = 0;
			WorkflowReferenceNamesVisitor visitor(precompileContext, resolvingResult, infos, generatedNameCount, errors);
			resolvingResult.context->instance->Accept(&visitor);
			return visitor.resolvedTypeInfo;
		}
	}
}