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

#define ERROR_CODE_PREFIX L"================================================================"

/***********************************************************************
Workflow_ValidateStatement
***********************************************************************/

		bool Workflow_ValidateStatement(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, types::ErrorList& errors, const WString& code, Ptr<workflow::WfStatement> statement)
		{
			bool failed = false;
			auto module = Workflow_CreateModuleWithInitFunction(context, resolvingResult, rootTypeDescriptor, statement);

			Workflow_GetSharedManager()->Clear(true, true);
			Workflow_GetSharedManager()->AddModule(module);
			Workflow_GetSharedManager()->Rebuild(true);
			if (Workflow_GetSharedManager()->errors.Count() > 0)
			{
				errors.Add(ERROR_CODE_PREFIX L"Failed to analyze the workflow code \"" + code + L"\".");
				FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
				{
					errors.Add(error->errorMessage);
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

		Ptr<workflow::runtime::WfAssembly> Workflow_PrecompileInstanceContext(Ptr<GuiInstanceContext> context, types::ErrorList& errors)
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

			types::ResolvingResult resolvingResult;
			rootTypeDescriptor = Workflow_CollectReferences(context, resolvingResult, errors);

			if (errors.Count() == previousErrorCount)
			{
				auto statements = MakePtr<WfBlockStatement>();
				Workflow_GenerateCreating(context, resolvingResult, rootTypeDescriptor, statements, errors);
				Workflow_GenerateBindings(context, resolvingResult, rootTypeDescriptor, statements, errors);
				auto module = Workflow_CreateModuleWithInitFunction(context, resolvingResult, rootTypeDescriptor, statements);

				Workflow_GetSharedManager()->Clear(true, true);
				Workflow_GetSharedManager()->AddModule(module);
				Workflow_GetSharedManager()->Rebuild(true);

				if (Workflow_GetSharedManager()->errors.Count() == 0)
				{
					return GenerateAssembly(Workflow_GetSharedManager());
				}
				else
				{
					errors.Add(ERROR_CODE_PREFIX L"Unexpected errors are encountered when initializing data binding.");
					FOREACH(Ptr<parsing::ParsingError>, error, Workflow_GetSharedManager()->errors)
					{
						errors.Add(error->errorMessage);
					}
					errors.Add(ERROR_CODE_PREFIX L"Print code for reference:");
					errors.Add(Workflow_ModuleToString(module));
				}
			}

			return nullptr;
		}

/***********************************************************************
Workflow_RunPrecompiledScript
***********************************************************************/

		Ptr<workflow::runtime::WfRuntimeGlobalContext> Workflow_RunPrecompiledScript(Ptr<GuiResource> resource, Ptr<GuiResourceItem> resourceItem, description::Value rootInstance)
		{
			auto compiled = resourceItem->GetContent().Cast<GuiInstanceCompiledWorkflow>();
			auto globalContext = MakePtr<WfRuntimeGlobalContext>(compiled->assembly);

			LoadFunction<void()>(globalContext, L"<initialize>")();
			auto resolver = MakePtr<GuiResourcePathResolver>(resource, resource->GetWorkingDirectory());
			LoadFunction<void(Value, Ptr<GuiResourcePathResolver>)>(globalContext, L"<initialize-instance>")(rootInstance, resolver);

			return globalContext;
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