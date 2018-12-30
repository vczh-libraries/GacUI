#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../GuiInstanceLocalizedStrings.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;

		using namespace controls;
		using namespace compositions;
		using namespace templates;

/***********************************************************************
FindInstanceLoadingSource
***********************************************************************/

		template<typename TCallback>
		auto FindByTag(Ptr<GuiInstanceContext> context, GlobalStringKey namespaceName, const WString& typeName, TCallback callback)
			-> typename RemoveCVR<decltype(callback({}).Value())>::Type
		{
			vint index = context->namespaces.Keys().IndexOf(namespaceName);
			if (index != -1)
			{
				Ptr<GuiInstanceContext::NamespaceInfo> namespaceInfo = context->namespaces.Values()[index];
				FOREACH(Ptr<GuiInstanceNamespace>, ns, namespaceInfo->namespaces)
				{
					auto fullName = GlobalStringKey::Get(ns->prefix + typeName + ns->postfix);
					if (auto nullable = callback(fullName))
					{
						return nullable.Value();
					}
				}
			}
			return {};
		}

		InstanceLoadingSource FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GlobalStringKey namespaceName, const WString& typeName)
		{
			return FindByTag(context, namespaceName, typeName, [](GlobalStringKey fullName)->Nullable<InstanceLoadingSource>
			{
				if (auto loader = GetInstanceLoaderManager()->GetLoader(fullName))
				{
					return { { loader, fullName } };
				}
				return {};
			});
		}

		Ptr<GuiResourceItem> FindInstanceResourceItem(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor, Ptr<GuiResourceClassNameRecord> record)
		{
			return FindByTag(context, ctor->typeNamespace, ctor->typeName.ToString(), [=](GlobalStringKey fullName)->Nullable<Ptr<GuiResourceItem>>
			{
				vint index = record->classResources.Keys().IndexOf(fullName.ToString());
				if (index != -1)
				{
					return record->classResources.Values()[index];
				}
				return {};
			});
		}

		InstanceLoadingSource FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor)
		{
			return FindInstanceLoadingSource(context, ctor->typeNamespace, ctor->typeName.ToString());
		}

/***********************************************************************
Workflow_PrecompileInstanceContext
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_PrecompileInstanceContext(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors)
		{
			auto module = Workflow_CreateModuleWithUsings(resolvingResult.context, moduleName);
			{
				auto block = Workflow_InstallCtorClass(resolvingResult, module);
				Workflow_GenerateCreating(precompileContext, resolvingResult, block, errors);
				Workflow_GenerateBindings(precompileContext, resolvingResult, block, errors);
			}
			return module;
		}

/***********************************************************************
WorkflowEventNamesVisitor
***********************************************************************/

		class WorkflowEventNamesVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			GuiResourcePrecompileContext&		precompileContext;
			types::ResolvingResult&				resolvingResult;
			List<types::PropertyResolving>&		candidatePropertyTypeInfos;
			Ptr<WfClassDeclaration>				instanceClass;
			GuiResourceError::List&				errors;

			IGuiInstanceLoader::TypeInfo		resolvedTypeInfo;

			WorkflowEventNamesVisitor(GuiResourcePrecompileContext& _precompileContext, types::ResolvingResult& _resolvingResult, List<types::PropertyResolving>& _candidatePropertyTypeInfos, Ptr<WfClassDeclaration> _instanceClass, GuiResourceError::List& _errors)
				:precompileContext(_precompileContext)
				, resolvingResult(_resolvingResult)
				, candidatePropertyTypeInfos(_candidatePropertyTypeInfos)
				, instanceClass(_instanceClass)
				, errors(_errors)
			{
			}

			///////////////////////////////////////////////////////////////////////////////////

			Ptr<WfDeclaration> ProcessEvent(
				Ptr<GuiAttSetterRepr::EventValue> handler,
				GlobalStringKey propertyName
				)
			{
				if (auto eventInfo = resolvedTypeInfo.typeInfo->GetTypeDescriptor()->GetEventByName(propertyName.ToString(), true))
				{
					auto decl = Workflow_GenerateEventHandler(precompileContext, eventInfo);
					decl->anonymity = WfFunctionAnonymity::Named;
					decl->name.value = handler->value;

					{
						auto att = MakePtr<WfAttribute>();
						att->category.value = L"cpp";
						att->name.value = L"Protected";

						decl->attributes.Add(att);
					}
					{
						auto att = MakePtr<WfAttribute>();
						att->category.value = L"cpp";
						att->name.value = L"UserImpl";

						decl->attributes.Add(att);
					}

					{
						auto block = MakePtr<WfBlockStatement>();
						decl->statement = block;

						auto stringExpr = MakePtr<WfStringExpression>();
						stringExpr->value.value = L"Not Implemented: " + handler->value;

						auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
						raiseStat->expression = stringExpr;
						block->statements.Add(raiseStat);
					}

					decl->classMember = MakePtr<WfClassMember>();
					decl->classMember->kind = WfClassMemberKind::Normal;
					return decl;
				}
				else
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, handler->attPosition,
						L"Precompile: Event \"" +
						propertyName.ToString() +
						L"\" cannot be found in type \"" +
						resolvedTypeInfo.typeName.ToString() +
						L"\"."));
				}
				return nullptr;
			}

			///////////////////////////////////////////////////////////////////////////////////

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					auto loader = GetInstanceLoaderManager()->GetLoader(resolvedTypeInfo.typeName);
					List<types::PropertyResolving> possibleInfos;
					auto prop = repr->setters.Keys()[index];

					WString errorPrefix;
					if (Workflow_GetPropertyTypes(errorPrefix, resolvingResult, loader, resolvedTypeInfo, prop, setter, possibleInfos, errors))
					{
						if (setter->binding == GlobalStringKey::_Set)
						{
							if (possibleInfos[0].info->support == GuiInstancePropertyInfo::SupportSet)
							{
								auto setTarget = dynamic_cast<GuiAttSetterRepr*>(setter->values[0].Obj());

								List<types::PropertyResolving> infos;
								WorkflowEventNamesVisitor visitor(precompileContext, resolvingResult, infos, instanceClass, errors);
								auto typeInfo = possibleInfos[0].info->acceptableTypes[0];
								visitor.resolvedTypeInfo.typeName = GlobalStringKey::Get(typeInfo->GetTypeDescriptor()->GetTypeName());
								visitor.resolvedTypeInfo.typeInfo = typeInfo;
								setTarget->Accept(&visitor);
							}
							else
							{
								errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" does not support the \"-set\" binding."));
							}
						}
						else
						{
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								WorkflowEventNamesVisitor visitor(precompileContext, resolvingResult, possibleInfos, instanceClass, errors);
								value->Accept(&visitor);
							}
						}
					}
				}

				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::EventValue>, handler, index, repr->eventHandlers.Values())
				{
					if (handler->binding == GlobalStringKey::Empty)
					{
						auto propertyName = repr->eventHandlers.Keys()[index];
						if (auto decl = ProcessEvent(handler, propertyName))
						{
							Workflow_RecordScriptPosition(precompileContext, handler->valuePosition, decl);
							instanceClass->declarations.Add(decl);
						}
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				auto context = resolvingResult.context;

				bool inferType = repr->typeNamespace == GlobalStringKey::Empty&&repr->typeName == GlobalStringKey::_InferType;

				bool noContextToInfer = false;
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
						else if (info->acceptableTypes.Count() == 0)
						{
							noContextToInfer = true;
						}
					}
					else if (candidatePropertyTypeInfos.Count() == 0)
					{
						noContextToInfer = true;
					}
				}
				else
				{
					if (repr == context->instance.Obj())
					{
						auto fullName = GlobalStringKey::Get(context->className);
						if (auto reprType = GetInstanceLoaderManager()->GetTypeInfoForType(fullName))
						{
							resolvedTypeInfo.typeName = fullName;
							resolvedTypeInfo.typeInfo = reprType;
						}
					}
					if (!resolvedTypeInfo.typeInfo)
					{
						auto source = FindInstanceLoadingSource(context, repr);
						resolvedTypeInfo.typeName = source.typeName;
						resolvedTypeInfo.typeInfo = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName);
					}
				}

				if (resolvedTypeInfo.typeInfo)
				{
					if (repr->setters.Count() == 1 && repr->setters.Keys()[0] == GlobalStringKey::Empty)
					{
						auto setter = repr->setters.Values()[0];
						if (setter->values.Count() == 1)
						{
							if (auto text = setter->values[0].Cast<GuiTextRepr>())
							{
								return;
							}
						}
					}
					Visit((GuiAttSetterRepr*)repr);
				}
				else if (inferType)
				{
					if (noContextToInfer)
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
							L"Precompile: Unable to resolve type \"_\" without any context."));
					}
					else
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
							L"Precompile: Unable to resolve type \"_\" since the current property accepts multiple types."));
					}
				}
				else
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, repr->tagPosition,
						L"Precompile: Failed to find type \"" +
						(repr->typeNamespace == GlobalStringKey::Empty
							? repr->typeName.ToString()
							: repr->typeNamespace.ToString() + L":" + repr->typeName.ToString()
							) +
						L"\"."));
				}
			}
		};

/***********************************************************************
Workflow_GenerateInstanceClass
***********************************************************************/

		class ReplaceDeclImplVisitor
			: public empty_visitor::DeclarationVisitor
			, public empty_visitor::VirtualCfeDeclarationVisitor
			, public empty_visitor::VirtualCseDeclarationVisitor
		{
		public:
			Func<Ptr<WfStatement>()>			statCtor;
			List<Ptr<WfDeclaration>>&			unprocessed;

			ReplaceDeclImplVisitor(Func<Ptr<WfStatement>()> _statCtor, List<Ptr<WfDeclaration>>& _unprocessed)
				:statCtor(_statCtor)
				, unprocessed(_unprocessed)
			{
			}

			void Dispatch(WfVirtualCfeDeclaration* node)override
			{
				node->Accept(static_cast<WfVirtualCfeDeclaration::IVisitor*>(this));
			}

			void Dispatch(WfVirtualCseDeclaration* node)override
			{
				node->Accept(static_cast<WfVirtualCseDeclaration::IVisitor*>(this));
			}

			void Visit(WfFunctionDeclaration* node)override
			{
				node->statement = statCtor();
			}

			void Visit(WfConstructorDeclaration* node)override
			{
				node->statement = statCtor();
			}

			void Visit(WfDestructorDeclaration* node)override
			{
				node->statement = statCtor();
			}

			void Visit(WfClassDeclaration* node)override
			{
				CopyFrom(unprocessed, node->declarations, true);
			}
		};

		Ptr<workflow::WfModule> Workflow_GenerateInstanceClass(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors, vint passIndex)
		{
			bool needFunctionBody = false;
			bool needEventHandler = false;
			switch (passIndex)
			{
			case IGuiResourceTypeResolver_Precompile::Instance_CollectInstanceTypes:
				needFunctionBody = false;
				needEventHandler = false;
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CollectEventHandlers:
				needFunctionBody = false;
				needEventHandler = true;
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_GenerateInstanceClass:
				needFunctionBody = true;
				needEventHandler = true;
				break;
			}
			auto classNameRecord = precompileContext.targetFolder->GetValueByPath(L"ClassNameRecord").Cast<GuiResourceClassNameRecord>();

			Ptr<ITypeInfo> baseType;
			Ptr<GuiResourceItem> baseTypeResourceItem;
			Ptr<GuiInstanceContext> baseTypeContext;
			Ptr<WfType> baseWfType;
			auto context = resolvingResult.context;
			{
				auto source = FindInstanceLoadingSource(context, context->instance.Obj());
				baseType = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName);
				if (!baseType)
				{
					baseTypeResourceItem = FindInstanceResourceItem(context, context->instance.Obj(), classNameRecord);
				}
				if (!baseType && !baseTypeResourceItem)
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, context->instance->tagPosition,
						L"Precompile: Failed to find type \"" +
						(context->instance->typeNamespace == GlobalStringKey::Empty
							? context->instance->typeName.ToString()
							: context->instance->typeNamespace.ToString() + L":" + context->instance->typeName.ToString()
							) +
						L"\"."));
					return nullptr;
				}
				if (baseTypeResourceItem && needEventHandler)
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, context->instance->tagPosition,
						L"[INTERNAL ERROR] Precompile: Failed to find compiled type in previous passes \"" +
						(context->instance->typeNamespace == GlobalStringKey::Empty
							? context->instance->typeName.ToString()
							: context->instance->typeNamespace.ToString() + L":" + context->instance->typeName.ToString()
							) +
						L"\"."));
					return nullptr;
				}
			}

			if (baseTypeResourceItem)
			{
				baseTypeContext = baseTypeResourceItem->GetContent().Cast<GuiInstanceContext>();

				List<WString> fragments;
				SplitTypeName(baseTypeContext->className, fragments);
				for (vint i = 0; i < fragments.Count(); i++)
				{
					if (baseWfType)
					{
						auto type = MakePtr<WfChildType>();
						type->parent = baseWfType;
						type->name.value = fragments[i];
						baseWfType = type;
					}
					else
					{
						auto type = MakePtr<WfTopQualifiedType>();
						type->name.value = fragments[i];
						baseWfType = type;
					}
				}
			}

			///////////////////////////////////////////////////////////////
			// Instance Class
			///////////////////////////////////////////////////////////////

			auto module = Workflow_CreateModuleWithUsings(context, moduleName);
			auto instanceClass = Workflow_InstallClass(context->className, module);
			{
				if (baseWfType)
				{
					instanceClass->baseTypes.Add(baseWfType);
				}
				else
				{
					auto typeInfo = MakePtr<TypeDescriptorTypeInfo>(baseType->GetTypeDescriptor(), TypeInfoHint::Normal);
					auto baseType = GetTypeFromTypeInfo(typeInfo.Obj());
					instanceClass->baseTypes.Add(baseType);
				}

				if (context->codeBehind)
				{
					auto value = MakePtr<WfStringExpression>();
					value->value.value = instanceClass->name.value;

					auto att = MakePtr<WfAttribute>();
					att->category.value = L"cpp";
					att->name.value = L"File";
					att->value = value;

					instanceClass->attributes.Add(att);
				}
			}

			///////////////////////////////////////////////////////////////
			// Inherit from Constructor Class
			///////////////////////////////////////////////////////////////

			if (needFunctionBody)
			{
				auto baseConstructorType = MakePtr<WfReferenceType>();
				baseConstructorType->name.value = instanceClass->name.value + L"Constructor";
				instanceClass->baseTypes.Add(baseConstructorType);

				{
					auto value = MakePtr<WfTypeOfTypeExpression>();
					value->type = CopyType(baseConstructorType);

					auto att = MakePtr<WfAttribute>();
					att->category.value = L"cpp";
					att->name.value = L"Friend";
					att->value = value;

					instanceClass->attributes.Add(att);
				}
			}

			///////////////////////////////////////////////////////////////
			// Helpers
			///////////////////////////////////////////////////////////////

			auto parseClassMembers = [&](const WString& code, const WString& name, List<Ptr<WfDeclaration>>& memberDecls, GuiResourceTextPos position)
			{
				WString wrappedCode = L"module parse_members; class Class {\r\n" + code + L"\r\n}";
				if (auto module = Workflow_ParseModule(precompileContext, { resolvingResult.resource }, wrappedCode, position, errors, { 1,0 }))
				{
					CopyFrom(memberDecls, module->declarations[0].Cast<WfClassDeclaration>()->declarations);
				}
			};

			auto addDecl = [=](Ptr<WfDeclaration> decl)
			{
				decl->classMember = MakePtr<WfClassMember>();
				decl->classMember->kind = WfClassMemberKind::Normal;
				instanceClass->declarations.Add(decl);
			};

			auto notImplemented = []()
			{
				auto block = MakePtr<WfBlockStatement>();

				auto stringExpr = MakePtr<WfStringExpression>();
				stringExpr->value.value = L"Not Implemented";

				auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
				raiseStat->expression = stringExpr;

				block->statements.Add(raiseStat);
				return block;
			};

			auto getDefaultType = [&](const WString& className)->Tuple<Ptr<ITypeInfo>, WString>
			{
				auto paramTd = GetTypeDescriptor(className);
				if (!paramTd)
				{
					auto source = FindInstanceLoadingSource(resolvingResult.context, {}, className);
					if (auto typeInfo = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName))
					{
						paramTd = typeInfo->GetTypeDescriptor();
					}
				}

				if (paramTd)
				{
					auto typeInfo = Workflow_GetSuggestedParameterType(paramTd);
					switch (typeInfo->GetDecorator())
					{
					case ITypeInfo::RawPtr: return { typeInfo,className + L"*" };
					case ITypeInfo::SharedPtr: return { typeInfo,className + L"^" };
					default: return { typeInfo,className };
					}
				}
				return { nullptr,className };
			};

			///////////////////////////////////////////////////////////////
			// ref.Members
			///////////////////////////////////////////////////////////////

			if (context->memberScript != L"")
			{
				List<Ptr<WfDeclaration>> memberDecls;
				parseClassMembers(context->memberScript, L"members of instance \"" + context->className + L"\"", memberDecls, context->memberPosition);

				if (!needFunctionBody)
				{
					List<Ptr<WfDeclaration>> unprocessed;
					CopyFrom(unprocessed, memberDecls);

					ReplaceDeclImplVisitor visitor(notImplemented, unprocessed);
					for (vint i = 0; i < unprocessed.Count(); i++)
					{
						unprocessed[i]->Accept(&visitor);
					}
				}

				CopyFrom(instanceClass->declarations, memberDecls, true);
			}

			///////////////////////////////////////////////////////////////
			// Constructor Declaration
			///////////////////////////////////////////////////////////////

			auto ctor = MakePtr<WfConstructorDeclaration>();
			ctor->constructorType = WfConstructorType::RawPtr;
			auto ctorBlock = (!needFunctionBody ? notImplemented() : MakePtr<WfBlockStatement>());
			ctor->statement = ctorBlock;

			if (baseWfType)
			{
				auto call = MakePtr<WfBaseConstructorCall>();
				ctor->baseConstructorCalls.Add(call);
				call->type = CopyType(instanceClass->baseTypes[0]);
				baseTypeContext = baseTypeResourceItem->GetContent().Cast<GuiInstanceContext>();

				FOREACH(Ptr<GuiInstanceParameter>, parameter, baseTypeContext->parameters)
				{
					auto parameterTypeInfoTuple = getDefaultType(parameter->className.ToString());
					auto expression = Workflow_ParseExpression(
						precompileContext,
						parameter->classPosition.originalLocation,
						L"cast("+parameterTypeInfoTuple.f1+L") (null of object)",
						parameter->classPosition,
						errors,
						{ 0,5 }
						);
					if (!expression)
					{
						auto nullExpr = MakePtr<WfLiteralExpression>();
						nullExpr->value = WfLiteralValue::Null;
						expression = nullExpr;
					}
					call->arguments.Add(expression);
				}
			}
			else if (auto group = baseType->GetTypeDescriptor()->GetConstructorGroup())
			{
				auto ctorInfo = group->GetMethod(0);
				vint count = ctorInfo->GetParameterCount();
				if (count > 0)
				{
					if (needFunctionBody)
					{
						if (auto call = resolvingResult.rootLoader->CreateRootInstance(precompileContext, resolvingResult, resolvingResult.rootTypeInfo, resolvingResult.rootCtorArguments, errors))
						{
							ctor->baseConstructorCalls.Add(call);
						}
					}
					else
					{
						auto call = MakePtr<WfBaseConstructorCall>();
						ctor->baseConstructorCalls.Add(call);

						call->type = CopyType(instanceClass->baseTypes[0]);
						for (vint i = 0; i < count; i++)
						{
							call->arguments.Add(CreateDefaultValue(ctorInfo->GetParameter(i)->GetType()));
						}
					}
				}
			}

			///////////////////////////////////////////////////////////////
			// ref.LocalizedString (Property)
			///////////////////////////////////////////////////////////////

			FOREACH(Ptr<GuiInstanceLocalized>, localized, context->localizeds)
			{
				if (auto lsTd = GetTypeDescriptor(localized->className.ToString()))
				{
					ITypeDescriptor* lsiTd = nullptr;
					if (auto group = lsTd->GetMethodGroupByName(L"Get", false))
					{
						vint count = group->GetMethodCount();
						for (vint i = 0; i < count; i++)
						{
							auto method = group->GetMethod(i);
							if (method->GetParameterCount() == 1)
							{
								auto paramTd = method->GetParameter(0)->GetType()->GetTypeDescriptor();
								if (paramTd == description::GetTypeDescriptor<Locale>())
								{
									lsiTd = method->GetReturn()->GetTypeDescriptor();
									break;
								}
							}
						}
					}

					if (lsiTd)
					{
						auto prop = MakePtr<WfAutoPropertyDeclaration>();
						addDecl(prop);

						prop->name.value = localized->name.ToString();
						prop->type = GetTypeFromTypeInfo(Workflow_GetSuggestedParameterType(lsiTd).Obj());
						prop->configConst = WfAPConst::Writable;
						prop->configObserve = WfAPObserve::Observable;

						auto localeNameExpr = MakePtr<WfStringExpression>();
						localeNameExpr->value.value = L"en-US";

						auto defaultLocalExpr = MakePtr<WfTypeCastingExpression>();
						defaultLocalExpr->strategy = WfTypeCastingStrategy::Strong;
						defaultLocalExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<Locale>::CreateTypeInfo().Obj());
						defaultLocalExpr->expression = localeNameExpr;

						auto getExpr = MakePtr<WfChildExpression>();
						getExpr->parent = GetExpressionFromTypeDescriptor(lsTd);
						getExpr->name.value = L"Get";

						auto callExpr = MakePtr<WfCallExpression>();
						callExpr->function = getExpr;
						callExpr->arguments.Add(defaultLocalExpr);
						prop->expression = callExpr;
					}
					else
					{
						errors.Add(GuiResourceError({ resolvingResult.resource }, localized->classPosition,
							L"Precompile: Class \"" +
							localized->className.ToString() +
							L"\" of localized strings \"" +
							localized->name.ToString() +
							L"\" is not a correct localized strings class."));
					}
				}
				else
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, localized->classPosition,
						L"Precompile: Class \"" +
						localized->className.ToString() +
						L"\" of localized strings \"" +
						localized->name.ToString() +
						L"\" cannot be found."));
				}
			}

			///////////////////////////////////////////////////////////////
			// ref.Parameter (Variable, Getter, CtorArgument)
			///////////////////////////////////////////////////////////////

			FOREACH(Ptr<GuiInstanceParameter>, parameter, context->parameters)
			{
				auto parameterTypeInfoTuple = getDefaultType(parameter->className.ToString());
				vint errorCount = errors.Count();
				auto type = Workflow_ParseType(precompileContext, { resolvingResult.resource }, parameterTypeInfoTuple.f1, parameter->classPosition, errors);

				if (!needFunctionBody && !parameterTypeInfoTuple.f0 && errorCount == errors.Count())
				{
					if (!type || type.Cast<WfReferenceType>() || type.Cast<WfChildType>() || type.Cast<WfTopQualifiedType>())
					{
						type = Workflow_ParseType(precompileContext, { resolvingResult.resource }, parameter->className.ToString() + L"*", parameter->classPosition, errors);
					}
				}
				if (type)
				{
					if (needFunctionBody)
					{
						auto decl = MakePtr<WfVariableDeclaration>();
						addDecl(decl);

						decl->name.value = L"<parameter>" + parameter->name.ToString();
						decl->type = CopyType(type);
						decl->expression = CreateDefaultValue(parameterTypeInfoTuple.f0.Obj());

						Workflow_RecordScriptPosition(precompileContext, parameter->tagPosition, (Ptr<WfDeclaration>)decl);
					}
					{
						auto decl = MakePtr<WfFunctionDeclaration>();
						addDecl(decl);

						decl->anonymity = WfFunctionAnonymity::Named;
						decl->name.value = L"Get" + parameter->name.ToString();
						decl->returnType = CopyType(type);
						if (needFunctionBody)
						{
							auto block = MakePtr<WfBlockStatement>();
							decl->statement = block;

							auto ref = MakePtr<WfReferenceExpression>();
							ref->name.value = L"<parameter>" + parameter->name.ToString();

							auto returnStat = MakePtr<WfReturnStatement>();
							returnStat->expression = ref;
							block->statements.Add(returnStat);
						}
						else
						{
							decl->statement = notImplemented();
						}

						Workflow_RecordScriptPosition(precompileContext, parameter->tagPosition, (Ptr<WfDeclaration>)decl);
					}
					{
						auto decl = MakePtr<WfPropertyDeclaration>();
						addDecl(decl);

						decl->name.value = parameter->name.ToString();
						decl->type = type;
						decl->getter.value = L"Get" + parameter->name.ToString();

						Workflow_RecordScriptPosition(precompileContext, parameter->tagPosition, (Ptr<WfDeclaration>)decl);
					}
					{
						auto argument = MakePtr<WfFunctionArgument>();
						argument->name.value = L"<ctor-parameter>" + parameter->name.ToString();
						argument->type = CopyType(type);
						ctor->arguments.Add(argument);
					}
					if (needFunctionBody)
					{
						auto refLeft = MakePtr<WfReferenceExpression>();
						refLeft->name.value = L"<parameter>" + parameter->name.ToString();

						auto refRight = MakePtr<WfReferenceExpression>();
						refRight->name.value = L"<ctor-parameter>" + parameter->name.ToString();

						auto assignExpr = MakePtr<WfBinaryExpression>();
						assignExpr->op = WfBinaryOperator::Assign;
						assignExpr->first = refLeft;
						assignExpr->second = refRight;

						auto exprStat = MakePtr<WfExpressionStatement>();
						exprStat->expression = assignExpr;

						ctorBlock->statements.Add(exprStat);

						Workflow_RecordScriptPosition(precompileContext, parameter->tagPosition, (Ptr<WfStatement>)exprStat);
					}
				}
			}

			///////////////////////////////////////////////////////////////
			// Event Handlers
			///////////////////////////////////////////////////////////////

			if (needEventHandler)
			{
				List<types::PropertyResolving> infos;
				WorkflowEventNamesVisitor visitor(precompileContext, resolvingResult, infos, instanceClass, errors);
				context->instance->Accept(&visitor);
			}

			addDecl(ctor);

			///////////////////////////////////////////////////////////////
			// Calling Constructor Class
			///////////////////////////////////////////////////////////////

			if (needFunctionBody)
			{
				{
					auto getRmExpr = MakePtr<WfChildExpression>();
					getRmExpr->parent = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<IGuiResourceManager>());
					getRmExpr->name.value = L"GetResourceManager";

					auto call1Expr = MakePtr<WfCallExpression>();
					call1Expr->function = getRmExpr;

					auto getResExpr = MakePtr<WfMemberExpression>();
					getResExpr->parent = call1Expr;
					getResExpr->name.value = L"GetResourceFromClassName";

					auto classNameExpr = MakePtr<WfStringExpression>();
					classNameExpr->value.value = context->className;

					auto call2Expr = MakePtr<WfCallExpression>();
					call2Expr->function = getResExpr;
					call2Expr->arguments.Add(classNameExpr);

					auto varDecl = MakePtr<WfVariableDeclaration>();
					varDecl->name.value = L"<resource>";
					varDecl->expression = call2Expr;

					auto varStat = MakePtr<WfVariableStatement>();
					varStat->variable = varDecl;

					ctorBlock->statements.Add(varStat);
				}
				{
					auto resRef = MakePtr<WfReferenceExpression>();
					resRef->name.value = L"<resource>";

					auto resRef2 = MakePtr<WfReferenceExpression>();
					resRef2->name.value = L"<resource>";

					auto wdRef = MakePtr<WfMemberExpression>();
					wdRef->parent = resRef2;
					wdRef->name.value = L"WorkingDirectory";

					auto newClassExpr = MakePtr<WfNewClassExpression>();
					newClassExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<GuiResourcePathResolver>>::CreateTypeInfo().Obj());
					newClassExpr->arguments.Add(resRef);
					newClassExpr->arguments.Add(wdRef);

					auto varDecl = MakePtr<WfVariableDeclaration>();
					varDecl->name.value = L"<resolver>";
					varDecl->expression = newClassExpr;

					auto varStat = MakePtr<WfVariableStatement>();
					varStat->variable = varDecl;

					ctorBlock->statements.Add(varStat);
				}
				{
					auto setRef = MakePtr<WfMemberExpression>();
					setRef->parent = MakePtr<WfThisExpression>();
					setRef->name.value = L"SetResourceResolver";

					auto resolverRef = MakePtr<WfReferenceExpression>();
					resolverRef->name.value = L"<resolver>";

					auto callExpr = MakePtr<WfCallExpression>();
					callExpr->function = setRef;
					callExpr->arguments.Add(resolverRef);

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = callExpr;

					ctorBlock->statements.Add(stat);
				}
				{
					auto initRef = MakePtr<WfMemberExpression>();
					initRef->parent = MakePtr<WfThisExpression>();
					{
						List<WString> fragments;
						SplitTypeName(resolvingResult.context->className, fragments);
						initRef->name.value = L"<" + From(fragments).Aggregate([](const WString& a, const WString& b) {return a + L"-" + b; }) + L">Initialize";
					}

					auto callExpr = MakePtr<WfCallExpression>();
					callExpr->function = initRef;
					callExpr->arguments.Add(MakePtr<WfThisExpression>());

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = callExpr;

					ctorBlock->statements.Add(stat);
				}
			}

			///////////////////////////////////////////////////////////////
			// ref.Ctor
			///////////////////////////////////////////////////////////////

			if (context->ctorScript != L"")
			{
				if (auto stat = Workflow_ParseStatement(precompileContext, { resolvingResult.resource }, context->ctorScript, context->ctorPosition, errors))
				{
					if (needFunctionBody)
					{
						if (!stat.Cast<WfBlockStatement>())
						{
							auto block = MakePtr<WfBlockStatement>();
							block->statements.Add(stat);
							stat = block;
						}

						auto decl = MakePtr<WfFunctionDeclaration>();
						decl->anonymity = WfFunctionAnonymity::Named;
						decl->name.value = L"<instance-ctor>";
						decl->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());
						decl->statement = stat;
						addDecl(decl);

						{
							auto refCtor = MakePtr<WfReferenceExpression>();
							refCtor->name.value = L"<instance-ctor>";

							auto callExpr = MakePtr<WfCallExpression>();
							callExpr->function = refCtor;

							auto exprStat = MakePtr<WfExpressionStatement>();
							exprStat->expression = callExpr;
							ctorBlock->statements.Add(exprStat);
						}
					}
				}
			}

			///////////////////////////////////////////////////////////////
			// Destructor
			///////////////////////////////////////////////////////////////

			auto dtor = MakePtr<WfDestructorDeclaration>();
			auto dtorBlock = MakePtr<WfBlockStatement>();
			dtor->statement = dtorBlock;

			///////////////////////////////////////////////////////////////
			// ref.Dtor
			///////////////////////////////////////////////////////////////

			if (context->dtorScript != L"")
			{
				if (auto stat = Workflow_ParseStatement(precompileContext, { resolvingResult.resource }, context->dtorScript, context->dtorPosition, errors))
				{
					if (needFunctionBody)
					{
						if (!stat.Cast<WfBlockStatement>())
						{
							auto block = MakePtr<WfBlockStatement>();
							block->statements.Add(stat);
							stat = block;
						}

						auto decl = MakePtr<WfFunctionDeclaration>();
						decl->anonymity = WfFunctionAnonymity::Named;
						decl->name.value = L"<instance-dtor>";
						decl->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());
						decl->statement = stat;
						addDecl(decl);

						{
							auto refDtor = MakePtr<WfReferenceExpression>();
							refDtor->name.value = L"<instance-dtor>";

							auto callExpr = MakePtr<WfCallExpression>();
							callExpr->function = refDtor;

							auto exprStat = MakePtr<WfExpressionStatement>();
							exprStat->expression = callExpr;
							dtorBlock->statements.Add(exprStat);
						}
					}
				}
			}

			///////////////////////////////////////////////////////////////
			// Clear Binding Subscriptions
			///////////////////////////////////////////////////////////////

			{
				auto ref = MakePtr<WfReferenceExpression>();
				ref->name.value = L"FinalizeGeneralInstance";

				Ptr<WfExpression> thisExpr = MakePtr<WfThisExpression>();
				ITypeDescriptor* types[] =
				{
					description::GetTypeDescriptor<GuiTemplate>(),
					description::GetTypeDescriptor<GuiCustomControl>(),
					description::GetTypeDescriptor<GuiControlHost>(),
				};

				if (!baseType)
				{
					auto currentContext = context;
					while (!baseType)
					{
						auto item = FindInstanceResourceItem(currentContext, currentContext->instance.Obj(), classNameRecord);
						if (!item) break;

						currentContext = item->GetContent().Cast<GuiInstanceContext>();
						auto source = FindInstanceLoadingSource(currentContext, currentContext->instance.Obj());
						baseType = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName);
					}
				}

				if (baseType)
				{
					for (auto td : types)
					{
						if (baseType->GetTypeDescriptor()->CanConvertTo(td))
						{
							ref->name.value = L"FinalizeInstanceRecursively";

							Ptr<ITypeInfo> typeInfo = MakePtr<TypeDescriptorTypeInfo>(td, TypeInfoHint::Normal);
							typeInfo = MakePtr<RawPtrTypeInfo>(typeInfo);

							auto inferExpr = MakePtr<WfInferExpression>();
							inferExpr->type = GetTypeFromTypeInfo(typeInfo.Obj());
							inferExpr->expression = thisExpr;
							thisExpr = inferExpr;
							break;
						}
					}
				}

				auto call = MakePtr<WfCallExpression>();
				call->function = ref;
				call->arguments.Add(thisExpr);

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = call;
				dtorBlock->statements.Add(stat);
			}

			addDecl(dtor);

			return module;
		}

/***********************************************************************
GuiWorkflowSharedManagerPlugin
***********************************************************************/

#undef ERROR_CODE_PREFIX

		class GuiWorkflowSharedManagerPlugin;
		GuiWorkflowSharedManagerPlugin* sharedManagerPlugin = 0;

		class GuiWorkflowSharedManagerPlugin : public Object, public IGuiPlugin
		{
		protected:
			Ptr<WfLexicalScopeManager>		workflowManager;

		public:

			GUI_PLUGIN_NAME(GacUI_Compiler_WorkflowSharedManager)
			{
			}

			void Load()override
			{
				sharedManagerPlugin = this;
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

			Ptr<WfLexicalScopeManager> TransferWorkflowManager()
			{
				auto result = workflowManager;
				workflowManager = nullptr;
				return result;
			}
		};
		GUI_REGISTER_PLUGIN(GuiWorkflowSharedManagerPlugin)

		WfLexicalScopeManager* Workflow_GetSharedManager()
		{
			return sharedManagerPlugin->GetWorkflowManager();
		}

		Ptr<WfLexicalScopeManager> Workflow_TransferSharedManager()
		{
			return sharedManagerPlugin->TransferWorkflowManager();
		}
	}
}
