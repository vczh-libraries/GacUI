#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace collections;

/***********************************************************************
WorkflowGenerateBindingVisitor
***********************************************************************/

		class WorkflowGenerateBindingVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::ResolvingResult&				resolvingResult;
			description::ITypeDescriptor*		rootTypeDescriptor;
			Ptr<WfBlockStatement>				statements;
			types::ErrorList&					errors;
			
			WorkflowGenerateBindingVisitor(Ptr<GuiInstanceContext> _context, types::ResolvingResult& _resolvingResult, description::ITypeDescriptor* _rootTypeDescriptor, Ptr<WfBlockStatement> _statements, types::ErrorList& _errors)
				:context(_context)
				, resolvingResult(_resolvingResult)
				, rootTypeDescriptor(_rootTypeDescriptor)
				, errors(_errors)
				, statements(_statements)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				IGuiInstanceLoader::TypeInfo reprTypeInfo;
				if (repr->instanceName != GlobalStringKey::Empty)
				{
					reprTypeInfo = resolvingResult.typeInfos[repr->instanceName];
				}
				
				if (reprTypeInfo.typeDescriptor && reprTypeInfo.typeDescriptor->GetValueSerializer() == nullptr)
				{
					FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
					{
						auto propertyName = repr->setters.Keys()[index];
						if (setter->binding != GlobalStringKey::Empty && setter->binding != GlobalStringKey::_Set)
						{
							if (auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding))
							{
								auto propertyResolving = resolvingResult.propertyResolvings[setter->values[0].Obj()];
								if (propertyResolving.info->scope != GuiInstancePropertyInfo::Constructor)
								{
									WString expressionCode = setter->values[0].Cast<GuiTextRepr>()->text;
									auto instancePropertyInfo = reprTypeInfo.typeDescriptor->GetPropertyByName(propertyName.ToString(), true);

									if (instancePropertyInfo || !binder->RequirePropertyExist())
									{
										if (auto statement = binder->GenerateInstallStatement(repr->instanceName, instancePropertyInfo, propertyResolving.loader, propertyResolving.propertyInfo, propertyResolving.info, expressionCode, errors))
										{
											if (Workflow_ValidateStatement(context, resolvingResult, rootTypeDescriptor, errors, expressionCode, statement))
											{
												statements->statements.Add(statement);	
											}
										}
									}
									else
									{
										errors.Add(L"Precompile: Binder \"" + setter->binding.ToString() + L"\" requires property \"" + propertyName.ToString() + L"\" to physically appear in type \"" + reprTypeInfo.typeName.ToString() + L"\".");
									}
								}
							}
							else
							{
								errors.Add(L"Precompile: The appropriate IGuiInstanceBinder of binding \"-" + setter->binding.ToString() + L"\" cannot be found.");
							}
						}
						else
						{
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								value->Accept(this);
							}
						}
					}
				}

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::EventValue>, handler, index, repr->eventHandlers.Values())
				{
					if (reprTypeInfo.typeDescriptor)
					{
						GlobalStringKey propertyName = repr->eventHandlers.Keys()[index];
						auto td = reprTypeInfo.typeDescriptor;
						auto eventInfo = td->GetEventByName(propertyName.ToString(), true);

						if (!eventInfo)
						{
							errors.Add(L"Precompile: Event \"" + propertyName.ToString() + L"\" cannot be found in type \"" + reprTypeInfo.typeName.ToString() + L"\".");
						}
						else
						{
							Ptr<WfStatement> statement;

							if (handler->binding == GlobalStringKey::Empty)
							{
								statement = Workflow_InstallEvent(repr->instanceName, eventInfo, handler->value);
							}
							else
							{
								auto binder = GetInstanceLoaderManager()->GetInstanceEventBinder(handler->binding);
								if (binder)
								{
									statement = binder->GenerateInstallStatement(repr->instanceName, eventInfo, handler->value, errors);
								}
								else
								{
									errors.Add(L"The appropriate IGuiInstanceEventBinder of binding \"-" + handler->binding.ToString() + L"\" cannot be found.");
								}
							}

							if (statement)
							{
								if (Workflow_ValidateStatement(context, resolvingResult, rootTypeDescriptor, errors, handler->value, statement))
								{
									statements->statements.Add(statement);
								}
							}
						}
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		void Workflow_GenerateBindings(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, Ptr<WfBlockStatement> statements, types::ErrorList& errors)
		{
			WorkflowGenerateBindingVisitor visitor(context, resolvingResult, rootTypeDescriptor, statements, errors);
			context->instance->Accept(&visitor);
		}
	}
}