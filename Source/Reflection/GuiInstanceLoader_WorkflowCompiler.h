/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Schema Representation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHE_WORKFLOWCOMPILER
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHE_WORKFLOWCOMPILER

#include "GuiInstanceLoader.h"

namespace vl
{
	namespace presentation
	{
		namespace types
		{
			typedef collections::Dictionary<GlobalStringKey, IGuiInstanceLoader::TypeInfo>		VariableTypeInfoMap;
			typedef collections::List<WString>													ErrorList;
		}
		extern workflow::analyzer::WfLexicalScopeManager*	Workflow_GetSharedManager();
		

/***********************************************************************
WorkflowCompiler (Parser)
***********************************************************************/
		
		extern Ptr<workflow::WfExpression>					Workflow_ParseExpression(const WString& code, types::ErrorList& errors);
		extern Ptr<workflow::WfStatement>					Workflow_ParseStatement(const WString& code, types::ErrorList& errors);

/***********************************************************************
WorkflowCompiler (Installation)
***********************************************************************/

		extern Ptr<workflow::WfStatement>					Workflow_InstallBindProperty(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> bindExpression);
		extern Ptr<workflow::WfStatement>					Workflow_InstallEvalProperty(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> evalExpression);
		extern Ptr<workflow::WfStatement>					Workflow_InstallEvalEvent(GlobalStringKey variableName, description::IEventInfo* eventInfo, Ptr<workflow::WfStatement> evalStatement);

/***********************************************************************
WorkflowCompiler (Compile)
***********************************************************************/

		extern Ptr<workflow::WfModule>						Workflow_CreateEmptyModule(Ptr<GuiInstanceContext> context);

		extern void											Workflow_CreatePointerVariable(Ptr<workflow::WfModule> module, GlobalStringKey name, description::ITypeDescriptor* type);
		extern void											Workflow_CreateVariablesForReferenceValues(Ptr<workflow::WfModule> module, types::VariableTypeInfoMap& types);
		extern void											Workflow_SetVariablesForReferenceValues(Ptr<workflow::runtime::WfRuntimeGlobalContext> context, Ptr<GuiInstanceEnvironment> env);

		extern bool											Workflow_ValidateStatement(Ptr<GuiInstanceContext> context, types::VariableTypeInfoMap& types, types::ErrorList& errors, const WString& code, Ptr<workflow::WfExpression>& statement);
		extern WString										Workflow_ModuleToString(Ptr<workflow::WfModule> module);
		extern void											Workflow_PrecompileInstanceContext(Ptr<GuiInstanceContext> context, types::ErrorList& errors);
		extern void											Workflow_RunPrecompiledScript(Ptr<GuiInstanceEnvironment> env);
	}
}

#endif