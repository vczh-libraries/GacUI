#include "GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;
		using namespace collections;

		using namespace controls;
		using namespace compositions;

/***********************************************************************
Workflow_InstallBindProperty
***********************************************************************/

		Ptr<workflow::WfExpression> Workflow_GetUriProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& protocol, const WString& path, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			Ptr<WfExpression> resourceExpr;
			{
				auto refResolver = Ptr(new WfReferenceExpression);
				refResolver->name.value = L"<this>";

				auto member = Ptr(new WfMemberExpression);
				member->parent = refResolver;
				member->name.value = L"ResolveResource";

				auto valueProtocol = Ptr(new WfStringExpression);
				valueProtocol->value.value = protocol;

				auto valuePath = Ptr(new WfStringExpression);
				valuePath->value.value = path;

				auto valueBool = Ptr(new WfLiteralExpression);
				valueBool->value = WfLiteralValue::True;

				auto call = Ptr(new WfCallExpression);
				call->function = member;
				call->arguments.Add(valueProtocol);
				call->arguments.Add(valuePath);
				call->arguments.Add(valueBool);

				resourceExpr = call;
			}

			auto td = propInfo->acceptableTypes[0]->GetTypeDescriptor();
			Ptr<ITypeInfo> convertedType;
			{
				if (td->GetSerializableType())
				{
					convertedType = TypeInfoRetriver<Ptr<GuiTextData>>::CreateTypeInfo();
				}
				else if (td == description::GetTypeDescriptor<INativeImage>() || td == description::GetTypeDescriptor<GuiImageData>())
				{
					convertedType = TypeInfoRetriver<Ptr<GuiImageData>>::CreateTypeInfo();
				}
				else
				{
					auto elementType = Ptr(new TypeDescriptorTypeInfo(td, TypeInfoHint::Normal));
					auto pointerType = Ptr(new SharedPtrTypeInfo(elementType));
					convertedType = pointerType;
				}
			}

			Ptr<WfExpression> convertedExpr;
			{
				auto cast = Ptr(new WfTypeCastingExpression);
				cast->expression = resourceExpr;
				cast->type = GetTypeFromTypeInfo(convertedType.Obj());
				cast->strategy = WfTypeCastingStrategy::Strong;

				convertedExpr = cast;
			}

			Ptr<WfExpression> evalExpr;
			{
				if (td->GetSerializableType())
				{
					auto member = Ptr(new WfMemberExpression);
					member->parent = convertedExpr;
					member->name.value = L"Text";

					auto elementType = Ptr(new TypeDescriptorTypeInfo(td, TypeInfoHint::Normal));

					auto cast = Ptr(new WfTypeCastingExpression);
					cast->expression = member;
					cast->type = GetTypeFromTypeInfo(elementType.Obj());
					cast->strategy = WfTypeCastingStrategy::Strong;

					evalExpr = cast;
				}
				else if (td == description::GetTypeDescriptor<INativeImage>())
				{
					auto member = Ptr(new WfMemberExpression);
					member->parent = convertedExpr;
					member->name.value = L"Image";

					evalExpr = member;
				}
				else
				{
					evalExpr = convertedExpr;
				}
			}

			return evalExpr;
		}

		Ptr<workflow::WfStatement> Workflow_InstallUriProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& protocol, const WString& path, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			auto evalExpr = Workflow_GetUriProperty(precompileContext, resolvingResult, loader, prop, propInfo, protocol, path, attPosition, errors);

			IGuiInstanceLoader::ArgumentMap arguments;
			{
				IGuiInstanceLoader::ArgumentInfo argumentInfo;
				argumentInfo.typeInfo = propInfo->acceptableTypes[0];
				argumentInfo.expression = evalExpr;
				argumentInfo.attPosition = attPosition;
				arguments.Add(prop.propertyName, argumentInfo);
			}

			return loader->AssignParameters(precompileContext, resolvingResult, prop.typeInfo, variableName, arguments, attPosition, errors);
		}

/***********************************************************************
Workflow_InstallBindProperty
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallBindProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> bindExpression)
		{
			auto subBlock = Ptr(new WfBlockStatement);
			{
				auto var = Ptr(new WfVariableDeclaration);
				var->name.value = L"<created-subscription>";
				var->expression = bindExpression;

				auto stat = Ptr(new WfVariableStatement);
				stat->variable = var;
				subBlock->statements.Add(stat);
			}
			{
				auto callback = Ptr(new WfFunctionDeclaration);
				callback->functionKind = WfFunctionKind::Normal;
				callback->anonymity = WfFunctionAnonymity::Anonymous;
				callback->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());;
				{
					auto arg = Ptr(new WfFunctionArgument);
					arg->name.value = L"<value>";
					arg->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
					callback->arguments.Add(arg);
				}
				auto callbackBlock = Ptr(new WfBlockStatement);
				callback->statement = callbackBlock;
				{
					auto refSubscribee = Ptr(new WfReferenceExpression);
					refSubscribee->name.value = variableName.ToString();

					auto member = Ptr(new WfMemberExpression);
					member->parent = refSubscribee;
					member->name.value = propertyInfo->GetName();

					auto var = Ptr(new WfVariableDeclaration);
					var->name.value = L"<old>";
					var->expression = member;

					auto stat = Ptr(new WfVariableStatement);
					stat->variable = var;
					callbackBlock->statements.Add(stat);
				}
				{
					ITypeInfo* propertyType = propertyInfo->GetReturn();
					if (propertyInfo->GetSetter() && propertyInfo->GetSetter()->GetParameterCount() == 1)
					{
						propertyType = propertyInfo->GetSetter()->GetParameter(0)->GetType();
					}

					auto refValue = Ptr(new WfReferenceExpression);
					refValue->name.value = L"<value>";

					auto cast = Ptr(new WfTypeCastingExpression);
					cast->strategy = WfTypeCastingStrategy::Strong;
					cast->expression = refValue;
					cast->type = GetTypeFromTypeInfo(propertyType);

					auto var = Ptr(new WfVariableDeclaration);
					var->name.value = L"<new>";
					var->expression = cast;

					auto stat = Ptr(new WfVariableStatement);
					stat->variable = var;
					callbackBlock->statements.Add(stat);
				}
				{
					auto refOld = Ptr(new WfReferenceExpression);
					refOld->name.value = L"<old>";

					auto refNew = Ptr(new WfReferenceExpression);
					refNew->name.value = L"<new>";

					auto compare = Ptr(new WfBinaryExpression);
					compare->op = WfBinaryOperator::EQ;
					compare->first = refOld;
					compare->second = refNew;

					auto ifStat = Ptr(new WfIfStatement);
					ifStat->expression = compare;
					callbackBlock->statements.Add(ifStat);

					auto ifBlock = Ptr(new WfBlockStatement);
					ifStat->trueBranch = ifBlock;

					auto returnStat = Ptr(new WfReturnStatement);
					ifBlock->statements.Add(returnStat);
				}
				{
					auto refSubscribee = Ptr(new WfReferenceExpression);
					refSubscribee->name.value = variableName.ToString();

					auto member = Ptr(new WfMemberExpression);
					member->parent = refSubscribee;
					member->name.value = propertyInfo->GetName();

					auto refNew = Ptr(new WfReferenceExpression);
					refNew->name.value = L"<new>";

					auto assign = Ptr(new WfBinaryExpression);
					assign->op = WfBinaryOperator::Assign;
					assign->first = member;
					assign->second = refNew;

					auto stat = Ptr(new WfExpressionStatement);
					stat->expression = assign;
					callbackBlock->statements.Add(stat);
				}

				auto funcExpr = Ptr(new WfFunctionExpression);
				funcExpr->function = callback;

				auto refBind = Ptr(new WfReferenceExpression);
				refBind->name.value = L"<created-subscription>";

				auto refEvent = Ptr(new WfMemberExpression);
				refEvent->parent = refBind;
				refEvent->name.value = L"ValueChanged";

				auto attachExpr = Ptr(new WfAttachEventExpression);
				attachExpr->event = refEvent;
				attachExpr->function = funcExpr;

				auto stat = Ptr(new WfExpressionStatement);
				stat->expression = attachExpr;
				subBlock->statements.Add(stat);
			}
			{
				auto refThis = Ptr(new WfReferenceExpression);
				refThis->name.value = L"<this>";

				auto member = Ptr(new WfMemberExpression);
				member->parent = refThis;
				member->name.value = L"AddSubscription";

				auto refBind = Ptr(new WfReferenceExpression);
				refBind->name.value = L"<created-subscription>";

				auto call = Ptr(new WfCallExpression);
				call->function = member;
				call->arguments.Add(refBind);

				auto stat = Ptr(new WfExpressionStatement);
				stat->expression = call;
				subBlock->statements.Add(stat);
			}

			return subBlock;
		}

/***********************************************************************
Workflow_InstallEvalProperty
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallEvalProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, Ptr<workflow::WfExpression> evalExpression, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			IGuiInstanceLoader::ArgumentMap arguments;
			{
				IGuiInstanceLoader::ArgumentInfo argumentInfo;
				argumentInfo.typeInfo = propInfo->acceptableTypes[0];
				argumentInfo.expression = evalExpression;
				argumentInfo.attPosition = attPosition;
				arguments.Add(prop.propertyName, argumentInfo);
			}

			return loader->AssignParameters(precompileContext, resolvingResult, prop.typeInfo, variableName, arguments, attPosition, errors);
		}

/***********************************************************************
Workflow_InstallEvent
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallEvent(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& handlerName)
		{
			vint count = eventInfo->GetHandlerType()->GetElementType()->GetGenericArgumentCount() - 1;

			auto subBlock = Ptr(new WfBlockStatement);
			{
				auto var = Ptr(new WfReferenceExpression);
				var->name.value = variableName.ToString();

				auto member = Ptr(new WfMemberExpression);
				member->parent = var;
				member->name.value = eventInfo->GetName();

				auto refThis = Ptr(new WfReferenceExpression);
				refThis->name.value = L"<this>";

				auto handler = Ptr(new WfMemberExpression);
				handler->parent = refThis;
				handler->name.value = handlerName;

				auto call = Ptr(new WfCallExpression);
				call->function = handler;
				for (vint i = 0; i < count; i++)
				{
					auto argument = Ptr(new WfOrderedNameExpression);
					argument->name.value = L"$" + itow(i + 1);
					call->arguments.Add(argument);
				}

				auto eventHandler = Ptr(new WfOrderedLambdaExpression);
				eventHandler->body = call;

				auto attachEvent = Ptr(new WfAttachEventExpression);
				attachEvent->event = member;
				attachEvent->function = eventHandler;

				auto stat = Ptr(new WfExpressionStatement);
				stat->expression = attachEvent;
				subBlock->statements.Add(stat);
			}

			return subBlock;
		}

/***********************************************************************
Workflow_GenerateEventHandler
***********************************************************************/

		Ptr<workflow::WfFunctionDeclaration> Workflow_GenerateEventHandler(GuiResourcePrecompileContext& precompileContext, description::IEventInfo* eventInfo)
		{
			auto func = Ptr(new WfFunctionDeclaration);
			func->functionKind = WfFunctionKind::Normal;
			func->anonymity = WfFunctionAnonymity::Anonymous;
			func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());

			vint count = eventInfo->GetHandlerType()->GetElementType()->GetGenericArgumentCount() - 1;
			bool standardName = false;
			if (count == 2)
			{
				auto senderType = eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(1)->GetTypeDescriptor();
				auto argumentType = eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(2)->GetTypeDescriptor();
				if (senderType == GetTypeDescriptor<GuiGraphicsComposition>())
				{
					auto expectedType = GetTypeDescriptor<GuiEventArgs>();
					List<ITypeDescriptor*> types;
					types.Add(argumentType);
					// TODO: (enumerable) foreach
					for (vint i = 0; i < types.Count(); i++)
					{
						auto type = types[i];
						if (type == expectedType)
						{
							standardName = true;
							break;
						}
						vint baseCount = type->GetBaseTypeDescriptorCount();
						for (vint j = 0; j < baseCount; j++)
						{
							auto baseType = type->GetBaseTypeDescriptor(j);
							if (!types.Contains(baseType))
							{
								types.Add(baseType);
							}
						}
					}
				}
			}

			if (standardName)
			{
				{
					auto arg = Ptr(new WfFunctionArgument);
					arg->name.value = L"sender";
					arg->type = GetTypeFromTypeInfo(eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(1));
					func->arguments.Add(arg);
				}
				{
					auto arg = Ptr(new WfFunctionArgument);
					arg->name.value = L"arguments";
					arg->type = GetTypeFromTypeInfo(eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(2));
					func->arguments.Add(arg);
				}
			}
			else
			{
				for (vint i = 0; i < count; i++)
				{
					auto arg = Ptr(new WfFunctionArgument);
					arg->name.value = L"arg" + itow(i + 1);
					arg->type = GetTypeFromTypeInfo(eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(i + 1));
					func->arguments.Add(arg);
				}
			}

			return func;
		}

/***********************************************************************
Workflow_InstallEvalEvent
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallEvalEvent(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, Ptr<workflow::WfStatement> evalStatement)
		{
			auto func = Workflow_GenerateEventHandler(precompileContext, eventInfo);

			if (evalStatement.Cast<WfBlockStatement>())
			{
				func->statement = evalStatement;
			}
			else if (evalStatement.Cast<WfCoProviderStatement>())
			{
				func->statement = evalStatement;
			}
			else
			{
				auto funcBlock = Ptr(new WfBlockStatement);
				funcBlock->statements.Add(evalStatement);
				func->statement = funcBlock;
			}

			auto subBlock = Ptr(new WfBlockStatement);

			{
				auto eventHandlerLambda = Ptr(new WfFunctionExpression);
				eventHandlerLambda->function = func;

				auto eventHandler = Ptr(new WfVariableDeclaration);
				eventHandler->name.value = L"<event-handler>";
				eventHandler->expression = eventHandlerLambda;

				auto stat = Ptr(new WfVariableStatement);
				stat->variable = eventHandler;
				subBlock->statements.Add(stat);
			}
			{
				auto var = Ptr(new WfReferenceExpression);
				var->name.value = variableName.ToString();

				auto member = Ptr(new WfMemberExpression);
				member->parent = var;
				member->name.value = eventInfo->GetName();

				auto eventHandler = Ptr(new WfReferenceExpression);
				eventHandler->name.value = L"<event-handler>";

				auto attachEvent = Ptr(new WfAttachEventExpression);
				attachEvent->event = member;
				attachEvent->function = eventHandler;

				auto stat = Ptr(new WfExpressionStatement);
				stat->expression = attachEvent;
				subBlock->statements.Add(stat);
			}

			return subBlock;
		}
	}
}