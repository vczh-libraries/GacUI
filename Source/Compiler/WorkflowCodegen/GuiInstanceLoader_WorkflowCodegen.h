/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Schema Representation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHE_WORKFLOWCODEGEN
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHE_WORKFLOWCODEGEN

#include "../GuiInstanceLoader.h"

namespace vl
{
	namespace presentation
	{
		namespace types
		{
			struct PropertyResolving
			{
				IGuiInstanceLoader*								loader = nullptr;
				IGuiInstanceLoader::PropertyInfo				propertyInfo;
				Ptr<GuiInstancePropertyInfo>					info;
			};

			typedef collections::Dictionary<GlobalStringKey, IGuiInstanceLoader::TypeInfo>		VariableTypeInfoMap;
			typedef collections::Dictionary<GlobalStringKey, Ptr<description::ITypeInfo>>		TypeOverrideMap;
			typedef collections::Dictionary<GuiValueRepr*, PropertyResolving>					PropertyResolvingMap;
			typedef collections::Group<GlobalStringKey, Ptr<GuiAttSetterRepr::EnvVarValue>>		EnvironmentVariableGroup;

			struct ResolvingResult : public Object, public Description<ResolvingResult>
			{
				Ptr<GuiResourceItem>							resource;						// compiling resource
				Ptr<GuiInstanceContext>							context;						// compiling context
				reflection::description::ITypeDescriptor*		rootTypeDescriptor = nullptr;	// type of the context
				collections::List<WString>						sharedModules;					// code of all shared workflow scripts
				EnvironmentVariableGroup						envVars;						// current environment variable value stacks

				collections::List<GlobalStringKey>				referenceNames;					// all reference names
				IGuiInstanceLoader::ArgumentMap					rootCtorArguments;
				IGuiInstanceLoader*								rootLoader = nullptr;
				IGuiInstanceLoader::TypeInfo					rootTypeInfo;

				VariableTypeInfoMap								typeInfos;						// type of references
				TypeOverrideMap									typeOverrides;					// extra type information of references
				PropertyResolvingMap							propertyResolvings;				// information of property values which are calling constructors
			};
		}
		extern workflow::analyzer::WfLexicalScopeManager*		Workflow_GetSharedManager();
		extern Ptr<workflow::analyzer::WfLexicalScopeManager>	Workflow_TransferSharedManager();
		

/***********************************************************************
WorkflowCompiler (Parser)
***********************************************************************/
		
		extern Ptr<workflow::WfExpression>						Workflow_ParseExpression(const WString& code, collections::List<Ptr<parsing::ParsingError>>& errors);
		extern Ptr<workflow::WfStatement>						Workflow_ParseStatement(const WString& code, collections::List<Ptr<parsing::ParsingError>>& errors);
		extern WString											Workflow_ModuleToString(Ptr<workflow::WfModule> module);
		extern Ptr<workflow::WfExpression>						Workflow_ParseTextValue(description::ITypeDescriptor* typeDescriptor, const WString& textValue, collections::List<Ptr<parsing::ParsingError>>& errors);

/***********************************************************************
WorkflowCompiler (Installation)
***********************************************************************/

		extern Ptr<workflow::WfStatement>						Workflow_InstallUriProperty(types::ResolvingResult& resolvingResult, GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& protocol, const WString& path, parsing::ParsingTextPos attPosition, GuiResourceError::List& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallBindProperty(types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> bindExpression);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvalProperty(types::ResolvingResult& resolvingResult, GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, Ptr<workflow::WfExpression> evalExpression, parsing::ParsingTextPos attPosition, GuiResourceError::List& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvent(types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& handlerName);
		extern Ptr<workflow::WfFunctionDeclaration>				Workflow_GenerateEventHandler(description::IEventInfo* eventInfo);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvalEvent(types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, Ptr<workflow::WfStatement> evalStatement);

/***********************************************************************
WorkflowCompiler (Compile)
***********************************************************************/

		extern Ptr<workflow::WfModule>							Workflow_CreateModuleWithUsings(Ptr<GuiInstanceContext> context);
		extern Ptr<workflow::WfClassDeclaration>				Workflow_InstallClass(const WString& className, Ptr<workflow::WfModule> module);
		extern Ptr<workflow::WfBlockStatement>					Workflow_InstallCtorClass(types::ResolvingResult& resolvingResult, Ptr<workflow::WfModule> module);

		extern void												Workflow_CreatePointerVariable(Ptr<workflow::WfClassDeclaration> ctorClass, GlobalStringKey name, description::ITypeDescriptor* type, description::ITypeInfo* typeOverride);
		extern void												Workflow_CreateVariablesForReferenceValues(Ptr<workflow::WfClassDeclaration> ctorClass, types::ResolvingResult& resolvingResult);
		
		struct InstanceLoadingSource
		{
			IGuiInstanceLoader*						loader;
			GlobalStringKey							typeName;
			Ptr<GuiResourceItem>					item;
			Ptr<GuiInstanceContext>					context;

			InstanceLoadingSource()
				:loader(0)
			{
			}

			InstanceLoadingSource(IGuiInstanceLoader* _loader, GlobalStringKey _typeName)
				:loader(_loader)
				, typeName(_typeName)
			{
			}

			InstanceLoadingSource(Ptr<GuiResourceItem> _item)
				:loader(0)
				, item(_item)
				, context(item->GetContent().Cast<GuiInstanceContext>())
			{
			}

			operator bool()const
			{
				return loader != 0 || context;
			}
		};

		extern description::ITypeDescriptor*					Workflow_CollectReferences(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors);
		extern void												Workflow_GenerateCreating(types::ResolvingResult& resolvingResult, Ptr<workflow::WfBlockStatement> statements, GuiResourceError::List& errors);
		extern void												Workflow_GenerateBindings(types::ResolvingResult& resolvingResult, Ptr<workflow::WfBlockStatement> statements, GuiResourceError::List& errors);

		extern InstanceLoadingSource							FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor);
		extern Ptr<workflow::WfModule>							Workflow_PrecompileInstanceContext(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors);
		extern Ptr<workflow::WfModule>							Workflow_GenerateInstanceClass(types::ResolvingResult& resolvingResult, GuiResourceError::List& errors, vint passIndex);

#define WORKFLOW_ENVIRONMENT_VARIABLE_ADD\
		FOREACH_INDEXER(GlobalStringKey, envVar, index, repr->environmentVariables.Keys())\
		{\
			auto value = repr->environmentVariables.Values()[index];\
			resolvingResult.envVars.Add(envVar, value);\
		}\

#define WORKFLOW_ENVIRONMENT_VARIABLE_REMOVE\
		FOREACH_INDEXER(GlobalStringKey, envVar, index, repr->environmentVariables.Keys())\
		{\
			auto value = repr->environmentVariables.Values()[index];\
			resolvingResult.envVars.Remove(envVar, value.Obj());\
		}\

	}
}

#endif