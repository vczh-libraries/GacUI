#include "GuiInstanceLoader.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"
#include "../Controls/GuiApplication.h"
#include "../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;
		using namespace parsing;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace controls;
		using namespace stream;

/***********************************************************************
GuiResourceInstanceBinder (uri)
***********************************************************************/

		class GuiResourceInstanceBinder : public Object, public IGuiInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Uri;
			}

			bool ApplicableToConstructorArgument()override
			{
				return true;
			}

			bool RequirePropertyExist()override
			{
				return false;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				WString protocol, path;
				if (!IsResourceUrl(code, protocol, path))
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, position, L"Precompile: \"" + code + L"\" is not a valid resource uri."));
					return nullptr;
				}
				else if (!precompileContext.resolver->ResolveResource(protocol, path))
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, position, L"Precompile: Resource \"" + code + L"\" does not exist."));
					return nullptr;
				}
				else
				{
					return Workflow_GetUriProperty(precompileContext, resolvingResult, loader, prop, propInfo, protocol, path, position, errors);
				}
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				WString protocol, path;
				if (!IsResourceUrl(code, protocol, path))
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, position, L"Precompile: \"" + code + L"\" is not a valid resource uri."));
					return nullptr;
				}
				else if (!precompileContext.resolver->ResolveResource(protocol, path))
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, position, L"Precompile: Resource \"" + code + L"\" does not exist."));
					return nullptr;
				}
				else
				{
					return Workflow_InstallUriProperty(precompileContext, resolvingResult, variableName, loader, prop, propInfo, protocol, path, position, errors);
				}
			}
		};

/***********************************************************************
GuiReferenceInstanceBinder (ref)
***********************************************************************/

		class GuiReferenceInstanceBinder : public Object, public IGuiInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Ref;
			}

			bool ApplicableToConstructorArgument()override
			{
				return false;
			}

			bool RequirePropertyExist()override
			{
				return false;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiReferenceInstanceBinder::GenerateConstructorArgument()#This binder does not support binding to constructor arguments. Please call ApplicableToConstructorArgument() to determine before calling this function.");
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				auto expression = MakePtr<WfReferenceExpression>();
				expression->name.value = code;
				return Workflow_InstallEvalProperty(precompileContext, resolvingResult, variableName, loader, prop, propInfo, expression, position, errors);
			}
		};

/***********************************************************************
GuiEvalInstanceBinder (eval)
***********************************************************************/

		class GuiEvalInstanceBinder : public Object, public IGuiInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Eval;
			}

			bool ApplicableToConstructorArgument()override
			{
				return true;
			}

			bool RequirePropertyExist()override
			{
				return false;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				return Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, code, position, errors);
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				if(auto expression = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, code, position, errors))
				{
					return Workflow_InstallEvalProperty(precompileContext, resolvingResult, variableName, loader, prop, propInfo, expression, position, errors);
				}
				return nullptr;
			}
		};

/***********************************************************************
GuiBindInstanceBinder (bind)
***********************************************************************/

		class GuiBindInstanceBinder : public Object, public IGuiInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Bind;
			}

			bool ApplicableToConstructorArgument()override
			{
				return false;
			}

			bool RequirePropertyExist()override
			{
				return true;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiBindInstanceBinder::GenerateConstructorArgument()#This binder does not support binding to constructor arguments. Please call ApplicableToConstructorArgument() to determine before calling this function.");
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				if(auto expression = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, code, position, errors))
				{
					auto inferExpr = MakePtr<WfInferExpression>();
					inferExpr->expression = expression;
					inferExpr->type = GetTypeFromTypeInfo(propertyInfo->GetReturn());

					auto bindExpr = MakePtr<WfBindExpression>();
					bindExpr->expression = inferExpr;

					return Workflow_InstallBindProperty(precompileContext, resolvingResult, variableName, propertyInfo, bindExpr);
				}
				return nullptr;
			}
		};

/***********************************************************************
GuiFormatInstanceBinder (format)
***********************************************************************/

		class GuiFormatInstanceBinder : public Object, public IGuiInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Format;
			}

			bool ApplicableToConstructorArgument()override
			{
				return false;
			}

			bool RequirePropertyExist()override
			{
				return true;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiFormatInstanceBinder::GenerateConstructorArgument()#This binder does not support binding to constructor arguments. Please call ApplicableToConstructorArgument() to determine before calling this function.");
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				if (auto expression = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, L"bind($\"" + code + L"\")", position, errors, { 0,7 })) // bind($"
				{
					return Workflow_InstallBindProperty(precompileContext, resolvingResult, variableName, propertyInfo, expression);
				}
				return nullptr;
			}
		};

/***********************************************************************
GuiLocalizedStringInstanceBinder (str)
***********************************************************************/

		class GuiLocalizedStringInstanceBinder : public Object, public IGuiInstanceBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Str;
			}

			bool ApplicableToConstructorArgument()override
			{
				return false;
			}

			bool RequirePropertyExist()override
			{
				return true;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiLocalizedStringInstanceBinder::GenerateConstructorArgument()#This binder does not support binding to constructor arguments. Please call ApplicableToConstructorArgument() to determine before calling this function.");
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				if (auto expression = Workflow_ParseExpression(precompileContext, { resolvingResult.resource }, code, position, errors, { 0,0 }))
				{
					vint errorCount = errors.Count();
					if (auto callExpr = expression.Cast<WfCallExpression>())
					{
						if (auto refExpr = callExpr->function.Cast<WfReferenceExpression>())
						{
							auto defaultLs=From(resolvingResult.context->localizeds)
								.Where([](Ptr<GuiInstanceLocalized> ls)
								{
									return ls->defaultStrings;
								})
								.First(nullptr);

							if (defaultLs)
							{
								auto thisExpr = MakePtr<WfReferenceExpression>();
								thisExpr->name.value = L"<this>";
								thisExpr->codeRange = refExpr->codeRange;

								auto thisMember = MakePtr<WfMemberExpression>();
								thisMember->parent = thisExpr;
								thisMember->name.value = defaultLs->name.ToString();
								thisMember->codeRange = refExpr->codeRange;

								auto refMember = MakePtr<WfMemberExpression>();
								refMember->parent = thisMember;
								refMember->name.value = refExpr->name.value;
								refMember->codeRange = refExpr->codeRange;

								callExpr->function = refMember;
								goto PASSED;
							}
							else
							{
								errors.Add({ position,L"Precompiled: Omitting the name of the localized strings requires specifying a default one in <ref.LocalizedStrings> by adding a Default=\"true\" attribute." });
							}
						}
						else if (auto memberExpr = callExpr->function.Cast<WfMemberExpression>())
						{
							if (auto refStrings = memberExpr->parent.Cast<WfReferenceExpression>())
							{
								auto thisExpr = MakePtr<WfReferenceExpression>();
								thisExpr->name.value = L"<this>";
								thisExpr->codeRange = refStrings->codeRange;

								auto thisMember = MakePtr<WfMemberExpression>();
								thisMember->parent = thisExpr;
								thisMember->name.value = refStrings->name.value;
								thisMember->codeRange = refStrings->codeRange;

								memberExpr->parent = thisMember;
								goto PASSED;
							}
						}
						
						errors.Add({ position,L"Precompiled: The function expression in binding \"-str\" should be a \"<string-name>\" or \"<localized-strings-name>.<string-name>\"." });
					PASSED:;
					}
					else
					{
						errors.Add({ position,L"Precompiled: Expression in binding \"-str\" should be a function call expression." });
					}

					if (errorCount == errors.Count())
					{
						auto bindExpr = MakePtr<WfBindExpression>();
						bindExpr->expression = expression;
						bindExpr->codeRange = expression->codeRange;

						return Workflow_InstallBindProperty(precompileContext, resolvingResult, variableName, propertyInfo, bindExpr);
					}
				}
				return nullptr;
			}
		};

/***********************************************************************
GuiEvalInstanceEventBinder (eval)
***********************************************************************/

		class GuiEvalInstanceEventBinder : public Object, public IGuiInstanceEventBinder
		{
		public:
			GlobalStringKey GetBindingName()override
			{
				return GlobalStringKey::_Eval;
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				bool coroutine = false;
				{
					auto reading = code.Buffer();
					while (true)
					{
						switch (*reading)
						{
						case ' ':
						case '\t':
						case '\r':
						case '\n':
							reading++;
							break;
						default:
							goto BEGIN_TESTING;
						}
					}
				BEGIN_TESTING:
					coroutine = *reading == '$';
				}

				auto parseFunction = coroutine ? &Workflow_ParseCoProviderStatement : &Workflow_ParseStatement;
				if (auto statement = parseFunction(precompileContext, { resolvingResult.resource }, code, position, errors, { 0,0 }))
				{
					return Workflow_InstallEvalEvent(precompileContext, resolvingResult, variableName, eventInfo, statement);
				}
				return nullptr;
			}
		};

/***********************************************************************
GuiPredefinedInstanceBindersPlugin
***********************************************************************/

		class GuiPredefinedInstanceBindersPlugin : public Object, public IGuiPlugin
		{
		public:

			GUI_PLUGIN_NAME(GacUI_Compiler_ParsersAndBinders)
			{
				GUI_PLUGIN_DEPEND(GacUI_Parser);
				GUI_PLUGIN_DEPEND(GacUI_Res_ResourceResolver);
				GUI_PLUGIN_DEPEND(GacUI_Instance);
				GUI_PLUGIN_DEPEND(GacUI_Instance_Reflection);
			}

			void Load()override
			{
				WfLoadTypes();
				GuiIqLoadTypes();
				{
					IGuiParserManager* manager = GetParserManager();
					manager->SetParsingTable(L"WORKFLOW", &WfLoadTable);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-TYPE", &WfParseType);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-EXPRESSION", &WfParseExpression);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-STATEMENT", &WfParseStatement);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-COPROVIDER-STATEMENT", &WfParseCoProviderStatement);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-DECLARATION", &WfParseDeclaration);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-MODULE", &WfParseModule);
					manager->SetParsingTable(L"INSTANCE-QUERY", &GuiIqLoadTable);
					manager->SetTableParser(L"INSTANCE-QUERY", L"INSTANCE-QUERY", &GuiIqParse);
				}
				{
					IGuiInstanceLoaderManager* manager=GetInstanceLoaderManager();

					manager->AddInstanceBinder(new GuiResourceInstanceBinder);
					manager->AddInstanceBinder(new GuiReferenceInstanceBinder);
					manager->AddInstanceBinder(new GuiEvalInstanceBinder);
					manager->AddInstanceBinder(new GuiBindInstanceBinder);
					manager->AddInstanceBinder(new GuiFormatInstanceBinder);
					manager->AddInstanceBinder(new GuiLocalizedStringInstanceBinder);
					manager->AddInstanceEventBinder(new GuiEvalInstanceEventBinder);
				}
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceBindersPlugin)
	}
}
