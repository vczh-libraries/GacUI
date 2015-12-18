#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

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

		Ptr<workflow::WfStatement> Workflow_InstallUriProperty(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, const WString& protocol, const WString& path)
		{
			auto subBlock = MakePtr<WfBlockStatement>();
			{
				auto refResolver = MakePtr<WfReferenceExpression>();
				refResolver->name.value = L"<resolver>";

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refResolver;
				member->name.value = L"ResolveResource";

				auto valueProtocol = MakePtr<WfStringExpression>();
				valueProtocol->value.value = protocol;

				auto valuePath = MakePtr<WfStringExpression>();
				valuePath->value.value = path;

				auto call = MakePtr<WfCallExpression>();
				call->function = member;
				call->arguments.Add(valueProtocol);
				call->arguments.Add(valuePath);

				auto varDesc = MakePtr<WfVariableDeclaration>();
				varDesc->name.value = L"<resource-item>";
				varDesc->expression = call;

				auto varStat = MakePtr<WfVariableStatement>();
				varStat->variable = varDesc;
				subBlock->statements.Add(varStat);
			}
			{
				auto refResourceItem = MakePtr<WfReferenceExpression>();
				refResourceItem->name.value = L"<resource-item>";

				auto isNull = MakePtr<WfTypeTestingExpression>();
				isNull->expression = refResourceItem;
				isNull->test = WfTypeTesting::IsNull;

				auto valueException = MakePtr<WfStringExpression>();
				valueException->value.value = L"Resource \"" + protocol + L"://" + path + L"\" does not exist.";

				auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
				raiseStat->expression = valueException;
				
				auto ifBlock = MakePtr<WfBlockStatement>();
				ifBlock->statements.Add(raiseStat);

				auto ifStat = MakePtr<WfIfStatement>();
				ifStat->expression = isNull;
				ifStat->trueBranch = ifBlock;
				subBlock->statements.Add(ifStat);
			}

			auto td = propertyInfo->GetReturn()->GetTypeDescriptor();
			Ptr<ITypeInfo> convertedType;
			if (td->GetValueSerializer())
			{
				convertedType = TypeInfoRetriver<Ptr<GuiTextData>>::CreateTypeInfo();
			}
			else if (td == description::GetTypeDescriptor<INativeImage>() || td == description::GetTypeDescriptor<GuiImageData>())
			{
				convertedType = TypeInfoRetriver<Ptr<GuiImageData>>::CreateTypeInfo();
			}
			else
			{
				convertedType = CopyTypeInfo(propertyInfo->GetReturn());
			}

			{
				auto refResourceItem = MakePtr<WfReferenceExpression>();
				refResourceItem->name.value = L"<resource-item>";

				auto cast = MakePtr<WfTypeCastingExpression>();
				cast->expression = refResourceItem;
				cast->type = GetTypeFromTypeInfo(convertedType.Obj());
				cast->strategy = WfTypeCastingStrategy::Weak;

				auto varDesc = MakePtr<WfVariableDeclaration>();
				varDesc->name.value = L"<resource-value>";
				varDesc->expression = cast;

				auto varStat = MakePtr<WfVariableStatement>();
				varStat->variable = varDesc;
				subBlock->statements.Add(varStat);
			}
			{
				auto refResourceValue = MakePtr<WfReferenceExpression>();
				refResourceValue->name.value = L"<resource-value>";

				auto isNull = MakePtr<WfTypeTestingExpression>();
				isNull->expression = refResourceValue;
				isNull->test = WfTypeTesting::IsNull;

				auto valueException = MakePtr<WfStringExpression>();
				valueException->value.value = L"Resource \"" + protocol + L"://" + path + L"\" cannot be read as type \"" + convertedType->GetTypeDescriptor()->GetTypeName() + L"\".";

				auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
				raiseStat->expression = valueException;

				auto ifBlock = MakePtr<WfBlockStatement>();
				ifBlock->statements.Add(raiseStat);

				auto ifStat = MakePtr<WfIfStatement>();
				ifStat->expression = isNull;
				ifStat->trueBranch = ifBlock;
				subBlock->statements.Add(ifStat);
			}

			Ptr<WfExpression> evalExpression;
			if (td->GetValueSerializer())
			{
				auto refResourceValue = MakePtr<WfReferenceExpression>();
				refResourceValue->name.value = L"<resource-value>";

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refResourceValue;
				member->name.value = L"Text";

				auto cast = MakePtr<WfTypeCastingExpression>();
				cast->expression = member;
				cast->type = GetTypeFromTypeInfo(propertyInfo->GetReturn());
				cast->strategy = WfTypeCastingStrategy::Strong;

				evalExpression = cast;
			}
			else if (td == description::GetTypeDescriptor<INativeImage>())
			{
				auto refResourceValue = MakePtr<WfReferenceExpression>();
				refResourceValue->name.value = L"<resource-value>";

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refResourceValue;
				member->name.value = L"Image";

				evalExpression = member;
			}
			else
			{
				auto refResourceValue = MakePtr<WfReferenceExpression>();
				refResourceValue->name.value = L"<resource-value>";

				evalExpression = refResourceValue;
			}

			{
				auto refSubscribee = MakePtr<WfReferenceExpression>();
				refSubscribee->name.value = variableName.ToString();

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refSubscribee;
				member->name.value = propertyInfo->GetName();

				auto assign = MakePtr<WfBinaryExpression>();
				assign->op = WfBinaryOperator::Assign;
				assign->first = member;
				assign->second = evalExpression;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = assign;
				subBlock->statements.Add(stat);
			}
			return subBlock;
		}

/***********************************************************************
Workflow_InstallBindProperty
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallBindProperty(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> bindExpression)
		{
			auto subBlock = MakePtr<WfBlockStatement>();
			{
				auto refThis = MakePtr<WfReferenceExpression>();
				refThis->name.value = L"<this>";

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refThis;
				member->name.value = L"AddSubscription";

				auto call = MakePtr<WfCallExpression>();
				call->function = member;
				call->arguments.Add(bindExpression);

				auto var = MakePtr<WfVariableDeclaration>();
				var->name.value = L"<subscription>";
				var->expression = call;

				auto stat = MakePtr<WfVariableStatement>();
				stat->variable = var;
				subBlock->statements.Add(stat);
			}
			{
				auto callback = MakePtr<WfFunctionDeclaration>();
				callback->anonymity = WfFunctionAnonymity::Anonymous;
				callback->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());;
				{
					auto arg = MakePtr<WfFunctionArgument>();
					arg->name.value = L"<value>";
					arg->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
					callback->arguments.Add(arg);
				}
				auto callbackBlock = MakePtr<WfBlockStatement>();
				callback->statement = callbackBlock;
				{
					auto refSubscribee = MakePtr<WfReferenceExpression>();
					refSubscribee->name.value = variableName.ToString();

					auto member = MakePtr<WfMemberExpression>();
					member->parent = refSubscribee;
					member->name.value = propertyInfo->GetName();

					auto var = MakePtr<WfVariableDeclaration>();
					var->name.value = L"<old>";
					var->expression = member;

					auto stat = MakePtr<WfVariableStatement>();
					stat->variable = var;
					callbackBlock->statements.Add(stat);
				}
				{
					ITypeInfo* propertyType = propertyInfo->GetReturn();
					if (propertyInfo->GetSetter() && propertyInfo->GetSetter()->GetParameterCount() == 1)
					{
						propertyType = propertyInfo->GetSetter()->GetParameter(0)->GetType();
					}

					auto refValue = MakePtr<WfReferenceExpression>();
					refValue->name.value = L"<value>";

					auto cast = MakePtr<WfTypeCastingExpression>();
					cast->strategy = WfTypeCastingStrategy::Strong;
					cast->expression = refValue;
					cast->type = GetTypeFromTypeInfo(propertyType);

					auto var = MakePtr<WfVariableDeclaration>();
					var->name.value = L"<new>";
					var->expression = cast;

					auto stat = MakePtr<WfVariableStatement>();
					stat->variable = var;
					callbackBlock->statements.Add(stat);
				}
				{
					auto refOld = MakePtr<WfReferenceExpression>();
					refOld->name.value = L"<old>";

					auto refNew = MakePtr<WfReferenceExpression>();
					refNew->name.value = L"<new>";

					auto compare = MakePtr<WfBinaryExpression>();
					compare->op = WfBinaryOperator::EQ;
					compare->first = refOld;
					compare->second = refNew;

					auto ifStat = MakePtr<WfIfStatement>();
					ifStat->expression = compare;
					callbackBlock->statements.Add(ifStat);

					auto ifBlock = MakePtr<WfBlockStatement>();
					ifStat->trueBranch = ifBlock;

					auto returnStat = MakePtr<WfReturnStatement>();
					ifBlock->statements.Add(returnStat);
				}
				{
					auto refSubscribee = MakePtr<WfReferenceExpression>();
					refSubscribee->name.value = variableName.ToString();

					auto member = MakePtr<WfMemberExpression>();
					member->parent = refSubscribee;
					member->name.value = propertyInfo->GetName();

					auto refNew = MakePtr<WfReferenceExpression>();
					refNew->name.value = L"<new>";

					auto assign = MakePtr<WfBinaryExpression>();
					assign->op = WfBinaryOperator::Assign;
					assign->first = member;
					assign->second = refNew;

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = assign;
					callbackBlock->statements.Add(stat);
				}

				auto funcExpr = MakePtr<WfFunctionExpression>();
				funcExpr->function = callback;

				auto refThis = MakePtr<WfReferenceExpression>();
				refThis->name.value = L"<subscription>";

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refThis;
				member->name.value = L"Subscribe";

				auto call = MakePtr<WfCallExpression>();
				call->function = member;
				call->arguments.Add(funcExpr);

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = call;
				subBlock->statements.Add(stat);
			}
			{
				auto refThis = MakePtr<WfReferenceExpression>();
				refThis->name.value = L"<subscription>";

				auto member = MakePtr<WfMemberExpression>();
				member->parent = refThis;
				member->name.value = L"Update";

				auto call = MakePtr<WfCallExpression>();
				call->function = member;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = call;
				subBlock->statements.Add(stat);
			}

			return subBlock;
		}

/***********************************************************************
Workflow_InstallEvalProperty
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallEvalProperty(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> evalExpression)
		{
			auto refSubscribee = MakePtr<WfReferenceExpression>();
			refSubscribee->name.value = variableName.ToString();

			auto member = MakePtr<WfMemberExpression>();
			member->parent = refSubscribee;
			member->name.value = propertyInfo->GetName();

			auto assign = MakePtr<WfBinaryExpression>();
			assign->op = WfBinaryOperator::Assign;
			assign->first = member;
			assign->second = evalExpression;

			auto stat = MakePtr<WfExpressionStatement>();
			stat->expression = assign;
			
			return stat;
		}

/***********************************************************************
Workflow_InstallEvent
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallEvent(GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& handlerName)
		{
			vint count = eventInfo->GetHandlerType()->GetElementType()->GetGenericArgumentCount() - 1;

			auto subBlock = MakePtr<WfBlockStatement>();
			{
				auto var = MakePtr<WfReferenceExpression>();
				var->name.value = variableName.ToString();

				auto member = MakePtr<WfMemberExpression>();
				member->parent = var;
				member->name.value = eventInfo->GetName();

				auto refThis = MakePtr<WfReferenceExpression>();
				refThis->name.value = L"<this>";

				auto handler = MakePtr<WfMemberExpression>();
				handler->parent = refThis;
				handler->name.value = handlerName;

				auto call = MakePtr<WfCallExpression>();
				call->function = handler;
				for (vint i = 0; i < count; i++)
				{
					auto argument = MakePtr<WfOrderedNameExpression>();
					argument->name.value = L"$" + itow(i + 1);
					call->arguments.Add(argument);
				}

				auto eventHandler = MakePtr<WfOrderedLambdaExpression>();
				eventHandler->body = call;

				auto attachEvent = MakePtr<WfAttachEventExpression>();
				attachEvent->event = member;
				attachEvent->function = eventHandler;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = attachEvent;
				subBlock->statements.Add(stat);
			}

			return subBlock;
		}

/***********************************************************************
Workflow_InstallEvalEvent
***********************************************************************/

		Ptr<workflow::WfStatement> Workflow_InstallEvalEvent(GlobalStringKey variableName, description::IEventInfo* eventInfo, Ptr<workflow::WfStatement> evalStatement)
		{
			auto func = MakePtr<WfFunctionDeclaration>();
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
					auto arg = MakePtr<WfFunctionArgument>();
					arg->name.value = L"sender";
					arg->type = GetTypeFromTypeInfo(eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(1));
					func->arguments.Add(arg);
				}
				{
					auto arg = MakePtr<WfFunctionArgument>();
					arg->name.value = L"arguments";
					arg->type = GetTypeFromTypeInfo(eventInfo->GetHandlerType()->GetElementType()->GetGenericArgument(2));
					func->arguments.Add(arg);
				}
			}
			else
			{
				auto type = TypeInfoRetriver<Value>::CreateTypeInfo();
				for (vint i = 0; i < count; i++)
				{
					auto arg = MakePtr<WfFunctionArgument>();
					arg->name.value = L"<argument>" + itow(i + 1);
					arg->type = GetTypeFromTypeInfo(type.Obj());
					func->arguments.Add(arg);
				}
			}
						
			auto funcBlock = MakePtr<WfBlockStatement>();
			funcBlock->statements.Add(evalStatement);
			func->statement = funcBlock;

			auto subBlock = MakePtr<WfBlockStatement>();

			{
				auto eventHandlerLambda = MakePtr<WfFunctionExpression>();
				eventHandlerLambda->function = func;

				auto eventHandler = MakePtr<WfVariableDeclaration>();
				eventHandler->name.value = L"<event-handler>";
				eventHandler->expression = eventHandlerLambda;

				auto stat = MakePtr<WfVariableStatement>();
				stat->variable = eventHandler;
				subBlock->statements.Add(stat);
			}
			{
				auto var = MakePtr<WfReferenceExpression>();
				var->name.value = variableName.ToString();

				auto member = MakePtr<WfMemberExpression>();
				member->parent = var;
				member->name.value = eventInfo->GetName();

				auto eventHandler = MakePtr<WfReferenceExpression>();
				eventHandler->name.value = L"<event-handler>";

				auto attachEvent = MakePtr<WfAttachEventExpression>();
				attachEvent->event = member;
				attachEvent->function = eventHandler;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = attachEvent;
				subBlock->statements.Add(stat);
			}

			return subBlock;
		}
	}
}