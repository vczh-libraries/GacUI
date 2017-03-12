#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../../Resources/GuiParserManager.h"

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

		InstanceLoadingSource FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor)
		{
			vint index = context->namespaces.Keys().IndexOf(ctor->typeNamespace);
			if (index != -1)
			{
				Ptr<GuiInstanceContext::NamespaceInfo> namespaceInfo = context->namespaces.Values()[index];
				FOREACH(Ptr<GuiInstanceNamespace>, ns, namespaceInfo->namespaces)
				{
					auto fullName = GlobalStringKey::Get(ns->prefix + ctor->typeName.ToString() + ns->postfix);
					if (auto loader = GetInstanceLoaderManager()->GetLoader(fullName))
					{
						return InstanceLoadingSource(loader, fullName);
					}
				}
			}
			return InstanceLoadingSource();
		}

/***********************************************************************
Workflow_PrecompileInstanceContext
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_PrecompileInstanceContext(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors)
		{
			auto module = Workflow_CreateModuleWithUsings(resolvingResult.context);
			{
				auto block = Workflow_InstallCtorClass(resolvingResult, module);
				Workflow_GenerateCreating(resolvingResult, block, errors);
				Workflow_GenerateBindings(resolvingResult, block, errors);
			}
			return module;
		}

/***********************************************************************
WorkflowEventNamesVisitor
***********************************************************************/

		class WorkflowEventNamesVisitor : public Object, public GuiValueRepr::IVisitor
		{
		public:
			types::ResolvingResult&				resolvingResult;
			Ptr<WfClassDeclaration>				instanceClass;
			GuiResourceError::List&				errors;
			IGuiInstanceLoader::TypeInfo		resolvedTypeInfo;

			WorkflowEventNamesVisitor(types::ResolvingResult& _resolvingResult, Ptr<WfClassDeclaration> _instanceClass, GuiResourceError::List& _errors)
				:resolvingResult(_resolvingResult)
				, instanceClass(_instanceClass)
				, errors(_errors)
			{
			}

			void Visit(GuiTextRepr* repr)override
			{
			}

			void Visit(GuiAttSetterRepr* repr)override
			{
				FOREACH_INDEXER(Ptr<GuiAttSetterRepr::SetterValue>, setter, index, repr->setters.Values())
				{
					if (setter->binding == GlobalStringKey::_Set)
					{
						auto prop = repr->setters.Keys()[index];
						IGuiInstanceLoader::PropertyInfo propertyInfo(resolvedTypeInfo, prop);
						auto errorPrefix = L"Precompile: Property \"" + propertyInfo.propertyName.ToString() + L"\" of type \"" + resolvedTypeInfo.typeName.ToString() + L"\"";

						auto loader = GetInstanceLoaderManager()->GetLoader(resolvedTypeInfo.typeName);
						auto currentLoader = loader;
						IGuiInstanceLoader::TypeInfo propTypeInfo;

						while (currentLoader)
						{
							if (auto propertyTypeInfo = currentLoader->GetPropertyType(propertyInfo))
							{
								if (propertyTypeInfo->support == GuiInstancePropertyInfo::NotSupport)
								{
									errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" is not supported."));
									goto SKIP_SET;
								}
								else
								{
									if (propertyTypeInfo->support == GuiInstancePropertyInfo::SupportSet)
									{
										propTypeInfo.typeDescriptor = propertyTypeInfo->acceptableTypes[0];
										propTypeInfo.typeName = GlobalStringKey::Get(propTypeInfo.typeDescriptor->GetTypeName());
									}
									else
									{
										errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, errorPrefix + L" does not support the \"-set\" binding."));
										goto SKIP_SET;
									}
								}

								if (!propertyTypeInfo->tryParent)
								{
									break;
								}
							}
							currentLoader = GetInstanceLoaderManager()->GetParentLoader(currentLoader);
						}

						if (propTypeInfo.typeDescriptor)
						{
							auto oldTypeInfo = resolvedTypeInfo;
							resolvedTypeInfo = propTypeInfo;
							FOREACH(Ptr<GuiValueRepr>, value, setter->values)
							{
								value->Accept(this);
							}
							resolvedTypeInfo = oldTypeInfo;
						}
						else
						{
							errors.Add(GuiResourceError({ resolvingResult.resource }, setter->attPosition, L"[INTERNAL ERROR]" + errorPrefix + L" does not exist."));
						}
					SKIP_SET:;
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
					if (handler->binding == GlobalStringKey::Empty)
					{
						auto propertyName = repr->eventHandlers.Keys()[index];
						if (auto eventInfo = resolvedTypeInfo.typeDescriptor->GetEventByName(propertyName.ToString(), true))
						{
							auto decl = Workflow_GenerateEventHandler(eventInfo);
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
							instanceClass->declarations.Add(decl);
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
					}
				}
			}

			void Visit(GuiConstructorRepr* repr)override
			{
				IGuiInstanceLoader::TypeInfo reprTypeInfo;

				auto context = resolvingResult.context;
				if (repr == context->instance.Obj())
				{
					auto fullName = GlobalStringKey::Get(context->className);
					if (auto reprTd = GetInstanceLoaderManager()->GetTypeDescriptorForType(fullName))
					{
						reprTypeInfo.typeName = fullName;
						reprTypeInfo.typeDescriptor = reprTd;
					}
				}

				if (!reprTypeInfo.typeDescriptor)
				{
					auto source = FindInstanceLoadingSource(context, repr);
					reprTypeInfo.typeName = source.typeName;
					reprTypeInfo.typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				}

				if (reprTypeInfo.typeDescriptor)
				{
					auto oldTypeInfo = resolvedTypeInfo;
					resolvedTypeInfo = reprTypeInfo;
					Visit((GuiAttSetterRepr*)repr);
					resolvedTypeInfo = oldTypeInfo;
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

		Ptr<workflow::WfModule> Workflow_GenerateInstanceClass(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors, vint passIndex)
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
			auto baseTd = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
			if (!baseTd)
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

			auto module = Workflow_CreateModuleWithUsings(context);
			auto instanceClass = Workflow_InstallClass(context->className, module);
			{
				auto typeInfo = MakePtr<TypeDescriptorTypeInfo>(baseTd, TypeInfoHint::Normal);
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

			auto parseClassMembers = [&](const WString& code, const WString& name, List<Ptr<WfDeclaration>>& memberDecls, GuiResourceTextPos position)
			{
				WString wrappedCode = L"module parse_members; class Class {\r\n" + code + L"\r\n}";
				if(auto module = Workflow_ParseModule({ resolvingResult.resource }, wrappedCode, position, errors, { 1,0 }))
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

			auto ctor = MakePtr<WfConstructorDeclaration>();
			ctor->constructorType = WfConstructorType::RawPtr;
			auto ctorBlock = (beforePrecompile ? notImplemented() : MakePtr<WfBlockStatement>());
			ctor->statement = ctorBlock;

			if (auto group = baseTd->GetConstructorGroup())
			{
				vint count = group->GetMethod(0)->GetParameterCount();
				if (count > 0)
				{
					if (!beforePrecompile)
					{
						if (auto call = resolvingResult.rootLoader->CreateRootInstance(resolvingResult, resolvingResult.rootTypeInfo, resolvingResult.rootCtorArguments, errors))
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
							auto nullExpr = MakePtr<WfLiteralExpression>();
							nullExpr->value = WfLiteralValue::Null;
							call->arguments.Add(nullExpr);
						}
					}
				}
			}

			FOREACH(Ptr<GuiInstanceParameter>, param, context->parameters)
			{
				if (auto type = Workflow_ParseType({ resolvingResult.resource }, param->className.ToString() + L"^", param->classPosition, errors))
				{
					if (!beforePrecompile)
					{
						auto decl = MakePtr<WfVariableDeclaration>();
						addDecl(decl);
						
						decl->name.value = L"<parameter>" + param->name.ToString();
						decl->type = CopyType(type);

						auto nullExpr = MakePtr<WfLiteralExpression>();
						nullExpr->value = WfLiteralValue::Null;
						decl->expression = nullExpr;
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
					}
					{
						auto decl = MakePtr<WfPropertyDeclaration>();
						addDecl(decl);

						decl->name.value = param->name.ToString();
						decl->type = type;
						decl->getter.value = L"Get" + param->name.ToString();
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
					}
				}
			}

			if (needEventHandler)
			{
				WorkflowEventNamesVisitor visitor(resolvingResult, instanceClass, errors);
				context->instance->Accept(&visitor);
			}
			addDecl(ctor);

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
					auto ctorRef = MakePtr<WfThisExpression>();

					auto initRef = MakePtr<WfMemberExpression>();
					initRef->parent = ctorRef;
					initRef->name.value = L"<initialize-instance>";

					auto refThis = MakePtr<WfThisExpression>();

					auto resolverRef = MakePtr<WfReferenceExpression>();
					resolverRef->name.value = L"<resolver>";

					auto castExpr = MakePtr<WfTypeCastingExpression>();
					castExpr->strategy = WfTypeCastingStrategy::Strong;
					castExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<GuiResourcePathResolver*>::CreateTypeInfo().Obj());
					castExpr->expression = resolverRef;

					auto callExpr = MakePtr<WfCallExpression>();
					callExpr->function = initRef;
					callExpr->arguments.Add(refThis);
					callExpr->arguments.Add(castExpr);

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = callExpr;

					ctorBlock->statements.Add(stat);
				}
			}

			{
				auto dtor = MakePtr<WfDestructorDeclaration>();
				addDecl(dtor);

				auto block = MakePtr<WfBlockStatement>();
				dtor->statement = block;

				auto ref = MakePtr<WfReferenceExpression>();
				ref->name.value = L"ClearSubscriptions";

				auto call = MakePtr<WfCallExpression>();
				call->function = ref;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = call;
				block->statements.Add(stat);
			}

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
