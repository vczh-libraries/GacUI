/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Schema Representation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHE_WORKFLOWCODEGEN
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHE_WORKFLOWCODEGEN

#include "../GuiInstanceLoader.h"
#include "../../Resources/GuiParserManager.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionPlugin.h"

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
				IGuiInstanceLoader::TypeInfo					rootTypeInfo;					// type of the context
				EnvironmentVariableGroup						envVars;						// current environment variable value stacks

				collections::List<GlobalStringKey>				referenceNames;					// all reference names
				IGuiInstanceLoader::ArgumentMap					rootCtorArguments;
				IGuiInstanceLoader*								rootLoader = nullptr;

				VariableTypeInfoMap								typeInfos;						// type of references
				PropertyResolvingMap							propertyResolvings;				// information of property values which are calling constructors
			};
		}
		extern workflow::analyzer::WfLexicalScopeManager*		Workflow_GetSharedManager();
		extern Ptr<workflow::analyzer::WfLexicalScopeManager>	Workflow_TransferSharedManager();
		

/***********************************************************************
WorkflowCompiler (Parser)
***********************************************************************/

		extern Ptr<workflow::WfType>							Workflow_ParseType					(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern Ptr<workflow::WfExpression>						Workflow_ParseExpression			(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern Ptr<workflow::WfStatement>						Workflow_ParseStatement				(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern Ptr<workflow::WfStatement>						Workflow_ParseCoProviderStatement	(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern Ptr<workflow::WfModule>							Workflow_ParseModule				(GuiResourcePrecompileContext& precompileContext, GuiResourceLocation location, const WString& code, GuiResourceTextPos position, collections::List<GuiResourceError>& errors, parsing::ParsingTextPos availableAfter = { 0,0 });

		extern WString											Workflow_ModuleToString(Ptr<workflow::WfModule> module);
		extern Ptr<workflow::WfExpression>						Workflow_ParseTextValue(GuiResourcePrecompileContext& precompileContext, description::ITypeDescriptor* typeDescriptor, GuiResourceLocation location, const WString& textValue, GuiResourceTextPos position, collections::List<GuiResourceError>& errors);

/***********************************************************************
WorkflowCompiler (Installation)
***********************************************************************/

		extern Ptr<workflow::WfExpression>						Workflow_GetUriProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& protocol, const WString& path, GuiResourceTextPos attPosition, GuiResourceError::List& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallUriProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& protocol, const WString& path, GuiResourceTextPos attPosition, GuiResourceError::List& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallBindProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> bindExpression);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvalProperty(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, Ptr<workflow::WfExpression> evalExpression, GuiResourceTextPos attPosition, GuiResourceError::List& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvent(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& handlerName);
		extern Ptr<workflow::WfFunctionDeclaration>				Workflow_GenerateEventHandler(GuiResourcePrecompileContext& precompileContext, description::IEventInfo* eventInfo);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvalEvent(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, Ptr<workflow::WfStatement> evalStatement);

/***********************************************************************
WorkflowCompiler (Compile)
***********************************************************************/

		extern Ptr<workflow::WfModule>							Workflow_CreateModuleWithUsings(Ptr<GuiInstanceContext> context, const WString& moduleName);
		extern Ptr<workflow::WfClassDeclaration>				Workflow_InstallClass(const WString& className, Ptr<workflow::WfModule> module);
		extern Ptr<workflow::WfBlockStatement>					Workflow_InstallCtorClass(types::ResolvingResult& resolvingResult, Ptr<workflow::WfModule> module);

		extern void												Workflow_CreatePointerVariable(Ptr<workflow::WfClassDeclaration> ctorClass, GlobalStringKey name, description::ITypeInfo* typeInfo);
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

		extern IGuiInstanceLoader::TypeInfo						Workflow_AdjustPropertySearchType(types::ResolvingResult& resolvingResult, IGuiInstanceLoader::TypeInfo resolvedTypeInfo, GlobalStringKey prop);
		extern bool												Workflow_GetPropertyTypes(WString& errorPrefix, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, IGuiInstanceLoader::TypeInfo resolvedTypeInfo, GlobalStringKey prop, Ptr<GuiAttSetterRepr::SetterValue> setter, collections::List<types::PropertyResolving>& possibleInfos, GuiResourceError::List& errors);
		extern Ptr<reflection::description::ITypeInfo>			Workflow_GetSuggestedParameterType(reflection::description::ITypeDescriptor* typeDescriptor);
		extern IGuiInstanceLoader::TypeInfo						Workflow_CollectReferences(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors);
		extern void												Workflow_GenerateCreating(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, Ptr<workflow::WfBlockStatement> statements, GuiResourceError::List& errors);
		extern void												Workflow_GenerateBindings(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, Ptr<workflow::WfBlockStatement> statements, GuiResourceError::List& errors);

		extern InstanceLoadingSource							FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GlobalStringKey namespaceName, const WString& typeName);
		extern Ptr<GuiResourceItem>								FindInstanceResourceItem(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor, Ptr<GuiResourceClassNameRecord> record);
		extern InstanceLoadingSource							FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor);
		extern Ptr<workflow::WfModule>							Workflow_PrecompileInstanceContext(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors);
		extern Ptr<workflow::WfModule>							Workflow_GenerateInstanceClass(GuiResourcePrecompileContext& precompileContext, const WString& moduleName, types::ResolvingResult& resolvingResult, GuiResourceError::List& errors, vint passIndex);

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

/***********************************************************************
WorkflowCompiler (ScriptPosition)
***********************************************************************/

		namespace types
		{
			struct ScriptPositionRecord
			{
				GuiResourceTextPos								position;
				parsing::ParsingTextPos							availableAfter;
				GuiResourceTextPos								computedPosition;
			};

			class ScriptPosition : public Object, public Description<ScriptPosition>
			{
				using NodePositionMap = collections::Dictionary<Ptr<parsing::ParsingTreeCustomBase>, ScriptPositionRecord>;
			public:
				NodePositionMap									nodePositions;
			};
		}

		extern void												Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfType> node, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern void												Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfExpression> node, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern void												Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfStatement> node, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern void												Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfDeclaration> node, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern void												Workflow_RecordScriptPosition(GuiResourcePrecompileContext& context, GuiResourceTextPos position, Ptr<workflow::WfModule> node, parsing::ParsingTextPos availableAfter = { 0,0 });
		extern Ptr<types::ScriptPosition>						Workflow_GetScriptPosition(GuiResourcePrecompileContext& context);
		extern void												Workflow_ClearScriptPosition(GuiResourcePrecompileContext& context);

	}
}

#endif