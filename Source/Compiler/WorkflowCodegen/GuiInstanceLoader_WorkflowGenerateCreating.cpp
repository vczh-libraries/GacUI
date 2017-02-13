#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;

/***********************************************************************
WorkflowGenerateCreatingVisitor
***********************************************************************/

		class WorkflowGenerateCreatingVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			types::ResolvingResult&				resolvingResult;
			Ptr<WfBlockStatement>				statements;
			GuiResourceError::List&				errors;
			
			WorkflowGenerateCreatingVisitor(types::ResolvingResult& _resolvingResult, Ptr<WfBlockStatement> _statements, GuiResourceError::List& _errors)
				:resolvingResult(_resolvingResult)
				, errors(_errors)
				, statements(_statements)
			{
			}

			IGuiInstanceLoader::ArgumentInfo GetArgumentInfo(GuiValueRepr* repr)
			{
				ITypeDescriptor* td = nullptr;
				bool serializable = false;
				WString textValue;
				ParsingTextPos textValuePosition;
				GuiConstructorRepr* ctor = nullptr;

				if (auto text = dynamic_cast<GuiTextRepr*>(repr))
				{
					td = resolvingResult.propertyResolvings[repr].info->acceptableTypes[0];
					serializable = true;
					textValue = text->text;
					textValuePosition = text->tagPosition;
				}
				else if ((ctor = dynamic_cast<GuiConstructorRepr*>(repr)))
				{
					if (ctor->instanceName == GlobalStringKey::Empty)
					{
						td = resolvingResult.propertyResolvings[repr].info->acceptableTypes[0];
					}
					else
					{
						td = resolvingResult.typeInfos[ctor->instanceName].typeDescriptor;
					}
					if ((td->GetTypeDescriptorFlags() & TypeDescriptorFlags::StructType) != TypeDescriptorFlags::Undefined)
					{
						serializable = true;
						auto value = ctor->setters.Values()[0]->values[0].Cast<GuiTextRepr>();
						textValue = value->text;
						textValuePosition = value->tagPosition;
					}
				}

				IGuiInstanceLoader::ArgumentInfo argumentInfo;
				argumentInfo.type = td;

				if (serializable)
				{
					List<Ptr<ParsingError>> parsingErrors;
					argumentInfo.expression = Workflow_ParseTextValue(td, textValue, parsingErrors);
					argumentInfo.valuePosition = textValuePosition;
					GuiResourceError::Transform(resolvingResult.resource, errors, parsingErrors, textValuePosition);
				}
				else
				{
					repr->Accept(this);

					auto ref = MakePtr<WfReferenceExpression>();
					ref->name.value = ctor->instanceName.ToString();
					argumentInfo.expression = ref;
				}

				return argumentInfo;
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				auto reprTypeInfo = resolvingResult.typeInfos[repr->instanceName];
				
				if (reprTypeInfo.typeDescriptor && (reprTypeInfo.typeDescriptor->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined)
				{
					WORKFLOW_ENVIRONMENT_VARIABLE_ADD

					Group<GlobalStringKey, IGuiInstanceLoader*> usedProps;
					FOREACH(GlobalStringKey, prop, From(repr->setters.Keys()).Reverse())
					{
						auto setter = repr->setters[prop];
						IGuiInstanceLoader::PropertyInfo propInfo(reprTypeInfo, prop);
						if (setter->binding == GlobalStringKey::_Set)
						{
							auto setTarget = dynamic_cast<GuiAttSetterRepr*>(setter->values[0].Obj());
							auto info = resolvingResult.propertyResolvings[setTarget];
							vint errorCount = errors.Count();
							if (auto expr = info.loader->GetParameter(resolvingResult, propInfo, repr->instanceName, setter->attPosition, errors))
							{
								auto refInstance = MakePtr<WfReferenceExpression>();
								refInstance->name.value = setTarget->instanceName.ToString();

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refInstance;
								assign->second = expr;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;

								statements->statements.Add(stat);
							}
							else if (errorCount == errors.Count())
							{
								errors.Add(GuiResourceError(resolvingResult.resource, setTarget->tagPosition,
									L"Precompile: Something is wrong when retriving the property \"" +
									prop.ToString() +
									L"\" from an instance of type \"" +
									reprTypeInfo.typeName.ToString() +
									L"\"."));
							}
							setTarget->Accept(this);
						}
						else if (setter->binding == GlobalStringKey::Empty)
						{
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								auto info = resolvingResult.propertyResolvings[value.Obj()];
								if (info.info->scope == GuiInstancePropertyInfo::Property)
								{
									if (info.info->support == GuiInstancePropertyInfo::SupportCollection)
									{
										if (!usedProps.Contains(prop, info.loader))
										{
											usedProps.Add(prop, info.loader);
										}

										vint errorCount = errors.Count();
										IGuiInstanceLoader::ArgumentMap arguments;
										arguments.Add(prop, GetArgumentInfo(value.Obj()));
										if (auto stat = info.loader->AssignParameters(resolvingResult, reprTypeInfo, repr->instanceName, arguments, setter->attPosition, errors))
										{
											statements->statements.Add(stat);
										}
										else if (errorCount == errors.Count())
										{
											errors.Add(GuiResourceError(resolvingResult.resource, value->tagPosition,
												L"Precompile: Something is wrong when assigning to property " +
												prop.ToString() +
												L" to an instance of type \"" +
												reprTypeInfo.typeName.ToString() +
												L"\"."));
										}
									}
									else if (!usedProps.Contains(prop, info.loader))
									{
										List<GlobalStringKey> pairedProps;
										info.loader->GetPairedProperties(propInfo, pairedProps);
										if (pairedProps.Count() == 0)
										{
											pairedProps.Add(prop);
										}

										IGuiInstanceLoader::ArgumentMap arguments;
										FOREACH(GlobalStringKey, pairedProp, pairedProps)
										{
											usedProps.Add(pairedProp, info.loader);
											auto pairedSetter = repr->setters[pairedProp];
											FOREACH(Ptr<GuiValueRepr>, pairedValue, pairedSetter->values)
											{
												auto pairedInfo = resolvingResult.propertyResolvings[pairedValue.Obj()];
												if (pairedInfo.loader == info.loader)
												{
													arguments.Add(pairedProp, GetArgumentInfo(pairedValue.Obj()));
												}
											}
										}

										vint errorCount = errors.Count();
										if (auto stat = info.loader->AssignParameters(resolvingResult, reprTypeInfo, repr->instanceName, arguments, setter->attPosition, errors))
										{
											statements->statements.Add(stat);
										}
										else if (errorCount == errors.Count())
										{
											WString propNames;
											FOREACH_INDEXER(GlobalStringKey, pairedProp, propIndex, pairedProps)
											{
												if (propIndex > 0)propNames += L", ";
												propNames += L"\"" + pairedProp.ToString() + L"\"";
											}
											errors.Add(GuiResourceError(resolvingResult.resource, value->tagPosition,
												L"Precompile: Something is wrong when assigning to properties " +
												propNames +
												L" to an instance of type \"" +
												reprTypeInfo.typeName.ToString() +
												L"\"."));
										}
									}
								}
							}
						}
					}

					WORKFLOW_ENVIRONMENT_VARIABLE_REMOVE
				}
			}

			void FillCtorArguments(GuiConstructorRepr* repr, IGuiInstanceLoader* loader, const IGuiInstanceLoader::TypeInfo& typeInfo, IGuiInstanceLoader::ArgumentMap& arguments)
			{
				List<GlobalStringKey> ctorProps;
				loader->GetConstructorParameters(typeInfo, ctorProps);

				WORKFLOW_ENVIRONMENT_VARIABLE_ADD

				FOREACH(GlobalStringKey, prop, ctorProps)
				{
					auto index = repr->setters.Keys().IndexOf(prop);
					if (index != -1)
					{
						auto setter = repr->setters.Values()[index];
						if (setter->binding == GlobalStringKey::Empty)
						{
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								auto argument = GetArgumentInfo(value.Obj());
								if (argument.type && argument.expression)
								{
									arguments.Add(prop, argument);
								}
							}
						}
						else if (auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding))
						{
							auto propInfo = IGuiInstanceLoader::PropertyInfo(typeInfo, prop);
							auto resolvedPropInfo = loader->GetPropertyType(propInfo);
							auto value = setter->values[0].Cast<GuiTextRepr>();
							if (auto expression = binder->GenerateConstructorArgument(resolvingResult, loader, propInfo, resolvedPropInfo, value->text, value->tagPosition, errors))
							{
								IGuiInstanceLoader::ArgumentInfo argument;
								argument.expression = expression;
								argument.type = resolvedPropInfo->acceptableTypes[0];
								arguments.Add(prop, argument);
							}
						}
						else
						{
							errors.Add(GuiResourceError(resolvingResult.resource, setter->attPosition,
								L"Precompile: The appropriate IGuiInstanceBinder of binding \"-" +
								setter->binding.ToString() +
								L"\" cannot be found."));
						}
					}
				}

				WORKFLOW_ENVIRONMENT_VARIABLE_REMOVE
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				IGuiInstanceLoader::TypeInfo ctorTypeInfo;
				if (resolvingResult.context->instance.Obj() == repr)
				{
					auto source = FindInstanceLoadingSource(resolvingResult.context, repr);
					ctorTypeInfo.typeName = source.typeName;
					ctorTypeInfo.typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				}
				else
				{
					ctorTypeInfo = resolvingResult.typeInfos[repr->instanceName];
				}

				auto ctorLoader = GetInstanceLoaderManager()->GetLoader(ctorTypeInfo.typeName);
				while (ctorLoader)
				{
					if (ctorLoader->CanCreate(ctorTypeInfo))
					{
						break;
					}
					ctorLoader = GetInstanceLoaderManager()->GetParentLoader(ctorLoader);
				}

				if (resolvingResult.context->instance.Obj() == repr)
				{
					resolvingResult.rootLoader = ctorLoader;
					resolvingResult.rootTypeInfo = ctorTypeInfo;
					FillCtorArguments(repr, ctorLoader, ctorTypeInfo, resolvingResult.rootCtorArguments);

					{
						auto refInstance = MakePtr<WfReferenceExpression>();
						refInstance->name.value = repr->instanceName.ToString();

						auto refThis = MakePtr<WfReferenceExpression>();
						refThis->name.value = L"<this>";

						auto assign = MakePtr<WfBinaryExpression>();
						assign->op = WfBinaryOperator::Assign;
						assign->first = refInstance;
						assign->second = refThis;

						auto stat = MakePtr<WfExpressionStatement>();
						stat->expression = assign;

						statements->statements.Add(stat);
					}

					if (resolvingResult.rootCtorArguments.Count() > 0)
					{
						if (auto stat = ctorLoader->InitializeRootInstance(resolvingResult, ctorTypeInfo, repr->instanceName, resolvingResult.rootCtorArguments, errors))
						{
							statements->statements.Add(stat);
						}
					}

					FOREACH(Ptr<GuiInstanceParameter>, parameter, resolvingResult.context->parameters)
					{
						auto refInstance = MakePtr<WfReferenceExpression>();
						refInstance->name.value = parameter->name.ToString();

						auto refThis = MakePtr<WfReferenceExpression>();
						refThis->name.value = L"<this>";

						auto refParameter = MakePtr<WfMemberExpression>();
						refParameter->parent = refThis;
						refParameter->name.value = parameter->name.ToString();

						auto assign = MakePtr<WfBinaryExpression>();
						assign->op = WfBinaryOperator::Assign;
						assign->first = refInstance;
						assign->second = refParameter;

						auto stat = MakePtr<WfExpressionStatement>();
						stat->expression = assign;

						statements->statements.Add(stat);
					}
				}
				else
				{
					IGuiInstanceLoader::ArgumentMap arguments;
					FillCtorArguments(repr, ctorLoader, ctorTypeInfo, arguments);

					vint errorCount = errors.Count();
					if (auto ctorStats = ctorLoader->CreateInstance(resolvingResult, ctorTypeInfo, repr->instanceName, arguments, repr->tagPosition, errors))
					{
						statements->statements.Add(ctorStats);
					}
					else if (errorCount == errors.Count())
					{
						errors.Add(GuiResourceError(resolvingResult.resource, repr->tagPosition,
							L"Precompile: Something is wrong when creating an instance of type \"" +
							ctorTypeInfo.typeName.ToString() +
							L"\"."));
					}
				}
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		void Workflow_GenerateCreating(types::ResolvingResult& resolvingResult, Ptr<WfBlockStatement> statements, GuiResourceError::List& errors)
		{
			WorkflowGenerateCreatingVisitor visitor(resolvingResult, statements, errors);
			resolvingResult.context->instance->Accept(&visitor);
		}
	}
}
