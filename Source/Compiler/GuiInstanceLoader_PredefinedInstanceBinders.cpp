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
				return false;
			}

			bool RequirePropertyExist()override
			{
				return false;
			}

			Ptr<workflow::WfExpression> GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				CHECK_FAIL(L"GuiResourceInstanceBinder::GenerateConstructorArgument()#This binder does not support binding to constructor arguments. Please call ApplicableToConstructorArgument() to determine before calling this function.");
			}
			
			Ptr<workflow::WfStatement> GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors)override
			{
				WString protocol, path;
				if (!IsResourceUrl(code, protocol, path))
				{
					errors.Add(GuiResourceError({ resolvingResult.resource }, position, L"Precompile: \"" + code + L"\" is not a valid resource uri."));
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
				if(auto statement = Workflow_ParseStatement(precompileContext, { resolvingResult.resource }, code, position, errors))
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
			GuiPredefinedInstanceBindersPlugin()
			{
			}

			void Load()override
			{
				WfLoadTypes();
				GuiIqLoadTypes();
			}

			void AfterLoad()override
			{
				{
					IGuiParserManager* manager = GetParserManager();
					manager->SetParsingTable(L"WORKFLOW", &WfLoadTable);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-TYPE", &WfParseType);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-EXPRESSION", &WfParseExpression);
					manager->SetTableParser(L"WORKFLOW", L"WORKFLOW-STATEMENT", &WfParseStatement);
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
					manager->AddInstanceEventBinder(new GuiEvalInstanceEventBinder);
					manager->AddInstanceBinder(new GuiBindInstanceBinder);
					manager->AddInstanceBinder(new GuiFormatInstanceBinder);
				}
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedInstanceBindersPlugin)
	}
}
