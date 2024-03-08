#include "GuiInstanceLoader.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"
#include "../Application/Controls/GuiApplication.h"
#include "../Resources/GuiParserManager.h"
#include "InstanceQuery/Generated/GuiInstanceQueryParser.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace instancequery;
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
				auto expression = Ptr(new WfReferenceExpression);
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
					auto inferExpr = Ptr(new WfInferExpression);
					inferExpr->expression = expression;
					inferExpr->type = GetTypeFromTypeInfo(propertyInfo->GetReturn());

					auto bindExpr = Ptr(new WfBindExpression);
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
								auto thisExpr = Ptr(new WfReferenceExpression);
								thisExpr->name.value = L"<this>";
								thisExpr->codeRange = refExpr->codeRange;

								auto thisMember = Ptr(new WfMemberExpression);
								thisMember->parent = thisExpr;
								thisMember->name.value = defaultLs->name.ToString();
								thisMember->codeRange = refExpr->codeRange;

								auto refMember = Ptr(new WfMemberExpression);
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
								auto thisExpr = Ptr(new WfReferenceExpression);
								thisExpr->name.value = L"<this>";
								thisExpr->codeRange = refStrings->codeRange;

								auto thisMember = Ptr(new WfMemberExpression);
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
						auto bindExpr = Ptr(new WfBindExpression);
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

		class GuiParser_WorkflowType : public IGuiParser<WfType>
		{
		protected:
			Ptr<workflow::Parser>						parser;

		public:
			GuiParser_WorkflowType(Ptr<workflow::Parser> _parser)
				:parser(_parser)
			{
			}

			Ptr<WfType> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(*parser.Obj(), errors);
				auto ast = ParseType(text, *parser.Obj());
				parser->OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_WorkflowExpression : public IGuiParser<WfExpression>
		{
		protected:
			Ptr<workflow::Parser>						parser;

		public:
			GuiParser_WorkflowExpression(Ptr<workflow::Parser> _parser)
				:parser(_parser)
			{
			}

			Ptr<WfExpression> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(*parser.Obj(), errors);
				auto ast = ParseExpression(text, *parser.Obj());
				parser->OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_WorkflowStatement : public IGuiParser<WfStatement>
		{
		protected:
			Ptr<workflow::Parser>						parser;

		public:
			GuiParser_WorkflowStatement(Ptr<workflow::Parser> _parser)
				:parser(_parser)
			{
			}

			Ptr<WfStatement> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(*parser.Obj(), errors);
				auto ast = ParseStatement(text, *parser.Obj());
				parser->OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_WorkflowCoProviderStatement : public IGuiParser<WfStatement>
		{
		protected:
			Ptr<workflow::Parser>						parser;

		public:
			GuiParser_WorkflowCoProviderStatement(Ptr<workflow::Parser> _parser)
				:parser(_parser)
			{
			}

			Ptr<WfStatement> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(*parser.Obj(), errors);
				auto ast = ParseCoProviderStatement(text, *parser.Obj());
				parser->OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_WorkflowDeclaration : public IGuiParser<WfDeclaration>
		{
		protected:
			Ptr<workflow::Parser>						parser;

		public:
			GuiParser_WorkflowDeclaration(Ptr<workflow::Parser> _parser)
				:parser(_parser)
			{
			}

			Ptr<WfDeclaration> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(*parser.Obj(), errors);
				auto ast = ParseDeclaration(text, *parser.Obj());
				parser->OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_WorkflowModule : public IGuiParser<WfModule>
		{
		protected:
			Ptr<workflow::Parser>						parser;

		public:
			GuiParser_WorkflowModule(Ptr<workflow::Parser> _parser)
				:parser(_parser)
			{
			}

			Ptr<WfModule> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(*parser.Obj(), errors);
				auto ast = ParseModule(text, *parser.Obj());
				parser->OnError.Remove(handler);
				return ast;
			}
		};

		class GuiParser_InstanceQuery : public IGuiParser<GuiIqQuery>
		{
		protected:
			instancequery::Parser						parser;

		public:
			Ptr<GuiIqQuery> ParseInternal(const WString& text, List<glr::ParsingError>& errors) override
			{
				auto handler = glr::InstallDefaultErrorMessageGenerator(parser, errors);
				auto ast = parser.ParseQueryRoot(text);
				parser.OnError.Remove(handler);
				return ast;
			}
		};

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

			void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
				if (controllerUnrelatedPlugins)
				{
					WorkflowAstLoadTypes();
					GuiInstanceQueryAstLoadTypes();
					{
						auto workflowParser = Ptr(new workflow::Parser);

						IGuiParserManager* manager = GetParserManager();
						manager->SetParser(L"WORKFLOW-TYPE", Ptr(new GuiParser_WorkflowType(workflowParser)));
						manager->SetParser(L"WORKFLOW-EXPRESSION", Ptr(new GuiParser_WorkflowExpression(workflowParser)));
						manager->SetParser(L"WORKFLOW-STATEMENT", Ptr(new GuiParser_WorkflowStatement(workflowParser)));
						manager->SetParser(L"WORKFLOW-COPROVIDER-STATEMENT", Ptr(new GuiParser_WorkflowCoProviderStatement(workflowParser)));
						manager->SetParser(L"WORKFLOW-DECLARATION", Ptr(new GuiParser_WorkflowDeclaration(workflowParser)));
						manager->SetParser(L"WORKFLOW-MODULE", Ptr(new GuiParser_WorkflowModule(workflowParser)));
						manager->SetParser(L"INSTANCE-QUERY", Ptr(new GuiParser_InstanceQuery));
					}
					{
						IGuiInstanceLoaderManager* manager = GetInstanceLoaderManager();

						manager->AddInstanceBinder(Ptr(new GuiResourceInstanceBinder));
						manager->AddInstanceBinder(Ptr(new GuiReferenceInstanceBinder));
						manager->AddInstanceBinder(Ptr(new GuiEvalInstanceBinder));
						manager->AddInstanceBinder(Ptr(new GuiBindInstanceBinder));
						manager->AddInstanceBinder(Ptr(new GuiFormatInstanceBinder));
						manager->AddInstanceBinder(Ptr(new GuiLocalizedStringInstanceBinder));
						manager->AddInstanceEventBinder(Ptr(new GuiEvalInstanceEventBinder));
					}
				}
			}

			void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceBindersPlugin)
	}
}
