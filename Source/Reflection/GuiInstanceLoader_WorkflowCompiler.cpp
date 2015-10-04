#include "GuiInstanceLoader_WorkflowCompiler.h"
#include "TypeDescriptors/GuiReflectionEvents.h"
#include "../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;

#define ERROR_CODE_PREFIX L"================================================================"

/***********************************************************************
Module
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_CreateEmptyModule(Ptr<GuiInstanceContext> context)
		{
			auto module = MakePtr<WfModule>();
			vint index = context->namespaces.Keys().IndexOf(GlobalStringKey());
			if (index != -1)
			{
				auto nss = context->namespaces.Values()[index];
				FOREACH(Ptr<GuiInstanceNamespace>, ns, nss->namespaces)
				{
					auto path = MakePtr<WfModuleUsingPath>();
					module->paths.Add(path);

					auto pathCode = ns->prefix + L"*" + ns->postfix;
					auto reading = pathCode.Buffer();
					while (reading)
					{
						auto delimiter = wcsstr(reading, L"::");
						auto begin = reading;
						auto end = delimiter ? delimiter : begin + wcslen(reading);

						auto wildcard = wcschr(reading, L'*');
						if (wildcard >= end)
						{
							wildcard = nullptr;
						}

						auto item = MakePtr<WfModuleUsingItem>();
						path->items.Add(item);
						if (wildcard)
						{
							if (begin < wildcard)
							{
								auto fragment = MakePtr<WfModuleUsingNameFragment>();
								item->fragments.Add(fragment);
								fragment->name.value = WString(begin, vint(wildcard - begin));
							}
							{
								auto fragment = MakePtr<WfModuleUsingWildCardFragment>();
								item->fragments.Add(fragment);
							}
							if (wildcard + 1 < end)
							{
								auto fragment = MakePtr<WfModuleUsingNameFragment>();
								item->fragments.Add(fragment);
								fragment->name.value = WString(wildcard + 1, vint(end - wildcard - 1));
							}
						}
						else if (begin < end)
						{
							auto fragment = MakePtr<WfModuleUsingNameFragment>();
							item->fragments.Add(fragment);
							fragment->name.value = WString(begin, vint(end - begin));
						}

						if (delimiter)
						{
							reading = delimiter + 2;
						}
						else
						{
							reading = nullptr;
						}
					}
				}
			}
			return module;
		}

/***********************************************************************
Variable
***********************************************************************/

		void Workflow_CreatePointerVariable(Ptr<workflow::WfModule> module, GlobalStringKey name, description::ITypeDescriptor* type)
		{
			auto var = MakePtr<WfVariableDeclaration>();
			var->name.value = name.ToString();
			{
				Ptr<TypeInfoImpl> elementType = new TypeInfoImpl(ITypeInfo::TypeDescriptor);
				elementType->SetTypeDescriptor(type);

				Ptr<TypeInfoImpl> pointerType = new TypeInfoImpl(ITypeInfo::RawPtr);
				pointerType->SetElementType(elementType);

				var->type = GetTypeFromTypeInfo(pointerType.Obj());
			}

			auto literal = MakePtr<WfLiteralExpression>();
			literal->value = WfLiteralValue::Null;
			var->expression = literal;

			module->declarations.Add(var);
		}

		void Workflow_GetVariableTypes(Ptr<GuiInstanceEnvironment> env, types::VariableTypeMap& types)
		{
			FOREACH_INDEXER(GlobalStringKey, name, index, env->scope->referenceValues.Keys())
			{
				auto value = env->scope->referenceValues.Values()[index];
				if (value.GetTypeDescriptor())
				{
					types.Add(name, value.GetTypeDescriptor());
				}
				else
				{
					types.Add(name, GetTypeDescriptor<Value>());
				}
			}
		}
		
		void Workflow_CreateVariablesForReferenceValues(Ptr<workflow::WfModule> module, types::VariableTypeMap& types)
		{
			for (vint i = 0; i < types.Count(); i++)
			{
				auto key = types.Keys()[i];
				auto value = types.Values()[i];
				Workflow_CreatePointerVariable(module, key, value);
			}
		}

		void Workflow_SetVariablesForReferenceValues(Ptr<workflow::runtime::WfRuntimeGlobalContext> context, Ptr<GuiInstanceEnvironment> env)
		{
			FOREACH_INDEXER(GlobalStringKey, name, index, env->scope->referenceValues.Keys())
			{
				vint variableIndex = context->assembly->variableNames.IndexOf(name.ToString());
				if (variableIndex != -1)
				{
					context->globalVariables->variables[variableIndex] = env->scope->referenceValues.Values()[index];
				}
			}
		}

/***********************************************************************
Workflow_ValidateExpression
***********************************************************************/

		bool Workflow_ValidateExpression(Ptr<GuiInstanceContext> context, types::VariableTypeMap& types, types::ErrorList& errors, IGuiInstanceLoader::PropertyInfo& bindingTarget, const WString& expressionCode, Ptr<workflow::WfExpression>& expression)
		{
			auto parser = GetParserManager()->GetParser<WfExpression>(L"WORKFLOW-EXPRESSION");
			expression = parser->TypedParse(expressionCode, errors);
			if (!expression)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to parse the workflow expression.");
				return false;
			}

			bool failed = false;
			auto td = bindingTarget.typeInfo.typeDescriptor;
			auto propertyInfo = td->GetPropertyByName(bindingTarget.propertyName.ToString(), true);
			if (!propertyInfo)
			{
				errors.Add(ERROR_CODE_PREFIX L"Property \"" + bindingTarget.propertyName.ToString() + L"\" does not exist in type \"" + td->GetTypeName() + L"\".");
				failed = true;
			}
			else if (!propertyInfo->IsReadable() || !propertyInfo->IsWritable())
			{
				errors.Add(ERROR_CODE_PREFIX L"Property \"" + bindingTarget.propertyName.ToString() + L"\" of type \"" + td->GetTypeName() + L"\" should be both readable and writable.");
				failed = true;
			}

			auto module = Workflow_CreateEmptyModule(context);
			Workflow_CreateVariablesForReferenceValues(module, types);
			{
				auto func = MakePtr<WfFunctionDeclaration>();
				func->anonymity = WfFunctionAnonymity::Named;
				func->name.value = L"<initialize-data-binding>";
				func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = expression;
				func->statement = stat;

				module->declarations.Add(func);
			}

			Workflow_GetSharedManager()->Clear(true, true);
			Workflow_GetSharedManager()->AddModule(module);
			Workflow_GetSharedManager()->Rebuild(true);
			if (Workflow_GetSharedManager()->errors.Count() > 0)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to analyze the workflow expression \"" + expressionCode + L"\".");
				FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
				{
					errors.Add(error->errorMessage);
				}
				failed = true;
			}
			else if (propertyInfo)
			{
				auto bind = expression.Cast<WfBindExpression>();
				auto result = Workflow_GetSharedManager()->expressionResolvings[(bind ? bind->expression : expression).Obj()];
				if (result.type)
				{
					ITypeInfo* propertyType = propertyInfo->GetReturn();
					if (propertyInfo->GetSetter() && propertyInfo->GetSetter()->GetParameterCount() == 1)
					{
						propertyType = propertyInfo->GetSetter()->GetParameter(0)->GetType();
					}
					if (!CanConvertToType(result.type.Obj(), propertyType, false))
					{
						errors.Add(ERROR_CODE_PREFIX L"Failed to analyze the workflow expression \"" + expressionCode + L"\".");
						errors.Add(
							WfErrors::ExpressionCannotImplicitlyConvertToType(expression.Obj(), result.type.Obj(), propertyType)
							->errorMessage);
						failed = true;
					}
				}
			}

			return !failed;
		}

/***********************************************************************
Workflow_CompileExpression
***********************************************************************/

		Ptr<workflow::runtime::WfAssembly> Workflow_CompileExpression(Ptr<GuiInstanceContext> context, types::VariableTypeMap& types, types::ErrorList& errors, const WString& expressionCode)
		{
			auto parser = GetParserManager()->GetParser<WfExpression>(L"WORKFLOW-EXPRESSION");
			auto expression = parser->TypedParse(expressionCode, errors);
			if (!expression)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to parse the workflow expression \"" + expressionCode + L"\".");
				return 0;
			}

			auto module = Workflow_CreateEmptyModule(context);
			Workflow_CreateVariablesForReferenceValues(module, types);
			{
				auto lambda = MakePtr<WfOrderedLambdaExpression>();
				lambda->body = expression;

				auto var = MakePtr<WfVariableDeclaration>();
				var->name.value = L"<initialize-data-binding>";
				var->expression = lambda;

				module->declarations.Add(var);
			}

			Workflow_GetSharedManager()->Clear(true, true);
			Workflow_GetSharedManager()->AddModule(module);
			Workflow_GetSharedManager()->Rebuild(true);
			if (Workflow_GetSharedManager()->errors.Count() > 0)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to analyze the workflow expression \"" + expressionCode + L"\".");
				FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
				{
					errors.Add(error->errorMessage);
				}
				return 0;
			}

			return GenerateAssembly(Workflow_GetSharedManager());
		}

/***********************************************************************
Workflow_CompileEventHandler
***********************************************************************/

		Ptr<workflow::runtime::WfAssembly> Workflow_CompileEventHandler(Ptr<GuiInstanceContext> context, types::VariableTypeMap& types, types::ErrorList& errors, IGuiInstanceLoader::PropertyInfo& bindingTarget, const WString& statementCode)
		{
			auto parser = GetParserManager()->GetParser<WfStatement>(L"WORKFLOW-STATEMENT");
			auto statement = parser->TypedParse(statementCode, errors);
			if (!statement)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to parse the workflow statement.");
				return 0;
			}

			auto module = Workflow_CreateEmptyModule(context);
			Workflow_CreateVariablesForReferenceValues(module, types);
			{
				auto func = MakePtr<WfFunctionDeclaration>();
				func->anonymity = WfFunctionAnonymity::Named;
				func->name.value = L"<event-handler>";
				func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());

				auto td = bindingTarget.typeInfo.typeDescriptor;
				auto eventInfo = td->GetEventByName(bindingTarget.propertyName.ToString(), true);
				if (eventInfo)
				{
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
				}
						
				auto block = MakePtr<WfBlockStatement>();
				block->statements.Add(statement);
				func->statement = block;

				module->declarations.Add(func);
			}

			Workflow_GetSharedManager()->Clear(true, true);
			Workflow_GetSharedManager()->AddModule(module);
			Workflow_GetSharedManager()->Rebuild(true);
			if (Workflow_GetSharedManager()->errors.Count() > 0)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to analyze the workflow statement \"" + statementCode + L"\".");
				FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
				{
					errors.Add(error->errorMessage);
				}
				return 0;
			}

			return GenerateAssembly(Workflow_GetSharedManager());
		}

/***********************************************************************
Workflow_CompileDataBinding
***********************************************************************/

		WString Workflow_ModuleToString(Ptr<workflow::WfModule> module)
		{
			stream::MemoryStream stream;
			{
				stream::StreamWriter writer(stream);
				WfPrint(module, L"", writer);
			}
			stream.SeekFromBegin(0);
			stream::StreamReader reader(stream);
			return reader.ReadToEnd();
		}

		Ptr<workflow::runtime::WfAssembly> Workflow_CompileDataBinding(Ptr<GuiInstanceContext> context, types::VariableTypeMap& types, description::ITypeDescriptor* thisType, types::ErrorList& errors, collections::List<WorkflowDataBinding>& dataBindings)
		{
			auto module = Workflow_CreateEmptyModule(context);
			Workflow_CreateVariablesForReferenceValues(module, types);
			Workflow_CreatePointerVariable(module, GlobalStringKey::Get(L"<this>"), thisType);

			auto func = MakePtr<WfFunctionDeclaration>();
			func->anonymity = WfFunctionAnonymity::Named;
			func->name.value = L"<initialize-data-binding>";
			func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());

			auto block = MakePtr<WfBlockStatement>();
			func->statement = block;
			module->declarations.Add(func);
			

			FOREACH(WorkflowDataBinding, dataBinding, dataBindings)
			{
				if (dataBinding.bindExpression.Cast<WfBindExpression>())
				{
					auto subBlock = MakePtr<WfBlockStatement>();
					block->statements.Add(subBlock);
					{
						auto refThis = MakePtr<WfReferenceExpression>();
						refThis->name.value = L"<this>";

						auto member = MakePtr<WfMemberExpression>();
						member->parent = refThis;
						member->name.value = L"AddSubscription";

						auto call = MakePtr<WfCallExpression>();
						call->function = member;
						call->arguments.Add(dataBinding.bindExpression);

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
							refSubscribee->name.value = dataBinding.variableName.ToString();

							auto member = MakePtr<WfMemberExpression>();
							member->parent = refSubscribee;
							member->name.value = dataBinding.propertyInfo->GetName();

							auto var = MakePtr<WfVariableDeclaration>();
							var->name.value = L"<old>";
							var->expression = member;

							auto stat = MakePtr<WfVariableStatement>();
							stat->variable = var;
							callbackBlock->statements.Add(stat);
						}
						{
							ITypeInfo* propertyType = dataBinding.propertyInfo->GetReturn();
							if (dataBinding.propertyInfo->GetSetter() && dataBinding.propertyInfo->GetSetter()->GetParameterCount() == 1)
							{
								propertyType = dataBinding.propertyInfo->GetSetter()->GetParameter(0)->GetType();
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
							refSubscribee->name.value = dataBinding.variableName.ToString();

							auto member = MakePtr<WfMemberExpression>();
							member->parent = refSubscribee;
							member->name.value = dataBinding.propertyInfo->GetName();

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
				}
				else if (dataBinding.bindExpression)
				{
					auto refSubscribee = MakePtr<WfReferenceExpression>();
					refSubscribee->name.value = dataBinding.variableName.ToString();

					auto member = MakePtr<WfMemberExpression>();
					member->parent = refSubscribee;
					member->name.value = dataBinding.propertyInfo->GetName();

					auto assign = MakePtr<WfBinaryExpression>();
					assign->op = WfBinaryOperator::Assign;
					assign->first = member;
					assign->second = dataBinding.bindExpression;

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = assign;
					block->statements.Add(stat);
				}
			}

			Workflow_GetSharedManager()->Clear(true, true);
			Workflow_GetSharedManager()->AddModule(module);
			Workflow_GetSharedManager()->Rebuild(true);
			WString moduleCode = Workflow_ModuleToString(module);

			if (Workflow_GetSharedManager()->errors.Count() > 0)
			{
				errors.Add(ERROR_CODE_PREFIX L"Unexpected errors are encountered when initializing data binding.");
				FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
				{
					errors.Add(error->errorMessage);
				}
				errors.Add(ERROR_CODE_PREFIX L"Print code for reference:");
				errors.Add(moduleCode);
				return 0;
			}
			return GenerateAssembly(Workflow_GetSharedManager());
		}

/***********************************************************************
Workflow_GetSharedManager
***********************************************************************/

		class WorkflowReferenceNamesVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::VariableTypeInfoMap&			typeInfos;
			types::ErrorList&					errors;

			IGuiInstanceLoader::TypeInfo		bindingTargetTypeInfo;
			vint								generatedNameCount;
			ITypeDescriptor*					rootTypeDescriptor;

			WorkflowReferenceNamesVisitor(Ptr<GuiInstanceContext> _context, types::VariableTypeInfoMap& _typeInfos, types::ErrorList& _errors)
				:context(_context)
				, typeInfos(_typeInfos)
				, errors(_errors)
				, generatedNameCount(0)
				, rootTypeDescriptor(0)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				auto reprTypeInfo = bindingTargetTypeInfo;
				auto loader = GetInstanceLoaderManager()->GetLoader(reprTypeInfo.typeName);

				if (repr->instanceName != GlobalStringKey::Empty && reprTypeInfo.typeDescriptor)
				{
					if (typeInfos.Keys().Contains(repr->instanceName))
					{
						errors.Add(L"Precompile: Parameter \"" + repr->instanceName.ToString() + L"\" conflict with an existing named object.");
					}
					else
					{
						typeInfos.Add(repr->instanceName, reprTypeInfo);
					}
				}
				
				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					IGuiInstanceLoader::TypeInfo propertyTypeInfo;

					if (setter->binding != GlobalStringKey::Empty && setter->binding != GlobalStringKey::_Set)
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceBinder(setter->binding);
						if (!binder)
						{
							errors.Add(L"The appropriate IGuiInstanceBinder of binding \"" + setter->binding.ToString() + L"\" cannot be found.");
						}
						else if (binder->RequireInstanceName() && repr->instanceName == GlobalStringKey::Empty && reprTypeInfo.typeDescriptor)
						{
							auto name = GlobalStringKey::Get(L"<precompile>" + itow(generatedNameCount++));
							repr->instanceName = name;
							typeInfos.Add(name, reprTypeInfo);
						}
					}

					if (setter->binding == GlobalStringKey::_Set)
					{
						IGuiInstanceLoader::PropertyInfo info;
						info.typeInfo = reprTypeInfo;
						info.propertyName = repr->setters.Keys()[index];
						auto currentLoader = loader;

						while (currentLoader)
						{
							auto typeInfo = currentLoader->GetPropertyType(info);
							if (typeInfo && typeInfo->support != GuiInstancePropertyInfo::NotSupport)
							{
								propertyTypeInfo.typeDescriptor = typeInfo->acceptableTypes[0];
								propertyTypeInfo.typeName = GlobalStringKey::Get(typeInfo->acceptableTypes[0]->GetTypeName());
								break;
							}
							currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
						}
					}

					FOREACH(Ptr<GuiValueRepr>, value, setter->values)
					{
						bindingTargetTypeInfo = propertyTypeInfo;
						value->Accept(this);
					}
				}

				FOREACH(Ptr<GuiAttSetterRepr::EventValue>, handler, repr->eventHandlers.Values())
				{
					if (handler->binding != GlobalStringKey::Empty)
					{
						auto binder = GetInstanceLoaderManager()->GetInstanceEventBinder(handler->binding);
						if (!binder)
						{
							errors.Add(L"The appropriate IGuiInstanceEventBinder of binding \"" + handler->binding.ToString() + L"\" cannot be found.");
						}
						else if (binder->RequireInstanceName() && repr->instanceName == GlobalStringKey::Empty && reprTypeInfo.typeDescriptor)
						{
							auto name = GlobalStringKey::Get(L"<precompile>" + itow(generatedNameCount++));
							repr->instanceName = name;
							typeInfos.Add(name, reprTypeInfo);
						}
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				bool found = false;
				if (repr == context->instance.Obj() && context->className)
				{
					auto fullName = GlobalStringKey::Get(context->className.Value());
					auto td = GetInstanceLoaderManager()->GetTypeDescriptorForType(fullName);
					if (td)
					{
						found = true;
						bindingTargetTypeInfo.typeName = fullName;
						bindingTargetTypeInfo.typeDescriptor = td;
					}
				}

				if (!found)
				{
					auto source = FindInstanceLoadingSource(context, repr);
					bindingTargetTypeInfo.typeName = source.typeName;
					bindingTargetTypeInfo.typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				}

				if (!bindingTargetTypeInfo.typeDescriptor)
				{
					errors.Add(
						L"Precompile: Failed to find type \"" +
						(repr->typeNamespace == GlobalStringKey::Empty
							? repr->typeName.ToString()
							: repr->typeNamespace.ToString() + L":" + repr->typeName.ToString()
							) +
						L"\".");
				}

				if (context->instance.Obj() == repr)
				{
					rootTypeDescriptor = bindingTargetTypeInfo.typeDescriptor;
				}
				Visit((GuiAttSetterRepr*)repr);
			}
		};

		class WorkflowCompileVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			Ptr<GuiInstanceContext>				context;
			types::VariableTypeInfoMap&			typeInfos;
			types::ErrorList&					errors;
			
			types::VariableTypeMap				types;
			List<WorkflowDataBinding>			dataBindings;

			WorkflowCompileVisitor(Ptr<GuiInstanceContext> _context, types::VariableTypeInfoMap& _typeInfos, types::ErrorList& _errors)
				:context(_context)
				, typeInfos(_typeInfos)
				, errors(_errors)
			{
				for (vint i = 0; i < typeInfos.Count(); i++)
				{
					auto key = typeInfos.Keys()[i];
					auto value = typeInfos.Values()[i];
					types.Add(key, value.typeDescriptor);
				}
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				IGuiInstanceLoader::TypeInfo reprTypeInfo;
				if (repr->instanceName != GlobalStringKey::Empty)
				{
					reprTypeInfo = typeInfos[repr->instanceName];
				}

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					if (reprTypeInfo.typeDescriptor)
					{
						GlobalStringKey propertyName = repr->setters.Keys()[index];
						Ptr<GuiInstancePropertyInfo> propertyInfo;
						IGuiInstanceLoader::PropertyInfo info;
						info.typeInfo = reprTypeInfo;
						info.propertyName = propertyName;

						{
							auto currentLoader = GetInstanceLoaderManager()->GetLoader(info.typeInfo.typeName);

							while (currentLoader && !propertyInfo)
							{
								propertyInfo = currentLoader->GetPropertyType(info);
								if (propertyInfo && propertyInfo->support == GuiInstancePropertyInfo::NotSupport)
								{
									propertyInfo = 0;
								}
								currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
							}
						}

						if (!propertyInfo)
						{
							errors.Add(L"Precompile: Cannot find property \"" + propertyName.ToString() + L"\" in type \"" + reprTypeInfo.typeName.ToString() + L"\".");
						}
						else
						{
							WString expressionCode;
							if (auto obj = setter->values[0].Cast<GuiTextRepr>())
							{
								expressionCode = obj->text;
							}

							if (setter->binding == GlobalStringKey::_Bind || setter->binding == GlobalStringKey::_Format)
							{
								WorkflowDataBinding dataBinding;
								dataBinding.variableName = repr->instanceName;

								if (setter->binding == GlobalStringKey::_Bind)
								{
									expressionCode = L"bind(" + expressionCode + L")";
								}
								else if (setter->binding == GlobalStringKey::_Format)
								{
									expressionCode = L"bind($\"" + expressionCode + L"\")";
								}

								Ptr<WfExpression> expression;
								if (Workflow_ValidateExpression(context, types, errors, info, expressionCode, expression))
								{
									dataBinding.propertyInfo = reprTypeInfo.typeDescriptor->GetPropertyByName(propertyName.ToString(), true);
									dataBinding.bindExpression = expression;
								}

								dataBindings.Add(dataBinding);
							}
							else if (setter->binding == GlobalStringKey::_Eval)
							{
								if (propertyInfo->scope != GuiInstancePropertyInfo::Property)
								{
									WString cacheKey = L"<att.eval>" + expressionCode;
									auto assembly = Workflow_CompileExpression(context, types, errors, expressionCode);
									context->precompiledCaches.Add(GlobalStringKey::Get(cacheKey), new GuiWorkflowCache(assembly));
								}
								else
								{
									WorkflowDataBinding dataBinding;
									dataBinding.variableName = repr->instanceName;
									Ptr<WfExpression> expression;
									if (Workflow_ValidateExpression(context, types, errors, info, expressionCode, expression))
									{
										dataBinding.propertyInfo = reprTypeInfo.typeDescriptor->GetPropertyByName(propertyName.ToString(), true);
										dataBinding.bindExpression = expression;
									}

									dataBindings.Add(dataBinding);
								}
							}
						}
					}

					FOREACH(Ptr<GuiValueRepr>, value, setter->values)
					{
						value->Accept(this);
					}
				}

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::EventValue>, handler, index, repr->eventHandlers.Values())
				{
					if (reprTypeInfo.typeDescriptor)
					{
						GlobalStringKey propertyName = repr->eventHandlers.Keys()[index];
						Ptr<GuiInstanceEventInfo> eventInfo;
						IGuiInstanceLoader::PropertyInfo info;
						info.typeInfo = reprTypeInfo;
						info.propertyName = propertyName;

						{
							auto currentLoader = GetInstanceLoaderManager()->GetLoader(info.typeInfo.typeName);

							while (currentLoader && !eventInfo)
							{
								eventInfo = currentLoader->GetEventType(info);
								if (eventInfo && eventInfo->support == GuiInstanceEventInfo::NotSupport)
								{
									eventInfo = 0;
								}
								currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
							}
						}

						if (!eventInfo)
						{
							errors.Add(L"Precompile: Cannot find event \"" + propertyName.ToString() + L"\" in type \"" + reprTypeInfo.typeName.ToString() + L"\".");
						}
						else
						{
							WString statementCode = handler->value;

							if (handler->binding == GlobalStringKey::_Eval)
							{
								WString cacheKey = L"<ev.eval><" + repr->instanceName.ToString() + L"><" + propertyName.ToString() + L">" + statementCode;
								auto assembly = Workflow_CompileEventHandler(context, types, errors, info, statementCode);
								context->precompiledCaches.Add(GlobalStringKey::Get(cacheKey), new GuiWorkflowCache(assembly));
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

		void Workflow_PrecompileInstanceContext(Ptr<GuiInstanceContext> context, types::ErrorList& errors)
		{
			ITypeDescriptor* rootTypeDescriptor = 0;
			types::VariableTypeInfoMap typeInfos;
			{
				FOREACH(Ptr<GuiInstanceParameter>, parameter, context->parameters)
				{
					auto type = GetTypeDescriptor(parameter->className.ToString());
					if (!type)
					{
						errors.Add(L"Precompile: Cannot find type \"" + parameter->className.ToString() + L"\".");
					}
					else if (typeInfos.Keys().Contains(parameter->name))
					{
						errors.Add(L"Precompile: Parameter \"" + parameter->name.ToString() + L"\" conflict with an existing named object.");
					}
					else
					{
						IGuiInstanceLoader::TypeInfo typeInfo;
						typeInfo.typeDescriptor = type;
						typeInfo.typeName = GlobalStringKey::Get(type->GetTypeName());
						typeInfos.Add(parameter->name, typeInfo);
					}
				}

				WorkflowReferenceNamesVisitor visitor(context, typeInfos, errors);
				context->instance->Accept(&visitor);
				rootTypeDescriptor = visitor.rootTypeDescriptor;
			}

			if (errors.Count() == 0)
			{
				WorkflowCompileVisitor visitor(context, typeInfos, errors);
				context->instance->Accept(&visitor);

				if (visitor.dataBindings.Count() > 0 && rootTypeDescriptor)
				{
					auto assembly = Workflow_CompileDataBinding(context, visitor.types, rootTypeDescriptor, errors, visitor.dataBindings);
					context->precompiledCaches.Add(GuiWorkflowCache::CacheContextName, new GuiWorkflowCache(assembly));
				}
			}
		}

/***********************************************************************
GuiWorkflowCache
***********************************************************************/

		const GlobalStringKey& GuiWorkflowCache::CacheTypeName = GlobalStringKey::_Workflow_Assembly_Cache;
		const GlobalStringKey& GuiWorkflowCache::CacheContextName = GlobalStringKey::_Workflow_Global_Context;

		GuiWorkflowCache::GuiWorkflowCache()
		{
		}

		GuiWorkflowCache::GuiWorkflowCache(Ptr<workflow::runtime::WfAssembly> _assembly)
			:assembly(_assembly)
		{
		}

		GlobalStringKey GuiWorkflowCache::GetCacheTypeName()
		{
			return CacheTypeName;
		}

/***********************************************************************
GuiWorkflowCacheResolver
***********************************************************************/

		GlobalStringKey GuiWorkflowCacheResolver::GetCacheTypeName()
		{
			return GuiWorkflowCache::CacheTypeName;
		}

		bool GuiWorkflowCacheResolver::Serialize(Ptr<IGuiResourceCache> cache, stream::IStream& stream)
		{
			if (auto obj = cache.Cast<GuiWorkflowCache>())
			{
				obj->assembly->Serialize(stream);
				return true;
			}
			else
			{
				return false;
			}
		}

		Ptr<IGuiResourceCache> GuiWorkflowCacheResolver::Deserialize(stream::IStream& stream)
		{
			auto assembly = new WfAssembly(stream);
			return new GuiWorkflowCache(assembly);
		}

/***********************************************************************
Workflow_GetSharedManager
***********************************************************************/

#undef ERROR_CODE_PREFIX

		class GuiWorkflowSharedManagerPlugin;
		GuiWorkflowSharedManagerPlugin* sharedManagerPlugin = 0;

		class GuiWorkflowSharedManagerPlugin : public Object, public IGuiPlugin
		{
		protected:
			Ptr<WfLexicalScopeManager>		workflowManager;

		public:
			GuiWorkflowSharedManagerPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
			{
				sharedManagerPlugin = this;

				auto manager=GetResourceResolverManager();
				manager->SetCacheResolver(new GuiWorkflowCacheResolver);
			}

			void Unload()override
			{
				sharedManagerPlugin = 0;
			}

			WfLexicalScopeManager* GetWorkflowManager()
			{
				if (!workflowManager)
				{
					workflowManager = new WfLexicalScopeManager(GetParserManager()->GetParsingTable(L"WORKFLOW"));
				}
				return workflowManager.Obj();
			}
		};
		GUI_REGISTER_PLUGIN(GuiWorkflowSharedManagerPlugin)

		WfLexicalScopeManager* Workflow_GetSharedManager()
		{
			return sharedManagerPlugin->GetWorkflowManager();
		}
	}
}