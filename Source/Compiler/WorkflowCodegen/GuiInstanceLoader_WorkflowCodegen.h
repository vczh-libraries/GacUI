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
			typedef collections::List<WString>													ErrorList;

			struct ResolvingResult : public Object, public Description<ResolvingResult>
			{
				Ptr<workflow::WfModule>							moduleForValidate;
				Ptr<workflow::WfBlockStatement>					moduleContent;

				collections::List<GlobalStringKey>				referenceNames;
				IGuiInstanceLoader::ArgumentMap					rootCtorArguments;
				IGuiInstanceLoader*								rootLoader = nullptr;
				IGuiInstanceLoader::TypeInfo					rootTypeInfo;

				VariableTypeInfoMap								typeInfos;
				TypeOverrideMap									typeOverrides;
				PropertyResolvingMap							propertyResolvings;
			};
		}
		extern workflow::analyzer::WfLexicalScopeManager*		Workflow_GetSharedManager();
		

/***********************************************************************
WorkflowCompiler (Parser)
***********************************************************************/
		
		extern Ptr<workflow::WfExpression>						Workflow_ParseExpression(const WString& code, types::ErrorList& errors);
		extern Ptr<workflow::WfStatement>						Workflow_ParseStatement(const WString& code, types::ErrorList& errors);
		extern WString											Workflow_ModuleToString(Ptr<workflow::WfModule> module);
		extern Ptr<workflow::WfExpression>						Workflow_ParseTextValue(description::ITypeDescriptor* typeDescriptor, const WString& textValue, types::ErrorList& errors);
		extern Ptr<workflow::WfExpression>						Workflow_CreateValue(description::ITypeDescriptor* typeDescriptor, const description::Value& value, types::ErrorList& errors);

/***********************************************************************
WorkflowCompiler (Installation)
***********************************************************************/

		extern Ptr<workflow::WfStatement>						Workflow_InstallUriProperty(GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& protocol, const WString& path, collections::List<WString>& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallBindProperty(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, Ptr<workflow::WfExpression> bindExpression);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvalProperty(GlobalStringKey variableName, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, Ptr<workflow::WfExpression> evalExpression, collections::List<WString>& errors);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvent(GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& handlerName);
		extern Ptr<workflow::WfFunctionDeclaration>				Workflow_GenerateEventHandler(description::IEventInfo* eventInfo);
		extern Ptr<workflow::WfStatement>						Workflow_InstallEvalEvent(GlobalStringKey variableName, description::IEventInfo* eventInfo, Ptr<workflow::WfStatement> evalStatement);

/***********************************************************************
WorkflowCompiler (Compile)
***********************************************************************/

		extern Ptr<workflow::WfModule>							Workflow_CreateModuleWithUsings(Ptr<GuiInstanceContext> context);
		extern Ptr<workflow::WfClassDeclaration>				Workflow_InstallClass(const WString& className, Ptr<workflow::WfModule> module);
		extern Ptr<workflow::WfBlockStatement>					Workflow_InstallCtorClass(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, Ptr<workflow::WfModule> module);

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

		extern InstanceLoadingSource							FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor);
		extern bool												Workflow_ValidateStatement(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, description::ITypeDescriptor* rootTypeDescriptor, types::ErrorList& errors, const WString& code, Ptr<workflow::WfStatement> statement);
		extern Ptr<workflow::WfModule>							Workflow_PrecompileInstanceContext(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors);
		extern Ptr<workflow::WfModule>							Workflow_GenerateInstanceClass(Ptr<GuiInstanceContext> context, types::ResolvingResult& resolvingResult, types::ErrorList& errors, vint passIndex);
	}
}

#endif