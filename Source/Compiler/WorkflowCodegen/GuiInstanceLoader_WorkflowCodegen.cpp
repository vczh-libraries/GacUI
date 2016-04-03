#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;

		using namespace controls;
		using namespace compositions;

/***********************************************************************
Workflow_ValidateStatement
***********************************************************************/

		bool Workflow_ValidateStatement(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, types::ErrorList& errors, const WString& code, Ptr<workflow::WfStatement> statement)
		{
			bool failed = false;

			if (!resolvingResult.moduleForValidate)
			{
				resolvingResult.moduleForValidate = Workflow_CreateModuleWithUsings(context);
				resolvingResult.moduleContent = Workflow_InstallCtorClass(context, resolvingResult, rootTypeDescriptor, resolvingResult.moduleForValidate);
			}
			resolvingResult.moduleContent->statements.Add(statement);

			Workflow_GetSharedManager()->Clear(true, true);
			Workflow_GetSharedManager()->AddModule(resolvingResult.moduleForValidate);
			Workflow_GetSharedManager()->Rebuild(true);
			if (Workflow_GetSharedManager()->errors.Count() > 0)
			{
				errors.Add(L"Failed to analyze the workflow code \"" + code + L"\".");
				FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
				{
					errors.Add(L"    " + error->errorMessage);
				}
				failed = true;
			}

			return !failed;
		}

/***********************************************************************
Workflow_PrecompileInstanceContext (Passes)
***********************************************************************/

		extern ITypeDescriptor* Workflow_CollectReferences(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors);
		extern void Workflow_GenerateCreating(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, Ptr<WfBlockStatement> statements, types::ErrorList& errors);
		extern void Workflow_GenerateBindings(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, Ptr<WfBlockStatement> statements, types::ErrorList& errors);

/***********************************************************************
Workflow_PrecompileInstanceContext
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_PrecompileInstanceContext(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors)
		{
			vint previousErrorCount = errors.Count();
			ITypeDescriptor* rootTypeDescriptor = 0;
			if (context->className == L"")
			{
				errors.Add(
					L"Precompile: Instance  \"" +
					(context->instance->typeNamespace == GlobalStringKey::Empty
						? context->instance->typeName.ToString()
						: context->instance->typeNamespace.ToString() + L":" + context->instance->typeName.ToString()
						) +
					L"\" should have the class name specified in the ref.Class attribute.");
			}

			rootTypeDescriptor = Workflow_CollectReferences(context, resolvingResult, errors);

			if (errors.Count() == previousErrorCount)
			{
				auto module = Workflow_CreateModuleWithUsings(context);
				{
					auto block = Workflow_InstallCtorClass(context, resolvingResult, rootTypeDescriptor, module);
					Workflow_GenerateCreating(context, resolvingResult, rootTypeDescriptor, block, errors);
					Workflow_GenerateBindings(context, resolvingResult, rootTypeDescriptor, block, errors);
				}
				return module;
			}

			return nullptr;
		}

/***********************************************************************
Workflow_GenerateInstanceClass
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_GenerateInstanceClass(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors, bool beforePrecompile)
		{
			auto source = FindInstanceLoadingSource(context, context->instance.Obj());
			auto baseTd = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
			if (!baseTd)
			{
				errors.Add(
					L"Precompile: Failed to find type \"" +
					(context->instance->typeNamespace == GlobalStringKey::Empty
						? context->instance->typeName.ToString()
						: context->instance->typeNamespace.ToString() + L":" + context->instance->typeName.ToString()
						) +
					L"\".");
				return nullptr;
			}

			ITypeDescriptor* ctorTd = nullptr;
			Ptr<ITypeInfo> ctorType;
			if (!beforePrecompile)
			{
				if (ctorTd = description::GetTypeDescriptor(context->className + L"<Ctor>"))
				{
					auto elementType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
					elementType->SetTypeDescriptor(ctorTd);

					auto pointerType = MakePtr<TypeInfoImpl>(ITypeInfo::SharedPtr);
					pointerType->SetElementType(elementType);

					ctorType = pointerType;
				}
				else
				{
					errors.Add(
						L"Precompile: Builder class for type \"" +
						(context->instance->typeNamespace == GlobalStringKey::Empty
							? context->instance->typeName.ToString()
							: context->instance->typeNamespace.ToString() + L":" + context->instance->typeName.ToString()
							) +
						L"\" does not exist.");
					return nullptr;
				}
			}

			auto module = Workflow_CreateModuleWithUsings(context);
			auto instanceClass = Workflow_InstallClass(context->className, module);
			{
				auto typeInfo = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
				typeInfo->SetTypeDescriptor(baseTd);
				auto baseType = GetTypeFromTypeInfo(typeInfo.Obj());
				instanceClass->baseTypes.Add(baseType);
			}

			auto typeParser = GetParserManager()->GetParser<WfType>(L"WORKFLOW-TYPE");
			auto parseType = [typeParser, &errors](const WString& code, const WString& name)->Ptr<WfType>
			{
				List<WString> parserErrors;
				if (auto type = typeParser->TypedParse(code, parserErrors))
				{
					return type;
				}
				else
				{
					errors.Add(L"Precompile: Failed to parse " + name + L": " + code);
					return nullptr;
				}
			};
			auto addDecl = [=](Ptr<WfDeclaration> decl)
			{
				auto member = MakePtr<WfClassMember>();
				member->kind = WfClassMemberKind::Normal;
				member->declaration = decl;
				instanceClass->members.Add(member);
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

			if (!beforePrecompile)
			{
				{
					auto decl = MakePtr<WfVariableDeclaration>();
					addDecl(decl);

					decl->name.value = L"<ctor>";
					decl->type = GetTypeFromTypeInfo(ctorType.Obj());

					auto nullExpr = MakePtr<WfLiteralExpression>();
					nullExpr->value = WfLiteralValue::Null;
					decl->expression = nullExpr;
				}
				FOREACH(GlobalStringKey, name, resolvingResult.referenceNames)
				{
					auto prop = ctorTd->GetPropertyByName(name.ToString(), false);

					auto decl = MakePtr<WfVariableDeclaration>();
					addDecl(decl);

					decl->name.value = prop->GetName();
					decl->type = GetTypeFromTypeInfo(prop->GetReturn());

					auto nullExpr = MakePtr<WfLiteralExpression>();
					nullExpr->value = WfLiteralValue::Null;
					decl->expression = nullExpr;
				}
			}

			FOREACH(Ptr<GuiInstanceState>, state, context->states)
			{
				if (auto type = parseType(state->typeName, L"state \"" + state->name.ToString() + L" of instance \"" + context->className + L"\""))
				{
					auto decl = MakePtr<WfVariableDeclaration>();
					addDecl(decl);

					decl->name.value = state->name.ToString();
					decl->type = type;
					if (state->value == L"")
					{
						auto nullExpr = MakePtr<WfLiteralExpression>();
						nullExpr->value = WfLiteralValue::Null;
						decl->expression = nullExpr;
					}
					else
					{
						auto stringExpr = MakePtr<WfStringExpression>();
						stringExpr->value.value = state->value;

						auto castExpr = MakePtr<WfTypeCastingExpression>();
						castExpr->strategy = WfTypeCastingStrategy::Strong;
						castExpr->type = CopyType(type);
						castExpr->expression = stringExpr;

						decl->expression = castExpr;
					}
				}
			}

			FOREACH(Ptr<GuiInstanceProperty>, prop, context->properties)
			{
				if (auto type = parseType(prop->typeName, L"property \"" + prop->name.ToString() + L" of instance \"" + context->className + L"\""))
				{
					if (!beforePrecompile)
					{
						auto decl = MakePtr<WfVariableDeclaration>();
						addDecl(decl);

						decl->name.value = L"<property>" + prop->name.ToString();
						decl->type = CopyType(type);
					}
					{
						auto decl = MakePtr<WfFunctionDeclaration>();
						addDecl(decl);

						decl->anonymity = WfFunctionAnonymity::Named;
						decl->name.value = L"Get" + prop->name.ToString();
						decl->returnType = CopyType(type);
						decl->statement = notImplemented();
					}
					if (!prop->readonly)
					{
						auto decl = MakePtr<WfFunctionDeclaration>();
						addDecl(decl);

						decl->anonymity = WfFunctionAnonymity::Named;
						decl->name.value = L"Set" + prop->name.ToString();
						{
							auto argument = MakePtr<WfFunctionArgument>();
							argument->name.value = L"value";
							argument->type = CopyType(type);
						}
						{
							auto voidType = MakePtr<WfPredefinedType>();
							voidType->name = WfPredefinedTypeName::Void;
							decl->returnType = voidType;
						}
						decl->statement = notImplemented();
					}
					{
						auto decl = MakePtr<WfEventDeclaration>();
						addDecl(decl);

						decl->name.value = prop->name.ToString() + L"Changed";
					}
					{
						auto decl = MakePtr<WfPropertyDeclaration>();
						addDecl(decl);

						decl->name.value = prop->name.ToString();
						decl->type = type;
						decl->getter.value = L"Get" + prop->name.ToString();
						if (!prop->readonly)
						{
							decl->setter.value = L"Set" + prop->name.ToString();
						}
						decl->valueChangedEvent.value = prop->name.ToString() + L"Changed";
					}
				}
			}

			auto ctor = MakePtr<WfConstructorDeclaration>();
			ctor->constructorType = WfConstructorType::RawPtr;
			auto ctorBlock = (beforePrecompile ? notImplemented() : MakePtr<WfBlockStatement>());
			ctor->statement = ctorBlock;

			if (auto group = baseTd->GetConstructorGroup())
			{
				if (!beforePrecompile)
				{
					Ptr<WfExpression> controlTemplate;
					{
						vint index = resolvingResult.rootCtorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
						if (index != -1)
						{
							controlTemplate = resolvingResult.rootCtorArguments.GetByIndex(index)[0].expression;
						}
					}
					if (auto call = resolvingResult.rootLoader->CreateRootInstance(resolvingResult.rootTypeInfo, controlTemplate, errors))
					{
						ctor->baseConstructorCalls.Add(call);
					}
				}
				else
				{
					vint count = group->GetMethod(0)->GetParameterCount();
					if (count > 0)
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
				if (auto type = parseType(param->className.ToString() + L"^", L"parameter \"" + param->name.ToString() + L" of instance \"" + context->className + L"\""))
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
					auto newClassExpr = MakePtr<WfNewClassExpression>();
					newClassExpr->type = GetTypeFromTypeInfo(ctorType.Obj());
					
					auto ctorRef = MakePtr<WfReferenceExpression>();
					ctorRef->name.value = L"<ctor>";

					auto assignExpr = MakePtr<WfBinaryExpression>();
					assignExpr->op = WfBinaryOperator::Assign;
					assignExpr->first = ctorRef;
					assignExpr->second = newClassExpr;

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = assignExpr;

					ctorBlock->statements.Add(stat);
				}
				{
					auto ctorRef = MakePtr<WfReferenceExpression>();
					ctorRef->name.value = L"<ctor>";

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
				FOREACH(GlobalStringKey, name, resolvingResult.referenceNames)
				{
					auto propRef = MakePtr<WfReferenceExpression>();
					propRef->name.value = name.ToString();

					auto ctorRef = MakePtr<WfReferenceExpression>();
					ctorRef->name.value = L"<ctor>";

					auto ctorPropRef = MakePtr<WfMemberExpression>();
					ctorPropRef->parent = ctorRef;
					ctorPropRef->name.value =  name.ToString();

					auto assignExpr = MakePtr<WfBinaryExpression>();
					assignExpr->op = WfBinaryOperator::Assign;
					assignExpr->first = propRef;
					assignExpr->second = ctorPropRef;

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = assignExpr;

					ctorBlock->statements.Add(stat);
				}
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
		};
		GUI_REGISTER_PLUGIN(GuiWorkflowSharedManagerPlugin)

		WfLexicalScopeManager* Workflow_GetSharedManager()
		{
			return sharedManagerPlugin->GetWorkflowManager();
		}
	}
}