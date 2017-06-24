#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/GuiInstanceCompiledWorkflow.h"

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

/***********************************************************************
FindInstanceLoadingSource
***********************************************************************/

		InstanceLoadingSource FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GlobalStringKey namespaceName, const WString& typeName)
		{
			vint index = context->namespaces.Keys().IndexOf(namespaceName);
			if (index != -1)
			{
				Ptr<GuiInstanceContext::NamespaceInfo> namespaceInfo = context->namespaces.Values()[index];
				FOREACH(Ptr<GuiInstanceNamespace>, ns, namespaceInfo->namespaces)
				{
					auto fullName = GlobalStringKey::Get(ns->prefix + typeName + ns->postfix);
					if (auto loader = GetInstanceLoaderManager()->GetLoader(fullName))
					{
						return InstanceLoadingSource(loader, fullName);
					}
				}
			}
			return InstanceLoadingSource();
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
			, public empty_visitor::VirtualDeclarationVisitor
		{
		public:
			Func<Ptr<WfStatement>()>			statCtor;
			List<Ptr<WfDeclaration>>&			unprocessed;

			ReplaceDeclImplVisitor(Func<Ptr<WfStatement>()> _statCtor, List<Ptr<WfDeclaration>>& _unprocessed)
				:statCtor(_statCtor)
				, unprocessed(_unprocessed)
			{
			}

			virtual void Dispatch(WfVirtualDeclaration* node)override
			{
				node->Accept(static_cast<WfVirtualDeclaration::IVisitor*>(this));
			}

			virtual void Visit(WfFunctionDeclaration* node)override
			{
				node->statement = statCtor();
			}

			virtual void Visit(WfConstructorDeclaration* node)override
			{
				node->statement = statCtor();
			}

			virtual void Visit(WfDestructorDeclaration* node)override
			{
				node->statement = statCtor();
			}

			virtual void Visit(WfClassDeclaration* node)override
			{
				CopyFrom(unprocessed, node->declarations, true);
			}
		};

		Ptr<workflow::WfModule> Workflow_GenerateInstanceClass(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors, vint passIndex)
		{
			bool beforePrecompile = false;
			bool needEventHandler = false;
			switch (passIndex)
			{
			case IGuiResourceTypeResolver_Precompile::Instance_CollectInstanceTypes:
				beforePrecompile = true;
				needEventHandler = false;
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_CollectEventHandlers:
				beforePrecompile = true;
				needEventHandler = true;
				break;
			case IGuiResourceTypeResolver_Precompile::Instance_GenerateInstanceClass:
				beforePrecompile = false;
				needEventHandler = true;
				break;
			}

			auto context = resolvingResult.context;
			auto source = FindInstanceLoadingSource(context, context->instance.Obj());
			auto baseType = GetInstanceLoaderManager()->GetTypeInfoForType(source.typeName);
			if (!baseType)
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

			///////////////////////////////////////////////////////////////
			// Instance Class
			///////////////////////////////////////////////////////////////

			auto module = Workflow_CreateModuleWithUsings(context, moduleName);
			auto instanceClass = Workflow_InstallClass(context->className, module);
			{
				auto typeInfo = MakePtr<TypeDescriptorTypeInfo>(baseType->GetTypeDescriptor(), TypeInfoHint::Normal);
				auto baseType = GetTypeFromTypeInfo(typeInfo.Obj());
				instanceClass->baseTypes.Add(baseType);

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

			if (!beforePrecompile)
			{
				auto baseType = MakePtr<WfReferenceType>();
				baseType->name.value = instanceClass->name.value + L"Constructor";
				instanceClass->baseTypes.Add(baseType);

				{
					auto value = MakePtr<WfTypeOfTypeExpression>();
					value->type = CopyType(baseType);

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
				if(auto module = Workflow_ParseModule(precompileContext, { resolvingResult.resource }, wrappedCode, position, errors, { 1,0 }))
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

			///////////////////////////////////////////////////////////////
			// ref.Members
			///////////////////////////////////////////////////////////////

			if (context->memberScript != L"")
			{
				List<Ptr<WfDeclaration>> memberDecls;
				parseClassMembers(context->memberScript, L"members of instance \"" + context->className + L"\"", memberDecls, context->memberPosition);

				if (beforePrecompile)
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
			auto ctorBlock = (beforePrecompile ? notImplemented() : MakePtr<WfBlockStatement>());
			ctor->statement = ctorBlock;

			if (auto group = baseType->GetTypeDescriptor()->GetConstructorGroup())
			{
				auto ctorInfo = group->GetMethod(0);
				vint count = ctorInfo->GetParameterCount();
				if (count > 0)
				{
					if (!beforePrecompile)
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
			// ref.Parameter (Variable, Getter, CtorArgument)
			///////////////////////////////////////////////////////////////

			FOREACH(Ptr<GuiInstanceParameter>, param, context->parameters)
			{
				bool isReferenceType = true;
				auto paramTd = GetTypeDescriptor(param->className.ToString());
				if (paramTd)
				{
					isReferenceType = (paramTd->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined;
				}

				if (auto type = Workflow_ParseType(precompileContext, { resolvingResult.resource }, param->className.ToString() + (isReferenceType ? L"^" : L""), param->classPosition, errors))
				{
					if (!beforePrecompile)
					{
						auto decl = MakePtr<WfVariableDeclaration>();
						addDecl(decl);

						decl->name.value = L"<parameter>" + param->name.ToString();
						decl->type = CopyType(type);

						if (isReferenceType)
						{
							auto nullExpr = MakePtr<WfLiteralExpression>();
							nullExpr->value = WfLiteralValue::Null;
							decl->expression = nullExpr;
						}
						else
						{
							decl->expression = CreateDefaultValue(MakePtr<TypeDescriptorTypeInfo>(paramTd, TypeInfoHint::Normal).Obj());
						}

						Workflow_RecordScriptPosition(precompileContext, param->tagPosition, (Ptr<WfDeclaration>)decl);
					}
					{
						auto decl = MakePtr<WfFunctionDeclaration>();
						addDecl(decl);

						decl->anonymity = WfFunctionAnonymity::Named;
						decl->name.value = L"Get" + param->name.ToString();
						decl->returnType = CopyType(type);
						if (!beforePrecompile)
						{
							auto block = MakePtr<WfBlockStatement>();
							decl->statement = block;

							auto ref = MakePtr<WfReferenceExpression>();
							ref->name.value = L"<parameter>" + param->name.ToString();

							auto returnStat = MakePtr<WfReturnStatement>();
							returnStat->expression = ref;
							block->statements.Add(returnStat);
						}
						else
						{
							decl->statement = notImplemented();
						}

						Workflow_RecordScriptPosition(precompileContext, param->tagPosition, (Ptr<WfDeclaration>)decl);
					}
					{
						auto decl = MakePtr<WfPropertyDeclaration>();
						addDecl(decl);

						decl->name.value = param->name.ToString();
						decl->type = type;
						decl->getter.value = L"Get" + param->name.ToString();

						Workflow_RecordScriptPosition(precompileContext, param->tagPosition, (Ptr<WfDeclaration>)decl);
					}
					{
						auto argument = MakePtr<WfFunctionArgument>();
						argument->name.value = L"<ctor-parameter>" + param->name.ToString();
						argument->type = CopyType(type);
						ctor->arguments.Add(argument);
					}
					if (!beforePrecompile)
					{
						auto refLeft = MakePtr<WfReferenceExpression>();
						refLeft->name.value = L"<parameter>" + param->name.ToString();

						auto refRight = MakePtr<WfReferenceExpression>();
						refRight->name.value = L"<ctor-parameter>" + param->name.ToString();

						auto assignExpr = MakePtr<WfBinaryExpression>();
						assignExpr->op = WfBinaryOperator::Assign;
						assignExpr->first = refLeft;
						assignExpr->second = refRight;

						auto exprStat = MakePtr<WfExpressionStatement>();
						exprStat->expression = assignExpr;

						ctorBlock->statements.Add(exprStat);

						Workflow_RecordScriptPosition(precompileContext, param->tagPosition, (Ptr<WfStatement>)exprStat);
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

			if (!beforePrecompile)
			{
				{
					auto presentationExpr = MakePtr<WfTopQualifiedExpression>();
					presentationExpr->name.value = L"presentation";

					auto rmExpr = MakePtr<WfChildExpression>();
					rmExpr->parent = presentationExpr;
					rmExpr->name.value = L"IGuiResourceManager";

					auto getRmExpr = MakePtr<WfChildExpression>();
					getRmExpr->parent = rmExpr;
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
					initRef->name.value = L"<initialize-instance>";

					auto refThis = MakePtr<WfThisExpression>();

					auto callExpr = MakePtr<WfCallExpression>();
					callExpr->function = initRef;
					callExpr->arguments.Add(refThis);

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
					if (!beforePrecompile)
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
					if (!beforePrecompile)
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
				ref->name.value = L"FinalizeInstanceRecursively";

				auto call = MakePtr<WfCallExpression>();
				call->function = ref;
				call->arguments.Add(MakePtr<WfThisExpression>());

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
			GuiWorkflowSharedManagerPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
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
