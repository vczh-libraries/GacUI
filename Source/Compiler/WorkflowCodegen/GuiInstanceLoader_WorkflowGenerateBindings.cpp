#include "GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace collections;
		using namespace reflection::description;

/***********************************************************************
WorkflowGenerateBindingVisitor
***********************************************************************/

		class WorkflowGenerateBindingVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			GuiResourcePrecompileContext&		precompileContext;
			types::ResolvingResult&				resolvingResult;
			Ptr<WfBlockStatement>				statements;
			GuiResourceError::List&				errors;
			
			WorkflowGenerateBindingVisitor(GuiResourcePrecompileContext& _precompileContext, types::ResolvingResult& _resolvingResult, Ptr<WfBlockStatement> _statements, GuiResourceError::List& _errors)
				:precompileContext(_precompileContext)
				, resolvingResult(_resolvingResult)
				, errors(_errors)
				, statements(_statements)
			{
			}

			///////////////////////////////////////////////////////////////////////////////////

			Ptr<WfStatement> ProcessPropertyBinding(
				GuiAttSetterRepr* repr,
				IGuiInstanceLoader::TypeInfo reprTypeInfo,
				Ptr<GuiAttSetterRepr::SetterValue> setter,
				GlobalStringKey propertyName
				)
			{
				if (auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding))
				{
					auto propertyResolving = resolvingResult.propertyResolvings[setter->values[0].Obj()];
					if (propertyResolving.info->usage == GuiInstancePropertyInfo::Property)
					{
						WString expressionCode = setter->values[0].Cast<GuiTextRepr>()->text;
						auto instancePropertyInfo = reprTypeInfo.typeInfo->GetTypeDescriptor()->GetPropertyByName(propertyName.ToString(), true);

						if (instancePropertyInfo || !binder->RequirePropertyExist())
						{
							if (auto statement = binder->GenerateInstallStatement(
								precompileContext,
								resolvingResult,
								repr->instanceName,
								instancePropertyInfo,
								propertyResolving.loader,
								propertyResolving.propertyInfo,
								propertyResolving.info,
								expressionCode,
								setter->values[0]->tagPosition,
								errors))
							{
								return statement;
							}
						}
						else
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition,
								L"Precompile: Binder \"" +
								setter->binding.ToString() +
								L"\" requires property \"" +
								propertyName.ToString() +
								L"\" to physically appear in type \"" +
								reprTypeInfo.typeName.ToString() +
								L"\"."));
						}
					}
				}
				else
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition,
						L"[INTERNAL-ERROR] Precompile: The appropriate IGuiInstanceBinder of binding \"-" +
						setter->binding.ToString() +
						L"\" cannot be found."));
				}
				return nullptr;
			}

			///////////////////////////////////////////////////////////////////////////////////

			Ptr<WfStatement> ProcessEventBinding(
				GuiAttSetterRepr* repr,
				IGuiInstanceLoader::TypeInfo reprTypeInfo,
				Ptr<GuiAttSetterRepr::EventValue> handler,
				GlobalStringKey propertyName
				)
			{
				auto td = reprTypeInfo.typeInfo->GetTypeDescriptor();
				auto eventInfo = td->GetEventByName(propertyName.ToString(), true);

				if (!eventInfo)
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, handler->attPosition,
						L"[INTERNAL-ERROR] Precompile: Event \"" +
						propertyName.ToString() +
						L"\" cannot be found in type \"" +
						reprTypeInfo.typeName.ToString() +
						L"\"."));
				}
				else
				{
					if (handler->binding == GlobalStringKey::Empty)
					{
						return Workflow_InstallEvent(precompileContext, resolvingResult, repr->instanceName, eventInfo, handler->value);
					}
					else
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceEventBinder(handler->binding);
						if (binder)
						{
							return binder->GenerateInstallStatement(precompileContext, resolvingResult, repr->instanceName, eventInfo, handler->value, handler->valuePosition, errors);
						}
						else
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, handler->attPosition,
								L"[INTERNAL-ERROR] The appropriate IGuiInstanceEventBinder of binding \"-" +
								handler->binding.ToString() +
								L"\" cannot be found."));
						}
					}
				}
				return nullptr;
			}

			///////////////////////////////////////////////////////////////////////////////////

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
				
				if (reprTypeInfo.typeInfo && (reprTypeInfo.typeInfo->GetTypeDescriptor()->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined)
				{
					WORKFLOW_ENVIRONMENT_VARIABLE_ADD

					FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
					{
						auto propertyName = repr->setters.Keys()[index];
						if (setter->binding != GlobalStringKey::Empty && setter->binding != GlobalStringKey::_Set)
						{
							if (auto statement = ProcessPropertyBinding(repr, reprTypeInfo, setter, propertyName))
							{
								Workflow_RecordScriptPosition(precompileContext, setter->values[0]->tagPosition, statement);
								statements->statements.Add(statement);
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

					FOREACH_INDEXER(Ptr<GuiAttSetterRepr::EventValue>, handler, index, repr->eventHandlers.Values())
					{
						if (reprTypeInfo.typeInfo)
						{
							GlobalStringKey propertyName = repr->eventHandlers.Keys()[index];
							if (auto statement = ProcessEventBinding(repr, reprTypeInfo, handler, propertyName))
							{
								Workflow_RecordScriptPosition(precompileContext, handler->valuePosition, statement);
								statements->statements.Add(statement);
							}
						}
					}

					WORKFLOW_ENVIRONMENT_VARIABLE_REMOVE
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		void Workflow_GenerateBindings(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, Ptr<WfBlockStatement> statements, GuiResourceError::List& errors)
		{
			WorkflowGenerateBindingVisitor visitor(precompileContext, resolvingResult, statements, errors);
			resolvingResult.context->instance->Accept(&visitor);

			FOREACH(Ptr<GuiInstanceLocalized>, localized, resolvingResult.context->localizeds)
			{
				auto code = L"bind(" + localized->className.ToString() + L"::Get(presentation::controls::GuiApplication::GetApplication().Locale))";
				if (auto bindExpr = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, code, localized->tagPosition, errors))
				{
					auto instancePropertyInfo = resolvingResult.rootTypeInfo.typeInfo->GetTypeDescriptor()->GetPropertyByName(localized->name.ToString(), true);
					if (auto statement = Workflow_InstallBindProperty(precompileContext, resolvingResult, resolvingResult.context->instance->instanceName, instancePropertyInfo, bindExpr))
					{
						Workflow_RecordScriptPosition(precompileContext, localized->tagPosition, statement);
						statements->statements.Add(statement);
					}
				}
			}
		}
	}
}