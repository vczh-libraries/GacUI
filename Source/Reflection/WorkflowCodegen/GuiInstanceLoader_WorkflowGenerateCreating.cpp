#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;

/***********************************************************************
WorkflowGenerateCreatingVisitor
***********************************************************************/

		class WorkflowGenerateCreatingVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::ResolvingResult&				resolvingResult;
			description::ITypeDescriptor*		rootTypeDescriptor;
			Ptr<WfBlockStatement>				statements;
			types::ErrorList&					errors;
			
			WorkflowGenerateCreatingVisitor(Ptr<GuiInstanceContext> _context, types::ResolvingResult& _resolvingResult, description::ITypeDescriptor* _rootTypeDescriptor, Ptr<WfBlockStatement> _statements, types::ErrorList& _errors)
				:context(_context)
				, resolvingResult(_resolvingResult)
				, rootTypeDescriptor(_rootTypeDescriptor)
				, errors(_errors)
				, statements(_statements)
			{
			}

			IGuiInstanceLoader::ArgumentInfo GetArgumentInfo(GuiValueRepr* repr)
			{
				ITypeDescriptor* td = nullptr;
				bool serializable = false;
				WString textValue;
				GuiConstructorRepr* ctor = nullptr;

				if (auto text = dynamic_cast<GuiTextRepr*>(repr))
				{
					td = resolvingResult.propertyResolvings[repr].info->acceptableTypes[0];
					serializable = true;
					textValue = text->text;
				}
				else if (ctor = dynamic_cast<GuiConstructorRepr*>(repr))
				{
					if (ctor->instanceName == GlobalStringKey::Empty)
					{
						td = resolvingResult.propertyResolvings[repr].info->acceptableTypes[0];
					}
					else
					{
						td = resolvingResult.typeInfos[ctor->instanceName].typeDescriptor;
					}
					if (td->GetValueSerializer() != nullptr)
					{
						serializable = true;
						textValue = ctor->setters.Values()[0]->values[0].Cast<GuiTextRepr>()->text;
					}
				}

				IGuiInstanceLoader::ArgumentInfo argumentInfo;
				argumentInfo.type = td;

				if (serializable)
				{
					if (td == description::GetTypeDescriptor<WString>())
					{
						auto str = MakePtr<WfStringExpression>();
						str->value.value = textValue;
						argumentInfo.expression = str;
					}
					else
					{
						auto str = MakePtr<WfStringExpression>();
						str->value.value = textValue;

						auto type = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
						type->SetTypeDescriptor(td);

						auto cast = MakePtr<WfTypeCastingExpression>();
						cast->type = GetTypeFromTypeInfo(type.Obj());
						cast->strategy = WfTypeCastingStrategy::Strong;
						cast->expression = str;

						argumentInfo.expression = cast;
					}
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
				
				if (reprTypeInfo.typeDescriptor && reprTypeInfo.typeDescriptor->GetValueSerializer() == nullptr)
				{
					Group<GlobalStringKey, IGuiInstanceLoader*> usedProps;
					FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
					{
						auto prop = repr->setters.Keys()[index];
						IGuiInstanceLoader::PropertyInfo propInfo(reprTypeInfo, prop);
						if (setter->binding == GlobalStringKey::_Set)
						{
							auto setTarget = dynamic_cast<GuiAttSetterRepr*>(setter->values[0].Obj());
							auto info = resolvingResult.propertyResolvings[setTarget];
							vint errorCount = errors.Count();
							if (auto expr = info.loader->GetParameter(propInfo, repr->instanceName, errors))
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
								errors.Add(L"Precompile: Something is wrong when retriving the property \"" + prop.ToString() + L"\" from an instance of type \"" + reprTypeInfo.typeName.ToString() + L"\".");
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
										if (auto stat = info.loader->AssignParameters(reprTypeInfo, repr->instanceName, arguments, errors))
										{
											statements->statements.Add(stat);
										}
										else if (errorCount == errors.Count())
										{
											errors.Add(L"Precompile: Something is wrong when assigning to property " + prop.ToString() + L" to an instance of type \"" + reprTypeInfo.typeName.ToString() + L"\".");
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
										if (auto stat = info.loader->AssignParameters(reprTypeInfo, repr->instanceName, arguments, errors))
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
											errors.Add(L"Precompile: Something is wrong when assigning to properties " + propNames + L" to an instance of type \"" + reprTypeInfo.typeName.ToString() + L"\".");
										}
									}
								}
							}
						}
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				if (context->instance.Obj() == repr)
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
				else
				{
					auto typeInfo = resolvingResult.typeInfos[repr->instanceName];
					auto loader = GetInstanceLoaderManager()->GetLoader(typeInfo.typeName);
					while (loader)
					{
						if (loader->CanCreate(typeInfo))
						{
							break;
						}
						loader = GetInstanceLoaderManager()->GetParentLoader(loader);
					}

					List<GlobalStringKey> ctorProps;
					loader->GetConstructorParameters(typeInfo, ctorProps);

					IGuiInstanceLoader::ArgumentMap arguments;
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
							else
							{
								errors.Add(L"Precompile: <BINDING-ON-CTOR-PROP-NOT-SUPPORTED-YET>");
							}
						}
					}

					vint errorCount = errors.Count();
					if (auto ctorStats = loader->CreateInstance(typeInfo, repr->instanceName, arguments, errors))
					{
						statements->statements.Add(ctorStats);
					}
					else if (errorCount == errors.Count())
					{
						errors.Add(L"Precompile: Something is wrong when creating an isntance of type \"" + typeInfo.typeName.ToString() + L"\".");
					}
				}
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		void Workflow_GenerateCreating(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, Ptr<WfBlockStatement> statements, types::ErrorList& errors)
		{
			WorkflowGenerateCreatingVisitor visitor(context, resolvingResult, rootTypeDescriptor, statements, errors);
			context->instance->Accept(&visitor);
		}
	}
}